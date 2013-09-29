#include "bowl.h"
#include "editctrl.h"
#include "mainwnd.h"
#include "kparse.h"
#include "repldlg.h"
#include "filedlg.h"
#include "printdlg.h"
#include "multfile.h"
#include "print.h"
#include "comdlg.h"
#include "bytesinfo.h"

int EditControl::idCounter = 0;
SearchDialog *EditControl::searchDialog = NULL;
ReplaceDialog *EditControl::replaceDialog = NULL;

void EditControl::startControl( void )
{
  BufferAdmin::addBufferHolder( this, textBuffer );
  id = idCounter++;
  char filename[255];
  sprintf( filename, "%s%s", KonSystem::bmpDir, "BACKGROUND.BMP" );
  backgroundBitmap = bmpGraphics.readBitmap( filename );
  south = new ViewScrollbar( this, 0, 0, 0, width - Scrollbar::getInset(1),
                             Scrollbar::getInset(0) );
  east = new ViewScrollbar( this, 0, width - Scrollbar::getInset(1),
                            Scrollbar::getInset(0),
                            Scrollbar::getInset(1),
                            height - Scrollbar::getInset(0) );

  viewWindow = new ViewWindow( this, this, infoBar, 0, Scrollbar::getInset(0),
                               width - Scrollbar::getInset(1),
                               height - Scrollbar::getInset(0) );

  bytesInfo = new BytesInfo( this, 0, 0, 1, 1 );
  sectionBrowser = new SectionBrowser( this, 0, 0, 20, 20 );

  // The commandqueue deletes itself on thread exit, but the edit control
  // should call close first
  commandQueue = new CommandQueue( this, this );
  commandQueue->start( 65535 );

  textBuffer->addLoadObserver( this );
  viewWindow->addMenuListener( this );
  viewWindow->addPositionListener( this );
}

EditControl::EditControl( Window *parent, TextBuffer *textBuffer,
             Syntax *syntax, InfoBar *infoBar,
             int x, int y, int width, int height ) :
  Window( parent, x, y, width, height ), viewInitialized(FALSE),
  scrollbarHandleDown(FALSE), isActive( FALSE ), parent(parent), 
  textBuffer(textBuffer), syntax(syntax), infoBar(infoBar), 
  isSimpleControl( FALSE ), currentView(NULL)
{
	startControl();
}

EditControl::EditControl( Window *parent, TextBuffer *textBuffer,
             Syntax *syntax, InfoBar *infoBar,
             int x, int y, int width, int height, BOOL isSimple ) :
  Window( parent, x, y, width, height ), viewInitialized(FALSE),
  scrollbarHandleDown(FALSE), isActive( FALSE ), parent(parent), 
  textBuffer(textBuffer), syntax(syntax), infoBar(infoBar), 
  isSimpleControl(isSimple), currentView(NULL)
{
	startControl();
}

void EditControl::startControl( TextBuffer *textBuffer,
             Syntax *syntax, InfoBar *infoBar,
             int x, int y, int width, int height, BOOL isSimple )
{
  this->textBuffer = textBuffer;
  this->syntax = syntax;
  this->infoBar = infoBar; 
  this->isSimpleControl = isSimple;
	startControl();
}

void EditControl::paint( Rect *rect, Graphics *g )
{
  if( viewInitialized )
  {
		g->setBackColor( Color::BKGR );
		g->fillRect( rect );
    return;
  }

  // Tile the bitmap

  int xs = getWidth() / backgroundBitmap->getWidth() + 1,
      ys = getHeight() / backgroundBitmap->getHeight() + 1;

  for( int y = 0; y < ys; y++ )
    for( int x = 0; x < xs; x++ )
      g->drawBitmap( backgroundBitmap, Graphics::ACTIVE,
                     x*backgroundBitmap->getWidth(),
                     y*backgroundBitmap->getHeight(),
                     backgroundBitmap->getWidth(),
                     backgroundBitmap->getHeight() );
}

void EditControl::setCursorVisible()
{
  viewWindow->screenToCursor();

  // Update scrollbars

  viewWindow->updateScrollbars();
}

BOOL EditControl::actionPerformed( AKeyEvent *keyEvent )
{
  if( keyEvent->key == 0 ) return FALSE;

  char *result = KeyBindings::getInstance()->lookupCommand(
                                              keyEvent->modifiers,
                                              keyEvent->key );

  // ESC is hardcoded for breaking commands!

  if( keyEvent->key == KeyDef::ESCAPE )
  {
    commandQueue->terminateCommand();
    MultipleFiles::terminateFileOperation();
    return TRUE;
  }
  else
  if( result == NULL &&
  		!(keyEvent->modifiers & KeyDef::kALT) &&
      keyEvent->key != 27 )  // Insert key into buffer
  {
    char str[30];

    if( keyEvent->key == '"' )
      sprintf( str, "insertString( '%c' );", keyEvent->key );
    else
    if( keyEvent->key == '\\' )
      sprintf( str, "insertString( \"\\%c\" );", keyEvent->key );
    else
      sprintf( str, "insertString( \"%c\" );", keyEvent->key );
    addCommand( str );
    return TRUE;
  }
  else
  if( result != EMPTY && result != INBINDING && result != NULL )
  {
    addCommand( result );
    return TRUE;
  }

  return FALSE;
}

void EditControl::actionPerformed( AMenuEvent *menuEvent )
{
  if( menuEvent->getId() != MENU_CLOSE )
    addCommand( PopupMenu::getCommandFromId( menuEvent->getId() ) );
}

TSyntax *EditControl::eval( char *name, TVariableList *vars, TEval *eval, 
                            int index )
{
  TSyntax *retval = NULL;

  int oldScreenLine = viewWindow->getWindowStartLine(),
      oldWinLines = viewWindow->getWindowLines(),
      oldLines = currentView->getTextlines();
  BOOL moveToCursor = FALSE, prevInsertMode = Settings::insertMode;
  View *lastView = currentView;

  currentView->storeCursorPos();
  currentView->resetHighlight();

	switch( index )
	{
		case 45 :	// pageUp
			viewWindow->pageUp();
			break;
		case 44 : // pageDown
			viewWindow->pageDown();
			break;
		case 111 : // snapShot
			viewWindow->snapshot();
			break;
		case 50 : // nextTab
			MainWindowAdmin::nextTab( parent );
			break;
		case 51 : // nextTab
			MainWindowAdmin::prevTab( parent );
			break;
		case 60 : // resetViewWin
		{
			TSyntax **keepWindowStart = vars->LookUp( "keepWindowStart" );

			if( keepWindowStart == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*keepWindowStart)->Type != SNumeric )
				eval->SendError( Strings::get(51), name );
			else
      {
        if( !((TNumeric *)*keepWindowStart)->num )
			    currentView->resetView();

			  viewWindow->reset( TRUE, !((TNumeric *)*keepWindowStart)->num );
      }
			break;
		}
		case 61 : // searchDialog
			Application::executeFromMainThread( this, 0, NULL );
			break;
		case 62 : // replaceDialog
			Application::executeFromMainThread( this, 1, NULL );
			break;
		case 66 : // saveDialog
		{
			TSyntax **filename = vars->LookUp( "filename" ),
							**title = vars->LookUp( "title" ),
              **encoding = vars->LookUp( "encoding");

			if( filename == NULL || title == NULL || encoding == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*filename)->Type != SString || (*title)->Type != SString || (*encoding)->Type != SNumeric )
				eval->SendError( Strings::get(51), name );
			else
			{
				FileDialogInfo fdInfo;
				fdInfo.filename = ((TString *)*filename)->string;
				fdInfo.title = ((TString *)*title)->string;
        fdInfo.encType = (EncodingTypes)((TNumeric *)*encoding)->num;
        fdInfo.type = 0;

				if( Application::executeFromMainThread( this, 12, &fdInfo ) )
        {
          ((TNumeric *)*encoding)->num = SystemFileDialog::selectedEncoding;
					return new TString( fdInfo.filename, COPY );
        }
				else
					return new TString( "", COPY );
      }
			return NULL;
		}
		case 158 : // loadDialog
		{
			TSyntax **filename = vars->LookUp( "filename" ),
							**title = vars->LookUp( "title" );

			if( filename == NULL || title == NULL )
				eval->SendError( Strings::get(47), name );
			else
			{
				FileDialogInfo fdInfo;
				fdInfo.filename = ((TString *)*filename)->string;
				fdInfo.title = ((TString *)*title)->string;
        fdInfo.type = 1;

				if( Application::executeFromMainThread( this, 12, &fdInfo ) )
					return new TString( fdInfo.filename, COPY );
				else
					return new TString( "", COPY );
      }
			return NULL;
		}
		case 67 : // saveFile
		{
			// Check if this is the scratch buffer. If so, execute command
			// fileSaveDialog

			if( BufferAdmin::isScratchName( textBuffer->getFilename() ) )
			{
				runCommand( "fileSaveDialog();" );
			}
			else
			{
				int result = saveFile();

				if( result != 0 ) 
				{
					PopupMessage( this, FileError::getPlainTextMessage(result), 
												 Strings::get(159), PopupMessage::error );
				}
			}
			break;
		}
		case 90 : // saveFileAs
		{
			TSyntax **string = vars->LookUp( "filename" ), **encoding = vars->LookUp("encoding");
			if( string == NULL || encoding == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*string)->Type != SString || (*encoding)->Type != SNumeric )
				eval->SendError( Strings::get(51), name );
			else
			{
				char filename[MAXPATH+1], oldFilename[MAXPATH+1];
				System::getFullPath( ((TString *)*string)->string, filename, MAXPATH );
				strcpy( oldFilename, textBuffer->getFilename());
				textBuffer->setFilename( filename );
        textBuffer->setEncoding( (EncodingTypes)((TNumeric *)*encoding)->num );
				int result = saveFile();

				if( result != 0 ) 
				{
					PopupMessage( this, FileError::getPlainTextMessage(result), 
												 Strings::get(159), PopupMessage::error );
					textBuffer->setFilename( oldFilename );
				}
				else
				{
					Application::executeFromMainThread( this, 5, NULL );
					textBuffer->filenameUpdated();
				}
			}
			break;
		}
		case 80 : // nextView
		{
			int pos = currentView->getPosition();
			if( currentView == asciiView )
				currentView = hexView;
			else
				currentView = asciiView;
			viewWindow->setView( currentView, FALSE );
			viewWindow->reset( FALSE, TRUE );
      size( -1, -1 );
			currentView->jumpToPosition( pos );
			moveToCursor = TRUE;
			char cmd[100];
			sprintf( cmd, "viewChanged(%d);", (currentView == asciiView) ? 0 : 1 );
			addCommand( cmd );
			break;
		}
		case 94 : // setView
		{
			TSyntax **view = vars->LookUp( "view" );

			if( view == NULL )
			{
				eval->SendError( Strings::get(47), name );
			}
			else
			if( (*view)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			int pos = currentView->getPosition();

			if( ((TNumeric *)*view)->num == 1 )
				currentView = hexView;
			else
				currentView = asciiView;

			viewWindow->setView( currentView, FALSE );
			viewWindow->reset( FALSE, TRUE );
			currentView->jumpToPosition( pos );
			moveToCursor = TRUE;

			char cmd[100];
			sprintf( cmd, "viewChanged(%d);", (currentView == asciiView) ? 0 : 1 );
			addCommand( cmd );
			break;
		}
		case 81 : // queryView
		{
			if( currentView == asciiView )
				return new TNumeric( 0, COPY );
			else
				return new TNumeric( 1, COPY );
		}
		case 73 : // startMacroRecord
		{
			commandQueue->startRecord();
			if( infoBar != NULL )
				infoBar->setRecord( TRUE );
			return NULL;
		}
		case 74 : // endMacroRecord
		{
			commandQueue->endRecord();
			if( infoBar != NULL )
				infoBar->setRecord( FALSE );
			return NULL;
		}
		case 75 : // isRecording
			return new TNumeric( commandQueue->isRecording(), COPY );
		case 97 : // saveMacro
		{
			int success = 0;

			TSyntax **string = vars->LookUp( "filename" );
			if( string == NULL )
			{
				eval->SendError( Strings::get(47), name );
			}
			else
			if( (*string)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			else
			if( commandQueue->getMacroCode() )
			{
				FILE *f = fopen( ((TString *)*string)->string, "w" );
				if( f != NULL )
				{
					success = 1;
					char *str = "recordedMacro()\n{\n";
					fwrite( str, strlen( str ), 1, f );
					fwrite( commandQueue->getMacroCode(),
									strlen( commandQueue->getMacroCode() ), 1, f );
					str = "}\n";
					fwrite( str, strlen( str ), 1, f );
					fclose( f );
				}
			}

			return new TNumeric( success, COPY );
		}
		case 87 : // printFile
		{
			char **printers;
			int count;
			PrinterGraphics::getPrinters( &printers, count );
			PrinterGraphics::setupPrinter( printers[0] );
			Print *print = new Print( currentView, printers[0], Print::TWOUP, 
    														"Header", "Footer", 1, 100, 10, 10, 10, 10, 
																"testdoc" );
			print->startPrint();
			delete print;
			return NULL;
		}
		case 113 :
		{
			if( Application::executeFromMainThread( this, 6, NULL ) )
			{
				int options = 0;
				int from, to;

				if( PrintDialog::twoUp ) options |= Print::TWOUP;
				if( PrintDialog::oddPages ) options |= Print::ODDPAGES;
				if( PrintDialog::evenPages ) options |= Print::EVENPAGES;
				if( PrintDialog::printSyntaxHL ) options |= Print::PRINTSYNTAXHL;
				if( PrintDialog::printColor )  options |= Print::PRINTCOLOR;

				if( PrintDialog::selection == DALL )
				{
					from = 1;
					to = currentView->getTextlines();
				}
				else
				if( PrintDialog::selection == DSELECTED )
				{
					from = currentView->getLineFromPosition( currentView->getMarkBegin() );
					to = currentView->getLineFromPosition( currentView->getMarkEnd() );
				}
				else
				{
					from = PrintDialog::from;
					to = PrintDialog::to;
				}

				Print *print = new Print( currentView, PrintDialog::printer, options, 
																	PrintDialog::header, PrintDialog::footer,
																	from, to, PrintDialog::leftMargin,
																	PrintDialog::rightMargin, PrintDialog::topMargin,
																	PrintDialog::bottomMargin, textBuffer->getFilename() );
				AbortPrintDialog *abortPrintDialog = (AbortPrintDialog *)
					Application::executeFromMainThread( this, 7, (void *)print );
				abortPrintDialog->setVisible(TRUE);

				print->startPrint();

				Application::executeFromMainThread( this, 8, (void *)abortPrintDialog );
				delete print;
			}
			return NULL;
		}
		case 36 : // queryLineDialog
		{
			TSyntax **title = vars->LookUp( "title" );
			TSyntax **query = vars->LookUp( "query" );
			TSyntax **data = vars->LookUp( "data" );

			if( title == NULL || query == NULL || data == NULL )
			{
				eval->SendError( Strings::get(47), name );
			}
			else
			if( (*title)->Type != SString || (*query)->Type != SString ||
					(*data)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			QueryLineArg queryLineArg;
			queryLineArg.title = ((TString*)*title)->string;
			queryLineArg.query = ((TString*)*query)->string;
			queryLineArg.initText = &((TString*)*data)->string;

			int res = Application::executeFromMainThread( this, 9, (void *)&queryLineArg );

			return new TNumeric( res, COPY );
		}
		case 123 : // executeKFromClipboard
		{
			TextElem *text;

			int len = ClipBoard::getText( &text );
			
			if( len )
			{
				addCommand( text );				
				delete text;
			}
			break;			
		}
		case 178 : // executeKFromKClipboard
		{		
      if( KonSystem::kClipboard != NULL && strlen( KonSystem::kClipboard ) > 0 )
				addCommand( KonSystem::kClipboard );
			break;			
		}
		case 124 : // copyMacroToClipboard
		{
			// Stop recording if running
			if( commandQueue->isRecording() ) commandQueue->endRecord();

			char *macroCode = commandQueue->getMacroCode();
			if( macroCode != NULL )
				ClipBoard::setText( macroCode, strlen( macroCode ) );
			break;
		}
		case 175 : // copyMacroToKClipboard
		{
			// Stop recording if running
			if( commandQueue->isRecording() ) commandQueue->endRecord();

			char *macroCode = commandQueue->getMacroCode();
			if( macroCode != NULL )
      {
        KonSystem::kClipboard = (char *)realloc( KonSystem::kClipboard, strlen(macroCode) + 1 );
        strcpy(KonSystem::kClipboard, macroCode );
      }
			break;
		}
		case 131 :
		{
			QueryLineDialog *dlg = new QueryLineDialog( this, this, "hejsan", "Hej igen", "test" );
			dlg->setVisible( TRUE );
			delete dlg;
			return 0;
		}
		case 137 :	// aboutDialog
		{
			Application::executeFromMainThread( this, 11, NULL );
			return NULL;
		}
    case 147 :
    {
			TSyntax **text = vars->LookUp( "text" );

			if( text == NULL )
			{
				eval->SendError( Strings::get(47), name );
			}
			else
			if( (*text)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			else
  			Application::executeFromMainThread( this, 13, ((TString *)*text)->string );

			return NULL;
    }
		case 6 : // getCurrentControlId
		{
			return new TNumeric( getId(), COPY );
		}
		case 171 : // screenLineDown
      viewWindow->screenLineUp();
      viewWindow->cursorToScreen();
			break;
		case 172 : // screenLineUp
      viewWindow->screenLineDown();
      viewWindow->cursorToScreen();
			break;
	  case 174 : // regenerateSections
      sectionBrowser->regenerate( TRUE );
	    return NULL;
		default:
	    retval = currentView->eval( name, vars, eval, index );
	}

	// Check if non-persistent blocks and we're off the block area

  if( !Settings::persistentBlocks )
  {
  	if( currentView->getMarkBegin() != currentView->getPosition() &&
    		currentView->getMarkEnd() != currentView->getPosition() &&
        !currentView->keepBlock() )
		{
    	// Reset block mark
      currentView->resetMark();
    }
  }

  if( currentView->getPosition() != currentView->getLineMarkCursorPosWhenSet() )
    currentView->resetLineMark();

	BOOL cursorMoved = currentView->cursorHasMoved();

  if( viewWindow->isUpdating() )
  {
    BOOL updateScrollbars = FALSE;
    // Move the screenstart if needed to show the cursor
    if( cursorMoved ||
        oldScreenLine != viewWindow->getWindowStartLine() ||
        oldWinLines != viewWindow->getWindowLines() || moveToCursor ||
        lastView != currentView ||
        currentView->getLine() < viewWindow->getWindowStartLine() ||
        currentView->getLine() >= viewWindow->getWindowStartLine() + viewWindow->getWindowLines() )
    {
      updateScrollbars = TRUE;

      viewWindow->screenToCursor();
      
      if( moveToCursor )
        viewWindow->repaint( NULL );

    }

    if( updateScrollbars ||
        oldLines != currentView->getTextlines() )
    {
        // Update scrollbars
        viewWindow->updateScrollbars();
    }
	}

  // If still the active window
  if( isActive && viewWindow->isUpdating() )
	{
		// Set the new cursor position
		if( cursorMoved || moveToCursor ||
				lastView != currentView ||
				prevInsertMode != Settings::insertMode )
		{
     	currentView->updateGraphicCursor();
			if( viewWindow->inFocus() )
				viewWindow->showCursor( FALSE );

			// And show position in the infobar

			char str[30];

			currentView->getPositionString( str );

			if( infoBar != NULL ) infoBar->updatePosition( str );
			updateHexInfo();
		}
  }

  return retval;
}

void EditControl::size( int oldWidth, int oldHeight )
{
  if( viewInitialized )
  {
    int topSpace = 0;

    if( Settings::showBytesInfo && currentView == hexView )
    {
      getGraphics()->setSystemFont();

      topSpace = BytesInfo::getInset() + getGraphics()->getMaxFontHeight();
      bytesInfo->setPos( 0, height - topSpace ); 
      bytesInfo->setSize( width, topSpace ); 
      bytesInfo->setVisible( TRUE );
    }
    else
      bytesInfo->setVisible( FALSE );
      
    BOOL showSectionBrowser = FALSE;

    if( Settings::showSectionBrowser && currentView == asciiView && asciiView->getCurrentSyntax() != NULL )
      showSectionBrowser = (asciiView->getCurrentSyntax()->sections > 0);

    if( showSectionBrowser )
    {
      getGraphics()->setSystemFont();

      topSpace = sectionBrowser->getHeight();
      sectionBrowser->setPos( 0, height - topSpace ); 
      sectionBrowser->setSize( width, topSpace ); 
      sectionBrowser->setVisible( TRUE );
    }
    else
      sectionBrowser->setVisible( FALSE );

    south->setSize( width - Scrollbar::getInset(1), Scrollbar::getInset(0) );
    south->setPos( 0, 0 );

    east->setSize( Scrollbar::getInset(1), height - Scrollbar::getInset(0) - topSpace );

    east->setPos( width - Scrollbar::getInset(1), Scrollbar::getInset(0) );

    viewWindow->setPos( 0, Scrollbar::getInset(0) );


    viewWindow->setSize( width - Scrollbar::getInset(1),
                         height - Scrollbar::getInset(0) - topSpace );

  }
}

void EditControl::textLoaded( BOOL isBinaryFile, BOOL reloaded )
{
  if( !reloaded )
  {
    asciiView = new AsciiView( this, textBuffer, syntax, infoBar, isSimpleControl );
    hexView = new HexView( this, textBuffer, infoBar );
    
    if( isBinaryFile )
      currentView = hexView;
    else
      currentView = asciiView;

    viewWindow->setView( currentView, TRUE );
    viewInitialized = TRUE;
    viewWindow->setVisible( TRUE );
    south->setVisible( TRUE );
    east->setVisible( TRUE );
  }
      
  viewWindow->updateScrollbars();
  viewWindow->resetLineDefs();
  size( 0, 0 );

  char str[30];
  currentView->getPositionString( str );
  if( infoBar != NULL ) infoBar->updatePosition( str );

  if( !reloaded )
  {
    // Set the buffer observers
    textBuffer->addDisplayObserver( viewWindow );
    viewWindow->addKeyListener( this );
    addScrollbarListener( this );
    commandQueue->executorReady();
    asciiView->addMoveListener( sectionBrowser );
  }
  sectionBrowser->regenerate( TRUE );

  viewWindow->resetLineDefs(); // Shouldn't be neccessary, but panic (possible) solution
                               // to repaint problem sometimes occuring at startup. Same
                               // for resetLineDefs above.
  repaint( NULL );

  if( reloaded )
    addCommand( "jumpToPosition(0);");
}

void EditControl::activate( BOOL active )
{
  isActive = active;

  if( active )
  {
    if( Settings::autogenerateSections && Settings::showSectionBrowser ) sectionBrowser->regenerate( FALSE );
    // Set this editcontrol in front of the list
    int i = ControlAdmin::getControlIndex( this );

    if( i != -1 )
    {
      ControlInfo *controlI = ControlAdmin::getControl( i );
			MainWindow *prevParent = controlI->parent;
      ControlAdmin::removeControl( i );
      ControlAdmin::insertControl( prevParent, this );
      // The main menu must be updated from the main thread as it
      // performs creation and destruction of windows
      Application::executeFromMainThread( prevParent, 0, NULL );
    }
    viewWindow->setFocus();
    // Update the main windows infoBar
    if( infoBar != NULL )
    {
      infoBar->setModified( textBuffer->textModified() );

      if( currentView != NULL )
      {
        char str[30];

        currentView->getPositionString( str );
        infoBar->updatePosition( str );

        infoBar->setRecord( commandQueue->isRecording() );
      }
    }
  }
}

int EditControl::saveFile()
{
	if( infoBar != NULL )
		infoBar->message( Strings::get( 80 ) );
	int res = currentView->saveFile();
	if( infoBar != NULL )
		infoBar->clearMessage();
	return res;
}

void EditControl::setHorzScrollbar( int pos, int visible, int total )
{
  if( !scrollbarHandleDown )
  {
    if( total == 0 )
      visible = viewWindow->getWidth(); 

    if( visible == total ) total = visible + visible / 5;
    
    if( pos != hScrollbarData.pos || 
        visible != hScrollbarData.visible ||
        total != hScrollbarData.total )
			south->setAll( pos, 1, total, visible, total );

    hScrollbarData.pos = pos;
    hScrollbarData.visible = visible;
    hScrollbarData.total = total;
  }    
}

void EditControl::actionPerformed( AScrollbarEvent *scrollbarEvent )
{
  if( scrollbarEvent->getAction() == Scrollbar::LINEDOWN )
    viewWindow->screenLineDown();
  else
  if( scrollbarEvent->getAction() == Scrollbar::LINEUP )
    viewWindow->screenLineUp();
  else
  if( scrollbarEvent->getAction() == Scrollbar::PAGEUP )
    viewWindow->screenPageUp();
  else
  if( scrollbarEvent->getAction() == Scrollbar::PAGEDOWN )
    viewWindow->screenPageDown();
  else
  if( scrollbarEvent->getAction() == Scrollbar::VERTRELEASED )
  {
    scrollbarHandleDown = FALSE;
    setVertScrollbar( vScrollbarData.pos, vScrollbarData.visible,
                      vScrollbarData.total );
  }
  else
  if( scrollbarEvent->getAction() == Scrollbar::VERTTRACK )
  {
    viewWindow->setScreenStart( scrollbarEvent->getPos() );
    scrollbarHandleDown = TRUE;
  }
  else
  if( scrollbarEvent->getAction() == Scrollbar::LINERIGHT )
    viewWindow->scrollColumn(10);
  else
  if( scrollbarEvent->getAction() == Scrollbar::LINELEFT )
    viewWindow->scrollColumn(-10);
  else
  if( scrollbarEvent->getAction() == Scrollbar::PAGERIGHT )
    viewWindow->scrollColumn( viewWindow->getWidth() / 5 );
  else
  if( scrollbarEvent->getAction() == Scrollbar::PAGELEFT )
    viewWindow->scrollColumn( -viewWindow->getWidth() / 5 );
  else
  if( scrollbarEvent->getAction() == Scrollbar::HORZTRACK )
  {
    viewWindow->setHorzMargin( scrollbarEvent->getPos()-1 );
    scrollbarHandleDown = TRUE;
  }
  else
  if( scrollbarEvent->getAction() == Scrollbar::HORZRELEASED )
  {
    scrollbarHandleDown = FALSE;
    setHorzScrollbar( scrollbarEvent->getPos()-1, hScrollbarData.visible,
                      hScrollbarData.total );
  }
}

int EditControl::queryReplaceDialog( char *foundString, char *replaceString, 
                                     int pos, int len, BOOL multiFile )
{
  QueryReplace qr( foundString, replaceString, pos, len, multiFile );
  return Application::executeFromMainThread( this, 3, &qr );
}

int EditControl::execFromMainThread( int task, void *arg )
{
  switch( task )
  {
    case 0 :
    {
			// Find the currently selected string if one is available
			TextElem *findString = NULL;

      if( viewHasValidMark() )
			{
				int blockSize = currentView->getMarkEnd() - currentView->getMarkBegin();
				if( blockSize > 100 ) blockSize = 100;
				findString = new TextElem[blockSize+1];
				currentView->getText( findString, currentView->getMarkBegin(), 
				                      blockSize );
				findString[blockSize] = 0;
			}
			
			if( searchDialog != NULL ) { delete searchDialog; searchDialog = NULL; }
			if( replaceDialog != NULL ) { delete replaceDialog; replaceDialog = NULL; }

      // Display search dialog
      searchDialog =
        new SearchDialog( Desktop::getInstance(), parent, findString );
      BOOL result = searchDialog->setVisible( TRUE );
			if( findString != NULL ) delete findString;
      return 1;
/*      delete searchDialog;

      if( result )
      {
        int l = SearchDialog::getStringLength();
        TextElem *searchString = new TextElem[l+1], *fixedString,
        				 *fixedPath, *fixedFiles;
        char *optionsString = new char[10];

        SearchDialog::getSettings( optionsString, searchString );

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

						cmd = new char[strlen( fixedString ) + strlen( optionsString ) +
  	                       strlen( fixedPath ) +
                           strlen( fixedFiles  ) + 40];
          }
          else
          {
            path = SearchDialog::getPath();

            fixedPath = KCommonCommands::getInstance()->fixString( path );

          	files = SearchDialog::getFiles();

		        fixedFiles = KCommonCommands::getInstance()->fixString( files );

						cmd = new char[strlen( fixedString ) + strlen( optionsString ) +
  	                       strlen( fixedPath ) +
                           strlen( fixedFiles  ) + 60];
          }

  	      sprintf( cmd, "findStringFiles( '%s', '%s','%s', '%s' );",
    	             fixedString, optionsString, fixedPath, fixedFiles );

	        if( path != fixedPath ) delete fixedPath;
  	      if( files != fixedFiles ) delete fixedFiles;
		      if( files != SearchDialog::getFiles() ) delete files;
        }

        addCommand( cmd );

        if( searchString != fixedString ) delete fixedString;

        delete cmd;
        delete searchString;
        delete optionsString;
      }
      return result;*/
    }
    case 1 :
    {
			// Find the currently selected string if one is available
			TextElem *findString = NULL;

      if( viewHasValidMark() )
			{
				int blockSize = currentView->getMarkEnd() - currentView->getMarkBegin();
				if( blockSize > 100 ) blockSize = 100;
				findString = new TextElem[blockSize+1];
				currentView->getText( findString, currentView->getMarkBegin(), 
				                      blockSize );
				findString[blockSize] = 0;
			}

			if( searchDialog != NULL ) { delete searchDialog; searchDialog = NULL; }
			if( replaceDialog != NULL ) { delete replaceDialog; replaceDialog = NULL; }

      // Display replace dialog
      replaceDialog =
        new ReplaceDialog( Desktop::getInstance(), parent, findString );
      BOOL result = replaceDialog->setVisible( TRUE );
			if( findString != NULL ) delete findString;
      return 1;
/*      delete replaceDialog;

			if( findString != NULL ) delete findString;

      if( result )
      {
        int l1 = ReplaceDialog::getFindStringLength(),
            l2 = ReplaceDialog::getReplaceStringLength();
        TextElem *searchString = new TextElem[l1+1], *fixedSearchString,
                 *replaceString = new TextElem[l2+1], *fixedReplaceString,
                 *fixedPath, *fixedFiles;
        char *optionsString = new char[10], *cmd;

        ReplaceDialog::getSettings( optionsString, searchString,
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

        addCommand( cmd );

        if( searchString != fixedSearchString ) delete fixedSearchString;
        if( replaceString != fixedReplaceString ) delete fixedReplaceString;

        delete cmd;
        delete searchString;
        delete replaceString;
        delete optionsString;
      }
      return result;*/
    }
/*    case 2 :
    case 4 :
    case 11 :
    {
      // Display File load dialog
      FileDialog *fileDialog =
        new FileDialog( Desktop::getInstance(), parent, 
                        (task==2) ? Strings::get(90) : Strings::get(118),
												(task==4) ? textBuffer->getFilename() : NULL );
      BOOL result = fileDialog->setVisible( TRUE );

      delete fileDialog;

      if( result )
      {
        char *command, 
             *pfString,
             *fixedFilename;

				if( task == 2 )
					pfString = "loadFile( '%s' );";
				else
				if( task == 4 )
					pfString = "saveFileAs( '%s' );";
				else
					pfString = "saveMacro( '%s' );";

				fixedFilename = KCommonCommands::getInstance()->
        									fixString( FileDialog::getFilename() );

        command = new char[strlen(pfString)+strlen(fixedFilename)+10];
        sprintf( command, pfString, fixedFilename );

        addCommand( command );
        delete command;
        if( fixedFilename != FileDialog::getFilename() )
        	delete fixedFilename;
      }

      return result;
    }*/
    case 12 :
    {
			FileDialogInfo *fdInfo = (FileDialogInfo *)arg;

      BOOL result;

      if( Settings::useSystemFileDialog )
      {
				char filters[MAXPATH*2+1];
        char cFilter[MAXPATH*2+1];
        char wFilters[MAXPATH*2+1];
				int fltPos = 0, fltIndex = -1;

				for( int i = 0; i < SyntaxContainer::getSyntaxCount(); i++ )
        {
          Syntax *s = SyntaxContainer::getSyntaxAt(i);

          if( currentView->getViewType() == VIEW_ASCIIVIEW )
          {
            if( s == ((AsciiView *)currentView)->getCurrentSyntax() )
              fltIndex = i+1;
          }

          strcpy( cFilter, s->name );
          strcat( cFilter, " Files (" );

          *wFilters = 0;

          for( int n = 0; n < s->extensions; n++ )
          {
            strcat( wFilters, "*" );
            strcat( wFilters, s->extensionList[n] );
            if( n < s->extensions - 1 )
              strcat( wFilters, ";" );
          }
          
          strcat( cFilter, wFilters );
          strcat( cFilter, ")" );

          memcpy( &filters[fltPos], cFilter, strlen(cFilter) );
          fltPos += strlen(cFilter);
          filters[fltPos++] = 0;
          memcpy( &filters[fltPos], wFilters, strlen(wFilters) );
          fltPos += strlen(wFilters);
          filters[fltPos++] = 0;
        }

        memcpy( &filters[fltPos], Strings::get(205), strlen(Strings::get(205)) );
        fltPos += strlen(Strings::get(205));
        filters[fltPos++] = 0;
        memcpy( &filters[fltPos], "*.*", 3 );
        fltPos += 3;
        filters[fltPos++] = 0;
        filters[fltPos++] = 0;

        if( fltIndex == -1 )
          fltIndex = SyntaxContainer::getSyntaxCount() + 1;

        SystemFileDialog sysFileDlg;

        result = sysFileDlg.display( Desktop::getInstance(), parent, fdInfo->title,
				                    fdInfo->filename, fdInfo->type, filters, (fltPos == 0) ? NULL : fltIndex, fdInfo->encType );
        fdInfo->filename = SystemFileDialog::returnedFile;
      }
      else
      {
        // Display File dialog
        FileDialog *fileDialog =
          new FileDialog( Desktop::getInstance(), parent, fdInfo->title,
				                  fdInfo->filename ); 

        result = fileDialog->setVisible( TRUE );
  
        fdInfo->filename = FileDialog::getFilename();

        delete fileDialog;
      }

      return result;
    }
    case 3:
    {
      // query replace dialog

      QueryReplace *qr = (QueryReplace *)arg;

      QueryReplaceDialog queryReplace( Desktop::getInstance(),
                                       viewWindow,
																			 parent,
                                       qr->foundStr, qr->replaceStr,
                                       qr->pos+qr->len, qr->multiFile );

      return queryReplace.setVisible( TRUE );
    }
    case 5 :
    {
		  MainMenu::updateWindowMenu();
    	break;
    }
    case 6 :
    {
      // Display Print dialog
      PrintDialog *printDialog =
        new PrintDialog( Desktop::getInstance(), parent );
      BOOL result = printDialog->setVisible( TRUE );

      delete printDialog;

      return result;
    }
    case 7 :
    {
      // Display Abort Print dialog
      AbortPrintDialog *abortPrintDialog =
        new AbortPrintDialog( Desktop::getInstance(), parent, (Print *)arg );
			return (int)abortPrintDialog;
    }
		case 8 :
		{
			delete (AbortPrintDialog *)arg;
			break;
		}
		case 9 : // Common dialog, query line
		{
			struct QueryLineArg *queryLineArg = (QueryLineArg *)arg;

			QueryLineDialog *queryLineDlg = 
				new QueryLineDialog( Desktop::getInstance(), parent, queryLineArg->title,
				                     queryLineArg->query, *queryLineArg->initText );
			int res = queryLineDlg->setVisible( TRUE );

			if( res )
			{
				delete *queryLineArg->initText;
				*queryLineArg->initText = new char[strlen( QueryLineDialog::text ) + 1];
				strcpy( *queryLineArg->initText, QueryLineDialog::text );
			}

			delete queryLineDlg;
			return res;
		}
    case 10 :	// Popup menu
    {
    	Point *point = (Point *)arg;
    	viewWindow->popupMenu( PopupMenu::getMenu(), point->x, point->y );
      return 0;
    }
    case 11 :
    {
      // Display About dialog
      AboutDialog *aboutDialog =
        new AboutDialog( parent );

      aboutDialog->setVisible( TRUE );

      delete aboutDialog;

      return 0;
    }
    case 13:
    {
      viewWindow->popupWindow( (char *)arg );
      break;
    }
  }
  return 0;
}

void EditControl::popupMenu( Point *point )
{
  Application::executeFromMainThread( this, 10, (void *)point );
}

void EditControl::fileRenamed()
{
  // Check if we need a new syntax definition

  Syntax *newSyntax = SyntaxContainer::retrieveSyntax(textBuffer->getFilename());

  if( newSyntax != syntax )
  {
    syntax = newSyntax;
    asciiView->setSyntax( syntax );
    viewWindow->repaint( NULL );
  }
}

void EditControl::actionPerformed( ASelectionEvent *selectionEvent )
{
  Section *section = sectionBrowser->getSection( selectionEvent->getSelected() );
  char cmd[100];
  sprintf( cmd, "jumpToPosition( %d );", section->sectionStartPos );
  addCommand(cmd);
  setFocus();
}


int EditControl::getCodeSections( CodeSection **section )
{
  return asciiView->getCodeSections( section );
}

void EditControl::updateHexInfo()
{
	if( currentView == hexView && Settings::showBytesInfo )
	{
		bytesInfo->bytes(textBuffer->getCharNoReader(hexView->getPosition()),
				textBuffer->getCharNoReader(hexView->getPosition()+1),
				textBuffer->getCharNoReader(hexView->getPosition()+2), 
				textBuffer->getCharNoReader(hexView->getPosition()+3) );
	}
}

void EditControl::actionPerformed( APositionEvent *positionEvent )
{
  updateHexInfo();
}

EditControl::~EditControl()
{                 
 	textBuffer->removeDisplayObserver( viewWindow );
  delete viewWindow;
  delete asciiView;
  delete hexView;
  textBuffer->removeLoadObserver( this );
	delete backgroundBitmap;
  delete bytesInfo;
  delete sectionBrowser;
}

