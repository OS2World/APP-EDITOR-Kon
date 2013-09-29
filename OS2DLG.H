MRESULT EXPENTRY DlgProc(HWND hWnd, ULONG iMessage,
				MPARAM mParam1, MPARAM mParam2);

/*
 * Dialog base class.
 * All controls added to a dialog will be deleted when the dialog
 * is deleted. Note that all built in controls will do this automatically.
 * User created classes which do not inherit from bult in controls will
 * have to call addControl(..) to achieve this effect.
 *
 * The dialog has a simple layoutmanager. On resizing of the dialog, included
 * controls will be resized and moved proportionally.
 *
 * The controls which should have focus should be set in the constructor.
 *
 * Dialogs can me modal by adding ISMODAL to the properties.
 */

class Dialog : public Window
{
  static int inSize;
  int modal;
  int orgWidth, orgHeight;
  struct OrgPos : public AObject
  {
    int x, y, width, height;
    OrgPos( int x, int y, int width, int height )
    {
      this->x = x;
      this->y = y;
      this->width = width;
      this->height = height;
    }
  };
  Vector<OrgPos> orgPosV;
  Window *frame;
  HWND lastControlFocused;
protected:
  void adjustForFontSize();
public:
  MRESULT dlgProc( HWND hWnd, ULONG iMessage, MPARAM mParam1, MPARAM mParam2);
  Dialog( Window *frame, Window *parent, char *title, int properties, int x, int y, int width, int height )
    : Window( x, y, width, height )
  {
  	lastControlFocused = NULL;
    this->frame = frame;
    if( properties & ADJUSTSIZE )
    {
      width += Dialog::getInset()*2;
      height += Dialog::getInset()*2+Dialog::getCaptionHeight();
    }

    oldWndProc = NULL;
    modal = properties & ISMODAL;
    this->parent = parent;
    this->modal = modal;
    Application::tempthis = this;
    type = DIALOG;
    hWndClient = NULLHANDLE;
    hWndFrame = WinLoadDlg( ((Window *)frame)->getHWND(), ((Window *)parent)->getHWND(),
                            DlgProc, NULLHANDLE,
                            (properties & NOTITLE) ? IDD_EMPTYDLGTEMPLATE :
                                                     IDD_DLGTEMPLATE, NULL );

    Application::tempthis = this;

    // OS/2 does not create a client window, so to make it easier to use the
    // dialog, we create one ourselves

    char fontStr[255];
    sprintf( fontStr, "%d.%s", Graphics::getDialogFontSize(), Graphics::getDialogFontName() );

    if( !(properties & NOTITLE) )
    {
      hWndClient = WinLoadDlg( hWndFrame, hWndFrame,
                               DlgProc, NULLHANDLE, IDD_EMPTYDLGTEMPLATE,
                               NULL );

      WinShowWindow( hWndClient, TRUE );
	    WinSetPresParam( hWndClient, PP_FONTNAMESIZE, strlen( fontStr ) + 1, (PVOID)fontStr );
    }
    else
	    WinSetPresParam( hWndFrame, PP_FONTNAMESIZE, strlen( fontStr ) + 1, (PVOID)fontStr );

    WinSetWindowText( hWndFrame, title );

    if( properties & CENTER )
      center( x, y );

    setSize( width, height );
    setPos( x, y );
    orgWidth = width;
    orgHeight = height;
  }

  void initSize();
  void focusHotkey( int key );

  virtual void addControl( AWindow *child )
  {
    Window::addControl( child );
    orgPosV.add( new OrgPos( child->getX(), child->getY(),
                 child->getWidth(), child->getHeight() ) );
  }

  void center( int &x, int &y )
  {
    x = (parent->getWidth() - width) / 2;
    y = (parent->getHeight() - height) / 2;
    remapCoordinates( (Window *)parent, frame, x, y );
  }

  virtual int setVisible( BOOL visible );

  int getClientWidth()
  {
    SWP swp;

    WinQueryWindowPos( hWndClient, &swp );

    return swp.cx;
  }

  int getClientHeight()
  {
    SWP swp;

    WinQueryWindowPos( hWndClient, &swp );

    return swp.cy;
  }

  static int getCaptionHeight()
  {
    return WinQuerySysValue( HWND_DESKTOP, SV_CYTITLEBAR );
  }
  static int getInset()
  {
    return WinQuerySysValue( HWND_DESKTOP, SV_CYDLGFRAME );
  }

  virtual BOOL close()
  {
    return FALSE;
  }

  virtual void setPos( int x, int y );
  virtual void setSize( int width, int height );
  virtual int focus( BOOL newFocus );
  virtual void size( int oldWidth, int oldHeight );
  void updateControlPos( Window *control );
  virtual void closeDialog( int replyValue )
  {
    WinDismissDlg( hWndFrame, replyValue );
  }
};

class SystemFileDialog
{
  static char prevDir[MAXPATH+1];
	static BOOL initialized;
	static FILEDLG fDlg;
public:
  static int SAVEAS, OPEN;
  static int selectedEncoding;
  static char returnedFile[MAXPATH+1];
  int display( Window *frame, Window *parent, char *title, char *path, 
	             int dlgType, char *filters, int fltIndex, EncodingTypes encType = ANSI );
};

