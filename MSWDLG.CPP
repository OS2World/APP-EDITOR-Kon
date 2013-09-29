#include "bowl.h"
#include "direct.h"
#include "shlobj.h"

BOOL CALLBACK DlgProcBase(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	Dialog *pWindow = (Dialog *)GetWindowLong( hWnd, GWL_USERDATA );

  if (pWindow == 0) {
		if (iMessage == WM_INITDIALOG) {
			pWindow = (Dialog *)Application::tempthis;
			SetWindowLong(hWnd,GWL_USERDATA,(ULONG)pWindow);
			return pWindow->dlgProc(hWnd,iMessage,wParam,lParam);
		} else
			return FALSE;
	} else
		return pWindow->dlgProc(hWnd,iMessage,wParam,lParam);
}

Dialog::Dialog( Window *frame, Window *parent, char *title, int properties, int x, int y, int width, int height )
  : Window( x, y, width, height )
{
  this->frame = frame;

  modal = properties & ISMODAL;
  if( modal == 0 ) modal = properties & ISMODALDONTDISABLEPARENT;
  
  this->frame = frame;
  this->parent = parent;
  this->modal = modal;
  Application::tempthis = this;
  type = DIALOG;
  hWndFrame = hWndClient = NULL;

  // Create the dialog

  hWndFrame = CreateDialog( Application::getInstance(),
                            MAKEINTRESOURCE( (properties & NOTITLE) ? IDD_EMPTYDLGTEMPLATE : IDD_DLGTEMPLATE ),
                            ((Window *)parent)->getHWND(), (DLGPROC)DlgProcBase );


  if( properties & NOTITLE )
    hWndClient = hWndFrame;
  else
  {
    SetWindowText( hWndFrame, title );

    Application::tempthis = this;

    // Create the working area dialog

    hWndClient = CreateDialog( Application::getInstance(), MAKEINTRESOURCE( IDD_EMPTYDLGTEMPLATE ),
                                     hWndFrame, (DLGPROC)DlgProcBase );
    ShowWindow( hWndClient, TRUE );
  }

  RECT rect;

  GetClientRect( hWndClient, &rect );
  graphics = new Graphics( hWndClient, rect.right - rect.left, rect.bottom - rect.top );

  if( properties & ADJUSTSIZE )
  {
    width += Dialog::getInset()*2;
    height += Dialog::getInset()*2+Dialog::getCaptionHeight();
  }

  if( properties & CENTER )
    center( x, y );

  setSize( width, height );
  setPos( x, y );

  orgWidth = width;
  orgHeight = height;

  graphics->setDialogFont();
}

void Dialog::adjustForFontSize()
{
  double widthMult, heightMult;

  // Resize dialog

  getGraphics()->setDialogFont();
  int thisHeight = getGraphics()->getFontHeight();
  int thisWidth = getGraphics()->getAverageCharWidth();

  if( thisHeight != Graphics::dialogDesignFontHeight ||
      thisWidth != Graphics::dialogDesignFontWidth )
  {
	  heightMult = ((double)thisHeight / (double)Graphics::dialogDesignFontHeight);
  	widthMult = ((double)thisWidth / (double)Graphics::dialogDesignFontWidth);

    // Adjust the size of the client window

    int cWidth = getClientWidth() * widthMult;
    int cHeight = getClientHeight() * heightMult;

	  setSize( cWidth + Dialog::getInset()*2, 
             cHeight + Dialog::getInset()*2+Dialog::getCaptionHeight() );
	}
}

void Dialog::center( int &x, int &y )
{
  // Translate parent pos to frame pos

  RECT rect;

  GetWindowRect( ((Window *)parent)->getHWND(), &rect );

  POINT pnt;

  pnt.x = rect.left;
  pnt.y = rect.bottom;

  x = pnt.x + (parent->getWidth() - width) / 2;
  y = ((Window *)frame)->getHeight()-pnt.y + (parent->getHeight() - height) / 2;
}

void Dialog::addControl( AWindow *child )
{
/*  if( !AGraphics::useBigFont && child->allowPlacementResize() )
  {
    ((Window *)child)->setSize( child->getWidth() * widthMult, child->getHeight() * heightMult );
    ((Window *)child)->setPos( child->getX() * widthMult, child->getY() * heightMult + (heightBefResize-height));
  }*/

  Window::addControl( child );

  orgPosV.add( new OrgPos( child->getX(), child->getY(),
               child->getWidth(), child->getHeight() ) );
  SendMessage( ((Window *)child)->getHWND(), WM_SETFONT, (WPARAM)graphics->getCurrentFontHandle(), MAKELPARAM( TRUE, 0 ) );
}

HWND Dialog::nextWindow( HWND currentHwnd, UINT cmd )
{
  HWND nextWin = GetWindow( currentHwnd, cmd );

  if( nextWin == NULL ) return currentHwnd;

   // Check if static class

  char classname[81];

  if( !GetClassName( nextWin, classname, 80 ) ) return currentHwnd;

  LONG style = GetWindowLong( nextWin, GWL_STYLE );

  if( !stricmp( classname, "STATIC" ) ||
      (!stricmp( classname, "BUTTON" ) && (style & BS_GROUPBOX) == BS_GROUPBOX) )
  {
    HWND resWin = nextWindow( nextWin, cmd );

    if( nextWin == resWin )
      return currentHwnd;
    else
      return resWin;
  }

  return nextWin;
}

BOOL Dialog::handleKEYDOWN( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	//Beep( 1000, 100 );
  HWND nextWin = NULL;
  if( wParam == VK_UP || wParam == VK_LEFT )
    nextWin = nextWindow( GetFocus(), GW_HWNDPREV );
  else
  if( wParam == VK_DOWN || wParam == VK_RIGHT )
    nextWin = nextWindow( GetFocus(), GW_HWNDNEXT );
  else
  if( wParam == VK_NEXT && (GetKeyState( VK_MENU ) & 0x80) )
  {
    if( parent->getType() == NOTEBOOK )
    {
      ((Notebook *)parent)->nextTab();
     return FALSE;
    }
  }
  else
  if( wParam == VK_PRIOR && (GetKeyState( VK_MENU ) & 0x80) )
  {
    if( parent->getType() == NOTEBOOK )
    {
      ((Notebook *)parent)->prevTab();
     return FALSE;
    }
  }
  else
  if( wParam == VK_ESCAPE )
  {
    if( parent->getType() == NOTEBOOK &&
        parent->getParent()->getType() == DIALOG )
      PostMessage( ((Window *)parent->getParent())->getHWND(),
                   WM_CLOSE, NULL, NULL );
    else
    	PostMessage( hWndFrame, WM_CLOSE, NULL, NULL );
    return FALSE;
  }

  if( nextWin != NULL )
  {
    SetFocus( nextWin );
    return FALSE;
  }
  return TRUE;
}

BOOL Dialog::handleCHAR( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
  if( (int)wParam == 0xd )  // Enter
  {
    // Find the default button

    for( int i = 0; i < controls.getSize(); i++ )
    {
      Window *control = (Window *)controls.getObjectAt(i);
      if( control->getType() == PUSHBUTTON &&
          ((control->getProperties() & ISDEFAULT) == ISDEFAULT) )
      {
          control->performButtonEvent( control, control->id );
          break;
      }
    }
    return FALSE;
  }

  if( (GetKeyState( VK_SHIFT ) & 0x80) && (int)wParam == 0x09 )
  {
    SendMessage( hWnd, WM_NEXTDLGCTL, 1, FALSE );
    return FALSE;
  }
  else
  if( (int)wParam == 0x09 )
  {
    SendMessage( hWnd, WM_NEXTDLGCTL, 0, FALSE );
    return FALSE;
  }
  else
  if( (int)wParam == 27 )
  {
    return FALSE;
  }

  // Hotkey

  if( GetKeyState( VK_LMENU ) & 0x80 )
  {
    focusHotkey( (int)wParam );
    return FALSE;
  }

//  focusHotkey( (int)wParam );
//  return FALSE;

  return TRUE;
}

void Dialog::focusHotkey( int key )
{
  char text[81];
  BOOL found = FALSE;

  // Get first child window of the dialog

  HWND focusWin = NULL, curWin = GetWindow( hWndClient, GW_CHILD );

  // Loop through the windows using GetWindow

  while( curWin )
  {
    GetWindowText( curWin, text, 80 );

    char *substr = strchr( text, '&' );

    if( substr != NULL )
    {
      if( toupper( *(substr+1) ) == toupper( key ) )
      {
        if( !GetClassName( curWin, text, 80 ) ) return;
        if( stricmp( text, "STATIC" ) )
          focusWin = curWin;
        else
          focusWin = GetWindow( curWin, GW_HWNDNEXT );
        found = TRUE;
        break;
      }
    }
    if( GetParent( curWin ) == hWndClient )
      focusWin = curWin;
    curWin = GetWindow( curWin, GW_HWNDNEXT );
  }

  if( !found ) return;

  GetClassName( focusWin, text, 80 );

  if( !stricmp( text, "BUTTON" ) )  // Simulate button click
    SendMessage( focusWin, BM_CLICK, NULL, NULL );

  if( found ) SetFocus( focusWin );
}

BOOL Dialog::dlgProc( HWND hWnd,UINT iMessage, WPARAM wParam, LPARAM lParam )
{
	switch( iMessage )
	{
	  case WM_INITDIALOG:
    {
      if( hWndFrame != hWnd )
      {
/*        graphics = new Graphics( GetDC( hWnd ), width, height );
        SetMapMode( graphics->hdc, MM_TEXT );*/
/*        graphics = new Graphics( hWnd, width, height );
        SetMapMode( GetDC( hWnd ), MM_TEXT );*/
      }
      return TRUE;
    }
    case WM_CLOSE :
    {
      closeDialog( 0 );
      break;
    }
    case WM_MOVE :
    {
      if( hWnd != hWndFrame ) break;
      RECT rect;

      GetWindowRect( hWnd, &rect );

      POINT pnt;

      pnt.x = rect.left;
      pnt.y = rect.top;

//      MapWindowPoints( HWND_DESKTOP, ((Window *)frame)->getHWND(), &pnt, 1 );

      x = pnt.x;
      y = frame->getHeight() - (pnt.y+height);
      move();
      break;
    }
    case WM_GETDLGCODE :
    {
      return DLGC_WANTTAB;
    }
    case WM_SYSKEYDOWN :
    case WM_KEYDOWN :
    {
    	if( handleKEYDOWN( hWnd, iMessage, wParam, lParam ) )
	      break;
      else
      	return FALSE;
    }
    case WM_SYSCHAR :
    case WM_CHAR :
    {
    	if( handleCHAR( hWnd, iMessage, wParam, lParam ) )
	      break;
      else
       	return FALSE;
    }
  }

  BOOL returned = TRUE;

  BOOL retval = stdWndProc( hWnd, iMessage, wParam, lParam, &returned );

  if( returned )
    return retval;

  return FALSE;
}

void Dialog::setPos( int x, int y )
{
  this->x = x;
  this->y = y;
  SetWindowPos( hWndFrame, HWND_TOP, x, frame->getHeight() - (height+y), 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
}

void Dialog::setSize( int width, int height )
{
  int oldWidth = this->width, oldHeight = this->height;

  // We have to move also

  RECT rect;

  GetWindowRect( hWndFrame, &rect );

  POINT pnt;

  pnt.x = rect.left;
  pnt.y = rect.top;

  if( frame != Desktop::getInstance() )
    MapWindowPoints( HWND_DESKTOP, ((Window *)frame)->getHWND(), &pnt, 1 );

  pnt.x;
  pnt.y -= (height - oldHeight);

  SetWindowPos( hWndFrame, HWND_TOP, pnt.x, pnt.y, width, height, SWP_NOZORDER | SWP_NOACTIVATE );

  graphics->setWidth( width );
  graphics->setHeight( width );
/*  this->width = width;
  this->height = height;
  size( oldWidth, oldHeight);*/
}

void Dialog::initSize()
{
  for( int i = 0; i < controls.getSize(); i++ )
  {
    Window *control = (Window *)controls.getObjectAt(i);
    control->setSizeInit( TRUE );
  }
}

void Dialog::size( int prevWidth, int prevHeight )
{
  // Resize all components

  int cHeight = getClientHeight();

  for( int i = 0; i < controls.getSize(); i++ )
  {
    Window *control = (Window *)controls.getObjectAt(i);
    OrgPos *orgPos = (OrgPos *)orgPosV.getObjectAt(i);
    
    if( control->getType() == SCOMBOBOX )
    {
      printf( "hej " );
    }

    if( control->getFollowParentSize() )
    {
      int nx = control->getX(),
          ny = control->getY(),
          nw = control->getWidth(),
          nh = control->getHeight();

      BOOL sizeInited = control->getSizeInit();

      if( control->getXMove() == PROPORTIONAL || !sizeInited )
         nx = (orgPos->x * width) / orgWidth;
      else 
      if( control->getXMove() == FOLLOW )
        nx += (width - prevWidth);

      if( control->getYMove() == PROPORTIONAL || !sizeInited )
        ny = (orgPos->y * height) / orgHeight;
      else 
      if( control->getYMove() == LOCK )
        ny += (height - prevHeight);

      if( control->getWMove() == PROPORTIONAL || !sizeInited )
        nw = (orgPos->width * width) / orgWidth;
      else 
      if( control->getWMove() == FOLLOW )
        nw = width - (prevWidth - nw);

      if( control->getHMove() == PROPORTIONAL || !sizeInited )
        nh = (orgPos->height * height) / orgHeight;
      else
      if( control->getHMove() == FOLLOW )
      {
				nh += (height - prevHeight);
			  ny -= (height - prevHeight);
        
				if( nh < 0 ) nh = 1;
      }

      if( (control->getHMove() == PROPORTIONAL || control->getHMove() == FOLLOW || !sizeInited) && 
          ny+nh > cHeight && control->getType() != SCOMBOBOX )
        nh = cHeight - ny;

      control->setSize( nw, nh );
      control->setPos( nx, ny );
    }
  }
}

int Dialog::setVisible( BOOL visible )
{
  if( visible )
  {
    ShowWindow( hWndFrame, SW_SHOW );

    if( modal )
    {
      if( modal != ISMODALDONTDISABLEPARENT ) ((Window *)parent)->enable( FALSE );

      MSG msg;

      while (GetMessage(&msg, NULL, NULL, NULL) )
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      if( modal != ISMODALDONTDISABLEPARENT ) ((Window *)parent)->enable( TRUE );
      return replyValue;
    }
  }
  else
    ShowWindow( hWndFrame, SW_HIDE );
  return -1;
}

void Dialog::closeDialog( int replyValue )
{
  this->replyValue = replyValue;

  if( modal )
  {
		((Window *)parent)->enable( TRUE );

		PostQuitMessage( replyValue );
	}		
	else
	{
		setVisible( FALSE ); // Not really close!!
	}
}

void Dialog::updateControlPos( Window *control )
{
  for( int i = 0; i < controls.getSize(); i++ )
  {
    Window *lControl = (Window *)controls.getObjectAt(i);

    if( control == NULL || lControl == control )
    {
      OrgPos *orgPos = (OrgPos *)orgPosV.getObjectAt(i);
      orgPos->x = lControl->getX();
      orgPos->y = lControl->getY();
      orgPos->width = lControl->getWidth();
      orgPos->height = lControl->getHeight();
      if( lControl == control ) return;
    }
  }
}

Dialog::~Dialog()
{
}

char SystemFileDialog::returnedFile[MAXPATH+1];
char SystemFileDialog::prevDir[MAXPATH+1] = "";
int SystemFileDialog::SAVEAS = 0;
int SystemFileDialog::OPEN = 1;
int SystemFileDialog::selectedEncoding = ANSI;
BOOL SystemFileDialog::dirInitialized = FALSE;
OPENFILENAME SystemFileDialog::ofname;

unsigned int CALLBACK fileDlgHook(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
  switch( uiMsg )
  {
    case WM_INITDIALOG :
    {
      // Enumerate the windows until the right one is found
      RECT eSize, cbSize;
      HWND hpdlg = (HWND)GetWindowLong( hdlg, GWL_HWNDPARENT );

      HWND cwnd = GetDlgItem( hpdlg, 0x480 ); // Editbox
      if( cwnd == NULL ) return FALSE;
      GetWindowRect( cwnd, &eSize );

      cwnd = GetDlgItem( hpdlg, 0x470 ); // ComboBox
      if( cwnd == NULL ) return FALSE;
      GetWindowRect( cwnd, &cbSize );

      int space = cbSize.top - eSize.bottom;
      int cbHeight = cbSize.bottom - cbSize.top;

      // Set size and position of child window
      RECT dlgSize;
      GetWindowRect( hpdlg, &dlgSize );

	    POINT ptl = {0, dlgSize.bottom - space - cbHeight};

      MapWindowPoints( Desktop::getInstance()->getHWND(), hpdlg, &ptl, 1 );

      SetWindowPos( hdlg, NULL, 0, ptl.y, dlgSize.right - dlgSize.left,
                    space + cbHeight, SWP_NOZORDER | SWP_NOACTIVATE );

      // ComboBox
      ptl.x = cbSize.left, ptl.y = dlgSize.bottom;

      MapWindowPoints( Desktop::getInstance()->getHWND(), hpdlg, &ptl, 1 );

      HWND comboBox = CreateWindowEx( WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_NOPARENTNOTIFY, "COMBOBOX", Strings::get(235), WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|CBS_HASSTRINGS|WS_EX_LEFT, 
                                    ptl.x, 0, 
                                    cbSize.right - cbSize.left, cbHeight+100, 
                                    hdlg, (HMENU)0x500, (HINSTANCE)GetWindowLong( hpdlg, GWL_HINSTANCE), NULL );
      LRESULT res;
      res = SendDlgItemMessage( hdlg, 0x500, CB_RESETCONTENT, (WPARAM)0, (LPARAM)NULL);
      res = SendDlgItemMessage( hdlg, 0x500, CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPCSTR)Strings::get(235));
      res = SendDlgItemMessage( hdlg, 0x500, CB_ADDSTRING, (WPARAM)0, (LPARAM)Strings::get(236));
      res = SendDlgItemMessage( hdlg, 0x500, CB_ADDSTRING, (WPARAM)0, (LPARAM)Strings::get(237));
      res = SendDlgItemMessage( hdlg, 0x500, CB_SETCURSEL, (WPARAM)SystemFileDialog::selectedEncoding, (LPARAM)0 );

      // Static
      cwnd = GetDlgItem( hpdlg, 0x441 );
      if( cwnd == NULL ) return FALSE;
      RECT sSize;
      GetWindowRect( cwnd, &sSize );
      int deltaY = sSize.top - cbSize.top, deltaX = cbSize.left - sSize.left;
      HWND label = CreateWindow( "STATIC", Strings::get(234), WS_CHILD|WS_VISIBLE|SS_LEFT, 
                                  ptl.x - deltaX, deltaY, sSize.right - sSize.left, sSize.bottom - sSize.top, 
                                  hdlg, (HMENU)0x510, Application::hInstance, NULL );
      
      HFONT hfont = (HFONT)SendMessage( cwnd, WM_GETFONT, 0, 0 );
      SendMessage( label, WM_SETFONT, (WPARAM)hfont, (LPARAM)TRUE );
      SendMessage( comboBox, WM_SETFONT, (WPARAM)hfont, (LPARAM)TRUE );
      return TRUE;
    }
    case WM_COMMAND :
    {
      if( LOWORD(wParam) == 0x500 )
      {
        HWND cwnd = (HWND)lParam;
        if( cwnd != NULL ) 
          SystemFileDialog::selectedEncoding = (int)SendMessage( cwnd, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
      }
      return FALSE;
    }
  }
  return FALSE;
}


int SystemFileDialog::display( Window *frame, Window *parent, char *title, 
                               char *path, int dlgType, char *filters, int fltIndex, 
                               EncodingTypes encType )
{
  strcpy( (char *)returnedFile, path );

  if( dirInitialized )
    _getcwd( prevDir, MAXPATH );
  else
    memset( (VOID *)&ofname, 0, sizeof(OPENFILENAME) );

  ofname.lStructSize = sizeof(OPENFILENAME);
  ofname.hwndOwner = parent->getHWNDFrame();

	ofname.lpstrFilter = filters;
	ofname.lpstrCustomFilter = NULL;
	ofname.nFilterIndex = fltIndex;
  ofname.lpstrFile = (char *)returnedFile;
  ofname.nMaxFile = MAXPATH;
  ofname.lpstrFileTitle = NULL;
  ofname.lpstrInitialDir = prevDir;
  ofname.lpstrTitle = title;
  ofname.Flags = ((dlgType==SAVEAS) ? OFN_OVERWRITEPROMPT : 0);
  ofname.lpstrDefExt = NULL;
  ofname.lpfnHook = fileDlgHook;
  ofname.hInstance = Application::hInstance;
  ofname.lpTemplateName = MAKEINTRESOURCE( IDD_EMPTYDLGTEMPLATE );
  SystemFileDialog::selectedEncoding = encType;

  if( dlgType == 0 )
    ofname.Flags |= OFN_EXPLORER | OFN_ENABLEHOOK | OFN_ENABLESIZING | OFN_ENABLETEMPLATE;

  BOOL result;

  if( dlgType == 0 )
    result = GetSaveFileName( &ofname );
  else
    result = GetOpenFileName( &ofname );

  if( result )
  {
    strcpy( prevDir, ofname.lpstrFile );

    char *term = strrchr( prevDir, '\\' );
    *term = NULL;
		dirInitialized = TRUE;
  }

  return result;
}

char *startDir;

INT CALLBACK BrowseCallbackProc(HWND hwnd, 
                                UINT uMsg,
                                LPARAM lp, 
                                LPARAM pData) 
{
   switch(uMsg) 
   {
   case BFFM_INITIALIZED: 
      SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)startDir);
      break;
   }
   return 0;
}

int BrowseForDir::display( Window *frame, Window *parent, char *title, char *resDir )
{
  BROWSEINFO brInfo;

  brInfo.hwndOwner = parent->getHWNDFrame();
  brInfo.pidlRoot = NULL;
  brInfo.pszDisplayName = resDir;
  brInfo.lpszTitle = title;
  brInfo.ulFlags = BIF_RETURNONLYFSDIRS;
  brInfo.lpfn = BrowseCallbackProc;
  startDir = resDir;
  LPCITEMIDLIST pidlChild = SHBrowseForFolder( &brInfo );
  if( pidlChild == NULL ) return 0;
  SHGetPathFromIDList(pidlChild, resDir);
  return 1;
}

