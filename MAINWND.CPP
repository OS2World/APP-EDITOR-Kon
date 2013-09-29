#include "bowl.h"
#include "k.h"
#include "mainwnd.h"
#include "bufadmin.h"
#include "prefsdlg.h"
#include "filedlg.h"

Vector<ToolButtonInfo>MainWindow::toolButtonInfo(FALSE);
BOOL MainWindow::toolbarModified = FALSE;
int MainWindow::tbSize = 1;
char *MainWindow::command[MAXCOMMANDS];
int MainWindow::commands = 0;
int MainWindow::separatorRestoreY = 150;
int MainWindow::cmdButtonWidth = 20;
int MainWindow::buttonWidth = 16;
BOOL MainWindow::separatorMinimized = FALSE;

int ToolbarDialog::NEW = 0;
int ToolbarDialog::SPACE = 1;
int ToolbarDialog::COMMAND = 2;

void BufferLoader::run()
{
  int length = fileReader->getFileLength(), pos = 0;
  int left = length;
  TextElem *text = new TextElem[BLOCKSIZE*2];
  TextElem *ctext = new TextElem[BLOCKSIZE*2];
  EncodingTypes encoding = ANSI;

  int writer = textBuffer->writeRequest();

  if( reload )
  {
    // Delete previous text
    textBuffer->deleteTextNoUndo(writer, 0, textBuffer->getSize(), FALSE, FALSE );
  }

  char progress[100];
  sprintf( progress, Strings::get( 79 ), 0 );
  infoBar->message( progress );

  int percentage = 0, lastPercentage = 0;
  
  while( left )
  {
    int read = (left < BLOCKSIZE) ? left : BLOCKSIZE;
    fileReader->readBlock( text, read );
    left -= read;

    // Check if this is a unicode file
    if( pos == 0 )
    {
      encoding = UnicodeHelper::checkEncoding( (unsigned char *)text, read );
      if( encoding != ANSI && Settings::queryIfUnicode )
      {
			  MessageBoxInfo mbInfo;

			  mbInfo.title = Strings::get(238);
			  mbInfo.message = Strings::get(239);
			  mbInfo.type = PopupMessage::querywithwarningNoCancel;

			  int result = Application::executeFromMainThread( mainWnd, 7, &mbInfo );
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

    textBuffer->insertTextNoUndo( writer, pos, ctext, read, FALSE, FALSE );
    pos += read;
    if( length > 20000000 )
      percentage = pos / (length / 100);
    else
      percentage = (pos * 100) / length;
    if( !(percentage % 10L) && percentage != lastPercentage)
    {
      lastPercentage = percentage;
      sprintf( progress, Strings::get( 79 ), percentage );
      infoBar->message( progress );
    }
  }
  infoBar->clearMessage();

  textBuffer->getUndoBuffer()->clearUndo();
  textBuffer->setUnmodified(FALSE);
  textBuffer->setEncoding( encoding );

  textBuffer->writeRelease();

  BTime timeWritten;

  System::getFileTime( textBuffer->getFilename(), &timeWritten );
  textBuffer->setTimeWritten(&timeWritten );

  delete fileReader;
  delete text;
  if( encoding != ANSI )
    delete ctext;

   textBuffer->finishedLoading( textBuffer->isBinary(), reload );
}


ToolbarDialog::ToolbarDialog( Window *frame, Window *parent, int type,
                              char *title, char *filename, char *command,
                              char *description ) :
    Dialog( frame, parent, title, AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
            0, 0, 400, 330 )
{
  char *namestr = "toolbarDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = getGraphics()->getFontHeight(),
      yStart = getClientHeight() - fontHeight*2 - 30,
      rWidth = (getClientWidth() - margin * 2 - 40) / 2,
      labelHeight = fontHeight + fontHeight / 3;

  // Command radiobutton

  commandButton = new RadioButton( this, Strings::get(4), NEWGROUP|ISTABSTOP,
                                   margin+10,
                                   yStart,
                                   rWidth, fontHeight + 10 );

  commandButton->addButtonListener( this );

  // Add space radiobutton

  spaceButton = new RadioButton( this, Strings::get(5), INGROUP|ISTABSTOP,
                                 margin + rWidth + 20,
                                 yStart,
                                 rWidth, fontHeight + 10 );

  spaceButton->addButtonListener( this );

  // If we modify a button, space shouldn't be possible to select

  if( type == COMMAND )
    spaceButton->enable( FALSE );

  // Create groupbox

  new GroupBox( this, Strings::get(3), 0, margin, belowPrevious() - 10,
                getClientWidth() - margin * 2,
                fontHeight * 3 + 15 );

  // Add command label

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2,
      lMargin = margin;

  cLabel = new Label( this, Label::LEFT, Strings::get(6),
                      lMargin, belowPrevious() - 10 - fontHeight,
                      getClientWidth() - margin - lMargin,
                      labelHeight );

  // Add commands combobox

  commandCombo = new SComboBox( this, ISTABSTOP, lMargin,
                               belowPrevious() - 10 - editBoxHeight,
                               getClientWidth() - margin - lMargin,
                               editBoxHeight, 100 );

  commandCombo->setMaxLength( 200 );
  commandCombo->setText( command );

  // Fill in all available commands
  int i;
  for( i = 0; i < FunctionList.getCount(); i++ )
    commandCombo->addItem( FunctionList.getFunction(i)->Name );

  // Add description label

  dLabel = new Label( this, Label::LEFT, Strings::get(7),
                      lMargin, belowPrevious() - fontHeight - 10,
                      getClientWidth() - margin - lMargin,
                      labelHeight );

  // Add descriptionfield

  descriptionField = new EntryField( this, ISTABSTOP|BORDER,
                                     lMargin,
                                     belowPrevious() - editBoxHeight - 10,
                                     getClientWidth() - margin - lMargin,
                                     editBoxHeight);

  descriptionField->setMaxLength( 80 );
  descriptionField->setText( description );

  // Create a window to hold all icons

  iconWindow = new IconWindow( this, margin,
                    belowPrevious() - 72,
                    getClientWidth() - margin * 2, 52 );

  // This is a control: Add it as a such

  addControl( iconWindow );

  // Read all bitmap file entries

  char pattern[MAXPATH+1];
  sprintf( pattern, "%s%s", KonSystem::bmpDir, "*.BMP" );

  Dir dir( pattern, DirEntry::ARCHIVED, 0 );

  for( i = 0; i < dir.getEntries(); i++ )
  {
    char filename[MAXPATH+1];
    sprintf( filename, "%s%s", KonSystem::bmpDir, dir.getEntryAt( i )->getName() );
    iconWindow->addIcon( filename );
  }

  iconWindow->selectIcon( filename );

  // Add scrollbar

  scrollbar = new Scrollbar( this, 0, margin,
                belowPrevious() - Scrollbar::getInset(0),
                getClientWidth() - margin*2, Scrollbar::getInset(0) );

  addScrollbarListener( this );

  // Add OK & Cancel buttons

  OKbutton = new Button( this, Strings::get(8), NEWGROUP|ISDEFAULT|ISTABSTOP, /*getInset() + */10, 10, 70, 30 );
  Button *Cancelbutton = new Button( this, Strings::get(9), INGROUP, /*getInset() + */90, 10, 70, 30 );

  OKbutton->addButtonListener( this );
  Cancelbutton->addButtonListener( this );

  adjustForFontSize();

  // Save/restore window position

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, x, y, width, height );

  setPos( x, y );
  setSize( width, height );

  scrollbar->setRatio( iconWindow->getIconsInWindow(), iconWindow->getIcons() );

  if( type == SPACE )
    spaceButton->check( TRUE );
  else
    commandButton->check( TRUE );

  setControlsVisible();
  commandCombo->setFocus();
}

void ToolbarDialog::IconWindow::paint( Rect *rect, Graphics *g )
{
  g->setBackColor( Color::BKGR );
  g->fillRect( rect  );
  for( int i = leftPos; i < icons.getSize(); i++ )
  {
    g->drawBitmap( ((Icon *)icons.getObjectAt(i))->getBitmap(),
                   enabled ? Graphics::ACTIVE : Graphics::INACTIVE,
                   (i - leftPos) * (getHeight()-2)+2, 2,
                   getHeight()-4, getHeight()-4 );
    if( (i - leftPos) * (getHeight()-2) > getWidth() ) break;
  }

  // Draw a frame around the selected icon
  if( enabled )
    g->setColor( Color::BLACK );
  else
    g->setColor( Color::SHADOW );
  Rect selRect( (selectedIcon - leftPos) * (getHeight()-2), 0,
                getHeight()-1, getHeight()-1 );
  g->drawRect( &selRect );
  Rect selRecti( (selectedIcon - leftPos) * (getHeight()-2) + 1, 1,
                 getHeight()-3, getHeight()-3 );
  g->drawRect( &selRecti );
}

void ToolbarDialog::IconWindow::selectIcon( char *filename )
{
  for( int i = 0; i < icons.getSize(); i++ )
    if( !stricmp( icons.getObjectAt(i)->getFileName(), filename ) )
    {
        selectedIcon = i;
      leftPos = i - getIconsInWindow() / 2;
      if( leftPos < 0 ) leftPos = 0;
      repaint( NULL );
      break;
    }
}

void ToolbarDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == commandButton )
  {
    commandCombo->enable( TRUE );
    descriptionField->enable( TRUE );
    iconWindow->enable( TRUE );
    scrollbar->enable( TRUE );
    cLabel->enable( TRUE );
    dLabel->enable( TRUE );
    buttonType = COMMAND;
  }
  else
  if( buttonEvent->getSource() == spaceButton )
  {
    commandCombo->enable( FALSE );
    descriptionField->enable( FALSE );
    iconWindow->enable( FALSE );
    scrollbar->enable( FALSE );
    cLabel->enable( FALSE );
    dLabel->enable( FALSE );
    buttonType = SPACE;
  }
  else
    closeDialog( buttonEvent->getSource() == OKbutton );
}

DockedErrorCollector::DockedErrorCollector( Window *parent, InfoBar *infobar ) : EditControl( parent )
{
  TextBuffer *textBuffer = BufferAdmin::queryBufferWithFilename( KErrorCollector::getBuffername() );

  if( textBuffer == NULL )
    textBuffer = new TextBuffer( KErrorCollector::getBuffername() );

  ControlAdmin::insertControl( (MainWindow *)parent, this );

  startControl(textBuffer, NULL, infobar, 0, 0, 200, 100, TRUE );
  
  textBuffer->addBufferObserver( this );
}

void DockedErrorCollector::insertion( TextBuffer *textBuffer, int reader, int position,
                                      int size, int lines )
{
  char cmd[100];
  sprintf( cmd, "jumpToPosition( %d );", position + size ); 
  addCommand( cmd );
}

void MainWindow::waitFocus() 
{
#if !defined __OS2__
	focusWait.request(); 
#endif
}

void MainWindow::releaseFocus() 
{ 
#if !defined __OS2__
	focusWait.release(); 
#endif
}

void MainWindow::actionPerformed( AMenuEvent *menuEvent )
{
  if( menuEvent->getSource() == this &&
      menuEvent->getId() != MENU_CLOSE )
  {
    ((EditControl *)notebook->getActiveWindow())->addCommand(
        MainMenu::getCommandFromId( menuEvent->getId() ) );
  }
  else
  if( menuEvent->getSource() == toolbar )
  {
    switch( menuEvent->getId() )
    {
      case 101 :
      {
        ToolbarDialog *toolbarDialog =
          new ToolbarDialog( Desktop::getInstance(), this,
                             ToolbarDialog::NEW, Strings::get(1), "", "", "" );
        if( toolbarDialog->setVisible( TRUE ) )
        {
          // Add the new tool
          if( toolbarDialog->getButtonType() == ToolbarDialog::SPACE )
            toolbar->addSpace();
          else
          {
            if( !toolbar->addImageButton( toolbarDialog->getSelectedIcon(),
                                          toolbarDialog->getCommand(),
                                          toolbarDialog->getDescription(), -1 ) )
            {
              PopupMessage popupMessage( Desktop::getInstance(),
                                         Strings::get(55), Strings::get(56),
                                         PopupMessage::error );
            }
						else
						{
							toolbarModified = TRUE;
							reReadToolbar();
						}
          }
        }

        delete toolbarDialog;
        break;
      }
      case 102 :
        tbSize = (toolbar->getHeight() == buttonWidth + Toolbar::getInset()*2 ) ? 2 : 1;

        toolbar->setSize( toolbar->getWidth(), buttonWidth * tbSize + Toolbar::getInset()*2 );
        // replace components in framewindow
        size( 0, 0 );
        break;
    }
    // remove listener
    toolbar->removeMenuListener( this );
  }
}

MainWindow::MainWindow( Window *parent, char *title, int x, int y, int width, int height ) :
FrameWindow( parent, title, x, y, width, height ), tempAutoClose(FALSE), executingFocus( FALSE )
{
  toolbar = new Toolbar( this, "Main toolbar", 0, 0, 10, buttonWidth * tbSize + Toolbar::getInset()*2 );

  char filename[MAXPATH+1];

  int i;
	for( i = 0; i < toolButtonInfo.getSize(); i++ )
	{
		ToolButtonInfo *tbInfo = (ToolButtonInfo *)toolButtonInfo.getObjectAt(i);

		if( tbInfo->type == Toolbar::IMAGE )
		{
		  sprintf( filename, "%s%s", KonSystem::bmpDir, tbInfo->bitmap );
			toolbar->addImageButton( filename, tbInfo->command, tbInfo->description, tbInfo->id );
		}
		else
			toolbar->addSpace();
	}

  getGraphics()->setSystemFont();

  int infoBarHeight = InfoBar::getInset() + getGraphics()->getMaxFontHeight();

  infoBar = new InfoBar( this, 0, 0, 10, infoBarHeight );

  notebook = new Notebook( this, 0, 150, 10, 10, Notebook::ALLOWCLOSE );

  combo = new SComboBox( this, 0, 0, 40, 100, 100, 120 );
  combo->setMaxLength( 200 );
  combo->addKeyListener( this );

  for( i = 0; i < commands; i++ )
    combo->addItem( command[i] );

  separator = new HSeparator( this, 0, separatorRestoreY, getWidth(), HSeparator::defaultHeight );
  separator->setVisible( TRUE );
  separator->setStoredY( separatorRestoreY  );

  if( separatorMinimized )
    separator->sMinimize(); 

  errorCollector = new DockedErrorCollector( this, infoBar );
  errorCollector->setVisible( TRUE );

  addMenu( MainMenu::getMenu() );

  addMenuListener( this );
  MainWindowAdmin::addWindow( this );
  toolbar->addToolbarListener( this );
  toolbar->addToolStateServer( this );

  int xx = getX(), yy = getY(), wwidth = getWidth(), hheight = getHeight();
  WinProperties::retrieveProperty( "MainWindow", xx, yy, wwidth, hheight );

  // Check if the size and pos is within screen boundaries

  if( xx < 0 ) xx = 0;

  if( xx + wwidth > Desktop::getInstance()->getWidth() )
  {
    int newWidth = Desktop::getInstance()->getWidth() - xx;

    if( newWidth < wwidth / 2 )
    {
      if( wwidth < Desktop::getInstance()->getWidth() )
        xx = Desktop::getInstance()->getWidth() - wwidth;
      else
      {
        wwidth = Desktop::getInstance()->getWidth();
        xx = 0;
      }
    }
    else
      wwidth = newWidth;
  }

  if( yy < 0 ) yy = 0;

  if( yy + hheight > Desktop::getInstance()->getHeight() )
  {
    int newHeight = Desktop::getInstance()->getHeight() - yy;

    if( newHeight < hheight / 2 )
    {
      if( hheight < Desktop::getInstance()->getHeight() )
        yy = Desktop::getInstance()->getHeight() - hheight;
      else
      {
        hheight = Desktop::getInstance()->getHeight();
        yy = 0;
      }
    }
    else
      hheight = newHeight;
  }

  setPos( xx, yy );
  setSize( wwidth, hheight );

  notebook->setVisible( TRUE );
  setFocus();
  if( Settings::showCommandBar )
    combo->setVisible( TRUE );
  else
    combo->setVisible( FALSE );

  toolbar->addMouseListener( this );
  
	toolbar->setVisible(Settings::showToolbar);
		
  infoBar->setVisible( TRUE );
  notebook->setSelectionListener( this );
  notebook->setCloseListener( this );

  adjustSeparatorPos( separator->getY() );

  separator->addMoveListener( this );
  addMouseListener( this );
  setPointer( PTR_HAND );
}

int MainWindow::execFromMainThread( int task, void *param )
{
  switch( task )
  {
    case 0 :
    {
      // The menu must be updated from the main thread

      MainMenu::updateWindowMenu();
      break;
    }
    case 1 :
    {
      updateMenu();
      break;
    }
    case 2 :
    {
      checkMenuItem( ((TwoInts *)param)->i1, ((TwoInts *)param)->i2 );
      break;
    }
    case 3 :
    {
      enableMenuItem( ((TwoInts *)param)->i1, ((TwoInts *)param)->i2 );
      break;
    }
		case 4 :
		{
			PrefsDialog *prefsDialog = new PrefsDialog( Desktop::getInstance(), this );
			BOOL res = prefsDialog->setVisible(TRUE);
			delete prefsDialog;
			if( res )
				MainWindowAdmin::updateAllWindows( MainWindowAdmin::FRAME | MainWindowAdmin::ACTIVECONTROL );
			break;
		}
		case 5 :
		{
			ToolButtonInfo *tbInfo = (ToolButtonInfo *)param;

			char filename[MAXPATH+1];

		  sprintf( filename, "%s%s", KonSystem::bmpDir, tbInfo->bitmap );

			toolbar->addImageButton( filename, tbInfo->command, tbInfo->description, tbInfo->id );
			break;
		}
		case 6 :
		{
			ToolButtonState *tbState = (ToolButtonState *)param;

			toolbar->setButtonActive( tbState->id, tbState->active );
			break;
		}
		case 7 :
		{
			MessageBoxInfo *mbInfo = (MessageBoxInfo *)param;
			PopupMessage mbox( this, mbInfo->title, mbInfo->message, mbInfo->type );
			return mbox.getResult();
		}
/*		case 8 :
		{
			LoadAndJumpInfo *lji = (LoadAndJumpInfo *)param;

			// Check if the file is already loaded

			ControlInfo *control = ControlAdmin::queryControlWithFilename( lji->filename );

			if( control == NULL )
			{
				// Load the file
				addFile( lji->filename, FALSE, TRUE );
				control = ControlAdmin::queryControlWithFilename( lji->filename );
			}

			// Jump to the correct position
  
			MainWindow *parent = control->parent;
			EditControl *editControl = control->editControl;

			parent->setFocus();  // Will destroy the controlInfo control
			parent->selectControl( control->editControl );
			editControl->getView()->jumpToLine( lji->line );
			editControl->getView()->cursorToColumn( lji->column );
			editControl->screenToCursor();
		}*/
  }
  return 0;
}

void MainWindow::showBufferModified( EditControl *control, BOOL isModified )
{
	notebook->setTabTextColor( notebook->getWindowTab( control ),
  													 (isModified) ? Color::RED : Color::BLACK );
	if( control == notebook->getActiveWindow() )
  	infoBar->setModified( isModified );
}

void MainWindow::filenameUpdated( EditControl *control )
{
	char *nbName;

  if( Settings::stripNameInNotebook )
    nbName = strrchr( control->getTextBuffer()->getFilename(), '\\' ) + 1;
  else
    nbName = control->getTextBuffer()->getFilename();

	notebook->setTabTitle( notebook->getWindowTab( control ), nbName );
}

void MainWindow::selectControl( EditControl *control )
{
	notebook->setTab( control );
}

void MainWindow::closeControl( EditControl *control )
{
  notebook->remove( control );

  ControlAdmin::removeControl( ControlAdmin::getControlIndex( control ) ) ;

  if( control != NULL )
    control->closeControl();
}

BOOL MainWindow::closeControl( EditControl *control, BOOL queryOnModified )
{
  if( queryOnModified )
  {
    if( control->textModified() )
    {
      notebook->setTab( control );
      char text[1000];
      sprintf( text, Strings::get(97), control->getTextBuffer()->getFilename() );
      PopupMessage querySave( this, Strings::get(96), text,
                              PopupMessage::query );
      if( querySave.getResult() == PopupMessage::cancel ) return FALSE;
      if( querySave.getResult() == PopupMessage::yes ) 
			{
			  if( BufferAdmin::isScratchName( control->getTextBuffer()->getFilename() ) )
				{
					// Show Save as dialog..

					FileDialogInfo fdInfo;
					fdInfo.filename = "";
					fdInfo.title = Strings::get(197);

					if( Application::executeFromMainThread( control, 12, &fdInfo ) )
					{
						control->getTextBuffer()->setFilename( FileDialog::getFilename() );
						int result = control->saveFile();
						if( result != 0 )
						{
							PopupMessage( this, FileError::getPlainTextMessage(result), 
														Strings::get(159), PopupMessage::error );
							return FALSE;
						}
					}
				}
				else
				{
					int result = control->saveFile();

					if( result != 0 )
					{
						PopupMessage( this, FileError::getPlainTextMessage(result), 
													Strings::get(159), PopupMessage::error );
						return FALSE;
					}
				}
			}
    }
  }

  closeControl( control );

  // Update menu
  Application::executeFromMainThread( this, 0, NULL );

  // Close this (main) window if no more controls in it, or open a blank buffer

  if( notebook->getTabs() == 0 )
  {
    if( Settings::autoCloseFrameWindow || tempAutoClose )
    {
      // Try to remove this window if it was a MainWindow (cast may not be true,
      // but it doesn't matter)
      MainWindowAdmin::removeWindow( this );
      // This was the last MainWindow, exit
      deleteMe();
      if( MainWindowAdmin::getWindows() == 0 ) Application::terminate();

    }
    else
    {
      char filename[MAXPATH];
       // Open up a blank scratch buffer
			BufferAdmin::getNextScratchName( filename );
      ControllerWindow::getInstance()->loadFile( this, filename, TRUE, TRUE, Settings::reuseExistingBuffer );
//      addFile( filename, TRUE, TRUE );
    }
  }

  return TRUE;
}

void MainWindow::closeActiveWindow()
{
  EditControl *control = (EditControl *)notebook->getActiveWindow();
  closeControl( control, TRUE );
}

EditControl *MainWindow::addFile( char *filename, BOOL threadedLoading, BOOL killScratchImmediately  )
{
  char fullPath[MAXPATH+1], *nbName;
  BOOL removeScratch = FALSE;

  // Get the full path for the filename

  if( BufferAdmin::isScratchName( filename ) )
  {
    strcpy( fullPath, filename );
    nbName = fullPath;
  }
  else
  {
    System::getFullPath( filename, fullPath, MAXPATH );

    if( Settings::stripNameInNotebook )
      nbName = strrchr( fullPath, '\\' ) + 1;
    else
      nbName = fullPath;

    removeScratch = (notebook->getTabs() == 1);
  }

  // Check if the file has already been loaded. If so, use that textbuffer

  TextBuffer *textBuffer = BufferAdmin::queryBufferWithFilename( fullPath );

  EditControl *cntrlWin;

  if( textBuffer == NULL )
  {
    // New copy
    textBuffer = new TextBuffer( fullPath );
    BufferLoader *bufferLoader = new BufferLoader( textBuffer, fullPath,
                                                   infoBar, FALSE, this );
    cntrlWin = new EditControl( notebook, textBuffer,
                                SyntaxContainer::retrieveSyntax( fullPath ),
                                infoBar, 10, 10, 100, 100 );
    notebook->add( cntrlWin, nbName );

    if( threadedLoading )
      bufferLoader->start( 16384 );
    else
    {
      bufferLoader->run();
      delete bufferLoader;
    }
  }
  else
  {
    // A copy already existed. Use that one.
    cntrlWin = new EditControl( notebook, textBuffer,
                                SyntaxContainer::retrieveSyntax( fullPath ),
                                infoBar, 10, 10, 100, 100 );
    notebook->add( cntrlWin, nbName );
  }
  ControlAdmin::insertControl( this, cntrlWin );

  // Check if there is an only unmodifed scratch buffer in the window.
  // If so, destroy it.

  if( removeScratch )
  {
		EditControl *eControl = (EditControl *)notebook->getWindow( 0 );

		if( BufferAdmin::isScratchName( eControl->getTextBuffer()->getFilename() ) &&
				!eControl->getTextBuffer()->textModified() )
		{
			if( killScratchImmediately )
				closeControl( eControl );
			else
				ControlAdmin::addToExitList( eControl, FALSE );
		}
  }

  cntrlWin->setFocus();

  // Add the file to the recent files list

  MainMenu::addNewFileToRecent( fullPath );

  // Run buildMainMenu()

	KCommonCommands::getInstance()->runCommand( "buildMainMenu();" );

  // Update menu
  MainMenu::updateWindowMenu();

	return cntrlWin;
}

void MainWindow::size( int oldWidth, int oldHeight )
{
  int newWidth = getClientWidth(), newHeight = getClientHeight();

  if( newWidth == 0 && newHeight == 0 ) return; // Minimized

  // Set toolbar position and size
  int toolbarY = newHeight - toolbar->getHeight();
  
  toolbar->setPos( 0, toolbarY );
  toolbar->setSize( newWidth, toolbar->getHeight() );

  if( !Settings::showToolbar)
  {
		toolbarY = newHeight;
		toolbar->setVisible( FALSE );
	}
	else
	{
		toolbar->setVisible( TRUE );
	}
  
  // Set infobar width
  infoBar->setSize( newWidth - cmdButtonWidth, infoBar->getHeight() );
  infoBar->setPos( 0, 0 );

  // Set comboBox width and ypos
  int comboboxY = infoBar->getHeight(), comboboxHeight = 0;

  if( Settings::showCommandBar )
  {
    comboboxHeight = combo->getEditBoxHeight();
    combo->setSize( newWidth, 150 );
    combo->setPos( 0, comboboxY );
    combo->setVisible(TRUE);
  }
  else
    combo->setVisible(FALSE);
    
  // Set separator pos and size
  separator->setSize( newWidth, HSeparator::defaultHeight );
  separator->setPos( 0, separator->getY() );
  adjustSeparatorPos( separator->getY() );

  separatorMinimized = (separator->sIsMinimized());

  if( separatorMinimized )
    separatorRestoreY = separator->getStoredY();

  // Set ErrorCollector pos and size
  int notebookY = separator->getY() + separator->getHeight();
  int errorColY = comboboxY + comboboxHeight;
  int colHeight = notebookY - errorColY - separator->getHeight();
  if( colHeight <= 0 ) colHeight = 1;
  errorCollector->setSize( newWidth, colHeight );
  errorCollector->setPos( 0, errorColY );

  // Set notebook pos and size
  notebook->setSize( getClientWidth(), toolbarY - notebookY );
  notebook->setPos( 0, notebookY );
  
  Rect cmdBtnRect(infoBar->getWidth(), infoBar->getY(), infoBar->getHeight(), cmdButtonWidth );

  repaint( NULL );
  repaint( &cmdBtnRect );
	notebook->repaint( NULL );
}

void MainWindow::updateCaptionText()
{
	char text[350];

  EditControl *currentControl = (EditControl *)notebook->getActiveWindow();

	if( currentControl != NULL )
  {
    sprintf( text, "Kon - %s", currentControl->getTextBuffer()->getFilename() );
    setCaptionText( text );  	
  }
}

void MainWindow::actionPerformed( AButtonEvent *buttonEvent )
{
	closeControl( (EditControl *)notebook->getWindow( buttonEvent->getId() ), 
	              TRUE );
}

void MainWindow::actionPerformed( ASelectionEvent *selectionEvent )
{
  char text[350];

  updateCaptionText();

  // Update menu

  EditControl *currentControl = (EditControl *)notebook->getActiveWindow();

  if( currentControl != NULL )
  {
	  sprintf( text, "undoAvailable( %d );", currentControl->getTextBuffer()->
                                         getUndoBuffer()->canUndo() );
  	currentControl->addCommand( text );

    sprintf( text, "redoAvailable( %d );", currentControl->getTextBuffer()->
                                         getUndoBuffer()->canRedo() );
    currentControl->addCommand( text );

    sprintf( text, "blockMarked( %d );", currentControl->viewHasValidMark() );
    currentControl->addCommand( text );

    sprintf( text, "setMenuChecks();");
    currentControl->addCommand( text );

    sprintf( text, "enableToolbarItems();");
    currentControl->addCommand( text );
  }
}

void MainWindow::actionPerformed( AWinMoveEvent *moveEvent )
{
  static BOOL moving = FALSE;
  
  if( !moving )
  {
    moving = TRUE;
    int lowerY = infoBar->getHeight();
    
    separatorMinimized = (separator->sIsMinimized());

    if( Settings::showCommandBar )
      lowerY += combo->getEditBoxHeight();
      
    if( moveEvent->getNewY() + separator->getHeight() > getClientHeight() - toolbar->getHeight() )
      separator->setPos( separator->getX(), getClientHeight() - toolbar->getHeight() - separator->getHeight() );

    if( moveEvent->getNewY() <  lowerY )
      separator->setPos( separator->getX(), lowerY );
      
    size( getX(), getY() );
    moving = FALSE;
  }
}

BOOL MainWindow::actionPerformed( AMouseEvent *mouseEvent )
{
  static BOOL buttonDown = FALSE;

  if( mouseEvent->getSource() == toolbar &&
      mouseEvent->getEvent() == AMouseEvent::CLICK &&
      mouseEvent->getButton() == AMouseEvent::RIGHT )
  {
    // Add a popupmenu
    Point *pos = mouseEvent->getPos();
    AMenuItem *add = new AMenuItem( Strings::get(1), 101, AMenuEntry::MENUITEM );
    AMenuItem *toggleSize = new AMenuItem( Strings::get(2), 102,
                                           AMenuEntry::MENUITEM );
    AMenu *menu = new AMenu( "Toolbar", 100 );
    menu->addMenuItem( add );
    menu->addMenuItem( toggleSize );
    toolbar->addMenuListener( this );
    toolbar->popupMenu( menu, pos->x, pos->y );
    delete menu;
  }
  else
  if( mouseEvent->getSource() == this )
  {
		if( (mouseEvent->getEvent() == AMouseEvent::DOWN ||
				 mouseEvent->getEvent() == AMouseEvent::DOUBLECLICK) && !buttonDown )
		{
      Point *pos = mouseEvent->getPos();
      if( pos->x > infoBar->getWidth() + infoBar->getX() && pos->y < infoBar->getHeight() + infoBar->getY() )
			{
				Settings::showCommandBar = !Settings::showCommandBar;
				buttonDown = TRUE;
        size(-1, -1);
        if( Settings::showCommandBar )
          combo->setFocus();
        else
        if( notebook->getTabs() > 0 )
          notebook->getActiveWindow()->activate( TRUE );
			}
		}

		if( mouseEvent->getEvent() == AMouseEvent::RELEASE )
			buttonDown = FALSE;
  }
  return TRUE;
}

void MainWindow::nextTab()
{
  notebook->nextTab();
}

void MainWindow::prevTab()
{
  notebook->prevTab();
}

int MainWindow::focus( BOOL newFocus )
{
  if( newFocus )
  {
    if( executingFocus ) return 1;
    waitFocus();

    executingFocus = TRUE;

    // Loop through all buffers (globally, not only in this window) and 
    // reload if file has been modified on disk by another program/user

    for( int i = 0; i < BufferAdmin::getBuffersCount(); i++ )
    {
      // Check if the file is modified on disk

      BTime newTime;

      if( System::getFileTime(BufferAdmin::getBuffer(i)->getFilename(), &newTime ) )
      {
        BTime oldTime;
        BufferAdmin::getBuffer(i)->getTimeWritten( &oldTime );

        if( memcmp( &oldTime, &newTime, sizeof( BTime )) )
        {
          // Query user
          PopupMessage *popup;
          char str[500];

          if( !BufferAdmin::getBuffer(i)->textModified() )
          {
            sprintf( str, Strings::get(218), BufferAdmin::getBuffer(i)->getFilename() );
  
            popup = new PopupMessage( this, Strings::get(217), str, PopupMessage::query );
          }
          else
          {
            sprintf( str, Strings::get(219), BufferAdmin::getBuffer(i)->getFilename() );

            popup = new PopupMessage( this, Strings::get(217), str, PopupMessage::querywithwarning );
          }

          int answer = popup->getResult();

          delete popup;

          if( answer == PopupMessage::yes )
          {
            BufferLoader *bufferLoader = 
              new BufferLoader( BufferAdmin::getBuffer(i), 
                                BufferAdmin::getBuffer(i)->getFilename(), infoBar, TRUE, this );
            bufferLoader->start( 16384 );
          }
          else
          if( answer == PopupMessage::cancel ) break; // No more checks
        }
      }
    }
    executingFocus = FALSE;

    releaseFocus();

    MainWindowAdmin::setActiveWindow( this );
    if( notebook->getTabs() > 0 )
      notebook->getActiveWindow()->activate( TRUE );

    return 1;
  }
  return 0;
}

BOOL MainWindow::close()
{
  // Check if all windows are OK to close

  tempAutoClose = TRUE;

  int i = 0;

  while( i < notebook->getTabs() )
  {
    if( !closeControl( (EditControl *)notebook->getWindow( i ), TRUE ) )
      return FALSE;
    else
      i = -1;
    i++;
  }

  tempAutoClose = FALSE;

  return FALSE;
}

void MainWindow::reReadToolbar()
{
  int i;
	for( i = 0; i < toolButtonInfo.getSize(); i++ )
	{
		if( toolButtonInfo.getObjectAt(i)->type != Toolbar::SPACE )
		{
			if( toolButtonInfo.getObjectAt(i)->command != NULL )
				delete toolButtonInfo.getObjectAt(i)->command;
			if( toolButtonInfo.getObjectAt(i)->description != NULL )
				delete toolButtonInfo.getObjectAt(i)->description;
			if( toolButtonInfo.getObjectAt(i)->bitmap != NULL )
				delete toolButtonInfo.getObjectAt(i)->bitmap;
		}
	}

	toolButtonInfo.removeAll();

  ToolButtonInfo *tbInfo;

	for( i = 0; i < toolbar->getToolsCount(); i++ )
	{
		char *command, *description, *bitmap;
		int id, type;

		tbInfo = new ToolButtonInfo;
	
		toolbar->getToolAt( i, type, &command, &bitmap, &description, id );

		tbInfo->id = id;
		if( type == Toolbar::IMAGE )
		{
			tbInfo->command = new char[strlen(command)+1];
			strcpy( tbInfo->command, command );
			tbInfo->description = new char[strlen(description)+1];
			strcpy( tbInfo->description, description );

			// Remove path

			char *bmp = strrchr( bitmap, '\\' );

			if( bmp == NULL )
				bmp = bitmap;
			else
				bmp++;

			tbInfo->bitmap = new char[strlen(bmp)+1];
			strcpy( tbInfo->bitmap, bmp );
			tbInfo->type = Toolbar::IMAGE;
		}
		else
			tbInfo->type = Toolbar::SPACE;
		toolButtonInfo.add( tbInfo );
	}
}

BOOL MainWindow::modifyButton( char **filename, char **command, char **description )
{
  // Only commandbuttons can be modified
  ToolbarDialog *toolbarDialog =
    new ToolbarDialog( Desktop::getInstance(), this,
                       ToolbarDialog::COMMAND, Strings::get(11),
                       *filename, *command, *description );

  BOOL result = toolbarDialog->setVisible( TRUE );
  if( result )
  {
    // Save the new result
    *filename = new char[strlen( toolbarDialog->getSelectedIcon())+1];
    strcpy( *filename, toolbarDialog->getSelectedIcon() );
    *command = new char[strlen( toolbarDialog->getCommand())+1];
    strcpy( *command, toolbarDialog->getCommand() );
    *description = new char[strlen( toolbarDialog->getDescription())+1];
    strcpy( *description, toolbarDialog->getDescription() );
    toolbarModified = TRUE;
		reReadToolbar();
  }

  delete toolbarDialog;
  return result;
}

void MainWindow::buttonDeleted( int id )
{
	toolbarModified = TRUE;
	reReadToolbar();
}

void MainWindow::buttonMoved()
{
	toolbarModified = TRUE;
	reReadToolbar();
}

void MainWindow::saveSettings( FILE *f )
{
	// Save the toolbar

	if( toolbarModified )
	{
		char filename[MAXPATH];

		sprintf( filename, "%s%s", KonSystem::kDir, "toolbar.k" );

		FILE *o = fopen( filename, "w" );

		fprintf( o, "%s\n", Strings::get(175) );
		fprintf( o, "%s\n", Strings::get(176) );
		fprintf( o, "%s\n", Strings::get(177) );
		fprintf( o, "%s\n\n", Strings::get(178) );

		fprintf( o, "setupToolbar()\n{\n" );

		for( int i = 0; i < toolButtonInfo.getSize(); i++ )
		{
			ToolButtonInfo *tbInfo = (ToolButtonInfo *)toolButtonInfo.getObjectAt(i);

			if( tbInfo->type == Toolbar::IMAGE )
				fprintf( o, "  addToolButton( \"%s\", \"%s\", \"%s\", %d );\n",
								 tbInfo->command, tbInfo->bitmap, tbInfo->description, tbInfo->id );
			else
				fprintf( o, "  addToolSpace();\n" );
		}

		fprintf( o, "}\n" );

		fclose( o );
	}

	long pos;
	if( !ClassProperties::savePropertyName( f, "MAINWINDOW003", &pos ) ) return;
	fwrite( &tbSize, sizeof( int ), 1, f );
  fwrite( &commands, sizeof( int ), 1, f );

  for( int i = 0; i < commands; i++ )
  {
  	int len = strlen( command[i] );
  	fwrite( &len, sizeof( int ), 1, f );
  	fwrite( command[i], len+1, 1, f );
  }

  fwrite( &separatorRestoreY, sizeof( separatorRestoreY ), 1, f );
  fwrite( &separatorMinimized, sizeof( separatorMinimized ), 1, f );
}

void MainWindow::restoreSettings( FILE * f )
{
	if( !ClassProperties::checkPropertyExists( f, "MAINWINDOW003" ) ) return;
	fread( &tbSize, sizeof( int ), 1, f );
  fread( &commands, sizeof( int ), 1, f );

  for( int i = 0; i < commands; i++ )
  {
  	int len;
    fread( &len, sizeof( int ), 1, f );
    command[i] = (char *)malloc( len + 1 );
  	fread( command[i], len + 1, 1, f );
  }

  fread( &separatorRestoreY, sizeof( separatorRestoreY ), 1, f );
  fread( &separatorMinimized, sizeof( separatorMinimized ), 1, f );
}

MainWindow::~MainWindow()
{
  removeMenuListener( this );
  separator->removeMoveListener( this );
	if( !isMinimized() && !isMaximized() )
	  WinProperties::updateProperty( "MainWindow", getX(), getY(),
  	                               getWidth(), getHeight() );
}

BOOL MainWindow::actionPerformed( AKeyEvent *keyEvent )
{
  if( keyEvent->key == KeyDef::ENTER || keyEvent->key == KeyDef::RETURN )
  {
    char *text = combo->getEditText();

    // Check if this command is not the same as the last one, if so add
    char lastCmd[256];
    BOOL add = TRUE;

    if( combo->getItems() > 0 )
    {
      combo->getItem( combo->getItems()-1, 255, lastCmd );
      add = strcmp( lastCmd, text );
    }

    if( add )
    {
      combo->addItem( text );
      combo->selectItem( combo->getItems() - 1 );
    }

    if( commands == 0 || strcmp( text, command[0] ) )
    {
      if( commands > 0 )
      	memmove( &command[1], &command[0], sizeof( *command ) * commands );

      if( commands == MAXCOMMANDS-1 )
        delete command[commands-1];
      else
	      commands++;

      command[0] = new char[strlen(text)+1];
      strcpy( command[0], text );
    }
    
    ((EditControl *)notebook->getActiveWindow())->addCommand( text );
    return TRUE;
  }
  else
  if( keyEvent->key == KeyDef::TAB )
  {
    if( notebook->getTabs() > 0 )
      notebook->getActiveWindow()->activate( TRUE );
  }
  return FALSE;
}

void MainWindow::focusCommandBar()
{
  if( !Settings::showCommandBar )
  {
    Settings::showCommandBar = TRUE;
    size( -1, -1 );
  }
  combo->setFocus();
}

void MainWindow::adjustSeparatorPos( int yNow )
{
  int lowerY = infoBar->getHeight();

  if( Settings::showCommandBar )
    lowerY += combo->getEditBoxHeight();
    
  if( yNow + separator->getHeight() > getClientHeight() - toolbar->getHeight() )
    separator->setPos( separator->getX(), getClientHeight() - toolbar->getHeight() - separator->getHeight() );

  if( yNow < lowerY || separator->sIsMinimized() )
    separator->setPos( separator->getX(), lowerY );

}

void MainWindow::paint( Rect *rect, Graphics *g )
{
  g->setSystemFont();

  char *btnString = "K";
  Point points[10];

  g->calcStringCharPos( 0, 0, strlen( btnString ), btnString, points );

  int xPos = getClientWidth() - cmdButtonWidth + (cmdButtonWidth - points[1].x) / 2;
  int btnX = getClientWidth() - cmdButtonWidth;

  int baseline = g->getFontBaseline();
  g->drawButton( btnX, 0, cmdButtonWidth, infoBar->getHeight(), !Settings::showCommandBar );

  g->setColor( Color::BLACK );
  g->drawString( xPos, baseline + (infoBar->getHeight() - g->getMaxFontHeight()) / 2, btnString );
}
