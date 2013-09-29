#include <direct.h>
#include <string.h>
#include "bowl.h"
#include "srchdlg.h"
#include "cntrladm.h"
#include "editctrl.h"
#include "mainwadm.h"
#include "mainwnd.h"

BOOL SearchDialog::initialized = FALSE;
BOOL SearchDialog::entireDoc = FALSE;
BOOL SearchDialog::forward = TRUE;
BOOL SearchDialog::selected = FALSE;
BOOL SearchDialog::caseSensitive = TRUE;
BOOL SearchDialog::wholeWords = FALSE;
BOOL SearchDialog::grep = FALSE;
BOOL SearchDialog::buffersOnly = FALSE;
BOOL SearchDialog::subdirs = FALSE;
BOOL SearchDialog::hexSearch = FALSE;
BOOL SearchDialog::wrapAround = TRUE;
int SearchDialog::strings = 0;
int SearchDialog::pathsCount = 0;
int SearchDialog::filesCount = 0;
int SearchDialog::buffersSelected = 0;
int SearchDialog::CANCEL = 0;
int SearchDialog::CURRENTBUFFER = 1;
int SearchDialog::MULTIPLEFILES = 2;
TextElem *SearchDialog::string[MAXSTRINGS+1];
TextElem *SearchDialog::files[MAXSTRINGS+1];
TextElem *SearchDialog::paths[MAXSTRINGS+1];
char **SearchDialog::buffers;

SearchDialog::SearchDialog( Window *frame, Window *parent, TextElem *findString ) :
  Dialog( frame, parent, Strings::get( 62 ),
          AWindow::CENTER | AWindow::ADJUSTSIZE,
          0, 0, 500, 330 )
{
  char *namestr = "searchDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight(),
      interSpace = fontHeight + fontHeight/ 2;

  Notebook *notebook =
    new Notebook( this, 0, 0, getClientWidth(), getClientHeight(), 
		              Color::BKGR, 0 );

  singleDlg = new Dialog( notebook, notebook, "", NOTITLE,0, 0,
                          getClientWidth(),
                          getClientHeight()-Notebook::getTabInset() );

  multiDlg = new Dialog( notebook, notebook, "", NOTITLE, 0, 0,
                         getClientWidth(),
                         getClientHeight()-Notebook::getTabInset() );
	
  notebook->add( singleDlg, Strings::get( 86 ) );
  notebook->add( multiDlg, Strings::get( 87 ) );

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;

  Label *lbl = new Label( singleDlg, Label::RIGHT, Strings::get(77), 10,
                          10, 80, editBoxHeight );

  searchCombo = new SComboBox( singleDlg, ISTABSTOP, 100,
                              10,
                              getClientWidth() - 120,
                              editBoxHeight, 100 );

  int comboHeight = searchCombo->getEditBoxHeight();
  yStart = singleDlg->getClientHeight() - comboHeight - comboHeight / 2;
  searchCombo->setPos( searchCombo->getX(), yStart);
  lbl->setSize( lbl->getWidth(), comboHeight);
  lbl->setPos( lbl->getX(), yStart);

  searchCombo->setMaxLength( MAXSTRINGSIZE );
  searchCombo->setWMove( FOLLOW );

  yStart -= comboHeight - fontHeight / 2;

  int top = yStart;

  yStart -= fontHeight * 3;

  caseCheck = new CheckBox( singleDlg, Strings::get(66),
                            ISTABSTOP, 20, yStart, 200, fontHeight*2 );

  yStart -= interSpace;

  wordCheck = new CheckBox( singleDlg, Strings::get(67),
                            0, 20, yStart, 200, fontHeight*2 );

  yStart -= interSpace;

  grepCheck = new CheckBox( singleDlg, Strings::get(68),
                            0, 20, yStart, 200, fontHeight*2 );

  int bottom = yStart - fontHeight / 2;

  yStart = top;

  yStart -= fontHeight * 3;

  hexSearchCheck = new CheckBox( singleDlg, Strings::get(247),
                            0, 230, yStart, 200, fontHeight*2 );

  yStart -= interSpace;

  wrapAroundCheck = new CheckBox( singleDlg, Strings::get(255),
                            0, 230, yStart, 200, fontHeight*2 );

  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( singleDlg, Strings::get(65), 0, 10, bottom,
                                 450, top - bottom );
                                                                 
  yStart = bottom - fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 3;

  fromCursorButton = new RadioButton( singleDlg, Strings::get(70),
                                      NEWGROUP | ISTABSTOP, 20, yStart, 120,
                                      fontHeight*2 );

  yStart -= interSpace;

  entireDocButton = new RadioButton( singleDlg, Strings::get(71),
                                     INGROUP, 20, yStart, 120,
                                     fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( singleDlg, Strings::get(69), 0, 10, yStart,
                       140, top - yStart );

  yStart = top - (fontHeight * 3);

  fwdButton = new RadioButton( singleDlg, Strings::get(73),
                                      NEWGROUP | ISTABSTOP, 170, yStart, 100,
                                      fontHeight*2 );

  yStart -= interSpace;

  bkwdButton = new RadioButton( singleDlg, Strings::get(74),
                                     INGROUP, 170, yStart, 100,
                                     fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( singleDlg, Strings::get(72), 0, 160, yStart,
                       120, top - yStart );

  yStart = top - (fontHeight*3);

  globalButton = new RadioButton( singleDlg, Strings::get(75),
                                      NEWGROUP | ISTABSTOP, 300, yStart, 100,
                                      fontHeight*2 );

  yStart -= interSpace;

  selectedButton = new RadioButton( singleDlg, Strings::get(76),
                                     INGROUP, 300, yStart, 100,
                                     fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( singleDlg, Strings::get(78), 0, 290, yStart,
                       120, top - yStart );

  yStart = 10;

  searchButton = new Button( singleDlg, Strings::get(63), AWindow::ISDEFAULT|ISTABSTOP,
                             10, yStart, 150, 30 );
  searchButton->setYMove( FOLLOW );
  cancel1Button = new Button( singleDlg, Strings::get(9), ISTABSTOP, 170, yStart, 70, 30 );
  cancel1Button->setYMove( FOLLOW );

	// Multi file dialog
  // ------------------------------------

  multSearchButton = new Button( multiDlg, Strings::get(64),
                                 AWindow::ISDEFAULT|ISTABSTOP, 10, 10, 150, 30 );
  multSearchButton->setYMove( FOLLOW );
  cancel2Button = new Button( multiDlg, Strings::get(9), ISTABSTOP, 170, 10, 70, 30 );
  cancel2Button->setYMove( FOLLOW );

  yStart = getClientHeight() - fontHeight*2 - 20;

  lblF = new Label( multiDlg, Label::RIGHT, Strings::get(98), 10,
                    yStart, 40, comboHeight );

  filesCombo = new SComboBox( multiDlg, ISTABSTOP, 60, yStart,
                             getClientWidth() - 80,
                             editBoxHeight, 100 );

  filesCombo->setMaxLength( MAXSTRINGSIZE );
  filesCombo->setWMove( FOLLOW );

  yStart -= fontHeight * 2;

  lblP = new Label( multiDlg, Label::RIGHT, Strings::get(99), 10,
                    yStart, 40, comboHeight );

  pathsCombo = new SComboBox( multiDlg, ISTABSTOP, 60, yStart,
                               getClientWidth() - 170,
                               editBoxHeight, 100 );

  pathsCombo->setMaxLength( MAXSTRINGSIZE );
  pathsCombo->setWMove( FOLLOW );

  currentDirButton = new Button( multiDlg, Strings::get(101), 0, 
                                 getClientWidth() - 100, yStart, 80, 
                                 30 );
  currentDirButton->setXMove( FOLLOW );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 3;

  subdirCheck = new CheckBox( multiDlg, Strings::get(102),
                            ISTABSTOP, 20, yStart, 200, fontHeight*2 );

  clearCheck = new CheckBox( multiDlg, Strings::get(252),
                            ISTABSTOP, 230, yStart, 200, fontHeight*2 );

  yStart -= interSpace;

  buffersCheck = new CheckBox( multiDlg, Strings::get(103),
                            0, 20, yStart, 200, fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( multiDlg, Strings::get(104), 0, 10, yStart,
                       450, top - yStart );

  yStart -= fontHeight * 2 - fontHeight / 2;

  lblB = new Label( multiDlg, Label::LEFT, Strings::get(100), 10,
                   yStart, 80, comboHeight );

  top = yStart - 2;

  openBuffersList = new ListBox( multiDlg, 
                                 NOADJUSTSIZE|ISTABSTOP|MULTIPLESEL|ISTABSTOP, 
  															 10, 50, getClientWidth() - 20, top - 50, 
                                 TRUE );
  openBuffersList->setHMove( FOLLOW );
  openBuffersList->setWMove( FOLLOW );

  singleDlg->setControlsVisible();
  multiDlg->setControlsVisible();
	setControlsVisible();
  notebook->setTab( 0 );

  // Set combo list box

  int i;
  for( i = 0; i < strings; i++ )
    searchCombo->addItem( string[i] );

	if( findString != NULL )
    searchCombo->setText( findString );
	else
  if( strings )
    searchCombo->setText( string[0] );

  // Set buttons

  if( entireDoc )
    entireDocButton->check( TRUE );
  else
    fromCursorButton->check( TRUE );

  if( forward )
    fwdButton->check( TRUE );
  else
    bkwdButton->check( TRUE );

  if( selected )
    selectedButton->check( TRUE );
  else
    globalButton->check( TRUE );

  if( caseSensitive )
    caseCheck->check( TRUE );
  if( wholeWords )
    wordCheck->check( TRUE );
  if( grep )
    grepCheck->check( TRUE );

  if( subdirs )
    subdirCheck->check( TRUE );
  if( buffersOnly )
    buffersCheck->check( TRUE );
  if( hexSearch )
    hexSearchCheck->check( TRUE );
  if( Settings::autoClearMessages )
    clearCheck->check( TRUE );
  if( wrapAround )
		wrapAroundCheck->check(TRUE);

  for( i = 0; i < filesCount; i++ )
    filesCombo->addItem( files[i] );

  if( filesCombo )
    filesCombo->setText( files[0] );

  for( i = 0; i < pathsCount; i++ )
    pathsCombo->addItem( paths[i] );

  if( pathsCombo )
    pathsCombo->setText( paths[0] );

  // Fill buffers list

  for( i = 0; i < ControlAdmin::getControls(); i++ )
    openBuffersList->addItem( ControlAdmin::getControl(i)->editControl->
                                getTextBuffer()->getFilename() );

  enableControls();

  searchButton->addButtonListener( this );
  multSearchButton->addButtonListener( this );
  cancel1Button->addButtonListener( this );
  cancel2Button->addButtonListener( this );
  buffersCheck->addButtonListener( this );
  currentDirButton->addButtonListener( this );

  singleDlg->updateControlPos( NULL );

  adjustForFontSize();

  initSize();

  // Save/restore window position
  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  center( x, y );
  WinProperties::retrieveProperty( name, x, y, width, height );

  setPos( x, y );

  filesCombo->setFocus();
  searchCombo->setFocus();
}

void SearchDialog::enableControls()
{
  BOOL bufOnly = buffersCheck->queryCheck();
  filesCombo->enable( !bufOnly );
  pathsCombo->enable( !bufOnly );
  currentDirButton->enable( !bufOnly );
  subdirCheck->enable( !bufOnly );
  lblP->enable( !bufOnly );
  lblF->enable( !bufOnly );
  openBuffersList->enable( bufOnly );
  lblB->enable( bufOnly );
}

void SearchDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == buffersCheck )
  {
    enableControls();
    return;
  }
  else
  if( buttonEvent->getSource() == currentDirButton )
  {
    char cdir[MAXSTRINGSIZE+1];
#if defined __OS2__
    _getcwd( cdir, MAXSTRINGSIZE );
    pathsCombo->setText( cdir );
#else
    BrowseForDir bfd;
    strcpy( cdir, pathsCombo->getEditText() );
    bfd.display( this, this, Strings::get(242), cdir );
    pathsCombo->setText( cdir );
#endif
    return;
  }
  else
  if( buttonEvent->getSource() == searchButton ||
  		buttonEvent->getSource() == multSearchButton )
  {
    // Store settings

    entireDoc = entireDocButton->queryCheck();
    forward = fwdButton->queryCheck();
    selected = selectedButton->queryCheck();
    caseSensitive = caseCheck->queryCheck();
    wholeWords = wordCheck->queryCheck();
    grep = grepCheck->queryCheck();
    subdirs = subdirCheck->queryCheck();
    buffersOnly = buffersCheck->queryCheck();
    hexSearch = hexSearchCheck->queryCheck();
    wrapAround = wrapAroundCheck->queryCheck();
    Settings::autoClearMessages = clearCheck->queryCheck();

    if( strings == 0 || strcmp( searchCombo->getEditText(), string[0] ) )
    {
      if( strings > 0 )
      	memmove( &string[1], &string[0], sizeof( *string ) * strings );

      if( strings == MAXSTRINGS-1 )
        delete string[strings-1];
      else
	      strings++;

      string[0] = new TextElem[searchCombo->getTextLength()+1];
      strcpy( string[0], searchCombo->getEditText() );
    }
    
    if( filesCount == 0 || strcmp( filesCombo->getEditText(), files[0] ) )
    {
      if( filesCount > 0 )
      	memmove( &files[1], &files[0], sizeof( *files ) * filesCount );

      if( filesCount == MAXSTRINGS-1 )
        delete files[filesCount-1];
      else
	      filesCount++;

      files[0] = new TextElem[filesCombo->getTextLength()+1];
      strcpy( files[0], filesCombo->getEditText() );
    }

    if( pathsCount == 0 || strcmp( pathsCombo->getEditText(), paths[0] ) )
    {
      if( pathsCount > 0 )
      	memmove( &paths[1], &paths[0], sizeof( *paths ) * pathsCount );

      if( pathsCount == MAXSTRINGS-1 )
        delete paths[pathsCount-1];
      else
	      pathsCount++;

      paths[0] = new TextElem[pathsCombo->getTextLength()+1];
      strcpy( paths[0], pathsCombo->getEditText() );
    }

		// Free bufferlist
    
		if( buffersSelected != 0 )
    {
    	while( buffersSelected )
      {
      	delete buffers[buffersSelected-1];
        buffersSelected--;
      }

			free( buffers );
    }
    
		// Fill bufferlist
    
		buffers = NULL;
    
		int currentIndex = -1;

		while( (currentIndex = openBuffersList->getNextSelectedIndex( currentIndex )) !=
    				-1 )
		{
    	buffersSelected++;
    	buffers = (char **)realloc( buffers, sizeof( char * ) * buffersSelected );
      buffers[buffersSelected-1] = new char[MAXSTRINGSIZE+1];
    	openBuffersList->getItem( currentIndex, MAXSTRINGSIZE, buffers[buffersSelected-1] );
    }
    
/*    closeDialog( (buttonEvent->getSource() == searchButton) ? 
    							CURRENTBUFFER : MULTIPLEFILES );*/
    doSearch( (buttonEvent->getSource() == searchButton) ? 
    							CURRENTBUFFER : MULTIPLEFILES );

    initialized = TRUE;
    return;
  }
  closeDialog( CANCEL );
}

void SearchDialog::getSettings( char *options, TextElem *searchString )
{
  *options = 0;

  strcpy( searchString, string[0] );

  if( entireDoc )
    strcat( options, "d" );
  if( !forward )
    strcat( options, "b" );
  if( selected )
    strcat( options, "s" );
  if( !caseSensitive )
    strcat( options, "u" );
  if( wholeWords )
    strcat( options, "w" );
  if( grep )
    strcat( options, "g" );
  if( subdirs )
    strcat( options, "r" );
  if( hexSearch )
    strcat( options, "h" );
  if( wrapAround )
    strcat( options, "x" );
}

void SearchDialog::saveSettings( FILE *f )
{
	long pos;
	if( !ClassProperties::savePropertyName( f, "SEARCH005", &pos ) ) return;
	fwrite( &initialized, sizeof( BOOL ), 1, f );
  fwrite( &entireDoc, sizeof( BOOL ), 1, f );
  fwrite( &forward, sizeof( BOOL ), 1, f );
  fwrite( &selected, sizeof( BOOL ), 1, f );
  fwrite( &caseSensitive, sizeof( BOOL ), 1, f );
  fwrite( &wholeWords, sizeof( BOOL ), 1, f );
  fwrite( &grep, sizeof( BOOL ), 1, f );
  fwrite( &buffersOnly, sizeof( BOOL ), 1, f );
  fwrite( &subdirs, sizeof( BOOL ), 1, f );
  fwrite( &strings, sizeof( int ), 1, f );
  fwrite( &hexSearch, sizeof( BOOL ), 1, f );
  fwrite( &wrapAround, sizeof( BOOL ), 1, f );

  int i;
  for( i = 0; i < strings; i++ )
  {
  	int len = strlen( string[i] );
  	fwrite( &len, sizeof( int ), 1, f );
  	fwrite( string[i], len+1, 1, f );
  }

  fwrite( &filesCount, sizeof( int ), 1, f );
  for( i = 0; i < filesCount; i++ )
  {
  	int len = strlen( files[i] );
  	fwrite( &len, sizeof( int ), 1, f );
  	fwrite( files[i], len+1, 1, f );
  }

  fwrite( &pathsCount, sizeof( int ), 1, f );
  for( i = 0; i < pathsCount; i++ )
  {
  	int len = strlen( paths[i] );
  	fwrite( &len, sizeof( int ), 1, f );
  	fwrite( paths[i], len+1, 1, f );
  }

	ClassProperties::savePropertySize( f, pos );
}

void SearchDialog::restoreSettings( FILE *f )
{
	if( !ClassProperties::checkPropertyExists( f, "SEARCH005" ) ) return;
	fread( &initialized, sizeof( BOOL ), 1, f );
  fread( &entireDoc, sizeof( BOOL ), 1, f );
  fread( &forward, sizeof( BOOL ), 1, f );
  fread( &selected, sizeof( BOOL ), 1, f );
  fread( &caseSensitive, sizeof( BOOL ), 1, f );
  fread( &wholeWords, sizeof( BOOL ), 1, f );
  fread( &grep, sizeof( BOOL ), 1, f );
  fread( &buffersOnly, sizeof( BOOL ), 1, f );
  fread( &subdirs, sizeof( BOOL ), 1, f );
  fread( &strings, sizeof( int ), 1, f );
  fread( &hexSearch, sizeof( BOOL ), 1, f );
  fread( &wrapAround, sizeof( BOOL ), 1, f );

  int i;
  for( i = 0; i < strings; i++ )
  {
  	int len;
    fread( &len, sizeof( int ), 1, f );
    string[i] = (char *)malloc( len + 1 );
  	fread( string[i], len + 1, 1, f );
  }

  fread( &filesCount, sizeof( int ), 1, f );
  for( i = 0; i < filesCount; i++ )
  {
  	int len;
    fread( &len, sizeof( int ), 1, f );
    files[i] = (char *)malloc( len + 1 );
  	fread( files[i], len + 1, 1, f );
  }

  fread( &pathsCount, sizeof( int ), 1, f );
  for( i = 0; i < pathsCount; i++ )
  {
  	int len;
    fread( &len, sizeof( int ), 1, f );
    paths[i] = (char *)malloc( len + 1 );
  	fread( paths[i], len + 1, 1, f );
  }
}

void SearchDialog::doSearch( int result )
{
  int l = SearchDialog::getStringLength();
  TextElem *searchString = new TextElem[l+1], *fixedString,
  				 *fixedPath, *fixedFiles;
  char *optionsString = new char[10];

  getSettings( optionsString, searchString );

  fixedString = KCommonCommands::getInstance()->fixString( searchString );

  // Add the command to the queue

  char *cmd;

	if( result == SearchDialog::CURRENTBUFFER )
  {
		cmd = new char[strlen( fixedString ) + strlen( optionsString ) +
                   40];

    sprintf( cmd, "findString( '%s', '%s' );",
             fixedString, optionsString );
	}
  else
  {
    char *files, *path = "";

		if( searchBuffersOnly() )
    {
      fixedPath = path;

    	int buffersLen = 1;
    	char **buffers = getSelectedBuffers();

      int i;
			for( i = 0; i < getSelectedBuffersCount(); i++ )
      	buffersLen += strlen( buffers[i] ) + 1;

			files = new char[buffersLen];
      *files = 0;

			for( i = 0; i < getSelectedBuffersCount(); i++ )
      {
      	strcat( files, buffers[i] );
        strcat( files, ";" );
      }

      files[strlen(files)-1] = 0;

      fixedFiles = KCommonCommands::getInstance()->fixString( files );

			cmd = new char[strlen( fixedString ) + strlen( optionsString ) +
                     strlen( fixedPath ) +
                     strlen( fixedFiles  ) + 40];
    }
    else
    {
      path = SearchDialog::getPath();

      fixedPath = KCommonCommands::getInstance()->fixString( path );

    	files = getFiles();

      fixedFiles = KCommonCommands::getInstance()->fixString( files );

			cmd = new char[strlen( fixedString ) + strlen( optionsString ) +
                     strlen( fixedPath ) +
                     strlen( fixedFiles  ) + 60];
    }

    sprintf( cmd, "findStringFiles( '%s', '%s','%s', '%s' );",
             fixedString, optionsString, fixedPath, fixedFiles );

    if( path != fixedPath ) delete fixedPath;
    if( files != fixedFiles ) delete fixedFiles;
    if( files != getFiles() ) delete files;
  }
  
  // Find active control and execute
  MainWindowAdmin::getActiveWindow()->getActiveControl()->addCommand( cmd );

  if( searchString != fixedString ) delete fixedString;

  delete cmd;
  delete searchString;
  delete optionsString;
}

SearchDialog::~SearchDialog()
{
  WinProperties::updateProperty( name, getX(), getY(),
                                 getWidth(), getHeight() );
	delete name;
	delete singleDlg;
	delete multiDlg;
}
