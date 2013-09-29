#include "bowl.h"

Desktop * Desktop::pDesktop = new Desktop();

int Scrollbar::PAGERIGHT = -1;
int Scrollbar::PAGELEFT = -2;
int Scrollbar::LINERIGHT = -3;
int Scrollbar::LINELEFT = -4;
int Scrollbar::PAGEUP = -5;
int Scrollbar::PAGEDOWN = -6;
int Scrollbar::LINEUP = -7;
int Scrollbar::LINEDOWN = -8;
int Scrollbar::HORZRELEASED = -9;
int Scrollbar::VERTRELEASED = -12;
int Scrollbar::HORZTRACK = -10;
int Scrollbar::VERTTRACK = -11;

int Label::LEFT = 8192;
int Label::CENTER = 16384;
int Label::RIGHT = 32768;

int PopupMessage::error = 1;
int PopupMessage::warning = 2;
int PopupMessage::message = 3;
int PopupMessage::query = 4;
int PopupMessage::ok = 5;
int PopupMessage::cancel = 6;
int PopupMessage::yes = 7;
int PopupMessage::no = 8;
int PopupMessage::querywithwarning = 9;
int PopupMessage::querywithwarningNoCancel = 10;

BOOL Window::baseClassRegistered = FALSE;

MRESULT EXPENTRY WndProc( HWND hWnd,ULONG iMessage,
                          MPARAM mParam1, MPARAM mParam2)
{
	Window *pWindow = (Window *)WinQueryWindowULong(hWnd,0);
  MRESULT res;

	if (pWindow == 0) {
		if (iMessage == WM_CREATE) {
			pWindow = (Window *)Application::tempthis;
			WinSetWindowULong(hWnd,0,(ULONG)pWindow);
			res = pWindow->wndProc(hWnd,iMessage,mParam1,mParam2);
		} else
			return WinDefWindowProc(hWnd,iMessage,mParam1,mParam2);
	} else
		res = pWindow->wndProc(hWnd,iMessage,mParam1,mParam2);

  if( pWindow != 0 && pWindow->killMe )
  {
    WinSetWindowULong(hWnd,0,(ULONG)0L);
    delete pWindow;
  }
  return res;
}

void Window::createWindow( Window *parent, Window *owner, char *text, 
                           char *windowClass,
                           Window *behind, ULONG windowStyle, int x, int y,
                           int width, int height )
{
  char str[50], *wClass;

  mouseButtonDown = FALSE;

  this->parent = parent;

  if( parent->getType() == DIALOG)
  	behind = (Window *)parent->getLastControl();

  type = WINDOW;
  this->id = Iterator::getNext();

  Application::tempthis = this;

  oldWndProc = NULL;

  if( windowClass == 0L )
  {
    currentPointer = WinQuerySysPointer( HWND_DESKTOP, SPTR_ARROW, FALSE );

		strcpy( str, "KONBASEWINDOW" );
		if( !baseClassRegistered )
		{
			baseClassRegistered = TRUE;
			if( !WinRegisterClass( Application::hab, (PSZ)str, //(PSZ)itoa( id, str, 10),
												WndProc, CS_SIZEREDRAW | CS_MOVENOTIFY | CS_SYNCPAINT,
												sizeof(Window *) ) )
			{
				exit( 1 );
			}
		}
    wClass = str;
  }
  else
  {
    currentPointer = NULLHANDLE;
    wClass = windowClass;
  }

	if( windowStyle & WS_DISABLED )
	{
		windowStyle &= ~WS_DISABLED;
		isPopupWin = TRUE;
	}

  hWndFrame = hWndClient = WinCreateWindow( (isPopupWin) ? HWND_DESKTOP : parent->getHWND(), wClass, text,
                                            windowStyle, x, y, width, height,
                                            owner->getHWND(),
                                            (behind == NULL) ? HWND_TOP : behind->getHWND(),
                                            id, 0L, 0L );

  // Subclass window

  if( wClass == windowClass )
  {
    WinSetWindowULong(hWndFrame,0,(ULONG)this);
    oldWndProc = WinSubclassWindow( hWndFrame, (PFNWP)WndProc );
  }

  if( hWndFrame == NULLHANDLE )
  {
    DosBeep( 1000, 100 );
    char wText[501];
    WinQueryWindowText( parent->getHWNDFrame(), 500, wText );
  }

  graphics = new Graphics( WinGetPS( hWndClient ) );
}

ULONG Window::xlatProperties( int properties )
{
  ULONG windowStyle = 0;

  if( properties & ISTABSTOP )
    windowStyle |= WS_TABSTOP;

  if( properties & NOADJUSTSIZE )
    windowStyle |= LS_NOADJUSTPOS;

  if( properties & NEWGROUP )
    windowStyle |= WS_GROUP;

  if( properties & ISDEFAULT )
    windowStyle |= BS_DEFAULT;

  if( properties & SYNCPAINT )
    windowStyle |= WS_SYNCPAINT;

  if( properties & CLIPSIBLINGS )
    windowStyle |= WS_CLIPSIBLINGS;

  if( properties & MULTIPLESEL )
    windowStyle |= LS_MULTIPLESEL;

/*  if( properties & MULTIPLESEL )
		windowStyle |= 0xf0000000;*/

	if( properties & POPUP )
		windowStyle |= WS_DISABLED;
		isPopupWin = TRUE;

	setDialogFilter( properties );

  return windowStyle;
}

Window::Window( Window *parent, char *windowClass, char *text,
        ULONG windowStyle,
        int x, int y, int width, int height ) : killMe(FALSE),
  AWindow( parent, x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
  isPopupWin(FALSE)
{
  createWindow( (Window *)parent, (Window *)parent, text, windowClass, /*behind*/(Window *)parent->getLastControl(), windowStyle,
                x, y, width, height );

  // Set default font if parent is a dialog NOT

//  if( parent->type == DIALOG )
    setFont( graphics->getDialogFontName(), graphics->getDialogFontSize() );
}

Window::Window( Window *parent, Window *owner, char *windowClass, char *text,
        ULONG windowStyle,
        int x, int y, int width, int height ) : killMe(FALSE),
  AWindow( parent, x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
  isPopupWin(FALSE)
{
  createWindow( (Window *)parent, (Window *)owner, text, windowClass, /*behind*/(Window *)parent->getLastControl(), windowStyle,
                x, y, width, height );

  // Set default font if parent is a dialog NOT

//  if( parent->type == DIALOG )
    setFont( graphics->getDialogFontName(), graphics->getDialogFontSize() );
}

void Window::deleteMe()
{
  killMe = TRUE;
  // Send a message get into the WndProc in case this method was called from
  // a thread
  WinPostMsg( hWndFrame, WM_USER+100, NULL, NULL );
}

MRESULT Window::stdWndProc( HWND hWnd, ULONG iMessage, MPARAM mParam1,
                            MPARAM mParam2, BOOL *returned )
{
  switch( iMessage )
  {
//    case WM_FOCUSCHANGE :
    case WM_SETSELECTION :
    case WM_ACTIVATE :
    {
      if( type != Window::LISTBOX &&
          type != Window::COMBOLISTBOX )
      {
        focus( (BOOL)mParam1 );//SHORT1FROMMP( mParam2 ) );
      }
      break;
    }
    case WM_SETFOCUS :
    {
      if( type != Window::LISTBOX &&
          type != Window::COMBOLISTBOX )
        focusSet( (BOOL)mParam2 );
      break;
    }
    case WM_BUTTON1DOWN :
    {
      if( type == SCROLLBAR )
        setFocus();
      HWND hwnd = WinQueryFocus( HWND_DESKTOP );
      if( mouseListener.getSize() != 0 )
      {
        mouseButtonDown = TRUE;
        WinSetCapture( HWND_DESKTOP, hWnd );
	  		WinStartTimer( Application::hab, hWndClient, 1, 50 );
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::LEFT, AMouseEvent::DOWN );
      }
      if( hwnd != WinQueryFocus( HWND_DESKTOP ) ) return (MRESULT)TRUE;
      break;
    }
    case WM_BUTTON1UP :
    {
      if( type == LISTBOX ||
          type == COMBOLISTBOX )
      {
        performFocusEvent(
          (int)WinSendMsg( (HWND)mParam2, LM_QUERYSELECTION, MPARAM(0), MPARAM(NULL) ) );
      }
      else
      if( mouseListener.getSize() != 0 )
      {
        mouseButtonDown = FALSE;
        WinStopTimer( Application::hab, hWndClient, 1 );
        WinSetCapture( HWND_DESKTOP, NULLHANDLE );
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::LEFT, AMouseEvent::RELEASE );
//        return (MPARAM)TRUE;
      }
      break;
    }
    case WM_BUTTON2DOWN :
    {
      if( mouseListener.getSize() != 0 )
      {
        WinSetCapture( HWND_DESKTOP, hWnd );
	  		WinStartTimer( Application::hab, hWndClient, 1, 50 );
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::RIGHT, AMouseEvent::DOWN );
      }
      break;
    }
    case WM_BUTTON2UP :
    {
      if( mouseListener.getSize() != 0 )
      {
        WinStopTimer( Application::hab, hWndClient, 1 );
        WinSetCapture( HWND_DESKTOP, NULLHANDLE );
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::RIGHT, AMouseEvent::RELEASE );
      }
      break;
    }
    case WM_BUTTON1CLICK :
    {
      if( mouseListener.getSize() != 0 )
      {
				// Check if triple click

				int newTick = WinGetCurrentTime( Application::hab );
				if( newTick - lastTick <= 
            WinQuerySysValue( HWND_DESKTOP, SV_DBLCLKTIME ) )
				{
	        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
		      performMouseEvent( &pos, AMouseEvent::LEFT, AMouseEvent::TRIPLECLICK );
				}
				else
        {
        	Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        	performMouseEvent( &pos, AMouseEvent::LEFT, AMouseEvent::CLICK );
				}
      }
      break;
    }
    case WM_BUTTON2CLICK :
    {
      if( mouseListener.getSize() != 0 )
      {
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::RIGHT, AMouseEvent::CLICK );
      }
      break;
    }
    case WM_BUTTON1DBLCLK :
    {
      if( mouseListener.getSize() != 0 )
      {
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::LEFT, AMouseEvent::DOUBLECLICK );
      }
			lastTick = WinGetCurrentTime( Application::hab );
      break;
    }
    case WM_BUTTON2DBLCLK :
    {
      if( mouseListener.getSize() != 0 )
      {
        Point pos( SHORT1FROMMP(mParam1), SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, AMouseEvent::RIGHT, AMouseEvent::DOUBLECLICK );
      }
      break;
    }
    case WM_MOUSEMOVE :
    {
      if( currentPointer != NULLHANDLE )
        WinSetPointer( HWND_DESKTOP, currentPointer );
      if( mouseListener.getSize() != 0 )
      {
        Point pos( (SHORT)SHORT1FROMMP(mParam1), (SHORT)SHORT2FROMMP(mParam1) );
        performMouseEvent( &pos, 0, AMouseEvent::MOVE );
      }
      if( currentPointer != NULLHANDLE )
       	return (MPARAM)TRUE;
      break;
    }
    case WM_TIMER :
    {
      if( mouseButtonDown )
      {
        POINTL ptl;
        WinQueryPointerPos( HWND_DESKTOP, &ptl );
        WinMapWindowPoints( HWND_DESKTOP, hWnd, &ptl, 1 );

        if( ptl.y < 0 || ptl.y > height )
        {
          Point pos( ptl.x, ptl.y );
          performMouseEvent( &pos, 0, AMouseEvent::MOVE );
        }
      }
      break;
    }
    case WM_SIZE :
    {
      int oldWidth = width, oldHeight = height;
      SWP swp;

      WinQueryWindowPos( hWndFrame, &swp );

      width = swp.cx;
      height = swp.cy;
      size( oldWidth, oldHeight );
      break;
    }
    case WM_HSCROLL :
    {
      int pos = SHORT1FROMMP( mParam2 ), id = SHORT1FROMMP( mParam1 );
      switch( SHORT2FROMMP( mParam2 ) )
      {
        case SB_PAGERIGHT :
        {
          performScrollbarEvent( id, Scrollbar::PAGERIGHT, pos );
          break;
        }
        case SB_PAGELEFT :
        {
          performScrollbarEvent( id, Scrollbar::PAGELEFT, pos );
          break;
        }
        case SB_LINERIGHT :
        {
          performScrollbarEvent( id, Scrollbar::LINERIGHT, pos );
          break;
        }
        case SB_LINELEFT :
        {
          performScrollbarEvent( id, Scrollbar::LINELEFT, pos );
          break;
        }
        case SB_SLIDERTRACK :
        {
          performScrollbarEvent( id, Scrollbar::HORZTRACK, pos );
          break;
        }
        case SB_SLIDERPOSITION :
        {
          performScrollbarEvent( id, Scrollbar::HORZRELEASED, pos );
          break;
        }
      }
      break;
    }
    case WM_SYSCOMMAND :
    {
      // Check if the parent is a dialog, then chain the WM_SYSCOMMAND

      if( parent->getType() == DIALOG )
        WinSendMsg( ((Window *)parent)->getHWND(), WM_SYSCOMMAND, mParam1, mParam2 );
      break;
    }
    case WM_VSCROLL :
    {
      int id = SHORT1FROMMP( mParam1 );
      Scrollbar *scrollbar = (Scrollbar *)getControl( id );
      int pos;
      if( scrollbar )
        pos = scrollbar->convertScrollPosToReal( SHORT1FROMMP( mParam2 ) );
      else
        pos = SHORT1FROMMP( mParam2 );

      switch( SHORT2FROMMP( mParam2 ) )
      {
        case SB_PAGEDOWN :
        {
          performScrollbarEvent( id, Scrollbar::PAGEDOWN, pos );
          break;
        }
        case SB_PAGEUP :
        {
          performScrollbarEvent( id, Scrollbar::PAGEUP, pos );
          break;
        }
        case SB_LINEDOWN :
        {
          performScrollbarEvent( id, Scrollbar::LINEDOWN, pos );
          break;
        }
        case SB_LINEUP :
        {
          performScrollbarEvent( id, Scrollbar::LINEUP, pos );
          break;
        }
        case SB_SLIDERTRACK :
        {
          performScrollbarEvent( id, Scrollbar::VERTTRACK, pos );
          break;
        }
        case SB_SLIDERPOSITION :
        {
          performScrollbarEvent( id, Scrollbar::VERTRELEASED, pos );
          break;
        }
      }
      break;
    }
    case WM_MOVE :
    {
      SWP swp;

      WinQueryWindowPos( hWndFrame, &swp );

      x = swp.x;
      y = swp.y;
      move();
      break;
    }
    case WM_CLOSE :
    {
      if( close() )
        break;
      else
        return (MPARAM)FALSE;
    }
    case WM_COMMAND :
    {
      if( SHORT1FROMMP( mParam2 ) == CMDSRC_PUSHBUTTON )
      {
        Window *control = (Window *)getControl( SHORT1FROMMP(mParam1));
        if( control != NULL )
        {
          if( control->type == PUSHBUTTON )
          {
            control->performButtonEvent( control, control->id );
            return FALSE;
          }
        }
        else
          return FALSE; // In case ESC would close the dialog
      }
      break;
    }
    case WM_CHAR :
    {
      if( type == LISTBOX )
      {
        performFocusEvent(
          (int)WinSendMsg( (HWND)mParam2, LM_QUERYSELECTION, MPARAM(0), MPARAM(NULL) ) );
      }

      if( (SHORT1FROMMP( mParam1 ) & KC_KEYUP) ||
          (SHORT1FROMMP( mParam1 ) & KC_DEADKEY) ||
          (SHORT1FROMMP( mParam1 ) & KC_INVALIDCOMP) )
        break;

/*			if( SHORT1FROMMP( mParam1 ) & KC_COMPOSITE )
				mParam1 = MPFROM2SHORT( SHORT1FROMMP( mParam1 ) & ~KC_VIRTUALKEY,
																SHORT2FROMMP( mParam1 ) );*/

      int modifiers = 0;

      if( SHORT1FROMMP( mParam1 ) & KC_SHIFT )
        modifiers |= KeyDef::SHIFT;

      if( SHORT1FROMMP( mParam1 ) & KC_ALT )
        modifiers |= KeyDef::kALT;

      if( SHORT1FROMMP( mParam1 ) & KC_CTRL)
        modifiers |= KeyDef::CTRL;

      if( SHORT1FROMMP( mParam1 ) & KC_VIRTUALKEY )
      {
        int key = 0, vKey = SHORT2FROMMP( mParam2 );

       // Maybe NumLock is on

        if( SHORT1FROMMP(mParam1) & KC_CHAR &&
            !(vKey == VK_ENTER || vKey == VK_NEWLINE || vKey == VK_TAB ||
              vKey == VK_BACKSPACE ))
          key = SHORT1FROMMP( mParam2 );
        else
        if( vKey == VK_UP )
          key = KeyDef::UP;
        else
        if( vKey == VK_DOWN )
          key = KeyDef::DOWN;
        else
        if( vKey == VK_LEFT )
          key = KeyDef::LEFT;
        else
        if( vKey == VK_RIGHT )
          key = KeyDef::RIGHT;
        else
        if( vKey == VK_PAGEUP )
          key = KeyDef::PAGEUP;
        else
        if( vKey == VK_PAGEDOWN )
          key = KeyDef::PAGEDOWN;
        else
        if( vKey == VK_INSERT )
          key = KeyDef::INSERT;
        else
        if( vKey == VK_DELETE )
          key = KeyDef::kDELETE;
        else
        if( vKey == VK_HOME )
          key = KeyDef::HOME;
        else
        if( vKey == VK_END )
          key = KeyDef::END;
        else
        if( vKey == VK_ESC )
          key = KeyDef::ESCAPE;
        else
        if( vKey == VK_F1 )
          key = KeyDef::F1;
        else
        if( vKey == VK_F2 )
          key = KeyDef::F2;
        else
        if( vKey == VK_F3 )
          key = KeyDef::F3;
        else
        if( vKey == VK_F4 )
          key = KeyDef::F4;
        else
        if( vKey == VK_F5 )
          key = KeyDef::F5;
        else
        if( vKey == VK_F6 )
          key = KeyDef::F6;
        else
        if( vKey == VK_F7 )
          key = KeyDef::F7;
        else
        if( vKey == VK_F8 )
          key = KeyDef::F8;
        else
        if( vKey == VK_F9 )
          key = KeyDef::F9;
        else
        if( vKey == VK_F10 )
          key = KeyDef::F10;
        else
        if( vKey == VK_F11 )
          key = KeyDef::F11;
        else
        if( vKey == VK_F12 )
          key = KeyDef::F12;
        else
        if( vKey == VK_ENTER )
          key = KeyDef::ENTER;
        else
        if( vKey == VK_NEWLINE )
          key = KeyDef::RETURN;
        else
        if( vKey == VK_BACKSPACE )
          key = KeyDef::BACKSPACE;
        else
        if( vKey == VK_TAB )
          key = KeyDef::TAB;
        else
        if( vKey == VK_BACKTAB )
        {
          key = KeyDef::TAB;
          modifiers |= KeyDef::SHIFT;
        }
        else
        if( vKey == VK_SPACE )
          key = KeyDef::SPACE;

	      if( parent->getType() == DIALOG )
  	    	((Dialog *)parent)->focusHotkey( key );

        if( key != 0 )
          if( performKeyEvent( this, modifiers, key ) ) return MPARAM(TRUE);

        // Do not pass up and down key to dialog if this is a combobox (would
        // move focus)

/*        if( getFilterDialogArrows() &&
            (vKey == VK_UP || vKey == VK_DOWN) )
          return (MPARAM)TRUE;*/
      }
      else
      {
        int key = SHORT1FROMMP( mParam2 );
        if( performKeyEvent( this, modifiers, key ) ) return MPARAM(TRUE);
      }
      break;
    }
    case WM_CONTROL :
    {
      Window *control = (Window *)getControl( SHORT1FROMMP(mParam1));
      if( control != NULL )
      {
        if( control->type == Window::LISTBOX ||
            control->type == Window::COMBOLISTBOX )
        {
          switch( SHORT2FROMMP(mParam1) )
          {
            case LN_KILLFOCUS : control->focus( FALSE ); break;
            case LN_SETFOCUS : control->focus( TRUE );break;
            case LN_ENTER :
              control->performSelectionEvent(
                (int)WinSendMsg( (HWND)mParam2, LM_QUERYSELECTION, MPARAM(0), MPARAM(NULL) ) );
              break;
          }
        }
        else
        if( control->type == Window::SCROLLBAR )
        {
          switch( SHORT2FROMMP(mParam1) )
          {
            case SLN_KILLFOCUS : control->focus( FALSE ); break;
            case SLN_SETFOCUS : control->setFocus(); break;
          }
        }
        else
        if( control->type == Window::RADIOBUTTON ||
            control->type == Window::CHECKBOX )
        {
          switch( SHORT2FROMMP( mParam1) )
          {
            case BN_CLICKED :
            case BN_DBLCLICKED :
              control->performButtonEvent( control, control->id );
              break;
          }
        }
      }
      break;
    }
		case DM_DRAGOVER:
		{
			PDRAGINFO pDInfo;
			PDRAGITEM pDItem;
  
	  	if( !acceptDropFiles )
				return MRFROM2SHORT(DOR_NODROPOP, 0);

			pDInfo = (PDRAGINFO)mParam1;
			DrgAccessDraginfo(pDInfo);
			pDItem = DrgQueryDragitemPtr(pDInfo, 0);
			USHORT   usOp = 0, usDrop = DOR_NEVERDROP;

			/* Inspect each item to see if it is acceptable */
			ULONG  ulItems = DrgQueryDragitemCount (pDInfo);
      for (INT i = 0; i < ulItems; i++)
			{
				pDItem = DrgQueryDragitemPtr(pDInfo, i);

				/* The item is acceptable only if it is copyable and the     */
				/* operation is a copy, or it is moveable and the operation  */
				/* is a move, and it can render <DRM_OS2FILE, NULL>          */
				if (pDItem->fsSupportedOps & DO_COPYABLE)
				{
					usOp = DO_COPY;
					/* Only check files, all types */
					if (DrgVerifyRMF(pDItem, "DRM_OS2FILE", 0))
						usDrop = DOR_DROP;
					else
					{
						usDrop = DOR_NEVERDROP;
						break;
					}
				}
				else
				{
					/* Must be a file but not droppable in type op */
					usDrop = DOR_NODROPOP;
          usOp = 0;
					break;
				}
			} /* end for all items dragged  */

			DrgFreeDraginfo(pDInfo);

			return MRFROM2SHORT(usDrop, usOp);
		}
		case DM_DROP:
		{
			PDRAGINFO pDInfo;
			PDRAGITEM pDItem;
			pDInfo = (PDRAGINFO)mParam1;
			DrgAccessDraginfo(pDInfo);
			pDItem = DrgQueryDragitemPtr(pDInfo, 0);
			ULONG  ulItems = DrgQueryDragitemCount (pDInfo);
			ULONG  p;
			char **files;

			files = (char **)malloc( sizeof( char * ) * ulItems );

			for (INT i=0; i < ulItems; i++)
			{
				CHAR    DragSrcPath[CCHMAXPATH], DragSrcName[CCHMAXPATH];
				pDItem = DrgQueryDragitemPtr(pDInfo, i);
				DrgQueryStrName(pDItem->hstrContainerName, sizeof(DragSrcPath), (PSZ)DragSrcPath);
				if (!(PSZ)DragSrcPath)
					break;
				if (DragSrcPath[(strlen(DragSrcPath)-1)] != '\\')
					strcat((PSZ)DragSrcPath, "\\");

				/* Use the dragitem source name, it's the real name */
				DrgQueryStrName(pDItem->hstrSourceName, sizeof(DragSrcName),
																									(PSZ)DragSrcName);
				if (!((PSZ)DragSrcName))
					break;
				strcat((PSZ)DragSrcPath, (PSZ)DragSrcName);
				FILESTATUS3  fsFile;
				DosError(FERR_DISABLEHARDERR);
				APIRET rc = DosQueryPathInfo(DragSrcPath, FIL_STANDARD, &fsFile,
																									sizeof(FILESTATUS3));
				DosError(FERR_ENABLEHARDERR);

				if (!rc && !(fsFile.attrFile & FILE_DIRECTORY))
				{
					files[i] = (char *)malloc( strlen(DragSrcPath) + 1 );
          strcpy( files[i], DragSrcPath );
					DrgSendTransferMsg(pDItem->hwndItem, DM_ENDCONVERSATION,
																			MPFROMLONG(pDItem->ulItemID),
																			MPFROMLONG(DMFL_TARGETSUCCESSFUL));
				}
				else
				{
					DosBeep(100,10);
//					EditorDisplay[BufNum]->DisplayMessage( pszDropNoFile );
				}
			}

			filesDropped( files, ulItems );

			for( i = 0; i < ulItems; i++ )
	      free( (void *)files[i] );

      free( (void *)files );

			DrgFreeDraginfo(pDInfo);
			break;
		}
    case WM_USER + 1 :
    {
      return (MPARAM)execFromMainThread( (int)mParam1, (void *)mParam2 );
    }
    case WM_USER + 2 :
    {
      setFocus();
      break;
    }
  }
  *returned = FALSE;
  return (MRESULT)NULL;
}

MRESULT Window::wndProc( HWND hWnd, ULONG iMessage, MPARAM mParam1, MPARAM mParam2)
{
  switch( iMessage )
  {
    case WM_COMMAND :
    {
      // A menu selection

      if( SHORT1FROMMP( mParam2 ) == CMDSRC_MENU )
      {
        processMenuEnd = TRUE;
        performMenuEvent( (int)mParam1 );
      }
      break;
    }
    case WM_INITMENU :
    {
      processMenuEnd = TRUE;
      break;
    }
    case WM_MENUSELECT :
    {
      if( SHORT2FROMMP( mParam1 ) )
        processMenuEnd = FALSE;
      break;
    }
    case WM_MENUEND :
    {
      if( processMenuEnd )
        performMenuEvent( MENU_CLOSE );
      break;
    }
    case WM_PAINT :
    {
      // Do not paint if subclassed window
      if( oldWndProc != NULL ) break;

      CURSORINFO cursorinfo;

      BOOL rc = WinQueryCursorInfo(HWND_DESKTOP, &cursorinfo);
      if( rc ) WinShowCursor( hWnd, 0 );

      RECTL rcl;
      HPS hps = WinBeginPaint( hWnd, NULLHANDLE, &rcl );
      Rect rect( rcl.xLeft, rcl.yBottom, rcl.xRight - rcl.xLeft, rcl.yTop - rcl.yBottom );
      Graphics *graph = new Graphics( hps );
      paint( &rect, graph );
      delete graph;
      WinEndPaint( hps );

      if( rc ) WinShowCursor( hWnd, 1 );
      return MPARAM(0);
    }
    case WM_ERASEBACKGROUND : return (MPARAM)FALSE;
  }

  BOOL returned = TRUE;
  MRESULT retVal = stdWndProc( hWnd, iMessage, mParam1, mParam2, &returned );

  if( !returned )
  {
    if( oldWndProc == NULL )
      return WinDefWindowProc( hWnd, iMessage, mParam1, mParam2 );
    else
      return oldWndProc( hWnd, iMessage, mParam1, mParam2 );
  }
  else
    return retVal;
}

void Window::setPointer( int type )
{
  LONG getPointer = -1;
  switch( type )
  {
    case PTR_ARROW : getPointer = SPTR_ARROW; break;
    case PTR_TEXT : getPointer = SPTR_TEXT; break;
    case PTR_WAIT : getPointer = SPTR_WAIT; break;
    case PTR_MOVE : getPointer = SPTR_SIZENS; break;
    case PTR_HAND : getPointer = SPTR_ARROW; break;
  }

  if( getPointer != -1 )
    currentPointer = WinQuerySysPointer( HWND_DESKTOP, getPointer, FALSE );
  else
    currentPointer = NULLHANDLE;

  // Display the new pointer by focring a WM_MOUSEMOVE

  POINTL pos;

  WinQueryPointerPos( HWND_DESKTOP, &pos );
  WinSetPointerPos( HWND_DESKTOP, pos.x, pos.y );
}

void Window::getCursorExtent( int &width, int &height )
{
	POINTERINFO pInfo;

	WinQueryPointerInfo( WinQueryPointer( HWND_DESKTOP ), &pInfo );
	width = pInfo.xHotspot;
	height = /*WinQuerySysValue( HWND_DESKTOP, SV_CYPOINTER ) - */pInfo.yHotspot;
}

void Window::getCursorPos( int &x, int &y )
{
  POINTL ptl;
  WinQueryPointerPos( HWND_DESKTOP, &ptl );
  WinMapWindowPoints( HWND_DESKTOP, hWndClient, &ptl, 1 );
  x = ptl.x;
  y = ptl.y;
}

void Window::setFocus()
{
  if( !WinSetFocus( HWND_DESKTOP, hWndFrame ) )
    WinPostMsg( hWndFrame, WM_USER+2, NULL, NULL );
}

void Window::activate( BOOL active )
{
  if( active )
    WinSetActiveWindow( HWND_DESKTOP, hWndFrame );
}

void Window::createMenu( HWND hWndParentMenu, AMenuEntry *menu )
{
  HWND hWndMenu;

  if( menu->type == AMenuEntry::SUBMENU )
  {
    for( int i = 0; i < ((AMenu *)menu)->items(); i++ )
    {
      AMenuEntry *menuItem = ((AMenu *)menu)->getItemAt( i );

      MENUITEM mi;

      mi.iPosition = MIT_END;
      mi.afStyle = 0;
      mi.afAttribute = ((menuItem->checked) ? MIA_CHECKED : NULL) |
                       ((menuItem->enabled) ? 0 : MIA_DISABLED);
      mi.id = menuItem->id;
      mi.hwndSubMenu = 0L;
      mi.hItem = 0;

      if( menuItem->type == AMenuEntry::SEPARATOR )
      {
        mi.afStyle = MIS_SEPARATOR;
        WinSendMsg( hWndParentMenu, MM_INSERTITEM, (MPARAM)&mi,
                    (MPARAM)menuItem->string );
      }
      else
      if( menuItem->type == AMenuEntry::MENUITEM )
      {
        char *str = new char[strlen( menuItem->string ) +
               ((menuItem->shortcut) ? strlen( menuItem->shortcut ) : 0) + 10];

        strcpy( str, menuItem->string );

        if( menuItem->shortcut )
        {
          strcat( str, "\t" );
          strcat( str, menuItem->shortcut );
        }

        WinSendMsg( hWndParentMenu, MM_INSERTITEM, (MPARAM)&mi,
                    (MPARAM)str );
        delete str;
      }
      else
      {
        hWndMenu = WinCreateWindow( hWndFrame, WC_MENU, "", 0, 0, 0, 0, 0,
                                    hWndFrame,HWND_TOP,
                                    FID_MENU, 0L, 0L );
        mi.hwndSubMenu = hWndMenu;
        WinSendMsg( hWndParentMenu, MM_INSERTITEM, (MPARAM)&mi,
                    (MPARAM)menuItem->string );
        createMenu( hWndMenu, menuItem );
      }
    }
  }
}

void Window::remapCoordinates( Window *from, Window *to, int &x, int &y )
{
	POINTL ptl = {x,y};

  WinMapWindowPoints( from->getHWND(), to->getHWND(), &ptl, 1 );
	x = ptl.x;
  y = ptl.y;
}

void Window::repaint( Rect *rect, BOOL children )
{
  if( rect == NULL )
    WinInvalidateRect( hWndClient, NULL, children );
  else
  {
    RECTL orect = { rect->x, rect->y,
                     rect->x + rect->width, rect->y + rect->height };
    WinInvalidateRect( hWndClient, &orect, children );
  }
}

void Window::processPaint()
{
	QMSG Message;

	while( WinPeekMsg( Application::hab, &Message, hWndClient, WM_PAINT, WM_PAINT,
									   PM_REMOVE ) )
	{
    RECTL rcl;
    HPS hps = WinBeginPaint( hWndClient, NULLHANDLE, &rcl );
    Rect rect( rcl.xLeft, rcl.yBottom, rcl.xRight - rcl.xLeft, rcl.yTop - rcl.yBottom );
    Graphics graph( hps );
    paint( &rect, &graph );
    WinEndPaint( hps );
	}
}

BOOL Window::isMinimized()
{
	SWP swp;
  WinQueryWindowPos( hWndFrame, &swp );
  return swp.fl & SWP_MINIMIZE;
}

BOOL Window::isMaximized()
{
	SWP swp;
  WinQueryWindowPos( hWndFrame, &swp );
  return swp.fl & SWP_MAXIMIZE;
}

void Window::restore()
{
	WinSetWindowPos( hWndFrame, NULLHANDLE, 0, 0, 0, 0, SWP_RESTORE );
}

void Window::scrollVert( int delta, int bottom, int top )
{
  RECTL scrollRect = { 0, bottom, width, top };
  RECTL invalidated;

  WinScrollWindow( hWndClient, 0, delta, &scrollRect, &scrollRect, NULLHANDLE,
                   &invalidated, NULL );
  // Why do I have to do this? I don't know!!! SW_INVALIDATEREGION doesn't
  // work if several WinScrollWindows are done in a row. The below line should
  // do the same thing, but apparently it doesn't
  WinInvalidateRect( hWndClient, &invalidated, FALSE );
}

void Window::scrollHorz( int delta, int left, int right )
{
  RECTL scrollRect = { left, 0, right, height };

  WinScrollWindow( hWndClient, delta, 0, &scrollRect, &scrollRect, NULLHANDLE,
                   NULL, SW_INVALIDATERGN );
}

int Window::getWidth() 
{ 
  SWP swp;

  WinQueryWindowPos( hWndFrame, &swp );

  width = swp.cx;

	return width; 
}

int Window::getHeight()
{
  SWP swp;

  WinQueryWindowPos( hWndFrame, &swp );

  height = swp.cy;

	return height;
}

void Window::setSize( int width, int height )
{
//  int oldWidth = this->width, oldHeight = this->height;
	WinSetWindowPos( hWndFrame, HWND_TOP, 0, 0, width, height, SWP_SIZE );
//  size( oldWidth, oldHeight);
}

FrameWindow::FrameWindow( AWindow *parent, STRING title, int x, int y, int width, int height ) :
  Window( x, y, width, height )
{
  oldWndProc = NULL;
  this->parent = parent;
  type = WINDOW;
  id = -1;
  char str[10];

  HWND hWndParent = ((Window *)parent)->getHWND();

  Application::tempthis = this;

  ULONG flCreate = FCF_STANDARD & // Set frame control flags
         ~FCF_SHELLPOSITION & ~FCF_ACCELTABLE | FCF_NOBYTEALIGN;

  flCreate = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER | FCF_TASKLIST |
             FCF_MINBUTTON | FCF_MAXBUTTON;

  if( !WinRegisterClass( Application::hab, (PSZ)itoa( Iterator::getNext(), str, 10),
                    WndProc, CS_SIZEREDRAW | CS_MOVENOTIFY | WS_CLIPCHILDREN,
                    sizeof(Window *) ) )
  {
    exit( 1 );
  }

  hWndFrame = WinCreateStdWindow( HWND_DESKTOP/*hWndParent*/, 0L, &flCreate,
                                (PSZ)str, title,
                                0L, NULLHANDLE, 1, &hWndClient );

  if( hWndFrame == NULLHANDLE ) exit( 1 );

  WinSetWindowPos( hWndFrame, HWND_TOP, x, y, width, height,
                   SWP_MOVE | SWP_SIZE );

  graphics = new Graphics( WinGetPS( hWndClient ) );
}

EntryField::EntryField( Window *parent, int properties, int x, int y, int width,
            int height) :
  Window( parent, xlatProperties( properties|FILTERDIALOGARROWS ), x, y, width, height ),
          selectedText(NULL)
{
  isBordered = properties & BORDER;
  properties ^= BORDER;
  internalEntryField = new Window( this, (char*)WC_ENTRYFIELD, "",
                             (xlatProperties(properties|CLIPSIBLINGS) ^ WS_TABSTOP) | ES_AUTOSCROLL,
                             0, 0, width, height );
//  if( parent->type == DIALOG )
    internalEntryField->setFont( graphics->getDialogFontName(),
                                 graphics->getDialogFontSize() );

	internalEntryField->setDialogFilter(FILTERDIALOGARROWS|PASSDIALOGKEYS);
  type = ENTRYFIELD;
  internalEntryField->size( 0, 0 );
  internalEntryField->setVisible(TRUE);
  parent->addControl( this );
  setMaxLength( 32 );
}

void EntryField::setFocus()
{
  internalEntryField->setFocus();
	selectText();
}

ListBox::ListBox( Window *parent, int properties, int x, int y, int width, int height,
         BOOL sort ) :
  Window( parent, (char*)WC_LISTBOX, "",
          xlatProperties(properties | CLIPSIBLINGS)|LS_HORZSCROLL,
          x, y, width, height ), sort(sort)
{
  type = LISTBOX;

  // Get the new pos and size if adjusted

  SWP swp;

  WinQueryWindowPos( hWndFrame, &swp );

  x = swp.x;
  y = swp.y;
  width = swp.cx;
  height = swp.cy;
  parent->addControl( this );
}

ListBox::ListBox( Window *parent, Window *owner, int properties, int x, int y, int width, int height,
         BOOL sort ) :
  Window( parent, owner, (char*)WC_LISTBOX, "",
          xlatProperties(properties | CLIPSIBLINGS)|LS_HORZSCROLL,
          x, y, width, height ), sort(sort)
{
  type = LISTBOX;

  // Get the new pos and size if adjusted

  SWP swp;

  WinQueryWindowPos( hWndFrame, &swp );

  x = swp.x;
  y = swp.y;
  width = swp.cx;
  height = swp.cy;
  owner->addControl( this );
}

int ListBox::getHeight()
{
  SWP swp;

  WinQueryWindowPos( hWndFrame, &swp );

  height = swp.cy;

	return height;
}

int SComboBox::buttonWidth = 17;
int SComboBox::STATIC = 32768;

int SComboBox::ComboListBox::focus( BOOL newFocus )
{
  if( newFocus == FALSE )
  {
    setVisible( FALSE );  // Hide me
    comboBox->buttonUp = TRUE;
    comboBox->listVisible = FALSE;
    comboBox->repaintButton();
  }
  return 0;
}

void SComboBox::ComboListBox::setFocus()
{
  WinFocusChange( HWND_DESKTOP, getHWNDFrame(), FC_NOLOSEFOCUS|FC_NOLOSEACTIVE|FC_NOLOSESELECTION);
}

SComboBox::SComboBox( Window *parent, int properties, int x, int y,
          int width, int height, int listHeight ) :
  Window( parent, xlatProperties( properties|FILTERDIALOGARROWS|PASSDIALOGKEYS ), x, y, width, height + listHeight )
{
  buttonUp = TRUE;
  orgHeight = height;
  orgListHeight = this->listHeight = listHeight;
  listVisible = FALSE;
  down = FALSE;
  maxLength = 80;

  type = COMBOBOX;

  // Create the editcontrol

  entryField = new EntryField( this, (properties ^ ISTABSTOP) | PASSDIALOGKEYS, 2, 2,
                               width-4 - buttonWidth - 1, height-4 );
  entryField->setVisible( TRUE );
  entryField->setOnTop();

  setReadOnly( (properties & STATIC) == STATIC );

  addMouseListener( this  );
  entryField->addKeyListener( this  );
//  entryField->addMouseListener( this );

  // Create listbox
  listBox = new ComboListBox( this, this, Window::NOADJUSTSIZE, x, y-listHeight,
                         width, listHeight );
  listBox->addFocusListener( this );
  listBox->addSelectionListener( this );

  setSize( width, height + listHeight );

  placeListBox();

  parent->addControl( this );
}

void SComboBox::actionPerformed( ASelectionEvent *s )
{
	//performSelectionEvent( s->getSelected() );
}

int SComboBox::focus( BOOL newFocus )
{
  if( newFocus )
  {
    entryField->setFocus();
    return 1;
  }
  return 0;
}

void SComboBox::paint( Rect *rect, Graphics *g )
{
  int height = getEditBoxHeight(); // Temp override

  g->drawShadedBox( 0, 0, width, height, Color::SHADOW, Color::SHADOW );
  g->drawShadedBox( 1, 1, width-2, height-2, Color::BLACK, Color::BKGR );

  // Draw the button

  g->drawButton( width - buttonWidth - 2, 2,
                 buttonWidth, height - 4, buttonUp );
  g->setColor( Color::BLACK );
  if( buttonUp )
    drawArrow( g, width - buttonWidth, 4,
               buttonWidth - 4, height - 8, dir );
  else
    drawArrow( g, width - buttonWidth + 1, 3,
               buttonWidth - 4, height - 8, dir );
  g->setColor( Color::SHADOW );
  g->drawLine( width - buttonWidth - 3, 2,
               width - buttonWidth - 3, height - 3 );
}

void SComboBox::size( int oldWidth, int oldHeight )
{
  entryField->setPos( 2, 2 );
  entryField->setSize( width-4 - buttonWidth - 1, getEditBoxHeight()-4 );

  // Need to resize listbox, since the dialog can't handle that
//  listHeight = (orgListHeight * height) / orgHeight;
  listBox->setSize( width, listHeight );
  //listBox->setPos( x, y-listHeight );
  //listBox->setOnTop();
  placeListBox();
}

void SComboBox::setText( char *text )
{
  entryField->setText( text );

	// Find the text in the listbox if it is available.

	int findIdx = listBox->findItem( text, FALSE );

	if( findIdx != -1 )
  {
		listBox->selectItem( findIdx );
    performSelectionEvent( findIdx );
	}
}

// Returns border size + buttonWidth if !horz, otherwise border size

int SComboBox::getInset( int horz )
{
  if( horz )
    return EntryField::getInset()*2+buttonWidth+4;
  else
    return 4;
}

int SComboBox::getEditBoxHeight()
{
  getGraphics()->setSystemFont();
  int fontHeight = getGraphics()->getMaxFontHeight();
  return fontHeight + 4;
}

void SComboBox::drawArrow( Graphics *g, int x, int y, int width, int height, int dir )
{
   Point *point[3];

   if( dir )
   {
     point[0] = new Point( x + 2, y + (height*2) / 3 );
     point[1] = new Point( x + width - 3, y + (height*2) / 3 );
     point[2] = new Point( x + width / 2, y + height / 3 );
   }
   else
   {
     point[0] = new Point( x + 2, y + height / 3 );
     point[1] = new Point( x + width / 2, y + (height*2) / 3 );
     point[2] = new Point( x + width - 3, y + height / 3 );
   }

   g->setColor( Color::BLACK );
   g->fillPolygon( 3, point );

   delete point[0];
   delete point[1];
   delete point[2];
}

BOOL SComboBox::actionPerformed( AMouseEvent *e )
{
  int repaint = FALSE;
  if( (e->getEvent() == AMouseEvent::DOWN ||
       e->getEvent() == AMouseEvent::DOUBLECLICK/* ||
       e->getEvent() == AMouseEvent::CLICK*/) && !down &&
       (e->getPos()->x >= width - buttonWidth || readOnly) )
  {
    if( listVisible )
    {
      buttonUp = TRUE;
      listBox->setVisible( FALSE );
      listVisible = FALSE;
      setFocus();
    }
    else
    {
      placeListBox();
      buttonUp = FALSE;
      listBox->setVisible( TRUE );
      listBox->setOnTop();
      listBox->setFocus();
      listVisible = TRUE;
    }
    repaint = TRUE;
    down = TRUE;
  }

  if( e->getEvent() == AMouseEvent::RELEASE )
    down = FALSE;

  if( repaint ) repaintButton();
  return FALSE;
}

void SComboBox::actionPerformed( AFocusEvent *focusEvent )
{
  char text[255];
  int selected = ((ListBox *)focusEvent->getSource())->getSelectedItem( 255, text );

  listBox->setVisible( FALSE );
  listVisible = FALSE;
  buttonUp = TRUE;
  repaintButton();

  if( selected == -1 ) return;  // Nothing selected

	if( ((ListBox *)focusEvent->getSource())->hasFocus() ) setFocus();

  entryField->setText( text );
	//setFocus();
  performSelectionEvent( selected ); // chain
  //performFocusEvent( focusEvent->getFocused() ); // chain
}


BOOL SComboBox::actionPerformed( AKeyEvent *keyEvent )
{
  if( listBox->getItems() == 0 ) return FALSE;

  int newPos = listBox->getSelectedIndex();

  if( keyEvent->key == KeyDef::DOWN )
  {
    if( newPos == -1 )
      newPos = 0;
    else
    if( newPos != listBox->getItems() - 1 )
      newPos++;
    else
      return TRUE;
  }
  else
  if( keyEvent->key == KeyDef::UP )
  {
    if( newPos == -1 )
      newPos = listBox->getItems() - 1;
    else
    if( newPos != 0 )
      newPos--;
    else
      return TRUE;
  }
  else
    return FALSE;

  listBox->selectItem( newPos );

  char text[300];

  listBox->getSelectedItem( 300, text );

  entryField->setText( text );
  performSelectionEvent( newPos ); // chain
  return TRUE;
}

void SComboBox::placeListBox()
{
	// Set the listbox at it's default position
  int lx = x, ly = y-listBox->getHeight();

  remapCoordinates( (Window *)parent, Desktop::getInstance(), lx, ly );
	listBox->setPos( lx, ly );
  dir = 1;

  // Check if listbox is above frame
  if( ly < 0 )
  {
    lx = x;
    ly = y + getEditBoxHeight();
    remapCoordinates( (Window *)parent, Desktop::getInstance(), lx, ly );

    listBox->setPos( lx, ly );
    dir = 0;
  }
}

void SComboBox::setSize( int width, int height )
{
  int ebHeight = getEditBoxHeight();
  listHeight = height - ebHeight;
  Window::setSize( width, ebHeight );
  this->height = height;
}
