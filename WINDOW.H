#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Iterator: Holds a static counter. Is used for creating new window id's */

class Iterator
{
  static int iterator;
public:
  static int getNext()
  {
    if( iterator > 5000 ) iterator = 100;
    return iterator++;
  }
};

/* Window base class
 * This class holds methods and variables that are common for all
 * platforms. User windows should inherit from the Window class.
 *
 * No listeners or controls are deleted on exit
 * Controls should be deleted in the Window class before window destruction
 */

enum controlMove { LOCK = 0, PROPORTIONAL = 1, FOLLOW = 2 };

class AWindow : public AObject
{
protected:
  int width, height, x, y, id;
  controlMove xMove, yMove, wMove, hMove;
  BOOL initSize, followParentSize;
  AWindow *parent;
  Vector<AMouseListener> mouseListener;
  Vector<AMenuListener> menuListener;
  Vector<AButtonListener> buttonListener;
  Vector<ASelectionListener> selectionListener;
  Vector<AScrollbarListener> scrollbarListener;
  Vector<AFocusListener> focusListener;
  Vector<AKeyListener> keyListener;
  Vector<AWinMoveListener> moveListener;
  Vector<AWindow> controls;
  virtual BOOL performMouseEvent( Point *pos, int button, int event );
  virtual void performMenuEvent( int id );
  virtual void performButtonEvent( AWindow *button, int id );
  virtual void performScrollbarEvent( int id, int action, int pos );
  virtual void performFocusEvent( int focusItem );
  virtual BOOL performKeyEvent( AWindow *window, int modifiers, int key );
  virtual void performMoveEvent( int oldX, int oldY, int newX, int newY );
  int type;
	ULONG filterDialogKeys;
public:
  /*
   * All window types supported. Note that these should not be specified
   * by the user.
   */
  static int WINDOW, COMBOBOX, ENTRYFIELD, LISTBOX, PUSHBUTTON, RADIOBUTTON,
             DIALOG, GROUPBOX, LABEL, CHECKBOX, SCROLLBAR, COMBOLISTBOX, 
						 NOTEBOOK, INTERNALENTRYFIELD, SCOMBOBOX;
  /* These constants control window behaviour. Refer to OS/2 PM programming
   * reference.
   */
  static ULONG ADJUSTSIZE, AUTOSCROLL, ISTABSTOP, NEWGROUP, INGROUP, NOADJUSTSIZE,
             ISMODAL, NOTITLE, CENTER, BORDER, ISDEFAULT, CLIPSIBLINGS,
             SYNCPAINT, MULTIPLESEL, NUMERIC, FILTERDIALOGARROWS, 
						 FILTERDIALOGTAB, PASSDIALOGKEYS, POPUP, ISMODALDONTDISABLEPARENT;

  static int MENU_CLOSE;

  int properties;

	void setDialogFilter( ULONG properties );

  virtual void performSelectionEvent( int selected );

	virtual BOOL allowPlacementResize() { return TRUE; }

  AWindow( AWindow *parent, int x, int y, int width, int height ) :
    mouseListener(FALSE), menuListener(FALSE), buttonListener(FALSE),
    selectionListener(FALSE), scrollbarListener(FALSE), focusListener(FALSE),
    keyListener(FALSE), moveListener(FALSE), controls(FALSE), xMove(LOCK), yMove(LOCK),
    wMove(LOCK), hMove(LOCK), initSize(FALSE), followParentSize(TRUE)
  {
    id = -1;
    type = WINDOW;
    this->parent = parent;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->properties = 0;
  }

  AWindow( int x, int y, int width, int height ) :
    mouseListener(FALSE), menuListener(FALSE), buttonListener(FALSE),
    selectionListener(FALSE), scrollbarListener(FALSE), focusListener(FALSE),
    keyListener(FALSE), moveListener(FALSE),controls(FALSE), xMove(LOCK), yMove(LOCK),
    wMove(LOCK), hMove(LOCK), initSize(FALSE), followParentSize(TRUE)
  {
    id = -1;
    type = WINDOW;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->properties = 0;
  }

  AWindow()  :
    mouseListener(FALSE), menuListener(FALSE), buttonListener(FALSE),
    selectionListener(FALSE), scrollbarListener(FALSE), focusListener(FALSE),
    keyListener(FALSE), moveListener(FALSE),controls(FALSE), xMove(LOCK), yMove(LOCK),
    wMove(LOCK), hMove(LOCK), initSize(FALSE), followParentSize(TRUE)
  {
    type = WINDOW; id = -1;
    this->properties = 0;
  }

  virtual int getX() { return x; }
  virtual int getY() { return y; }
  virtual int getWidth() { return width; }
  virtual int getHeight() { return height; }
  virtual int getClientWidth() { return width; }
  virtual int getClientHeight() { return height; }
  void setXMove( controlMove move ) { xMove = move; }
  void setYMove( controlMove move ) { yMove = move; }
  void setWMove( controlMove move ) { wMove = move; }
  void setHMove( controlMove move ) { hMove = move; }
  void setFollowParentSize( BOOL follow ) { followParentSize = follow; }
  BOOL getFollowParentSize() { return followParentSize;}
  BOOL getSizeInit() { return initSize; }
  virtual BOOL setSizeInit( BOOL setInit );
  controlMove getXMove() { return xMove; }
  controlMove getYMove() { return yMove; }
  controlMove getWMove() { return wMove; }
  controlMove getHMove() { return hMove; }
  int getType() { return type; }
  void setType( int type ) { this->type = type; }
	BOOL getFilterDialogArrows() { return filterDialogKeys & 1; }
	BOOL getFilterDialogTab() { return filterDialogKeys & 2; }
	BOOL getPassDialogKeys() { return filterDialogKeys & 4; }
	ULONG getFilter() { return filterDialogKeys; }
  int getProperties() { return properties; }
  virtual int setVisible( BOOL visible ) { return TRUE; };
  AWindow *getParent() { return parent; }
  virtual void addMouseListener( AMouseListener *mouseListener )
  {
    this->mouseListener.add( mouseListener );
  }
  virtual void addSelectionListener( ASelectionListener *selectionListener )
  {
    this->selectionListener.add( selectionListener );
  }
  virtual void addMenuListener( AMenuListener *menuListener )
  {
    this->menuListener.add( menuListener );
  }
  virtual void addButtonListener( AButtonListener *buttonListener )
  {
    this->buttonListener.add( buttonListener );
  }
  virtual void addScrollbarListener( AScrollbarListener *scrollbarListener )
  {
    this->scrollbarListener.add( scrollbarListener );
  }
  virtual void removeMenuListener( AMenuListener *menuListener )
  {
    this->menuListener.removeObject( menuListener );
  }

  virtual void addKeyListener( AKeyListener *keyListener )
  {
    this->keyListener.add( keyListener );
  }

  virtual void addFocusListener( AFocusListener *focusListener )
  {
    this->focusListener.add( focusListener );
  }

  virtual void addMoveListener( AWinMoveListener *moveListener )
  {
    this->moveListener.add( moveListener );
  }
  virtual void removeMoveListener( AWinMoveListener *moveListener )
  {
    this->moveListener.removeObject( moveListener );
  }

  virtual void addControl( AWindow *child )
  {
    controls.add( child );
  }

  virtual void setControlsVisible();

  int belowPrevious()
  {
    if( controls.getSize() > 0 )
      return controls.getObjectAt(controls.getSize()-1)->getY();
    return 0;
  }

  AWindow *getControl( int id );
  AWindow *getLastControl()
  {
    if( controls.getSize() > 0 )
      return (AWindow *)controls.getObjectAt(controls.getSize()-1);
    else
      return NULL;
  }
  virtual void size( int oldWidth, int oldHeight ) {}
  virtual void move() { performMoveEvent( -1, -1, getX(), getY());}

  // Return TRUE if the app should terminate when a window close 
  // is issued

  virtual BOOL close() { return TRUE; }
};

/* Desktop */

class ADesktop : public AWindow
{
public:
  ADesktop() : AWindow( (AWindow *)0L, 0, 0, 0, 0 ) {}
};

/* AMenuEntry
 * Base class for menu entrys
 * Menues implements the compose design model
 */

class AMenuEntry : public AObject
{
public:
  char *string, *shortcut;
  int type, id;
  static int SEPARATOR, SUBMENU, MENUITEM;
  BOOL checked, enabled;
  AMenuEntry( char *string, int id, int type )
  {
    checked = FALSE;
    enabled = TRUE;
    if( string == 0L )
      this->string = 0L;
    else
    {
      this->string = new char[strlen( string )+1];
      strcpy( this->string, string );
    }
    this->type = type;
    this->id = id;
    shortcut = NULL;
  }

  void deleteInternals()
  {
    if( string ) delete string;
  }

  void updateShortcut( char *shortcut )
  {
    if( this->shortcut )
      delete this->shortcut;
    if( shortcut == 0L )
      this->shortcut = 0L;
    else
    {
      this->shortcut = new char[strlen( shortcut )+1];
      strcpy( this->shortcut, shortcut );
    }
  }

  void check( BOOL checked )
  {
    this->checked = checked;
  }

  void enable( BOOL enabled )
  {
    this->enabled = enabled;
  }

  virtual ~AMenuEntry()
  {
		if( shortcut ) delete shortcut;
    deleteInternals();
  }
};

/* AMenuItem
 * Holds a menu item. See example code how to create menus
 */

class AMenuItem : public AMenuEntry
{
public:
  AMenuItem( char *string, int id, int type ) :
    AMenuEntry( string, id, type ) {};
};

/* AMenu
 * A Menu. Contains menuitems or new menus.
 * Note that all menu items in the menu will be deleted.
 */

class AMenu : public AMenuEntry
{
  Vector<AMenuEntry> content;
public:
  AMenu( char *string, int id ) :
    AMenuEntry( string, id, AMenuEntry::SUBMENU ) {}
  void addMenuItem( AMenuEntry *menuItem ) { content.add( menuItem ); }
  void insertMenuItemAt( int index, AMenuEntry *menuItem)
  {
    if( string ) { free( string ); string = NULL; }
    content.insertAt( index, menuItem );
  }
  void removeMenuItemAt( int index ) { content.removeAt( index ); }
  AMenuEntry *getItemAt( int index )
  {
    return (AMenuEntry *)content.getObjectAt( index );
  }
  int items() { return content.getSize(); }
  void removeAllItems() { content.removeAll(); }
};

