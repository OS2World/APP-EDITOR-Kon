#include <string.h>
#include "bowl.h"
#include "filedlg.h"
#include "direct.h"
#include "ctype.h"

int FileDialog::files = 0;
int FileDialog::dirs = 0;
int FileDialog::currentDrive = -1;
char *FileDialog::filename[MAXFILES+1];
char *FileDialog::oldDir[MAXFILES+1];
char *FileDialog::currentDirs['Z'-'A'+1];
char FileDialog::currentDir[MAXPATH+1];
char FileDialog::filePattern[MAXPATH+1];

FileDialog::FileDialog( Window *frame, Window *parent, char *title, char *initFilename ) :
  Dialog( frame, parent, title,
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 500, 330 )
{
  char *namestr = "fileDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*2;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2,
      labelHeight = fontHeight + fontHeight / 3;

  int lblTop = yStart;

  Label *lbl = new Label( this, Label::LEFT, Strings::get(91), 10,
                          lblTop, 80, labelHeight );

  filesCombo = new SComboBox( this, ISTABSTOP, 10,
                              yStart,
                              getClientWidth() - 20,
                              editBoxHeight, 100 );

  int comboHeight = filesCombo->getEditBoxHeight();
  yStart -= comboHeight + comboHeight / 2;
  filesCombo->setPos( filesCombo->getX(), yStart);
  lbl->setSize( lbl->getWidth(), comboHeight);
  lbl->setPos( lbl->getX(), lblTop);

  filesCombo->setMaxLength( MAXFILESIZE );
  filesCombo->setWMove( FOLLOW );

  yStart -= comboHeight + fontHeight / 2;

  int iTop = yStart;

  yStart = 10 + 30 + fontHeight;

  lbl = new Label( this, Label::LEFT, Strings::get(93), 10,
                   iTop, 80, labelHeight );

  fileList = new ListBox( this, NOADJUSTSIZE|ISTABSTOP, 10, yStart,
                          getClientWidth() / 2 - 20, iTop - yStart, TRUE );
  fileList->setWMove( PROPORTIONAL );
  fileList->setHMove( FOLLOW );

	yStart = iTop;

  lbl = new Label( this, Label::LEFT, Strings::get(92), getClientWidth() / 2 + 10,
                   iTop, 80, labelHeight );
  lbl->setXMove( PROPORTIONAL );

  yStart -= comboHeight + comboHeight / 2;

  drivesCombo = new SComboBox( this, ISTABSTOP | SComboBox::STATIC, getClientWidth() / 2 + 10,
                              yStart,
                              getClientWidth() / 2 - 20,
                              editBoxHeight, 100 );

  drivesCombo->setMaxLength( MAXFILESIZE );
  drivesCombo->setWMove( PROPORTIONAL );
  drivesCombo->setXMove( PROPORTIONAL );

  yStart -= editBoxHeight + fontHeight / 2;

  lblTop = yStart;

  lbl = new Label( this, Label::LEFT, Strings::get(95), getClientWidth() / 2 + 10,
                   lblTop, 80, labelHeight );
  lbl->setXMove( PROPORTIONAL );

  yStart -= comboHeight + comboHeight / 2;

  dirsCombo = new SComboBox( this, ISTABSTOP | SComboBox::STATIC, getClientWidth() / 2 + 10,
                            yStart,
                            getClientWidth() / 2 - 20,
                            editBoxHeight, 100 );

  dirsCombo->setMaxLength( MAXFILESIZE );
  dirsCombo->setWMove( PROPORTIONAL );
  dirsCombo->setXMove( PROPORTIONAL );

  yStart -= editBoxHeight + fontHeight / 2;

  int top = yStart;

  yStart = iTop;

//--
  yStart = 10 + 30 + fontHeight;

  lbl = new Label( this, Label::LEFT, Strings::get(94),
                   getClientWidth() / 2 + 10, top, 80, labelHeight );
  lbl->setXMove( PROPORTIONAL );

  dirList = new ListBox( this, NOADJUSTSIZE|ISTABSTOP, getClientWidth() / 2 + 10,
                         yStart, getClientWidth() / 2 - 20, top - yStart, TRUE );

  dirList->setWMove( PROPORTIONAL );
  dirList->setXMove( PROPORTIONAL );
  dirList->setHMove( FOLLOW );

  yStart = 10;

  OKButton = new Button( this, Strings::get(8), AWindow::ISDEFAULT,
                         10, yStart, 70, 30 );
  OKButton->setYMove( FOLLOW );

  cancelButton = new Button( this, Strings::get(9), 0, 90, yStart, 70, 30 );
  cancelButton->setYMove( FOLLOW );

  // Fill drives combo

  char current;

  Drive *firstDrive = System::getDrivesInfo( current ), *drive;

	drive = firstDrive;

  char str[20];
  int selected = 0, driveNum = 0;

  if( currentDrive == -1 ) *currentDir = 0; // Reset currentDir if this is the first run

  while( drive )
  {
    sprintf( str, "%c: [%s]", drive->letter, drive->name );
    drivesCombo->addItem( str );
    if( drive->active )
    {
      if( currentDrive == -1 )
        currentDrive = toupper(drive->letter)-'A';
    }

    if( currentDrive == toupper(drive->letter)-'A' )
      selected = driveNum;

    drive = drive->next;
    driveNum++;
  }

	delete firstDrive;

  if( *currentDir == 0 )
  {
    _getdcwd( currentDrive + 1, currentDir, MAXPATH );
    strcpy( filePattern, "*" );
    for( int i = 0; i < 'Z'-'A'+1; i++ )
      currentDirs[i] = NULL;
  }

	setControlsVisible();

  drivesCombo->selectItem( selected );

  // Fill file list

  char pattern[MAXPATH+1];

  strcpy( pattern, currentDir );
  if( pattern[strlen(pattern)-1] != '\\' )
    strcat( pattern, "\\" );
  strcat( pattern, "*" );

  Dir fdir( pattern, DirEntry::ARCHIVED, 0 );

  int i;
  for( i = 0; i < fdir.getEntries(); i++ )
    fileList->addItem( fdir.getEntryAt( i )->getName() );

  // Fill dir list

  Dir ddir( pattern, DirEntry::DIR|DirEntry::ARCHIVED, DirEntry::DIR );

  for( i = 0; i < ddir.getEntries(); i++ )
    dirList->addItem( ddir.getEntryAt( i )->getName() );

  // Fill old dirs list

  for( i = 0; i < dirs; i++ )
    dirsCombo->addItem( oldDir[i] );

  insertDir( currentDir );

  // Fill filenames list

  for( i = 0; i < files; i++ )
    filesCombo->addItem( filename[i] );

  if( files > 0 )
    filesCombo->selectItem( 0 );

	if( initFilename != NULL )
		filesCombo->setText( initFilename );

  filesCombo->setFocus();

	filesCombo->addKeyListener( this );
  drivesCombo->addSelectionListener( this );
  dirsCombo->addSelectionListener( this );
  fileList->addMouseListener( this );
  fileList->addFocusListener( this );
  dirList->addMouseListener( this );

  OKButton->addButtonListener( this );
  cancelButton->addButtonListener( this );

  updateControlPos( NULL );

  adjustForFontSize();
  initSize();

  // Save/restore window position

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, x, y, width, height );

  setSize( width, height );

  center( x, y );

  setPos( x, y );
}

void FileDialog::insertFile( char *file )
{
  char *endFile;

  // We want only the filename

  endFile = strrchr( file, '\\' );

  if( endFile == NULL )
    endFile = strrchr( file, ':' );

  if( endFile == NULL )
    endFile = file;
  else
    endFile++;  // Move past ':' or '\'

  char fullPath[MAXPATH+1];

  strcpy( fullPath, currentDir );
  if( fullPath[strlen(fullPath)-1] != '\\' )
    strcat( fullPath, "\\" );
  strcat( fullPath, endFile );

  for( int i = 0; i < files; i++ )
    if( !stricmp( fullPath, filename[i] ) ) 
  {
  	// Move to front
    char *foo = filename[0];
    filename[0] = filename[i];
    filename[i] = foo;
  	return;
  }

  if( files == MAXFILES - 1 )
    delete filename[files-1];
  else
    files++;

  memmove( &filename[1], &filename[0], sizeof( char * ) * (files-1) );
  filename[0] = new char[strlen( fullPath ) + 1];
  strcpy( filename[0], fullPath );
}

void FileDialog::insertDir( char *dir )
{
  for( int i = 0; i < dirs; i++ )
    if( !strcmp( dir, oldDir[i] ) )
    {
      currentDirs[toupper( *oldDir[i])-'A' ] = oldDir[i];
      dirsCombo->selectItem( i );   // Select the dir
      return;
    }

  if( dirs == MAXFILES - 1 )
    delete oldDir[dirs-1];
  else
    dirs++;

  // Select the new dir in the dir list

  memmove( &oldDir[1], &oldDir[0], sizeof( char * ) * (dirs-1) );

  oldDir[0] = new char[strlen( dir ) + 1];
  strcpy( oldDir[0], dir );

  currentDirs[toupper( *oldDir[0])-'A' ] = oldDir[0];

  dirsCombo->insertItem( 0, oldDir[0] );
  dirsCombo->selectItem( 0 );
}

void FileDialog::fillLists()
{
  char pattern[MAXPATH+1];

  strcpy( pattern, currentDir );
  if( pattern[strlen(pattern)-1] != '\\' )
    strcat( pattern, "\\" );
  strcat( pattern, filePattern );

  fileList->clearItems();
  dirList->clearItems();

  // Fill file list

  Dir fdir( pattern, DirEntry::ARCHIVED, 0 );

  int i;
  for( i = 0; i < fdir.getEntries(); i++ )
    fileList->addItem( fdir.getEntryAt( i )->getName() );

  // Fill dir list

  strcpy( pattern, currentDir );
  if( pattern[strlen(pattern)-1] != '\\' )
    strcat( pattern, "\\" );
  strcat( pattern, "*" );

  Dir ddir( pattern, DirEntry::DIR|DirEntry::ARCHIVED, DirEntry::DIR );

  for( i = 0; i < ddir.getEntries(); i++ )
    dirList->addItem( ddir.getEntryAt( i )->getName() );

  filesCombo->selectText();
}

BOOL FileDialog::actionPerformed( AKeyEvent *keyEvent )
{
	// Do not process with modifiers or non alphanumeric keys
	if( keyEvent->modifiers != 0 || keyEvent->key < 0x20 ||
  		keyEvent->key > 255 )
  	return FALSE;

	// We get the key before it has been processed by the control.
  // Add the key to the string

  filesCombo->clearSelection();

	int len = filesCombo->getTextLength();

	char *selText = new char[len+2];
  strcpy( selText, filesCombo->getEditText() );
  selText[len] = keyEvent->key;
  selText[len+1] = 0;

	int pos = fileList->findItem( selText, TRUE );

	delete selText;

	if( pos != -1 )
  {
  	char file[MAXPATH+1];
    fileList->getItem( pos, MAXPATH, file );
  	fileList->selectItem( pos );
    filesCombo->setText( file );
    filesCombo->selectText( len+1, strlen( file ) );
	  return TRUE;
  }
  return FALSE;
}

void FileDialog::actionPerformed( ASelectionEvent *selectionEvent )
{
  BOOL fill = FALSE;

  if( selectionEvent->getSource() == drivesCombo )
  {
    char *drive = drivesCombo->getEditText();

    currentDrive = toupper( *drive ) - 'A';

    _getdcwd( currentDrive + 1, currentDir, MAXPATH );
    insertDir( currentDir );
    fill = TRUE;
  }
  else
  if( selectionEvent->getSource() == dirsCombo )
  {
    strcpy( currentDir, dirsCombo->getEditText() );
    currentDrive = toupper( *currentDir ) - 'A';

    char drive[100];

    // Find the correct entry in the drivesCombo

    for( int i = 0; i < drivesCombo->getItems(); i++ )
    {
      drivesCombo->getItem( i, 99, drive );
      if( toupper( *drive ) == currentDrive + 'A' )
      {
        drivesCombo->selectItem( i );
        break;
      }
    }

    fill = TRUE;
  }

  if( fill )
  {
    filesCombo->setText( "" );
    fillLists();
  }
}

void FileDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == OKButton )
  {
    // Get the filename in the filesCombo

    char fname[MAXPATH+1];

    strcpy( fname, filesCombo->getEditText() );

    if( *fname == 0 ) // Empty string, do nothing
      return;

    // See if there are any wildcards

    if( strchr( fname, '*' ) != NULL || strchr( fname, '?' ) != NULL )
    {
      char *endPos = fname+strlen( fname ) - 1;
      while( endPos != fname && *(endPos-1) != '\\' && *(endPos-1) != ':' )
        endPos--;

      if( strchr( endPos, '*' ) != NULL || strchr( endPos, '?' ) != NULL )
      {
        // Nothing's wrong, we have a file pattern

        strcpy( filePattern, endPos );

        if( endPos != fname ) // A path or drive too
        {
          *(endPos-1) = 0;

          int oldDrive = currentDrive;  // Store if somethings wrong

          if( *(fname+1) == ':' )
            currentDrive = toupper(*fname) - 'A';
          else
          {
            memmove( fname+2, fname, strlen( fname ) + 1 );
            *fname = currentDrive+'A';
            *(fname+1) = ':';
          }

          char prevDir[MAXPATH+1];

          _getdcwd( oldDrive + 1, prevDir, MAXPATH );  // Store current dir
          if( *(endPos-1) == ':' || chdir( fname ) == 0 )
          {
            _getdcwd( currentDrive + 1, currentDir, MAXPATH );

            insertDir( currentDir );
          }
          else
          {
            currentDrive = oldDrive;
            return; // Didn't work
          }
          chdir( prevDir );  // Restore current directory
        }
        fillLists();
        return;
      }
      else
        return;
    }
    else
    {
      // See if this is a directory, by trying to do a chdir on it..

      char prevDir[MAXPATH+1], newDir[MAXPATH+1];

      strcpy( newDir, fname );

      if( newDir[1] != ':' )
      {
        memmove( newDir+2, newDir, strlen( fname ) + 1 );
        *newDir = currentDrive+'A';
        *(newDir+1) = ':';
      }

      _getdcwd( newDir[0]-'A'+1, prevDir, MAXPATH );  // Store current dir

      if( currentDirs[toupper( *newDir ) -'A'] != NULL )
        chdir( currentDirs[toupper( *newDir ) -'A'] );  // Change relative

      if( chdir( newDir ) == 0 )
      {
        // It was

        currentDrive = toupper(*newDir)-'A';

        _getdcwd( currentDrive + 1, currentDir, MAXPATH );

        chdir( prevDir );

        insertDir( currentDir );

        fillLists();
        return;
      }
      else  // If it has a drive reference "X:FILE", or "X:"
      if( fname[1] == ':' &&
          toupper( *fname >= 'A' ) && toupper( *fname ) <= 'Z' )
      {
        currentDrive = toupper(*fname)-'A';

        // See if the dir is found in the currentDirs list

        if( currentDirs[toupper( *fname ) -'A'] != NULL )
          strcpy( currentDir, currentDirs[toupper( *fname ) -'A'] );
        else // Not found
          _getdcwd( currentDrive + 1, currentDir, MAXPATH );

        insertDir( currentDir );

        if( strlen( fname ) == 2 )
        {
          // Drive only
          fillLists();
          return;
        }
      }

      // Set new current drive and dir if needed

      if( fname[1] == ':' )
        currentDrive = toupper( *fname ) - 'A';

      strcpy( newDir, fname );
      char *l = strrchr( newDir, '\\' );

      if( l != NULL )
      {
        if( *(l-1) == ':' ) l++;
        *l = 0;
      }
      chdir( newDir );      // Relative path
      _getdcwd( currentDrive + 1, currentDir, MAXPATH );
      chdir( prevDir );
      insertDir( currentDir );
    }

    insertFile( fname );

    // It was a file

    closeDialog( TRUE );
    return;
  }
  closeDialog( FALSE );
}

void FileDialog::actionPerformed( AFocusEvent *focusEvent )
{
	char text[MAXPATH+1];
  if( focusEvent->getSource() != fileList ) return;
	((ListBox*)focusEvent->getSource())->getSelectedItem( MAXPATH, text );
  filesCombo->setText( text );
}

BOOL FileDialog::actionPerformed( AMouseEvent *mouseEvent )
{
	if( mouseEvent->getEvent() != AMouseEvent::DOUBLECLICK ) return TRUE;

  if( mouseEvent->getSource() == fileList )
  {
    char file[MAXPATH+1];
    fileList->getSelectedItem( MAXPATH, file );

    insertFile( file );
    closeDialog( TRUE );
  }
  else
  if( mouseEvent->getSource() == dirList )
  {
    char newDir[MAXPATH+1], prevDir[MAXPATH+1];
    strcpy( newDir, " :" );
    newDir[0] = currentDrive + 'A';
    dirList->getSelectedItem( 297, &newDir[2] );
    _getdcwd( currentDrive + 1, prevDir, MAXPATH );  // Store current dir
    chdir( currentDir );
    chdir( newDir );
    _getdcwd( currentDrive + 1, currentDir, MAXPATH );
    chdir( prevDir );  // Restore current directory
    insertDir( currentDir );
    filesCombo->setText( "" );
    fillLists();
 }
 return TRUE;
}