#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PTR_ARROW 1
#define PTR_WAIT 2
#define PTR_TEXT 3
#define PTR_MOVE 4
#define PTR_HAND 5

LRESULT APIENTRY WndProc( HWND hWnd, UINT iMessage,
                          WPARAM mParam1, LPARAM mParam2);

/* Window: Base window class for MS Windows */

class FrameWindow;
class ListBox;
class EntryField;
class RadioButton;
class CheckBox;
class Dialog;
class Scrollbar;
class Label;
class SComboBox;

class Window : public AWindow
{
  BOOL isEnabled, isPopup;
  Graphics *graphics;
	static BOOL baseClassRegistered;
  //PFNWP oldWndProc;

  // The following classes should have access to private variables

  friend FrameWindow;
  friend ListBox;
  friend EntryField;
  friend RadioButton;
  friend CheckBox;
  friend Dialog;
  friend Scrollbar;
  friend Label;
  friend SComboBox;

  // Actual creation of a window

  void createWindow( Window *parent, char *text, char *windowClass,
                     Window *behind, ULONG windowStyle, int x, int y, int width,
                     int height );
  LRESULT stdWndProc( HWND hWnd, UINT iMessage, WPARAM mParam1, LPARAM mParam2,
                      BOOL *returned);
  void createMenu( HMENU hMenu, AMenuEntry *menu );
	// Traverse the menu to find a menu id
	HMENU findMenuId( HMENU menu, int id );
  HCURSOR currentPointer;
  BOOL mouseButtonDown;
  int click;
  BOOL ownsCursor;
	BOOL lastTick;
	BOOL hasParentAtLevel( Window *window, int level );
	char className[50];
	void registerBaseClass();
protected:
  HWND hWndClient, hWndFrame;
public:
  BOOL killMe;  // Has to be public in order for the WndProc to reach it easily
  LRESULT wndProc( HWND hWnd, UINT iMessage, WPARAM mParam1, LPARAM mParam2);
  WNDPROC oldWndProc;
  Window() : AWindow(), ownsCursor(FALSE), isEnabled(TRUE), killMe(FALSE), lastTick(0), isPopup(FALSE) {}
  Window( int x, int y, int width, int height ) :
    AWindow( x, y, width, height ), ownsCursor(FALSE), isEnabled(TRUE),
    killMe(FALSE), lastTick(0), isPopup(FALSE) { strcpy( className, "" ); };

  // This is the constructor the user should use

  Window( Window *parent, int x, int y, int width, int height ) :
    AWindow( parent, x, y, width, height ), ownsCursor(FALSE), isEnabled(TRUE),
    killMe(FALSE), lastTick(0), isPopup(FALSE)
  {
    createWindow( (Window *)parent, "", (char *)0L, NULL, WS_CHILD, x, y, width, height );
  };

  Window( Window *parent, char *title, int x, int y, int width, int height ) :
    AWindow( parent, x, y, width, height ), ownsCursor(FALSE), isEnabled(TRUE),
    killMe(FALSE), lastTick(0), isPopup(FALSE)
  {
    createWindow( (Window *)parent, title, (char *)0L, NULL, WS_CHILD, 
                  x, y, width, height );
  };

	BOOL hasFocus() 
	{
		return GetFocus() == hWndFrame || GetFocus() == hWndClient;
	}

  void getCursorPos( int &x, int &y );

	void acceptDrop( BOOL accept );
	virtual void filesDropped( char **files, int count ) {}

  // These constructors are mainly for subclasses which implements controls

  Window( Window *parent, ULONG windowStyle, int x, int y,
          int width, int height ) :
    AWindow( parent, x, y, width, height ), ownsCursor(FALSE), isEnabled(TRUE),
    killMe(FALSE), lastTick(0)
  {
    createWindow( (Window *)parent, "", (char *)0L, NULL, windowStyle | WS_CHILD,
                   x, y, width, height );
  };

  Window( Window *parent, char *windowClass, char *text,
          ULONG windowStyle,
          int x, int y, int width, int height ) :
    AWindow( parent, x, y, width, height ), ownsCursor(FALSE), isEnabled(TRUE),
    killMe(FALSE), lastTick(0)
  {
    createWindow( (Window *)parent, text, windowClass, 
                  (Window *)parent->getLastControl(), 
    							windowStyle | (windowStyle & WS_POPUP ? 0 : WS_CHILD),
                  x, y, width, height );
  }

  // Call deleteMe in order to delete this window class after message processing
  // is done. This makes it possible for a window class to delete itself.

  void deleteMe();

  ULONG xlatProperties( ULONG properties );
  virtual int execFromMainThread( int task, void *data ) { return 0; }

  void setPointer( int type );
  virtual void setCursor( int x, int y, int width, int height );
  virtual void hideCursor();

	void getCursorExtent( int &width, int &height );

  virtual int setVisible( BOOL visible, BOOL activate )
  {
    return ShowWindow( hWndFrame, ((visible) ? ((activate) ? SW_SHOWNORMAL : 0) : SW_HIDE) |
		                              ((activate) ? 0 : SW_SHOWNOACTIVATE));
  }

  virtual int setVisible( BOOL visible )
  {
		return setVisible( visible, TRUE );
  }

  // In the focus method, it is possible to set the focus to another window
  // Return 1 if focus is changed in this method

  virtual int focus( BOOL newFocus ) { return 0; }

  // In the focusSet method, it is NOT possible to set the focus to another
  // but cursor creation does work.

  virtual void focusSet( BOOL newFocus ) {}

  virtual void setFocus();

  virtual void activate( BOOL active )
  {
    if( active )
      setFocus();
  }

  virtual void setOnTop()
  {
    SetWindowPos( hWndFrame, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
  }

  virtual void setAbove( Window *wnd )
  {
    SetWindowPos( hWndFrame, wnd->getHWND(), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
  }

  virtual void enable( BOOL enabled )
  {
    EnableWindow( hWndFrame, enabled );
  }

  virtual void setFont( Font *font, int pointSize )
  {
/*    char fontStr[255];
    sprintf( fontStr, "%d.%s", pointSize, font->name );
    WinSetPresParam( hWndFrame, PP_FONTNAMESIZE, strlen( fontStr ) + 1, (PVOID)fontStr );*/
  }

  virtual HWND getHWND() { return hWndClient; };
  virtual HWND getHWNDFrame() { return hWndFrame; };
  virtual int getHeight();
  virtual int getWidth();
  int getColorIndex( ColorValue color )
  {
    return 0;//GpiQueryColorIndex( WinGetPS( hWndClient ), 0, color );
  }

  virtual void paint( Rect *rect, Graphics *g )
  {
  }

  virtual Graphics *getGraphics()
  {
    return graphics;
  }

  virtual void setSize( int width, int height );

  virtual void setPos( int x, int y );

  virtual void repaint( Rect *rect )
  {
    repaint( rect, TRUE );
  }

  /*
   * A popup menu is used the same way as an ordinary window.
   * Add menulisteners the same way as for caption menues
   */

  void popupMenu( AMenu *menu, int x, int y );

  virtual void repaint( Rect *rect, BOOL children );
  ~Window();
  virtual void scrollVert( int delta, int bottom, int top );
  virtual void scrollHorz( int delta, int left, int right );
	virtual void setText( char *text );

  // Paints may be pipelined for later processing. In order to process them
  // now for this window, call this method.

  void processPaint();

	static void remapCoordinates( Window *from, Window *to, int &x, int &y );

	BOOL sendDataToWindow( char *windowTitle, char *message );
  static BOOL popupWindow( char *windowTitle );
	virtual void dataReceived( char *message ) {}
	static BOOL W2K_SetForegroundWindow (HWND hWndToForce);
  BOOL isMinimized();
  BOOL isMaximized();
	void restore();
  void setParent( Window *parent )
  {
    SetParent( this->getHWNDFrame(), NULL );
  }
};

/* Desktop: Window class for the desktop */

class Desktop : public Window
{
static Desktop *pDesktop;
public:
  Desktop() : Window() {};
  virtual int getX() { return 0; }
  virtual int getY() { return 0; };
  virtual int getWidth() { return GetSystemMetrics( SM_CXSCREEN ); }
  virtual int getHeight() { return GetSystemMetrics( SM_CYSCREEN ); }
  virtual int getClientWidth() { return GetSystemMetrics( SM_CXSCREEN ); }
  virtual int getClientHeight() { return GetSystemMetrics( SM_CYSCREEN ); }
  virtual HWND getHWND() { return GetDesktopWindow(); };
  virtual HWND getHWNDFrame() { return GetDesktopWindow(); };
  static Desktop *getInstance() { return pDesktop; }
};

/* FrameWindow: A window class whith border and system buttons. This class
   can have menues attached */

class FrameWindow : public Window
{
//  PVOID menuResource;
	static BOOL fbaseClassRegistered;
public:
  FrameWindow( AWindow *parent, STRING title, int x, int y, int width, int height );

  /* Note that addMenu causes size(int,int) to be called! Therefore it is
     a good idea to add a menu last when all controls have been placed */

  void addMenu( AMenu *menu )
  {
    // Delete the old menu

    HMENU oldMenu = GetMenu( hWndFrame );

    if( oldMenu != NULL )
      DestroyMenu( oldMenu );

    // Create the new menu

    HMENU hMenu = CreateMenu();

    createMenu( hMenu, menu );

    SetMenu( hWndFrame, hMenu );

    // Display changes

    DrawMenuBar( hWndFrame );
  }

  void checkMenuItem( int id, BOOL check );
  void enableMenuItem( int id, BOOL enable );

  void removeMenu()
  {
    DestroyMenu( GetMenu( hWndFrame ) );
    DrawMenuBar( hWndFrame );
  }

  virtual void paint( Rect *rect, Graphics *g )
  {
    g->setColor( Color::LIGHT );
    g->setBackColor( Color::BKGR );
    g->fillRect( rect );
  }

  virtual int getClientWidth()
  {
    RECT rect;

    GetClientRect( hWndFrame, &rect );
    return rect.right - rect.left;
  }

  virtual int getClientHeight()
  {
    RECT rect;

    GetClientRect( hWndFrame, &rect );
    return rect.bottom - rect.top;
  }

  void setCaptionText( char *text )
  {
    SetWindowText( hWndFrame, text );
  }
  ~FrameWindow() {}
};

/* EntryField
 * properties: BORDER - surrounds the editfield with a border.
 *                      The width of the border can be retrievded from
 *                      getInset()
 */

class EntryField : public Window
{
  friend Window;
  Window *internalEntryField;
  int isBordered, maxLength;
  char *selectedText;
public:
  EntryField( Window *parent, int properties, int x, int y, int width,
              int height);

  virtual void addKeyListener( AKeyListener *keyListener )
  {
    internalEntryField->addKeyListener( keyListener );
  }

  virtual void size( int oldWidth, int oldHeight );

  virtual void move()
  {
    size( 0, 0 );
  }

  virtual void enable( BOOL enabled )
  {
    Window::enable( enabled );
    internalEntryField->enable( enabled );
  }

  void setReadOnly( BOOL state )
  {
    SendMessage( internalEntryField->getHWND(), EM_SETREADONLY, (WPARAM)state, NULL );
  }

  // Returns border width

  static int getInset() { return 4; }

  void paint( Rect *rect, Graphics *g )
  {
    if( isBordered )
    {
      g->drawShadedBox( 0, 0, width, height, Color::GRAY, Color::LIGHT );
      g->drawShadedBox( 1, 1, width-2, height-2, Color::BLACK, Color::SHADOW );
    }
  }

  void setText( char *text );

  void selectText()
  {
    SendMessage( internalEntryField->getHWND(), EM_SETSEL,
                WPARAM( 0 ), (LPARAM)-1 );
  }

  void selectText( int start, int end )
  {
    SendMessage( internalEntryField->getHWND(), EM_SETSEL,
                WPARAM( start ), (LPARAM)end );
  }

  char *getText()
  {
    GetWindowText( internalEntryField->getHWND(), selectedText, maxLength );
    return selectedText;
  }

  int getTextLength()
  {
    return GetWindowTextLength( internalEntryField->getHWND() );
  }

  virtual void setFocus()
  {
    internalEntryField->setFocus();
		selectText();
  }

  virtual int focus( BOOL newFocus )
  {
    if( newFocus )
    {
      internalEntryField->setFocus();
			selectText();
      return 1;
    }
    return 0;
  }

  void setMaxLength( int chars )
  {
    maxLength = chars;

    if( selectedText != NULL )
      delete selectedText;

    selectedText = new char[maxLength+1];

    SendMessage( internalEntryField->getHWND(), EM_SETLIMITTEXT,
                (WPARAM)maxLength, (LPARAM)0 );
  }

  void clearSelection()
  {
    SendMessage( internalEntryField->getHWND(), EM_REPLACESEL,
                WPARAM( FALSE ), (LPARAM)"" );
  }

  ~EntryField()
  {
    delete selectedText;
    delete internalEntryField;
  }
};

/* ListBox
 * properties: NOADJUSTSIZE - Listboxes are by default adjusted in size
 *                            in order to make full entries fit. Disable
 *                            by adding this property.
 *             SORTED       - Sort the entries
 */

class ListBox : public Window, public virtual AKeyListener
{
  virtual BOOL actionPerformed( AKeyEvent *keyEvent );
public:
  ListBox( Window *parent, int properties, int x, int y, int width, int height, BOOL sort ) :
    Window( parent, (char*)"LISTBOX", "",
            xlatProperties(properties | CLIPSIBLINGS | FILTERDIALOGARROWS) |
              (sort ? LBS_SORT:0) | WS_VSCROLL | WS_HSCROLL | WS_BORDER | LBS_NOTIFY | ((properties & POPUP) ? 0 : WS_CHILD),
            x, y, width, height )
  {
    type = LISTBOX;

    // Get the new pos and size if adjusted

    parent->addControl( this );
    addKeyListener( this );
  }

  void insertItem( int index, char *text )
  {
    SendMessage( hWndFrame, LB_INSERTSTRING, (WPARAM)index, (LPARAM)text );
  }

  void addItem( char *text )
  {
    SendMessage( hWndFrame, LB_ADDSTRING, (WPARAM)0, (LPARAM)text );
  }

  void selectItem( int index )
  {
    SendMessage( hWndFrame, LB_SETCURSEL, (WPARAM)index, NULL );
		performSelectionEvent( (WPARAM)index );
  }

  int getItems()
  {
    return (int)SendMessage( hWndFrame, LB_GETCOUNT, NULL, NULL );
  }

  void clearItems();

  // Returns -1 if none selected

  int getSelectedIndex()
  {
    int selected = (int)SendMessage( hWndFrame, LB_GETCURSEL, NULL, NULL );
    return (selected == LB_ERR) ? -1 : selected;
  }

  void getItem( int index, int len, char *text )
  {
    SendMessage( hWndFrame, LB_GETTEXT, (WPARAM)index, (LPARAM)text );
  }

  int getSelectedItem( int len, char *text )
  {
    int selected = getSelectedIndex();
    getItem( selected, len, text );
    return selected;
  }

	int findItem( char *string, BOOL prefix )
  {
  	int found = (int)SendMessage( hWndFrame, LB_FINDSTRING,
    									 					 (WPARAM)-1,
																 LPARAM(string) );

    return (found == LB_ERR) ? -1 : found;
  }

	int getNextSelectedIndex( int index );

};

/* ListBox
 * properties: NOADJUSTSIZE - Listboxes are by default adjusted in size
 *                            in order to make full entries fit. Disable
 *                            by adding this property.
 *             SORTED       - Sort the entries
 */

class SComboBox : public Window, public virtual AKeyListener, public virtual AFocusListener
{
  virtual void actionPerformed( AFocusEvent *focusEvent );
  int isBordered, maxLength;
  char *selectedText;
  BOOL isStatic;
public:
  LRESULT wndProcCombo( HWND hWnd, UINT iMessage, WPARAM mParam1, LPARAM mParam2);
  WNDPROC oldWndProcCombo;
  static int STATIC;
  SComboBox( Window *parent, int properties, int x, int y, int width, int height, int listHeight );
  void insertItem( int index, char *text )
  {
    SendMessage( hWndFrame, CB_INSERTSTRING, (WPARAM)index, (LPARAM)text );
  }

  void addItem( char *text )
  {
    SendMessage( hWndFrame, CB_ADDSTRING, (WPARAM)0, (LPARAM)text );
  }

  void selectItem( int index )
  {
    SendMessage( hWndFrame, CB_SETCURSEL, (WPARAM)index, NULL );
		performSelectionEvent( (WPARAM)index );
  }

  int getItems()
  {
    return (int)SendMessage( hWndFrame, CB_GETCOUNT, NULL, NULL );
  }

  void clearItems();

  // Returns -1 if none selected

  int getSelectedIndex()
  {
    int selected = (int)SendMessage( hWndFrame, CB_GETCURSEL, NULL, NULL );
    return (selected == LB_ERR) ? -1 : selected;
  }

  void getItem( int index, int len, char *text )
  {
    SendMessage( hWndFrame, CB_GETLBTEXT, (WPARAM)index, (LPARAM)text );
  }

  int getSelectedItem( int len, char *text )
  {
    int selected = getSelectedIndex();
    getItem( selected, len, text );
    return selected;
  }

	int findItem( char *string, BOOL prefix )
  {
  	int found = (int)SendMessage( hWndFrame, CB_FINDSTRING,
    									 					 (WPARAM)-1,
																 LPARAM(string) );

    return (found == CB_ERR) ? -1 : found;
  }

	int getNextSelectedIndex( int index );
  static int getInset( int horz );
  char *getEditText();
  int getTextLength();
  virtual void setText( char *text );
  void selectText();
  void selectText( int start, int end );
  void setMaxLength( int chars );
  virtual void setSize( int width, int height );
  virtual void setPos( int x, int y );
  virtual int getEditBoxHeight();
  void clearSelection()
  {
  }
};

/* RadioButton: To form a new group: Set property AWindow::newGroup.
                All buttons in the group should be created successively
                and have the property AWindow::inGroup. */

class RadioButton : public Window
{
public:
  RadioButton( Window *parent, char *text, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)"BUTTON", text,
            xlatProperties(properties) | BS_AUTORADIOBUTTON | WS_CHILD,
            x, y, width, height )
  {
    type = RADIOBUTTON;
    parent->addControl( this );
  }
  int queryCheck()
  {
    return (int)SendMessage( hWndFrame, BM_GETCHECK, (WPARAM)NULL, (LPARAM)NULL );
  }
  void check( int state )
  {
    SendMessage( hWndFrame, BM_SETCHECK, (WPARAM)state, (LPARAM)NULL );
  }
};

/* CheckBox */

class CheckBox : public Window
{
public:
  CheckBox( Window *parent, char *text, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)"BUTTON", text,
            xlatProperties(properties) | BS_AUTOCHECKBOX | WS_CHILD,
            x, y, width, height )
  {
    type = CHECKBOX;
    parent->addControl( this );
  }
  int queryCheck()
  {
    return (int)SendMessage( hWndFrame, BM_GETCHECK, (WPARAM)NULL, (LPARAM)NULL );
  }
  void check( int state )
  {
    SendMessage( hWndFrame, BM_SETCHECK, (WPARAM)state, (LPARAM)NULL );
  }
};

/* Group box */

class GroupBox : public Window
{
public:
  GroupBox( Window *parent, char *text, int properties, int x, int y,
            int width, int height ) :
    Window( parent, (char*)"BUTTON", text,
            xlatProperties(properties) | BS_GROUPBOX | WS_VISIBLE | WS_CHILD,
            x, y, width, height )
  {
    type = GROUPBOX;
    parent->addControl( this );
  }
};

/* Button
 * properties: ISDEFAULT - Set this to make the button default
 */

class Button : public Window
{
public:
  Button( Window *parent, char *text, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)"BUTTON", text,
            xlatProperties(properties) | ((properties&ISDEFAULT) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON) | WS_CHILD,
            x, y, width, height )
  {
    this->properties = properties;
    type = PUSHBUTTON;
    parent->addControl( this );
  }
};

/* Scrollbar */

class Scrollbar : public Window
{
  int firstLine;
public:
  static int PAGERIGHT, PAGELEFT, LINERIGHT, LINELEFT,
             PAGEDOWN, PAGEUP, LINEDOWN, LINEUP, HORZRELEASED, VERTRELEASED,
             HORZTRACK, VERTTRACK;
  Scrollbar( Window *parent, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)"SCROLLBAR", "",
            xlatProperties(properties) |
              ((width > height) ? SBS_HORZ : SBS_VERT) | WS_CHILD,// | SBS_SIZEBOX,
            x, y, width, height )
  {
    type = SCROLLBAR;
    parent->addControl( this );
    firstLine = 0;
  }

  // Select the size of the thumb

  void setRatio( int visible, int total )
  {
    // Not in windows
    SCROLLINFO sinfo;
    sinfo.cbSize = sizeof( SCROLLINFO );
    sinfo.fMask = SIF_PAGE;
    sinfo.nPage = visible;
    SetScrollInfo( hWndFrame, SB_CTL, &sinfo, TRUE );
  }

  // Position within range

  void setThumbPos( int pos )
  {
    SCROLLINFO sinfo;
    sinfo.cbSize = sizeof( SCROLLINFO );
    sinfo.fMask = SIF_POS;
    sinfo.nPos = pos;
    SetScrollInfo( hWndFrame, SB_CTL, &sinfo, TRUE );
  }

  // Scrollbar range

  void setRange( int pos, int first, int last )
  {
    firstLine = first;
    SCROLLINFO sinfo;
    sinfo.cbSize = sizeof( SCROLLINFO );
    sinfo.fMask = SIF_RANGE | SIF_POS;
    sinfo.nPos = pos;
    sinfo.nMin = first;
    sinfo.nMax = last;
    SetScrollInfo( hWndFrame, SB_CTL, &sinfo, TRUE );
  }

	void setAll( int pos, int first, int last, int visible, int total )
  {
    firstLine = first;
    SCROLLINFO sinfo;
    sinfo.cbSize = sizeof( SCROLLINFO );
    sinfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
    sinfo.nPos = pos;
    sinfo.nMin = first;
    sinfo.nMax = last;
    sinfo.nPage = visible;
    SetScrollInfo( hWndFrame, SB_CTL, &sinfo, TRUE );
  }

  // If dir == 0, the height of the horizontal scrollbar is returned,
  // othersize the height of the vertical scrollbar
  static int getInset( int dir )
  {
    return GetSystemMetrics( dir ? SM_CXHSCROLL : SM_CXVSCROLL );//WinQuerySysValue( HWND_DESKTOP, dir ? SV_CYHSCROLL : SV_CXVSCROLL );
  }

  int getFirst() { return firstLine; }
};

/* Label
 * properties: LEFT, CENTER, RIGHT - Position of text
 */

class Label: public Window
{
public:
  static int LEFT, CENTER, RIGHT;
  Label( Window *parent, int properties, char *text, int x, int y,
         int width, int height ) :
    Window( parent, (char*)"STATIC", text,
            xlatProperties(properties) | WS_CHILD,
            x, y, width, height )
  {
    type = LABEL;
    parent->addControl( this );
  }
  ULONG xlatProperties( int properties )
  {
    ULONG windowStyle = 0;

    if( properties & LEFT )
      windowStyle |= SS_LEFT;

    if( properties & CENTER )
      windowStyle |= SS_CENTER;

    if( properties & RIGHT )
      windowStyle |= SS_RIGHT;

    if( windowStyle == 0 )
      windowStyle = SS_LEFT;

    return Window::xlatProperties( properties ) | windowStyle;
  }
};

class PopupMessage
{
  int result;
public:
  static int error, warning, message, query, ok, cancel, yes, no, querywithwarning, querywithwarningNoCancel;
  PopupMessage( Window *parent, char *title, char *message, int type )
  {
    UINT style;
    if( type == error ) style = MB_OK|MB_ICONEXCLAMATION;
    if( type == warning ) style = MB_OK|MB_ICONWARNING;
    if( type == this->message ) style = MB_OK|MB_ICONINFORMATION;
    if( type == query ) style = MB_YESNOCANCEL|MB_ICONQUESTION;
    if( type == querywithwarning ) style = MB_YESNOCANCEL|MB_ICONWARNING;
    if( type == querywithwarningNoCancel ) style = MB_YESNO|MB_ICONWARNING;

    style |= MB_APPLMODAL;
    result = MessageBox( parent->getHWNDFrame(), message, title, style );
  }
  int getResult()
  {
    switch( result )
    {
      case IDOK : return ok;
      case IDCANCEL : return cancel;
      case IDYES : return yes;
      case IDNO : return no;
    }
    return 0;
  }
};
