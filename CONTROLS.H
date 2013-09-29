/******************************************************************************
 * Notebook control
 *
 * Windows are added to the notebook by calling add with the window and
 * tab text.
 *
 * The extra space added by the tabs can be retrieved with getInset()
 * A ButtonListener may be attached to catch close events.
 */

#include "popupwin.h"

class Notebook : public Window, public virtual AMouseListener, 
                 public virtual AMenuListener
{
  struct Tab : public AObject
  {
    char *title;
    int width, stringStart;
    Window *window;
    ColorValue textColor;
    Tab( char *title, Window *window )
    {
      this->title = new char[strlen(title)+1];
      strcpy( this->title, title );
      this->window = window;
      textColor = Color::BLACK;
      width = 0;
      stringStart = 0;
    }
		void setTitle( char *title )
		{
      delete this->title;
      this->title = new char[strlen(title)+1];
      strcpy( this->title, title );
		}
    ~Tab()
    {
      delete title;
    }
  };

  Font *font;
  int fontSize, buttonWidth, leftButton, rightButton;
  Vector<Tab> tab;
  int activeTab;
  ColorValue backgroundColor, activeColor;
  ASelectionListener *listener;
	AButtonListener *closeListener;

  int getTabsAreaHeight( Graphics *g );
  int tabXstart, spacing;

  void leftJustifyTablist();
  void rightJustifyTablist();
  void setActiveTabVisible();
  void drawButton( Graphics *g, int pos, int dir, int tabsHeight );
  void drawButton( Graphics *g, int pos, int dir );
  void drawTab( Graphics *g, int i, int xPos, int tabsHeight );
  void setTabsWidth();
  void setFont( char *font, int fontSize )
  {
    this->font = getGraphics()->getFontFromName( font, 0 );
    this->fontSize = fontSize;
  }

  void setWindowSize( Window *window );
  void resizeWindows();
  void setTab( int tab, BOOL deactivateOld );
	void repaintTabs();
  void create( ColorValue activeColor );
	BOOL allowClose, down, closeActive;
	void drawCloseButton( Graphics *g, BOOL up );
	void repaintCloseButton();
	int menuTabClicked;
	int tabAtPos( int x );
public:
  static int VERTSPACE, TABINTERIORSPACING, PUSH, EXTRA, ALLOWCLOSE,
	           CLOSEBUTTONWIDTHHEIGHT;
  virtual BOOL actionPerformed( AMouseEvent *mouseEvent );
	virtual BOOL allowPlacementResize() { return FALSE; }
  void actionPerformed( AMenuEvent *menuEvent );

  Notebook( Window *parent, int x, int y, int width, int height, 
	          int properties )
    : Window( parent, x, y, width, height ), 
		  allowClose( properties & ALLOWCLOSE )
  {
    create( Color::WHITE );
  }

  Notebook( Window *parent, int x, int y, int width, int height, 
	         ColorValue activeColor, int properties  )
    : Window( parent, x, y, width, height ), 
		  allowClose( properties & ALLOWCLOSE )
  {
	  create( activeColor );
  }

  virtual BOOL setSizeInit( BOOL setInit );

  static int getTabInset()
  {
    return PUSH + VERTSPACE + EXTRA;
  }

  int getTabs()
  {
    return tab.getSize();
  }

  void nextTab();
  void prevTab();

  int getActiveTab() { return activeTab; }

  Window *getActiveWindow() 
  {
  	if( activeTab < tab.getSize() && activeTab >= 0 )
	  	return tab.getObjectAt( activeTab )->window; 
    else
    	return NULL;
  }

  Window *getWindow( int pos ) { return tab.getObjectAt( pos )->window; }

  void add( Window *window, char *title );

  void remove( Window *window );

  void setSelectionListener( ASelectionListener *listener )
  {
    this->listener = listener;
  }

  void setCloseListener( AButtonListener *listener )
  {
    this->closeListener = listener;
  }

  void size( int oldWidth, int oldHeight );

  void setTab( int toTab ) { setTab( toTab, TRUE ); }
  void setTab( Window *window ) { setTab( getWindowTab( window ), TRUE ); }
  void setTabTextColor( int ctab, ColorValue color );
  void setTabTitle( int ctab, char *title );
  int getWindowTab( Window *window );

  void paint( Rect *rect, Graphics *g );
  virtual void enable( BOOL enabled );
  virtual ~Notebook() {}
};

/******************************************************************************
 * Combobox control
 *
 * The combobox control is a combined EntryField and ListBox
 *
 */

class ComboBox : public Window, public virtual AMouseListener,
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
  BOOL down;
  class ComboListBox : public ListBox
  {
    ComboBox *comboBox;
  public:
    ComboListBox( ComboBox *comboBox, Window *parent, int properties,
                  int x, int y, int width, int height ) :
      ListBox( parent, properties|FILTERDIALOGTAB, x, y, width, height, FALSE )
      {
        this->type = COMBOLISTBOX;
        this->comboBox = comboBox;
      }

    virtual int focus( BOOL newFocus )
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
  };

  virtual BOOL actionPerformed( AMouseEvent *e );
  virtual void actionPerformed( ASelectionEvent *s );
  virtual void actionPerformed( AFocusEvent *focusEvent );
  virtual BOOL actionPerformed( AKeyEvent *keyEvent );

  void placeListBox();

  int orgHeight, orgListHeight, maxLength;

public:
  ComboBox( Window *parent, int properties, int x, int y,
            int width, int height, int listHeight );

  virtual void addKeyListener( AKeyListener *keyListener )
  {
    entryField->addKeyListener( keyListener );
  }

  void setReadOnly( BOOL state )
  {
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

  char *getSelectedText()
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

  static int getInset( int horz );
	~ComboBox() 
	{
		//delete entryField;
	}
};

/******************************************************************************
 * Toolbar control
 * Note that toolbars should be created with it's initial height in the
 * constructor
 *
 * Tools are added using addImageButton(..), and spaces with addSpaceButton()
 *
 * The toolbar can be manipulated by the user. No intervening by the caller
 * is required, except if the user wishes to modify a button.
 * The toolbar will then call modifyButton from the ToolStateServer
 * with the properties for the selected button.
 */

/* The ToolStateServer "owns" the toolbar. Changes are reported to the tsserver,
 * and the tsserver show the modify dialog.
 * A ToolStateServer is added by calling
 * addToolStateServer( ToolStateServer * );
 */

class ToolStateServer
{
public:
  virtual BOOL queryToolActive( char *command ) { return TRUE; }

  // The parameters returned must not be deleted!
  // Deletion of them will be handled by the toolbar
  // Note that only image (command) buttons can be modified
  virtual BOOL modifyButton( char **fileName, char **command,
                             char **helpText ) {return FALSE;}

	// Used to notify a button deleted
	virtual void buttonDeleted( int id ) {}
	virtual void buttonMoved() {};
};

class Toolbar;

class ToolbarThread : public Thread
{
  Toolbar *toolbar;
  BOOL exitThread;
public:
  ToolbarThread( Toolbar *toolbar ) : Thread(TRUE), exitThread(FALSE) { this->toolbar = toolbar; }
  virtual void run();
  void exit() { exitThread = TRUE; }
};

class Toolbar : public Window, public virtual AMenuListener
{
  BmpGraphics bmpGraphics;
  char *name;
  Vector<AToolbarListener> toolbarListener;
  ToolStateServer *toolStateServer;

  /***************************************************************
   * ToolButton
   */

  class ImageButton;
  class Space;
  friend ImageButton;
  friend Space;

  class ToolButton : public Window, public virtual AMouseListener
  {
    int moving, button, id;
		BOOL isActive;
    Toolbar *toolbar;
  protected:
    char *command, *description;
    int type, state;

    virtual BOOL actionPerformed( AMouseEvent *e );

    int queryToolActive()
    {
/*      if( ((Toolbar *)parent)->toolStateServer != NULL )
        return (((Toolbar *)parent)->toolStateServer->queryToolActive( command ))
               ? Graphics::ACTIVE : Graphics::INACTIVE ;
      else
        return Graphics::ACTIVE;*/
			return (isActive) ? Graphics::ACTIVE : Graphics::INACTIVESHADE;
    }

  public:
    virtual int equals( AObject * ) {return FALSE;}
    static int UP, DOWN;
    ToolButton( char *command, char *description, Toolbar *parent, int id, 
								int x, int y, int width, int height ) :
      Window( parent, xlatProperties(CLIPSIBLINGS|SYNCPAINT), x, y, width, height ),
			isActive(TRUE), id(id)
    {
      toolbar = parent;
      this->command = new char[strlen( command ) + 1];
      this->description = new char[strlen( description ) + 1];
      state = UP;
      strcpy( this->command, command );
      strcpy( this->description, description );
      addMouseListener( this );
      moving = FALSE;
      button = AMouseEvent::RELEASE;
    }
    char *getDescription() { return description; };
    char *getCommand() { return command; }
    int getType() { return type; }
		int getId() { return id; }
		void setActive( BOOL isActive );
    virtual ~ToolButton()
    {
      delete command;
      delete description;
    }
  };

  /***************************************************************
   * ImageButton
   */

  class ImageButton : public ToolButton
  {
  protected:
    Bitmap *image;
    char *filename;
    BmpGraphics *g;
  public:
    ImageButton( char *filename, BmpGraphics *g, char *command, char *description, Toolbar *parent,
                 int id, int x, int y, int width, int height );

    // The parameters for this call must not be deleted by the caller!

    void modify( char *filename, char *command, char *description )
    {
      delete image;
      this->description = description;
      this->command = command;
      this->filename = filename;
      image = g->readBitmap( filename );
      this->filename = new char[strlen(filename)+1];
      strcpy( this->filename, filename );
      repaint( NULL );
    }
    virtual BOOL actionPerformed( AMouseEvent *e );
    virtual void paint( Rect *rect, Graphics *g );
    char *getFilename() { return filename; }
    virtual ~ImageButton()
    {
      delete filename;
      delete image;
    }
  };

  /***************************************************************
   * Space
   */

  class Space : public ToolButton, public virtual AMouseListener
  {
  public:
    Space( Toolbar *parent, int x, int y, int width, int height ) :
      ToolButton( "", "", parent, -1, x, y, width, height )
    {
      type = Toolbar::SPACE;
    }
    virtual void paint( Rect *rect, Graphics *g );
    virtual BOOL actionPerformed( AMouseEvent *e );
  };

  Vector<ToolButton> buttonList;
  int buttonWidth, horzMargin, spacing;
  static int vertMargin, buttonMargin;
  int lastPickedButtonPos, hotspot;
  ToolButton *pickedUpButton;
  friend ToolButton;

  void notifyObservers( char *command );
  ToolButton *toolButtonAt( int x, int y );
  ToolbarThread *tbThread;
protected:
  int lastX, lastY, lastSampleX, lastSampleY;
  PopupWindow *popupWindow;
  ToolButton *activeBubbleButton;
public:
  static int IMAGE, SPACE;
  /***************************************************************
   * Toolbar constructor
   ***************************************************************/

  Toolbar( Window *parent, char *name, int x, int y, int width, int height )
    : Window( parent, xlatProperties(SYNCPAINT), x, y, width, height ), 
      popupWindow( NULL ), buttonList(TRUE), toolbarListener(FALSE)
  {
    this->name = new char[strlen(name)+1];
    strcpy( this->name, name );
    buttonList.deleteContent( FALSE );  // Do not delete buttons when removed from list
    horzMargin = 5;
    spacing = 2;
    toolStateServer = NULL;
    tbThread = new ToolbarThread(this);
    tbThread->start(1024);  // Start the toolhelp thread
  }

  void checkPopup();

  void addToolbarListener( AToolbarListener *toolbarListener )
  {
    this->toolbarListener.add( toolbarListener );
  }

  void addToolStateServer( ToolStateServer *toolStateServer )
  {
    this->toolStateServer = toolStateServer;
  }

  static int getInset()
  {
    return vertMargin;
  }
  void pushButtons( int start, int end, int dir );
  void pickupButton( ToolButton *button, int xPosInside );
  void moveButton( int xPos );
  void placeButton( int xPos );
  void recalcButtonsPos();
	void setButtonActive( int id, BOOL active );
  virtual void size( int oldWidth, int oldHeight );
  int xPosAt( int pos );

  // addImageButton returns FALSE if the command is already present in the
  // toolbar.

  BOOL addImageButton( char *filename, char *command, char *description, int id );
  BOOL addSpace();

  void actionPerformed( AMenuEvent *menuEvent );

  virtual void paint( Rect *rect, Graphics *g )
  {
    g->setBackColor( Color::BKGR );
    g->fillRect( rect );
    g->setColor( Color::LIGHT );
    g->drawLine( 0, height - 1, width, height - 1 );
    g->setColor( Color::SHADOW );
    g->drawLine( 0, 0, width, 0 );
  }

	int getToolsCount() { return buttonList.getSize(); }
	BOOL getToolAt( int pos, int &type, char **command, char **image, char **description, int &id );

  virtual ~Toolbar()
  {
    delete name;
    buttonList.deleteContent( TRUE );
    tbThread->exit();
  }

  virtual int execFromMainThread( int task, void * );
};

class ValueSet : public Window, public virtual AMouseListener, public virtual AKeyListener
{
	ColorValue *grid;
	char **text;
	int cols, rows;
	int position;
	int type;
	BOOL hasFocus;
public:
	static int RGBCOLOR, TEXT;
	ValueSet( Window *parent, int properties, int x, int y, int width, int height, int cols, int rows, int type );
	virtual void paint( Rect *rect, Graphics *g );
	void setCursorPos( int col, int row );
	void getCursorPos( int &col, int &row );
	void setColor( int col, int row, ColorValue color );
	ColorValue getColor( int col, int row );
  virtual BOOL actionPerformed( AMouseEvent *mouseEvent );
  virtual BOOL actionPerformed( AKeyEvent *keyEvent );
  virtual void focusSet( BOOL newFocus );
	~ValueSet();
};

class HSeparator : public Window, public virtual AMouseListener
{
  BOOL buttonDown, minimized;
  int restoreY;
  int getButtonXPos();
  static int buttonWidth;
public:
  static int defaultHeight;
	HSeparator( Window *parent, int x, int y, int width, int height );
  virtual BOOL actionPerformed( AMouseEvent *mouseEvent );
	virtual void paint( Rect *rect, Graphics *g );
  int getStoredY() { return restoreY; }
  void setStoredY( int restoreY ) { this->restoreY = restoreY; }
  void sRestore();
  void sMinimize();
  BOOL sIsMinimized() { return minimized; }
};
