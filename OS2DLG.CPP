#include <ctype.h>
#include <direct.h>
#include "bowl.h"

MRESULT EXPENTRY DlgProc(HWND hWnd, ULONG iMessage,
				MPARAM mParam1, MPARAM mParam2)
{
	Dialog *pWindow = (Dialog *)WinQueryWindowULong(hWnd,0);

  if (iMessage == WM_INITDLG) {
    pWindow = (Dialog *)Application::tempthis;
    WinSetWindowULong(hWnd,0,(ULONG)pWindow);
    return pWindow->dlgProc(hWnd,iMessage,mParam1,mParam2);
  }
  else
  if( pWindow == 0 )
    return WinDefDlgProc(hWnd,iMessage,mParam1,mParam2);
  else
    return pWindow->dlgProc(hWnd,iMessage,mParam1,mParam2);
}

int Dialog::inSize = 0;

void Dialog::focusHotkey( int key )
{
  char text[81];
  BOOL found = FALSE;

  // Get first child window of the dialog

  HWND focusWin = NULL, curWin = WinQueryWindow( hWndClient, QW_BOTTOM );

  // Loop through the windows using GetWindow

  while( curWin )
  {
    WinQueryWindowText( curWin, 80, text );

    if( WinSendMsg( curWin, WM_MATCHMNEMONIC, MPARAM( key ), NULL ) )
    {
      if( !WinQueryClassName( curWin, 80, text ) ) return;
      if( stricmp( text, "#5" ) )
        focusWin = curWin;
      else
        focusWin = WinQueryWindow( curWin, QW_PREV );
      found = TRUE;
      break;
    }
    curWin = WinQueryWindow( curWin, QW_PREV );
  }

  if( found )
  {
    WinSetFocus( HWND_DESKTOP, focusWin );
    WinQueryClassName( focusWin, 80, text );
    if( !strcmp( text, "#3" ) )
    {
      if( (WinQueryWindowULong( focusWin, QWL_STYLE ) & BS_AUTOCHECKBOX) == BS_AUTOCHECKBOX )
        WinSendMsg( focusWin, BM_SETCHECK,
                    (MPARAM)!WinSendMsg( focusWin, BM_QUERYCHECK, NULL, NULL),
                    NULL );
      else
        WinSendMsg( focusWin, BM_SETCHECK, (MPARAM)TRUE, NULL );
    }
  }
}

MRESULT  Dialog::dlgProc( HWND hWnd,ULONG iMessage,MPARAM mParam1,
                          MPARAM mParam2 )
{
  char str[40];
	switch( iMessage )
	{
	  case WM_INITDLG:
    {
      hWndClient = hWnd;
      graphics = new Graphics( WinGetPS( hWndClient ) );
      return (MPARAM)TRUE;
    }
    case WM_WINDOWPOSCHANGED :
    {
      if( hWndClient == hWndFrame ||
          hWndClient != hWnd )
      {
        PSWP swp = (PSWP)mParam1;
        if( swp->fl & SWP_SIZE )
        {
          int oldWidth = width, oldHeight = height;
          width = swp->cx;
          height = swp->cy;
          size( oldWidth, oldHeight );
				}
      }
      break;
    }
    case WM_SYSCOMMAND :
    {
      if( hWnd != hWndFrame )
        WinSendMsg( hWndFrame, WM_SYSCOMMAND, mParam1, mParam2 );
      else
      if( hWndFrame == hWndClient )
      {
        // Send to parent window which should be a notebook
        WinSendMsg( ((Window *)parent)->getHWND(), WM_SYSCOMMAND, mParam1,
                    mParam2 );
      }
      break;
    }
    case WM_CHAR :
    {
      // Cursor movement

      if( SHORT1FROMMP( mParam1 ) & KC_KEYUP ) break;

      if( SHORT1FROMMP( mParam1 ) & KC_VIRTUALKEY )
      {
        if( SHORT2FROMMP( mParam2 ) == VK_ESC )
        {
          if( parent->getType() == NOTEBOOK &&
              parent->getParent()->getType() == DIALOG )
            WinPostMsg( ((Window *)parent->getParent())->getHWND(), 
                        WM_CLOSE, NULL, NULL );
          else
            WinPostMsg( hWndFrame, WM_CLOSE, NULL, NULL );
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_PAGEDOWN )
        {
          if( (SHORT1FROMMP( mParam1 ) & KC_ALT) &&
              parent->getType() == NOTEBOOK )
            ((Notebook *)parent)->nextTab();
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_PAGEUP )
        {
          if( (SHORT1FROMMP( mParam1 ) & KC_ALT) &&
              parent->getType() == NOTEBOOK )
            ((Notebook *)parent)->prevTab();
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_TAB )
        {
          WinSetFocus( HWND_DESKTOP,
                       WinEnumDlgItem( hWndClient, WinQueryFocus(HWND_DESKTOP),
                                       EDI_NEXTTABITEM ) );
          return MPARAM(TRUE);
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_BACKTAB )
        {
          WinSetFocus( HWND_DESKTOP,
                       WinEnumDlgItem( hWndClient, WinQueryFocus(HWND_DESKTOP),
                                       EDI_PREVTABITEM ) );
          return MPARAM(TRUE);
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_UP ||
            SHORT2FROMMP( mParam2 ) == VK_LEFT )
        {
          WinSetFocus( HWND_DESKTOP,
                       WinEnumDlgItem( hWndClient, WinQueryFocus(HWND_DESKTOP),
                                       EDI_PREVGROUPITEM ) );
          return MPARAM(TRUE);
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_DOWN ||
            SHORT2FROMMP( mParam2 ) == VK_RIGHT )
        {
          WinSetFocus( HWND_DESKTOP,
                       WinEnumDlgItem( hWndClient, WinQueryFocus(HWND_DESKTOP),
                                       EDI_NEXTGROUPITEM ) );
          return MPARAM(TRUE);
        }
        else
        if( SHORT2FROMMP( mParam2 ) == VK_ENTER ||
            SHORT2FROMMP( mParam2 ) == VK_NEWLINE )
        {
          // Find the default button
  
          for( int i = 0; i < controls.getSize(); i++ )
          {
            Window *control = (Window *)controls.getObjectAt(i);
            if( control->getType() == PUSHBUTTON &&
                ((control->getProperties() & ISDEFAULT) == ISDEFAULT) )
            {
                control->performButtonEvent( control, control->id );
                return (MPARAM)TRUE;
            }
          }
          break;
        }
      }
      else
      if( SHORT1FROMMP( mParam1 ) & KC_ALT )
      {
        // Shortcut
        focusHotkey( SHORT1FROMMP( mParam2 ) );
        return MPARAM(TRUE);
      }
     break;
    }
    case WM_CLOSE :
    {
      closeDialog( close() );
      return (MPARAM)FALSE;
    }
  }

  BOOL returned = TRUE;
	MRESULT retVal = stdWndProc( hWnd, iMessage, mParam1, mParam2, &returned );

  if( !returned )
  {
    if( oldWndProc == NULL )
      return WinDefDlgProc( hWnd, iMessage, mParam1, mParam2 );
    else
      return oldWndProc( hWnd, iMessage, mParam1, mParam2 );
  }
  else
    return retVal;
}

void Dialog::adjustForFontSize()
{
  double widthMult, heightMult;

  // Resize dialog

  getGraphics()->setDialogFont();
  int thisHeight = (double)getGraphics()->getFontHeight();
  int thisWidth = (double)getGraphics()->getAverageCharWidth();

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
	if( inSize == 1 ) return;

  if( hWndClient != NULLHANDLE && hWndClient != hWndFrame )
	{
		inSize = 1;
    WinSetWindowPos( hWndClient, HWND_TOP, getInset(), getInset(),
                     width-2*getInset(),
                     height-2*getInset()-getCaptionHeight(),
                     SWP_SIZE | SWP_MOVE );
		inSize = 0;
	}

  // Resize all components
  int cHeight = getClientHeight();

  for( int i = 0; i < controls.getSize(); i++ )
  {
    Window *control = (Window *)controls.getObjectAt(i);
    OrgPos *orgPos = (OrgPos *)orgPosV.getObjectAt(i);

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

      if( (control->getHMove() == PROPORTIONAL || control->getHMove() == FOLLOW 
			    || !sizeInited) && ny+nh > cHeight )
        nh = cHeight - ny;

      control->setPos( nx, ny );
      control->setSize( nw, nh );
    }
  }
}

/*void Dialog::size( int, int )
{
  // Resize the client window if one exists
  if( hWndClient != NULLHANDLE && hWndClient != hWndFrame )
    WinSetWindowPos( hWndClient, HWND_TOP, getInset(), getInset(),
                     width-2*getInset(),
                     height-2*getInset()-getCaptionHeight(),
                     SWP_SIZE | SWP_MOVE );
  // Resize all components
  for( int i = 0; i < controls.getSize(); i++ )
  {
    Window *control = (Window *)controls.getObjectAt(i);
    OrgPos *orgPos = (OrgPos *)orgPosV.getObjectAt(i);

    control->setPos( (orgPos->x * width) / orgWidth,
                      (orgPos->y * height) / orgHeight );
    control->setSize( (orgPos->width * width) / orgWidth,
                       (orgPos->height * height) / orgHeight );
  }
}
*/
int Dialog::setVisible( BOOL visible )
{
  if( visible )
  {
    WinShowWindow( hWndFrame, TRUE );
    if( modal )
      return WinProcessDlg( hWndFrame );
  }
  else
    WinShowWindow( hWndFrame, FALSE );
  return -1;
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

int Dialog::focus( BOOL newFocus )
{
	if( !newFocus )
  	lastControlFocused = WinQueryFocus( HWND_DESKTOP );
  else
  if( lastControlFocused != NULL )
  {
  	WinSetFocus( HWND_DESKTOP, lastControlFocused );
    return TRUE;
  }
  return FALSE;
}

void Dialog::setPos( int x, int y )
{
	this->x = x;
	this->y = y;
	WinSetWindowPos( hWndFrame, HWND_TOP, x, y, 0, 0, SWP_MOVE );
}

void Dialog::setSize( int width, int height )
{
  //int oldWidth = this->width, oldHeight = this->height;
	WinSetWindowPos( hWndFrame, HWND_TOP, 0, 0, width, height, SWP_SIZE );
	//this->width = width;
	//this->height = height;
  //size( oldWidth, oldHeight);
}

char SystemFileDialog::returnedFile[MAXPATH+1];
char SystemFileDialog::prevDir[MAXPATH+1];
int SystemFileDialog::SAVEAS = 0;
int SystemFileDialog::OPEN = 1;
int SystemFileDialog::selectedEncoding = ANSI;
BOOL SystemFileDialog::initialized = FALSE;
FILEDLG SystemFileDialog::fDlg;

int SystemFileDialog::display( Window *frame, Window *parent, char *title, 
                               char *path, int dlgType, char *filters, 
															 int fltIndex, EncodingTypes encType )
{
	char dir[MAXPATH+1];

	if( strlen( prevDir ) > 0 )
	{
		if( strlen( dir ) > 2 && prevDir[1] == ':' )
			_chdrive( toupper( prevDir[0] ) -'A' + 1 );
		chdir( prevDir );
	}
	_getcwd( dir, MAXPATH );

	if( !initialized )
		memset( &fDlg, 0, sizeof( FILEDLG ) );

	fDlg.cbSize = sizeof( FILEDLG);
	fDlg.pszTitle = title;
	strcpy( fDlg.szFullFile, path );
	fDlg.fl = FDS_CENTER;

	if( dlgType == OPEN )
		fDlg.fl |= FDS_OPEN_DIALOG;
	else
		fDlg.fl |= FDS_SAVEAS_DIALOG;

	WinFileDlg( frame->getHWNDFrame(), 
              parent->getHWNDFrame(), &fDlg );

  BOOL result = (fDlg.lReturn == DID_OK);

  if( result )
	{
	  strcpy( returnedFile, fDlg.szFullFile );
		strcpy( prevDir, returnedFile );
		initialized = TRUE;

		char *bs = strrchr( prevDir, '\\' );
		if( bs != NULL )
			*bs = 0;
	}

	return result;
}