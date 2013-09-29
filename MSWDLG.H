BOOL CALLBACK DlgProcBase(HWND hWnd, UINT iMessage,
				WPARAM wParam, LPARAM lParam );

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
  Window *frame;
  int modal;
  int orgWidth, orgHeight, widthBefResize, heightBefResize;
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
  int replyValue;
  HWND nextWindow( HWND currentHwnd, UINT cmd );
protected:
  void adjustForFontSize();
public:
  BOOL dlgProc( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
  Dialog( Window *frame, Window *parent, char *title, int properties, int x, int y, int width, int height );
	virtual BOOL allowPlacementResize() { return FALSE; }

  BOOL handleCHAR( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
  void focusHotkey( int key );
  BOOL handleKEYDOWN( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
  void initSize();

  void center( int &x, int &y );

  virtual void addControl( AWindow *child );

  virtual int setVisible( BOOL visible );

  virtual int getClientWidth()
  {
    RECTL rect;

    GetWindowRect( hWndClient, (LPRECT)&rect );
    return rect.right - rect.left;
  }

  virtual void setPos( int x, int y );
  virtual void setSize( int width, int height );
  virtual int getClientHeight()
  {
    RECTL rect;

    GetWindowRect( hWndClient, (LPRECT)&rect );
    return rect.bottom - rect.top;
  }

  static int getCaptionHeight()
  {
    return GetSystemMetrics( SM_CYCAPTION );
  }
  static int getInset()
  {
    return GetSystemMetrics( SM_CXFRAME );
  }

  virtual void size( int oldWidth, int oldHeight );
  virtual BOOL close()
  {
  	closeDialog( 0 );
  	return FALSE;
  }
  virtual void closeDialog( int replyValue );
  void updateControlPos( Window *control );
	~Dialog();
};

class SystemFileDialog
{
  static char prevDir[MAXPATH+1];
	static BOOL dirInitialized;
  static OPENFILENAME ofname;
public:
  static int selectedEncoding;
  static int SAVEAS, OPEN;
  static char returnedFile[MAXPATH+1];
  int display( Window *frame, Window *parent, char *title, char *path, int dlgType,
               char *filters, int fltIndex, EncodingTypes encType = ANSI );
};

class BrowseForDir
{
public:
  int display( Window *frame, Window *parent, char *title, char *resDir );
};