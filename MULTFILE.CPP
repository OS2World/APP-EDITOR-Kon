#include "bowl.h"
#include "multop.h"
#include "multfile.h"
#include "kerrcoll.h"
#include "cntrladm.h"
#include "editctrl.h"
#include "mainwnd.h"

BOOL MultipleFiles::terminate = FALSE;

int MultipleFiles::searchFile( char *file )
{
  // See if the file is already loaded

  ControlInfo *control = ControlAdmin::queryControlWithFilename( file );

  View *view;
  TextBuffer *textBuffer = NULL;

  if( control != NULL )
    view = control->editControl->getView();
  else
  {
    // Load the text

    #define BLOCKSIZE 5164
    BlockFileReader *fileReader = new BlockFileReader( file );
    textBuffer = new TextBuffer( file );
    int length = fileReader->getFileLength(), pos = 0;
    int left = length;
    TextElem *text = new TextElem[BLOCKSIZE];
    TextElem *ctext = new TextElem[BLOCKSIZE*2];
    EncodingTypes encoding = ANSI;
  
    int writer = textBuffer->writeRequest();
  
    while( left )
    {
      int read = (left < BLOCKSIZE) ? left : BLOCKSIZE;
      fileReader->readBlock( text, read );
      left -= read;

      if( pos == 0 )
      {
        encoding = UnicodeHelper::checkEncoding( (unsigned char *)text, read );
        if( encoding != ANSI && Settings::queryIfUnicode )
        {
			    MessageBoxInfo mbInfo;

			    mbInfo.title = Strings::get(238);
			    mbInfo.message = Strings::get(239);
			    mbInfo.type = PopupMessage::querywithwarningNoCancel;

			    int result = Application::executeFromMainThread( (Window *)control->parent, 7, &mbInfo );
          if( result == PopupMessage::no ) encoding = ANSI;
        }
        if( encoding == ANSI ) 
          delete ctext;
        else
          UnicodeHelper::fixFirstBytes( (unsigned char *)text, read );
      }

      if( encoding != ANSI )
        UnicodeHelper::unicodeToAnsi(text, ctext, read, encoding );
      else
        ctext = text;

      textBuffer->insertTextNoUndo( writer, pos, ctext, read, FALSE, TRUE );
      pos += read;
    }

    textBuffer->writeRelease();
    textBuffer->setEncoding( encoding );
    textBuffer->finishedLoading( FALSE, FALSE );
    delete fileReader;
    delete text;
    if( encoding != ANSI )
      delete ctext;

    BufferAdmin::addBufferHolder( this, textBuffer );

    // Open a stand-alone asciiview

    view = new AsciiView( NULL, textBuffer, NULL, NULL );
  }

  if( !operation->operateFile( file, view, textBuffer == NULL ) )
    terminate = TRUE;

  if( textBuffer != NULL )
  {
    delete view;
    BufferAdmin::removeBufferHolder( this );
  }

  return 0;
}

void MultipleFiles::searchDir( char *dir, char *files )
{
  char *orgFile = new char[strlen(files)+1];
	char *file = orgFile;

  strcpy( file, files );
  
	while( file && *file )
  {
  	if( terminate ) break;
  	char *nextFile = strchr( file, ';' );
    
		if( nextFile )
    {
    	*nextFile = 0;
      nextFile++;
    }
    
	  // Find all files in this dir
    
	  if( strchr( file, '*' ) || strchr( file, '?' ) )
    {
  	  char *pattern = new char[strlen(dir)+strlen(file)+2];
  
      strcpy( pattern, dir );
  
      if( pattern[strlen(pattern)-1] != '\\' )
    	  strcat( pattern, "\\" );
  
      strcat( pattern, file );
  
  	  Dir dDir( pattern, DirEntry::ARCHIVED, 0 );
      
		  for( int i = 0; i < dDir.getEntries(); i++ )
      {
		  	if( terminate ) break;
        char *filename = dDir.getEntryAt(i)->getName();
  	    char *fullName = new char[strlen(dir)+strlen(filename)+2];
    
        strcpy( fullName, dir );
    
        if( fullName[strlen(fullName)-1] != '\\' )
    	    strcat( fullName, "\\" );
    
        strcat( fullName, filename );
    
    	  searchFile( fullName );

        delete fullName;
      }
  
      delete pattern;
    }
    else
		  searchFile( file );

		file = nextFile;
  }
        
	if( recursive && !terminate )
  {
  	// Find all subdirectories

  	char *pattern = new char[strlen(dir)+3];

    strcpy( pattern, dir );

    if( pattern[strlen(pattern)-1] != '\\' )
    	strcat( pattern, "\\" );

    strcat( pattern, "*" );

  	Dir eDir( pattern, DirEntry::DIR, DirEntry::DIR );
    
		for( int i = 0; i < eDir.getEntries(); i++ )
    {
  		if( terminate ) break;
      char *sDir = eDir.getEntryAt(i)->getName();

      // Remove . and ,,

      char *last = strrchr( sDir, '\\' );

      if( last == NULL )
        last = sDir;
      else
        last++;

      if( !strcmp( last, "." ) || !strcmp( last, ".." ) ) continue;

  	  char *pDir = new char[strlen(dir)+strlen(sDir)+3];
  
      strcpy( pDir, dir );
  
      if( pDir[strlen(pDir)-1] != '\\' ) strcat( pDir, "\\" );
  
      strcat( pDir, sDir );
  
    	searchDir( pDir, files );

      delete pDir;
    }

    delete pattern;
  }
  
	delete orgFile;
}

MultipleFiles::MultipleFiles( MultipleFilesOperation *operation, char *path, 
                              char *files, BOOL recursive ) : 
  operation(operation), recursive(recursive), path(path), files(files)
{ 
  char *orgDir = new char[strlen(path)+1];
	char *dir = orgDir;
  
	terminate = FALSE;

  strcpy( dir, path );

	KErrorCollector::getInstance()->show();
	//KErrorCollector::getInstance()->setFocus();
  
	KErrorCollector::getInstance()->message( Strings::get( 115 ) );
  
	if( !*dir )	// Buffers only
  {
    while( files )
    {
  		if( terminate ) break;
  	  char *nextFile = strchr( files, ';' );
  
      if( nextFile )
      {
    	  *nextFile = 0;
        nextFile++;
      }
      
  	  searchFile( files );
      
		  files = nextFile;
    }
  }
	else
	while( dir && *dir )
  {
  	if( terminate ) break;
  	char *nextDir = strchr( dir, ';' );

    if( nextDir )
    {
    	*nextDir = 0;
      nextDir++;
    }
    
	  searchDir( dir, files );
    
		dir = nextDir;
  }
  
	delete orgDir;
  
	if( !terminate )
		KErrorCollector::getInstance()->message( Strings::get( 116 ) );
  else
		KErrorCollector::getInstance()->message( Strings::get( 117 ) );
}