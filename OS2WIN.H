#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PTR_ARROW 1
#define PTR_WAIT 2
#define PTR_TEXT 3
#define PTR_MOVE 4
#define PTR_HAND 5

MRESULT EXPENTRY WndProc( HWND hWnd,ULONG iMessage,
                          MPARAM mParam1, MPARAM mParam2);

/* Window: Base window class for OS/2 PM */

class FrameWindow;
class ListBox;
class EntryField;
class RadioButton;
class CheckBox;
class Dialog;
class Scrollbar;
class Label;

class Window : public AWindow
{
  HWND hWndClient, hWndFrame;
  Graphics *graphics;
  PFNWP oldWndProc;
	static BOOL baseClassRegistered;

  // The following classes should have access to private variables

  friend FrameWindow;
  friend ListBox;
  friend EntryField;
  friend RadioButton;
  friend CheckBox;
  friend Dialog;
  friend Scrollbar;
  friend Label;

  // Actual creation of a window

  void createWindow( Window *parent, Window *owner, char *text, char *windowClass/*, int id*/,
                     Window *behind, ULONG windowStyle, int x, int y, int width,
                     int height );
  MRESULT stdWndProc( HWND hWnd, ULONG iMessage, MPARAM mParam1, MPARAM mParam2,
                      BOOL *returned);
  void createMenu( HWND hWndParentMenu, AMenuEntry *menu );
  HPOINTER currentPointer;
  BOOL mouseButtonDown, processMenuEnd;
	BOOL lastTick, acceptDropFiles;
	BOOL isPopupWin;
public:
  BOOL killMe;  // Has to be public in order for the WndProc to reach it easily
  MRESULT wndProc( HWND hWnd,ULONG iMessage, MPARAM mParam1, MPARAM mParam2);
  Window() : killMe(FALSE), AWindow(), lastTick(0), acceptDropFiles(FALSE), 
	           isPopupWin(FALSE) {}
  Window( int x, int y, int width, int height ) : killMe(FALSE),
    AWindow( x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
	  isPopupWin(FALSE) {};

  // This is the constructor the user should use

  Window( Window *parent, int x, int y, int width, int height ) : killMe(FALSE),
    AWindow( parent, x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
    isPopupWin(FALSE)
  {
    createWindow( (Window *)parent, (Window *)parent, "", (char *)0L, NULL, 0, x, y, width, height );
  };

  Window( Window *parent, char *title, int x, int y, int width, int height ) : killMe(FALSE),
    AWindow( parent, x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
    isPopupWin(FALSE)
  {
    createWindow( (Window *)parent, (Window *)parent, title, (char *)0L, NULL, 0, x, y, width, height );
  };

  // These constructors are mainly for subclasses which implements controls

  Window( Window *parent, ULONG windowStyle, int x, int y,
          int width, int height ) : killMe(FALSE),
    AWindow( parent, x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
    isPopupWin(FALSE)
  {
    createWindow( (Window *)parent, (Window *)parent, "", (char *)0L, NULL, windowStyle,
                   x, y, width, height );
  };

  Window( Window *parent, Window *owner, ULONG windowStyle, int x, int y,
          int width, int height ) : killMe(FALSE),
    AWindow( parent, x, y, width, height ), lastTick(0), acceptDropFiles(FALSE),
    isPopupWin(FALSE)
  {
    createWindow( (Window *)parent, (Window *)owner, "", (char *)0L, NULL, windowStyle,
                   x, y, width, height );
  };

  Window( Window *parent, char *windowClass, char *text,
          ULONG windowStyle,
          int x, int y, int width, int height );

  Window( Window *parent, Window *owner, char *windowClass, char *text,
          ULONG windowStyle,
          int x, int y, int width, int height );

  // Call deleteMe in order to delete this window class after message processing
  // is done. This makes it possible for a window class to delete itself.

  void deleteMe();

	BOOL hasFocus() 
	{
		return WinQueryFocus(HWND_DESKTOP) == hWndFrame || 
           WinQueryFocus(HWND_DESKTOP) == hWndClient;
	}

  void getCursorPos( int &x, int &y );

	void acceptDrop( BOOL accept ) { acceptDropFiles = accept; }
	virtual void filesDropped( char **files, int count ) {}

  ULONG xlatProperties( int properties );
  virtual int execFromMainThread( int task, void *data ) { return 0; }

  void setPointer( int type );

  virtual void setCursor( int x, int y, int width, int height )
  {
    WinCreateCursor( hWndClient, x, y, width, height,
                     CURSOR_SOLID | CURSOR_FLASH, NULL );
    WinShowCursor( hWndClient, TRUE );
  }

  virtual void hideCursor()
  {
    WinDestroyCursor( hWndClient );
  }

	void getCursorExtent( int &width, int &height );

  virtual int setVisible( BOOL visible )
  {
    return WinShowWindow( hWndFrame, visible );
  }

  virtual int setVisible( BOOL visible, BOOL activate )
  {
    return WinShowWindow( hWndFrame, visible );
  }

  // In the focus method, it is possible to set the focus to another window
  // Return 1 if focus is changed in this method

  virtual int focus( BOOL newFocus ) { return 0; }

  // In the focusSet method, it is NOT possible to set the focus to another
  // but cursor creation does work.

  virtual void focusSet( BOOL newFocus ) {}

  virtual void setFocus();

  virtual void activate( BOOL active );

  virtual void setOnTop()
  {
    WinSetWindowPos( hWndFrame, HWND_TOP, 0, 0, 0, 0, SWP_ZORDER );
  }

  virtual void enable( BOOL enabled )
  {
    WinEnableWindow( hWndFrame, enabled );
  }

  virtual void setFont( char *font, int pointSize )
  {
    char fontStr[255];
    sprintf( fontStr, "%d.%s", pointSize, font );
    WinSetPresParam( hWndFrame, PP_FONTNAMESIZE, strlen( fontStr ) + 1, (PVOID)fontStr );
  }

  virtual HWND getHWND() { return hWndClient; };
  virtual HWND getHWNDFrame() { return hWndFrame; };

	void remapCoordinates( Window *from, Window *to, int &x, int &y );

  int getColorIndex( ColorValue color )
  {
    return GpiQueryColorIndex( WinGetPS( hWndClient ), 0, color );
  }

  virtual void paint( Rect *rect, Graphics *g )
  {
    g->setColor( Color::WHITE );
    g->setBackColor( Color::RED );
    g->fillRect( rect );
    FontList *fontList = g->getFonts();
    g->selectFont( &fontList->font[49], 15, 25 );
    g->drawString( 10, 10, "hejsan" );
  }

  virtual Graphics *getGraphics()
  {
    return graphics;
  }

  virtual int getWidth();
  virtual int getHeight();
  virtual void setSize( int width, int height );
  virtual void setPos( int x, int y )
  {
    this->x = x;
    this->y = y;
    WinSetWindowPos( hWndFrame, HWND_TOP, x, y, 0, 0, SWP_MOVE );
  }

  virtual void repaint( Rect *rect )
  {
    repaint( rect, TRUE );
  }

  /*
   * A popup menu is used the same way as an ordinary window.
   * Add menulisteners the same way as for caption menues
   */

  void popupMenu( AMenu *menu, int x, int y )
  {
    HWND hWndMenu = WinCreateMenu( hWndFrame, NULLHANDLE );

    createMenu( hWndMenu, menu );

    // Popup the menu

    WinPopupMenu( ((Window *)parent)->getHWND(),  hWndClient, hWndMenu,
                  x+this->x, y+this->y, 0,
                  PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1 );
  }

  virtual void repaint( Rect *rect, BOOL children );
  virtual ~Window()
  {
    WinDestroyWindow( hWndFrame );
    delete graphics;
  }
  void scrollVert( int delta, int bottom, int top );
  void scrollHorz( int delta, int left, int right );

  // Paints may be pipelined for later processing. In order to process them
  // now for this window, call this method.

  void processPaint();

  // Not implemented on OS/2

	BOOL sendDataToWindow( char *, char * ) { return FALSE; }
	static BOOL popupWindow( char * ) { return TRUE; }
  BOOL isMinimized();
  BOOL isMaximized();
	void restore();

  void setText( char *text )
  {
    WinSetWindowText( hWndFrame, text );
  }
};

/* Desktop: Window class for the desktop */

class Desktop : public Window
{
static Desktop *pDesktop;
public:
  Desktop() : Window() {};
  int getX() { return 0; }
  int getY() { return 0; };
  int getWidth() { return WinQuerySysValue( HWND_DESKTOP, SV_CXFULLSCREEN ); };
  int getHeight() { return WinQuerySysValue( HWND_DESKTOP, SV_CYFULLSCREEN ); };
  virtual HWND getHWND() { return HWND_DESKTOP; };
  virtual HWND getHWNDFrame() { return HWND_DESKTOP; };
  static Desktop *getInstance() { return pDesktop; }
};

/* FrameWindow: A window class whith border and system buttons. This class
   can have menues attached */

class FrameWindow : public Window
{
//  PVOID menuResource;
public:
  FrameWindow( AWindow *parent, STRING title, int x, int y, int width, int height );

  /* Note that addMenu causes size(int,int) to be called! Therefore it is
     a good idea to add a menu last when all controls have been placed */

  void addMenu( AMenu *menu )
  {
    // Delete the old menu

    WinDestroyWindow( WinWindowFromID( hWndFrame, FID_MENU ) );

    HWND hWndMenu = WinCreateMenu( hWndFrame, NULLHANDLE );

    createMenu( hWndMenu, menu );

    // Display changes

    WinSendMsg( hWndFrame, WM_UPDATEFRAME, NULL, NULL );
  }

  void checkMenuItem( int id, BOOL check )
  {
    WinCheckMenuItem( WinWindowFromID( hWndFrame, FID_MENU ), id, check );
  }

  void enableMenuItem( int id, BOOL enable )
  {
    WinEnableMenuItem( WinWindowFromID( hWndFrame, FID_MENU ), id, enable );
  }

  void removeMenu()
  {
    WinDestroyWindow( WinWindowFromID( hWndFrame, FID_MENU ) );
    WinSendMsg( hWndFrame, WM_UPDATEFRAME, NULL, NULL );
  }

  virtual void paint( Rect *rect, Graphics *g )
  {
    g->setColor( Color::LIGHT );
    g->setBackColor( Color::BKGR );
    g->fillRect( rect );
  }

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

  void setCaptionText( char *text )
  {
    WinSetWindowText( hWndFrame, text );
  }
};

/* EntryField
 * properties: BORDER - surrounds the editfield with a border.
 *                      The width of the border can be retrievded from
 *                      getInset()
 */

class EntryField : public Window
{
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

  void size( int oldWidth, int oldHeight )
  {
    if( isBordered )
    {
      internalEntryField->setPos( 2, 2 );
      internalEntryField->setSize( width - 4, height - 4 );
    }
    else
    {
      internalEntryField->setPos( 0, 0 );
      internalEntryField->setSize( width, height );
    }
  }

  virtual void enable( BOOL enabled )
  {
    Window::enable( enabled );
    internalEntryField->enable( enabled );
  }

  // Returns border width

  static getInset() { return 2; }

  void paint( Rect *rect, Graphics *g )
  {
    if( isBordered )
    {
      g->drawShadedBox( 0, 0, width, height, Color::SHADOW, Color::LIGHT );
      g->drawShadedBox( 1, 1, width-2, height-2, Color::SHADOW, Color::BKGR );
    }
  }

  void setText( char *text )
  {
    WinSetWindowText( internalEntryField->getHWND(), text );
    WinSendMsg( internalEntryField->getHWND(), EM_SETSEL,
                MPFROM2SHORT( 0, 65535 ), (MPARAM)0L );
  }

  void selectText()
  {
    WinSendMsg( internalEntryField->getHWND(), EM_SETSEL,
                MPFROM2SHORT( 0, 65535 ), (MPARAM)0L );
  }

  void selectText( int start, int end )
  {
    WinSendMsg( internalEntryField->getHWND(), EM_SETSEL,
                MPFROM2SHORT( start, end ), (MPARAM)0L );
  }

  char *getText()
  {
    WinQueryWindowText( internalEntryField->getHWND(), maxLength, selectedText );
    return selectedText;
  }

  int getTextLength()
  {
    return WinQueryWindowTextLength( internalEntryField->getHWND() );
  }

  virtual void setFocus();

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

    WinSendMsg( internalEntryField->getHWND(), EM_SETTEXTLIMIT,
                (MPARAM)maxLength, (MPARAM)0 );
  }

  void setReadOnly( BOOL state )
  {
    WinSendMsg( internalEntryField->getHWND(), EM_SETREADONLY,
                (MPARAM)state, (MPARAM)0 );
  }

  void clearSelection()
  {
    WinSendMsg( internalEntryField->getHWND(), EM_CLEAR,
                (MPARAM)0, (MPARAM)0 );
  }

  Window *getInternal() { return internalEntryField; }

  virtual ~EntryField()
  {
    delete selectedText;
    delete internalEntryField;
  }

  virtual void addMouseListener( AMouseListener *mouseListener )
  {
    internalEntryField->addMouseListener( mouseListener );
  }
};

/* ListBox
 * properties: NOADJUSTSIZE - Listboxes are by default adjusted in size
 *                            in order to make full entries fit. Disable
 *                            by adding this property.
 *             SORTED       - Sort the entries
 *             MULTIPLESEL  - Allows multiple selection
 */

class ListBox : public Window
{
  BOOL sort;
public:
  ListBox( Window *parent, int properties, int x, int y, int width, int height,
           BOOL sort );
  ListBox( Window *parent, Window *owner, int properties, int x, int y, int width, int height,
           BOOL sort );

  void clearItems()
  {
    WinSendMsg( hWndFrame, LM_DELETEALL, NULL, NULL );
  }

  void insertItem( int index, char *text )
  {
    WinSendMsg( hWndFrame, LM_INSERTITEM, MPARAM(0), MPARAM(text) );
  }

  void addItem( char *text )
  {
    WinSendMsg( hWndFrame, LM_INSERTITEM, MPARAM((sort) ? LIT_SORTASCENDING : LIT_END), MPARAM(text) );
  }

  void selectItem( int index )
  {
    WinSendMsg( hWndFrame, LM_SELECTITEM, MPFROMSHORT( index ),
                MPFROMSHORT( TRUE ));
  }

  int getItems()
  {
    return (int)WinSendMsg( hWndFrame, LM_QUERYITEMCOUNT, NULL, NULL );
  }

	// Start with -1

	int getNextSelectedIndex( int index )
  {
  	if( index == -1 ) index = LIT_FIRST;
    int selected = (int)WinSendMsg( hWndFrame, LM_QUERYSELECTION,
                                    MPFROMSHORT(index), 0L );
    return (selected == LIT_NONE) ? -1 : selected;
  }

  // Returns -1 if none selected

  int getSelectedIndex()
  {
  	return getNextSelectedIndex( 0 );
  }

  void getItem( int index, int len, char *text )
  {
    WinSendMsg( hWndFrame, LM_QUERYITEMTEXT, MPFROM2SHORT( index, len ),
                MPARAM(text) );
  }

  int getSelectedItem( int len, char *text )
  {
    int selected = getSelectedIndex();
    getItem( selected, len, text );
    return selected;
  }

	int findItem( char *string, BOOL prefix )
  {
  	int found = (int)WinSendMsg( hWndFrame, LM_SEARCHSTRING,
    									 					 MPFROM2SHORT( (prefix) ? LSS_PREFIX :
                                 						 						  LSS_CASESENSITIVE,
                                               LIT_FIRST),
																 MPARAM(string) );

    return (found == LIT_NONE || found==LIT_ERROR) ? -1 : found;
  }
  virtual int getHeight();
};

class SComboBox : public Window, public virtual AMouseListener,
                 public virtual ASelectionListener, public virtual AKeyListener,
                 public virtual AFocusListener
{
  friend Window;
protected:
  class ComboListBox;
private:
  friend ComboListBox;

  EntryField *entryField;
  int listHeight, listVisible, dir;
  void drawArrow( Graphics *g, int x, int y, int width, int height, int dir );

  ComboListBox *listBox;

  void repaintButton()
  {
    Rect rect( width - buttonWidth - 2, 2,
               width - 2, height - 2 );
    repaint( &rect );
  }

  HWND hWndTest;

protected:
  static int buttonWidth;
  int buttonUp;
  BOOL down, readOnly;
  class ComboListBox : public ListBox
  {
    SComboBox *comboBox;
  public:
    ComboListBox( SComboBox *comboBox, Window *parent, int properties,
                  int x, int y, int width, int height ) :
      ListBox( Desktop::getInstance(), parent, properties|FILTERDIALOGTAB, x, y, width, height, FALSE )
      {
        this->type = COMBOLISTBOX;
        this->comboBox = comboBox;
      }

    virtual int focus( BOOL newFocus );
    virtual void setFocus();
  };

  virtual BOOL actionPerformed( AMouseEvent *e );
  virtual void actionPerformed( ASelectionEvent *s );
  virtual void actionPerformed( AFocusEvent *focusEvent );
  virtual BOOL actionPerformed( AKeyEvent *keyEvent );

  void placeListBox();

  int orgHeight, orgListHeight, maxLength;

public:
  static int STATIC;
  SComboBox( Window *parent, int properties, int x, int y,
            int width, int height, int listHeight );

  virtual void addKeyListener( AKeyListener *keyListener )
  {
    entryField->addKeyListener( keyListener );
  }

  void setReadOnly( BOOL state )
  {
    readOnly = state;
    entryField->setReadOnly( state );
  }

  virtual void setFocus()
  {
    entryField->setFocus();
  }

  virtual int focus( BOOL newFocus );

  virtual void enable( BOOL enabled )
  {
    Window::enable( enabled );
    entryField->enable( enabled );
  }

  void setMaxLength( int chars )
  {
    maxLength = chars;
    entryField->setMaxLength( chars );
  }

  virtual void paint( Rect *rect, Graphics *g );

  virtual void size( int oldWidth, int oldHeight );

  void addItem( char *text )
  {
    listBox->addItem( text );
  }

  void insertItem( int index, char *text )
  {
    listBox->insertItem( index, text );
  }

  int getItems()
  {
    return listBox->getItems();
  }

  void getItem( int index, int len, char *text )
  {
    listBox->getItem( index, len, text );
  }

  void selectItem( int index )
  {
    char *text = new char[maxLength+1];
    listBox->selectItem( index );
    listBox->getSelectedItem( maxLength, text );
    entryField->setText( text );
    delete text;
  }

  char *getEditText()
  {
    return entryField->getText();
  }

  int getTextLength()
  {
    return entryField->getTextLength();
  }

  virtual void setText( char *text );

  void selectText()
  {
    entryField->selectText();
  }

  void selectText( int start, int end )
  {
    entryField->selectText( start, end );
  }

  void clearSelection()
  {
  	entryField->clearSelection();
  }

	void clearItems()
	{
		clearSelection();
		listBox->clearItems();
	}

  int getSelectedItem( int len, char *text )
  {
    return listBox->getSelectedItem( len, text );
  }

  static int getInset( int horz );
  virtual int getEditBoxHeight();
  virtual void setSize( int width, int height );
  virtual int getClientHeight() { return height;}
  virtual int getHeight() { return height;}

	~SComboBox() 
	{
		//delete entryField;
	}
};

/* RadioButton: To form a new group: Set property AWindow::newGroup.
                All buttons in the group should be created successively
                and have the property AWindow::inGroup. */

class RadioButton : public Window
{
public:
  RadioButton( Window *parent, char *text, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)WC_BUTTON, text,
            xlatProperties(properties) | BS_AUTORADIOBUTTON,
            x, y, width, height )
  {
    type = RADIOBUTTON;
    parent->addControl( this );
  }
  int queryCheck()
  {
    return (int)WinSendMsg( hWndFrame, BM_QUERYCHECK, (MPARAM)NULL, (MPARAM)NULL );
  }
  void check( int state )
  {
    WinSendMsg( hWndFrame, BM_SETCHECK, MPFROMSHORT(state), (MPARAM)NULL );
  }
};

/* CheckBox */

class CheckBox : public Window
{
public:
  CheckBox( Window *parent, char *text, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)WC_BUTTON, text,
            xlatProperties(properties) | BS_AUTOCHECKBOX,
            x, y, width, height )
  {
    type = CHECKBOX;
    parent->addControl( this );
  }
  int queryCheck()
  {
    return (int)WinSendMsg( hWndFrame, BM_QUERYCHECK, (MPARAM)NULL, (MPARAM)NULL );
  }
  void check( int state )
  {
    WinSendMsg( hWndFrame, BM_SETCHECK, MPFROMSHORT(state), (MPARAM)NULL );
  }
};

/* Group box */

class GroupBox : public Window
{
public:
  GroupBox( Window *parent, char *text, int properties, int x, int y,
            int width, int height ) :
    Window( parent, (char*)WC_STATIC, text,
            xlatProperties(properties) | SS_GROUPBOX | DT_LEFT | DT_TOP | DT_MNEMONIC | WS_VISIBLE | WS_GROUP,
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
    Window( parent, (char*)WC_BUTTON, text,
            xlatProperties(properties) | BS_PUSHBUTTON,
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
  int firstLine, lastLine;
  int adjustPos( int pos )
  {
    int orgPos = pos;

    if( lastLine > 32767 )
      pos = (((pos * 1000) / lastLine) * 32767) / 1000;

    if( orgPos > 0 && pos == 0 )
      pos = 1;

    if( orgPos < lastLine && pos == lastLine )
      pos = lastLine-1;

    return pos;
  }

public:
  static int PAGERIGHT, PAGELEFT, LINERIGHT, LINELEFT,
             PAGEDOWN, PAGEUP, LINEDOWN, LINEUP, HORZRELEASED, VERTRELEASED,
             HORZTRACK, VERTTRACK;
  Scrollbar( Window *parent, int properties, int x, int y, int width, int height ) :
    Window( parent, (char*)WC_SCROLLBAR, "",
            xlatProperties(properties) |
              ((width > height) ? SBS_HORZ : SBS_VERT) | SBS_AUTOTRACK,
            x, y, width, height )
  {
    type = SCROLLBAR;
    parent->addControl( this );
    firstLine = lastLine = 0;
  }

  int convertScrollPosToReal( int pos )
  {
    if( lastLine > 32767 )
      return (((pos * 1000) / 32767) * lastLine) / 1000;
    else
      return pos;
  }

  // Select the size of the thumb

  void setRatio( int visible, int total )
  {
    WinSendMsg( hWndFrame,  SBM_SETTHUMBSIZE, MPFROM2SHORT( visible, total ),
                NULL );
  }

  // Position within range

  void setThumbPos( int pos )
  {
    pos = adjustPos( pos );

    WinSendMsg( hWndFrame, SBM_SETPOS, MPFROMSHORT( pos ), NULL );
  }

  // Scrollbar range

  void setRange( int pos, int first, int last )
  {
    firstLine = first;
    lastLine = last;

    if( last > 32767 )
      last = 32767;

    pos = adjustPos( pos );

    WinSendMsg( hWndFrame, SBM_SETSCROLLBAR, MPFROMSHORT(pos),
                MPFROM2SHORT( first, last ) );
  }

	void setAll( int pos, int first, int last, int visible, int total )
  {
  	setRatio( visible, total );
    setRange( pos, first, last );
	}

  // If dir == 0, the height of the horizontal scrollbar is returned,
  // othersize the height of the vertical scrollbar

  static int getInset( int dir )
  {
    return WinQuerySysValue( HWND_DESKTOP, dir ? SV_CYHSCROLL : SV_CXVSCROLL );
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
    Window( parent, (char*)WC_STATIC, text,
            xlatProperties(properties) | SS_TEXT | DT_VCENTER | DT_WORDBREAK | DT_MNEMONIC,
            x, y, width, height )
  {
    type = LABEL;
    parent->addControl( this );
  }
  ULONG xlatProperties( int properties )
  {
    ULONG windowStyle = 0;

    if( properties & LEFT )
      windowStyle |= DT_LEFT;

    if( properties & CENTER )
      windowStyle |= DT_CENTER;

    if( properties & RIGHT )
      windowStyle |= DT_RIGHT;

    return Window::xlatProperties( properties ) | windowStyle;
  }
};

class PopupMessage
{
  int result;
public:
  static int error, warning, message, query, ok, cancel, yes, no, 
             querywithwarning, querywithwarningNoCancel;
  PopupMessage( Window *parent, char *title, char *message, int type )
  {
    int style = 0;

    if( type == error ) style =  MB_OK|MB_ICONEXCLAMATION;
    if( type == warning ) style =  MB_OK|MB_WARNING;
    if( type == this->message ) style =  MB_OK|MB_INFORMATION;
    if( type == query ) style =  MB_YESNOCANCEL|MB_QUERY;
    if( type == querywithwarning ) style = MB_YESNOCANCEL|MB_WARNING;

    result = WinMessageBox( HWND_DESKTOP, parent->getHWND(), message, title,
                            1000, MB_MOVEABLE | style );
  }
  int getResult()
  {
    switch( result )
    {
      case MBID_OK : return ok;
      case MBID_CANCEL : return cancel;
      case MBID_YES : return yes;
      case MBID_NO : return no;
    }
    return 0;
  }
};
