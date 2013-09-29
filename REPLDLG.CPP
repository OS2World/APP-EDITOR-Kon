#include <direct.h>
#include <string.h>
#include "bowl.h"
#include "srchdlg.h"
#include "repldlg.h"
#include "viewwin.h"
#include "cntrladm.h"
#include "editctrl.h"
#include "mainwadm.h"
#include "mainwnd.h"

BOOL ReplaceDialog::initialized = FALSE;
BOOL ReplaceDialog::prompt = TRUE;
BOOL ReplaceDialog::replaceAll = FALSE;
int ReplaceDialog::strings = 0;
TextElem *ReplaceDialog::string[MAXSTRINGS+1];

ReplaceDialog::ReplaceDialog( Window *frame, Window *parent, TextElem *findString ) :
  Dialog( frame, parent, Strings::get( 83 ),
          AWindow::CENTER | AWindow::ADJUSTSIZE,
          0, 0, 500, 385 )
{
  char *namestr = "ReplaceDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*2 - 20,
      interSpace = fontHeight + fontHeight / 2,
      labelHeight = fontHeight + fontHeight / 3;

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

  yStart -= fontHeight * 2;

  Label *lbl2 = new Label( singleDlg, Label::RIGHT, Strings::get(81), 10,
                          yStart, 80, comboHeight );

  replaceCombo = new SComboBox( singleDlg, ISTABSTOP, 100,
                               yStart,
                               getClientWidth() - 120,
                               editBoxHeight, 100 );

  replaceCombo->setMaxLength( MAXSTRINGSIZE );
  replaceCombo->setWMove( FOLLOW );

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

  yStart -= interSpace;

  wrapAroundCheck = new CheckBox( singleDlg, Strings::get(255),
                            0, 20, yStart, 200, fontHeight*2 );

  int bottom = yStart - fontHeight / 2;

  yStart = top;

  yStart -= fontHeight * 3;

  promptCheck = new CheckBox( singleDlg, Strings::get(82),
                            0, 230, yStart, 200, fontHeight*2 );

  yStart -= interSpace;

  replaceAllCheck = new CheckBox( singleDlg, Strings::get(89),
                            0, 230, yStart, 200, fontHeight*2 );

  yStart -= interSpace;

  hexSearchCheck = new CheckBox( singleDlg, Strings::get(247),
                            0, 230, yStart, 200, fontHeight*2 );

  yStart = bottom;

  GroupBox *gBox = new GroupBox( singleDlg, Strings::get(65), 0, 10, yStart,
                                 450, top - bottom );
  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 2 + 20;

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

  yStart = top - (fontHeight * 3);

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

  searchButton = new Button( singleDlg, Strings::get(84), AWindow::ISDEFAULT|ISTABSTOP,
                             10, yStart, 190, 30 );
  searchButton->setYMove( FOLLOW );
  cancel1Button = new Button( singleDlg, Strings::get(9), ISTABSTOP, 210, yStart, 70, 30 );
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

  yStart -= interSpace;

  buffersCheck = new CheckBox( multiDlg, Strings::get(103),
                            0, 20, yStart, 200, fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( multiDlg, Strings::get(104), 0, 10, yStart,
                       220, top - yStart );

  yStart -= fontHeight * 2 - fontHeight / 2;

  lblB = new Label( multiDlg, Label::LEFT, Strings::get(100), 10,
                   yStart, 80, labelHeight );

  top = yStart - 2;

  openBuffersList = new ListBox( multiDlg, 
  															 NOADJUSTSIZE|ISTABSTOP|MULTIPLESEL|ISTABSTOP, 
  															 10, 50, getClientWidth() - 20, top - 50, 
                                 TRUE );
  openBuffersList->setWMove( FOLLOW );
  openBuffersList->setHMove( FOLLOW );

  notebook->setTab( 0 );

  // Set combo list box
  int i;
  for( i = 0; i < SearchDialog::strings; i++ )
    searchCombo->addItem( SearchDialog::string[i] );

	if( findString != NULL )
    searchCombo->setText( findString );
	else
  if( SearchDialog::strings )
    searchCombo->setText( SearchDialog::string[0] );

  for( i = 0; i < strings; i++ )
    replaceCombo->addItem( string[i] );

  if( strings )
    replaceCombo->setText( string[0] );

  // Set buttons

  if( SearchDialog::entireDoc )
    entireDocButton->check( TRUE );
  else
    fromCursorButton->check( TRUE );

  if( SearchDialog::forward )
    fwdButton->check( TRUE );
  else
    bkwdButton->check( TRUE );

  if( SearchDialog::selected )
    selectedButton->check( TRUE );
  else
    globalButton->check( TRUE );

  if( SearchDialog::caseSensitive )
    caseCheck->check( TRUE );
  if( SearchDialog::wholeWords )
    wordCheck->check( TRUE );
  if( SearchDialog::grep )
    grepCheck->check( TRUE );

  if( prompt )
    promptCheck->check( TRUE );

  if( replaceAll )
    replaceAllCheck->check( TRUE );
  
  if( SearchDialog::subdirs )
    subdirCheck->check( TRUE );
  if( SearchDialog::buffersOnly )
    buffersCheck->check( TRUE );
  if( SearchDialog::hexSearch )
    hexSearchCheck->check( TRUE );
  if( SearchDialog::wrapAround )
    wrapAroundCheck->check( TRUE );

  for( i = 0; i < SearchDialog::filesCount; i++ )
    filesCombo->addItem( SearchDialog::files[i] );

  if( SearchDialog::files )
    filesCombo->setText( SearchDialog::files[0] );

  for( i = 0; i < SearchDialog::pathsCount; i++ )
    pathsCombo->addItem( SearchDialog::paths[i] );

  if( SearchDialog::paths )
    pathsCombo->setText( SearchDialog::paths[0] );

  // Fill buffers list

  for( i = 0; i < ControlAdmin::getControls(); i++ )
    openBuffersList->addItem( ControlAdmin::getControl(i)->editControl->
                                getTextBuffer()->getFilename() );

	singleDlg->setControlsVisible();
	multiDlg->setControlsVisible();
	setControlsVisible();

  filesCombo->setFocus();
  searchCombo->setFocus();

  enableControls();

  searchButton->addButtonListener( this );
  cancel1Button->addButtonListener( this );
  multSearchButton->addButtonListener( this );
  cancel2Button->addButtonListener( this );
  buffersCheck->addButtonListener( this );
  currentDirButton->addButtonListener( this );

  singleDlg->updateControlPos( NULL );

  adjustForFontSize();

  // Save/restore window position

  initSize();

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  center( x, y );
  WinProperties::retrieveProperty( name, x, y, width, height );

//  setSize( width, height );
  setPos( x, y );
}

void ReplaceDialog::enableControls()
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

void ReplaceDialog::actionPerformed( AButtonEvent *buttonEvent )
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

    SearchDialog::entireDoc = entireDocButton->queryCheck();
    SearchDialog::forward = fwdButton->queryCheck();
    SearchDialog::selected = selectedButton->queryCheck();
    SearchDialog::caseSensitive = caseCheck->queryCheck();
    SearchDialog::wholeWords = wordCheck->queryCheck();
    SearchDialog::grep = grepCheck->queryCheck();
    prompt = promptCheck->queryCheck();
    replaceAll = replaceAllCheck->queryCheck();
    SearchDialog::subdirs = subdirCheck->queryCheck();
    SearchDialog::buffersOnly = buffersCheck->queryCheck();
    SearchDialog::hexSearch = hexSearchCheck->queryCheck();
    SearchDialog::wrapAround = wrapAroundCheck->queryCheck();

    if( SearchDialog::strings == 0 || strcmp( searchCombo->getEditText(),
                                              SearchDialog::string[0] ) )
    {
      if( SearchDialog::strings > 0 )
	      memmove( &SearchDialog::string[1], &SearchDialog::string[0],
  	             sizeof( *SearchDialog::string ) * (SearchDialog::strings) );

      if( SearchDialog::strings == MAXSTRINGS-1 )
        delete SearchDialog::string[SearchDialog::strings-1];
      else
	      SearchDialog::strings++;

      SearchDialog::string[0] = new TextElem[searchCombo->getTextLength()+1];
      strcpy( SearchDialog::string[0], searchCombo->getEditText() );
    }

    if( strings == 0 || strcmp( replaceCombo->getEditText(), string[0] ) )
    {
      if( strings > 0 )
      	memmove( &string[1], &string[0], sizeof( *string ) * strings );

      if( strings == MAXSTRINGS-1 )
        delete string[strings-1];
      else
	      strings++;

      string[0] = new TextElem[replaceCombo->getTextLength()+1];
      strcpy( string[0], replaceCombo->getEditText() );
    }

    if( SearchDialog::filesCount == 0 || strcmp( filesCombo->getEditText(), SearchDialog::files[0] ) )
    {
      if( SearchDialog::filesCount > 0 )
      	memmove( &SearchDialog::files[1], &SearchDialog::files[0], sizeof( *SearchDialog::files ) * SearchDialog::filesCount );

      if( SearchDialog::filesCount == MAXSTRINGS-1 )
        delete SearchDialog::files[SearchDialog::filesCount-1];
      else
	      SearchDialog::filesCount++;

      SearchDialog::files[0] = new TextElem[filesCombo->getTextLength()+1];
      strcpy( SearchDialog::files[0], filesCombo->getEditText() );
    }

    if( SearchDialog::pathsCount == 0 || strcmp( pathsCombo->getEditText(), SearchDialog::paths[0] ) )
    {
      if( SearchDialog::pathsCount > 0 )
      	memmove( &SearchDialog::paths[1], &SearchDialog::paths[0], sizeof( *SearchDialog::paths ) * SearchDialog::pathsCount );

      if( SearchDialog::pathsCount == MAXSTRINGS-1 )
        delete SearchDialog::paths[SearchDialog::pathsCount-1];
      else
	      SearchDialog::pathsCount++;

      SearchDialog::paths[0] = new TextElem[pathsCombo->getTextLength()+1];
      strcpy( SearchDialog::paths[0], pathsCombo->getEditText() );
    }

		// Free bufferlist
    
		if( SearchDialog::buffersSelected != 0 )
    {
    	while( SearchDialog::buffersSelected )
      {
      	delete SearchDialog::buffers[SearchDialog::buffersSelected-1];
        SearchDialog::buffersSelected--;
      }

			free( SearchDialog::buffers );
    }
    
		// Fill bufferlist
    
		SearchDialog::buffers = NULL;
    
		int currentIndex = -1;

		while( (currentIndex = openBuffersList->getNextSelectedIndex( currentIndex )) !=
    				-1 )
		{
    	SearchDialog::buffersSelected++;
    	SearchDialog::buffers = (char **)realloc( SearchDialog::buffers, sizeof( char * ) * SearchDialog::buffersSelected );
      SearchDialog::buffers[SearchDialog::buffersSelected-1] = new char[MAXSTRINGSIZE+1];
    	openBuffersList->getItem( currentIndex, MAXSTRINGSIZE, SearchDialog::buffers[SearchDialog::buffersSelected-1] );
    }
    
    doReplace( (buttonEvent->getSource() == searchButton) ? 
    							SearchDialog::CURRENTBUFFER : SearchDialog::MULTIPLEFILES );

    SearchDialog::initialized = TRUE;
    return;
  }
  closeDialog( FALSE );
}

void ReplaceDialog::getSettings( char *options, TextElem *searchString,
                                 TextElem *replaceString )
{
  *options = 0;

  strcpy( searchString, SearchDialog::string[0] );
  strcpy( replaceString, string[0] );

  if( SearchDialog::entireDoc )
    strcat( options, "d" );
  if( !SearchDialog::forward )
    strcat( options, "b" );
  if( SearchDialog::selected )
    strcat( options, "s" );
  if( !SearchDialog::caseSensitive )
    strcat( options, "u" );
  if( SearchDialog::wholeWords )
    strcat( options, "w" );
  if( SearchDialog::grep )
    strcat( options, "g" );
  if( !prompt )
    strcat( options, "n" );
  if( replaceAll )
    strcat( options, "a" );
  if( SearchDialog::subdirs )
    strcat( options, "r" );
  if( SearchDialog::hexSearch )
    strcat( options, "h" );
  if( SearchDialog::wrapAround )
    strcat( options, "x" );
}


int QueryReplaceDialog::REPLACE = 1;
int QueryReplaceDialog::REPLACEALL = 2;
int QueryReplaceDialog::REPLACEINALLFILES = 3;
int QueryReplaceDialog::DONTREPLACE = 4;
int QueryReplaceDialog::SKIPREST = 5;
int QueryReplaceDialog::CANCEL = 0;

QueryReplaceDialog::QueryReplaceDialog( Window *frame, ViewWindow *viewWin,
																					Window *parentFrame,
  											 			 char *foundString, char *replaceString, 
                               int position, BOOL multiFile ) :
  Dialog( frame, parentFrame, Strings::get( 105 ),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 500, (multiFile) ? 200 : 150 )
{
  char *namestr = " QueryReplaceDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );
  *name = ((multiFile) ? 'M' : 'S');

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      interSpace = fontHeight / 2,
      labelHeight = fontHeight,
      yStart = getClientHeight() - labelHeight - interSpace,
      bYStart = (multiFile) ? 50 : 10;

  new Label( this, Label::LEFT, Strings::get( 106 ),
             margin, yStart, 500-margin*2, labelHeight );

  yStart -= labelHeight + interSpace;

  new Label( this, Label::LEFT, foundString,
             margin, yStart, 500-margin*2, labelHeight );

  yStart -= labelHeight + interSpace;

  new Label( this, Label::LEFT, Strings::get( 107 ),
             margin, yStart, 500-margin*2, labelHeight );

  yStart -= labelHeight + interSpace;

  new Label( this, Label::LEFT, replaceString,
             margin, yStart, 500-margin*2, labelHeight );

  int xStart = margin, bWidth = 100, bInterspace = 10;

  replaceButton = new Button( this, Strings::get(108), AWindow::ISDEFAULT,
                              xStart, bYStart, bWidth, 30 );
  replaceButton->addButtonListener( this );

  xStart += bWidth + bInterspace;

  dontReplaceButton = new Button( this, Strings::get(109), 0,
                              xStart, bYStart, bWidth, 30 );

  dontReplaceButton->addButtonListener( this );

  xStart += bWidth + bInterspace;

  replaceAllButton = new Button( this, Strings::get(110), 0,
                              xStart, bYStart, bWidth, 30 );

  replaceAllButton->addButtonListener( this );

  xStart += bWidth + bInterspace;

  if( multiFile )
  {
    bYStart -= 40;
    xStart = margin;

    replaceInAllFilesButton = new Button( this, Strings::get(111), 
                              0,
                              xStart, bYStart, bWidth + 30, 30 );

    replaceInAllFilesButton->addButtonListener( this );

    xStart += bWidth + bInterspace + 30;

    skipRestButton = new Button( this, Strings::get(112), 0,
                              xStart, bYStart, bWidth, 30 );

    skipRestButton->addButtonListener( this );

    xStart += bWidth + bInterspace;
  }

  cancelButton = new Button( this, Strings::get(9), 0,
                              xStart, bYStart, bWidth, 30 );


  cancelButton->addButtonListener( this );

  adjustForFontSize();

  int foo;

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, foo, foo, width, height );

  setSize( width, height );

  center( x, y );

	int stringYPos = viewWin->getPositionVertPixel( position, FALSE );

	remapCoordinates( viewWin, Desktop::getInstance(), foo, stringYPos );

  y = stringYPos - getHeight() - 10;

  if( y < 0 )
  {
	  stringYPos = viewWin->getPositionVertPixel( position, TRUE );
  
	  remapCoordinates( viewWin, Desktop::getInstance(), foo, stringYPos );
    
    y = stringYPos + 10;
  }

  setPos( x, y );

  setControlsVisible();

  replaceButton->setFocus();
}


void QueryReplaceDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == replaceButton )
    closeDialog( REPLACE );
  else
  if( buttonEvent->getSource() == replaceAllButton )
    closeDialog( REPLACEALL );
  else
  if( buttonEvent->getSource() == replaceInAllFilesButton )
    closeDialog( REPLACEINALLFILES );
  else
  if( buttonEvent->getSource() == dontReplaceButton )
    closeDialog( DONTREPLACE );
  else
  if( buttonEvent->getSource() == skipRestButton )
    closeDialog( SKIPREST );
  else
    closeDialog( CANCEL );
}

void ReplaceDialog::saveSettings( FILE *f )
{
	long pos;
	if( !ClassProperties::savePropertyName( f, "REPLACE004", &pos ) ) return;
  fwrite( &initialized, sizeof( BOOL ), 1, f );
  fwrite( &prompt, sizeof( BOOL ), 1, f );
  fwrite( &replaceAll, sizeof( BOOL ), 1, f );

  fwrite( &strings, sizeof( int ), 1, f );
  for( int i = 0; i < strings; i++ )
  {
  	int len = strlen( string[i] );
  	fwrite( &len, sizeof( int ), 1, f );
  	fwrite( string[i], len+1, 1, f );
  }
	ClassProperties::savePropertySize( f, pos );
}

void ReplaceDialog::restoreSettings( FILE *f )
{
	if( !ClassProperties::checkPropertyExists( f, "REPLACE004" ) ) return;
  fread( &initialized, sizeof( BOOL ), 1, f );
  fread( &prompt, sizeof( BOOL ), 1, f );
  fread( &replaceAll, sizeof( BOOL ), 1, f );

  fread( &strings, sizeof( int ), 1, f );
  for( int i = 0; i < strings; i++ )
  {
  	int len;
    fread( &len, sizeof( int ), 1, f );
    string[i] = (char *)malloc( len + 1 );
  	fread( string[i], len + 1, 1, f );
  }

}

void ReplaceDialog::doReplace( int result )
{
  int l1 = getFindStringLength(),
      l2 = getReplaceStringLength();
  TextElem *searchString = new TextElem[l1+1], *fixedSearchString,
           *replaceString = new TextElem[l2+1], *fixedReplaceString,
           *fixedPath, *fixedFiles;
  char *optionsString = new char[10], *cmd;

  getSettings( optionsString, searchString,
  														replaceString );

  fixedSearchString = KCommonCommands::getInstance()->
  											fixString( searchString );
  fixedReplaceString = KCommonCommands::getInstance()->
  											fixString( replaceString );

  // Add the command to the queue

	if( result == SearchDialog::CURRENTBUFFER )
  {
    cmd = new char[strlen( fixedSearchString ) +
                   strlen( optionsString ) +
                   strlen( fixedReplaceString ) + 50];

    sprintf( cmd, "replaceString( '%s', '%s', '%s' );",
             fixedSearchString, fixedReplaceString, optionsString );
  }
  else
  {
    char *files, *path = "";

		if( SearchDialog::searchBuffersOnly() )
    {
      fixedPath = path;

    	int buffersLen = 1;
    	char **buffers = SearchDialog::getSelectedBuffers();

      int i;
			for( i = 0; i < SearchDialog::getSelectedBuffersCount(); i++ )
      	buffersLen += strlen( buffers[i] ) + 1;

			files = new char[buffersLen];
      *files = 0;

			for( i = 0; i < SearchDialog::getSelectedBuffersCount(); i++ )
      {
      	strcat( files, buffers[i] );
        strcat( files, ";" );
      }

      files[strlen(files)-1] = 0;

      fixedFiles = KCommonCommands::getInstance()->fixString( files );

			cmd = new char[strlen( fixedSearchString ) + 
                     strlen( fixedReplaceString ) + 
                     strlen( optionsString ) +
                     strlen( fixedPath ) +
                     strlen( fixedFiles  ) + 50];
    }
    else
    {
      path = SearchDialog::getPath();

      fixedPath = KCommonCommands::getInstance()->fixString( path );

    	files = SearchDialog::getFiles();

      fixedFiles = KCommonCommands::getInstance()->fixString( files );

			cmd = new char[strlen( fixedSearchString ) + 
                     strlen( fixedReplaceString ) + 
                     strlen( optionsString ) +
                     strlen( fixedPath ) +
                     strlen( fixedFiles  ) + 60];
    }

    sprintf( cmd, "replaceStringFiles( '%s', '%s', '%s','%s', '%s' );",
             fixedSearchString, fixedReplaceString, optionsString, fixedPath, fixedFiles );

    if( path != fixedPath ) delete fixedPath;
    if( files != fixedFiles ) delete fixedFiles;
    if( files != SearchDialog::getFiles() ) delete files;
  }

  // Find active control and execute
  MainWindowAdmin::getActiveWindow()->getActiveControl()->addCommand( cmd );

  if( searchString != fixedSearchString ) delete fixedSearchString;
  if( replaceString != fixedReplaceString ) delete fixedReplaceString;

  delete cmd;
  delete searchString;
  delete replaceString;
  delete optionsString;
}

ReplaceDialog::~ReplaceDialog()
{
  WinProperties::updateProperty( name, getX(), getY(),
                                 getWidth(), getHeight() );
	delete name;
	delete singleDlg;
	delete multiDlg;
}
