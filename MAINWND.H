#if !defined MAINWINDOW

#define MAINWINDOW

#include "bowl.h"
#include "buffer.h"
#include "editctrl.h"
#include "infobar.h"
#include "cntrladm.h"
#include "mainmenu.h"
#include "mainwadm.h"
#include "ctrlwin.h"

struct MessageBoxInfo
{
	char *title;
	char *message;
	int type;
};

struct ToolButtonInfo : public AObject
{
	char *command, *description, *bitmap, *shortcut;
	int id, type;		// type == 1 -> image
};

struct ToolButtonState
{
	int id;
	BOOL active;
};

struct LoadAndJumpInfo
{
	char *filename;
	int line, column;
};

class BufferLoader : public Thread
{
  TextBuffer *textBuffer;
  char *filename;
  BlockFileReader *fileReader;
  InfoBar *infoBar;
  BOOL reload;
  MainWindow *mainWnd;
public:
  BufferLoader( TextBuffer *textBuffer, char *filename, InfoBar *infoBar, BOOL reload, MainWindow *mainWnd ) :
                Thread( TRUE ), reload(reload), mainWnd(mainWnd)
  {
    fileReader = new BlockFileReader( filename );
    this->textBuffer = textBuffer;
    this->infoBar = infoBar;
  }
  virtual void run();
};

class ToolbarDialog : public Dialog, public virtual AScrollbarListener,
                      public virtual AButtonListener
{
  char *name;
  Scrollbar *scrollbar;
  SComboBox *commandCombo;
  EntryField *descriptionField;
  RadioButton *commandButton, *spaceButton;
  Button *OKbutton;
  Label *cLabel, *dLabel;
  class IconWindow : public Window, public virtual AMouseListener
  {
    BmpGraphics bmpGraphics;
    BOOL enabled;
  public:
    class Icon : public AObject
    {
      char *filename;
      Bitmap *bitmap;
    public:
      Icon( char *filename, BmpGraphics *g )
      {
        this->filename = new char[strlen(filename)+1];
        strcpy( this->filename, filename );
        // Load the bitmap
        bitmap = g->readBitmap( filename );
      }
      Bitmap *getBitmap() { return bitmap; }
      char *getFileName() { return filename; }
      ~Icon() { delete filename; }
    };
    Vector<Icon> icons;
    int selectedIcon, leftPos;
  public:
    IconWindow( Window *parent, int x, int y, int width, int height ) :
      Window( parent, x, y, width, height ), selectedIcon(0), leftPos(0)
    {
      enabled = TRUE;
      addMouseListener( this );
    };
    virtual void enable( BOOL enabled )
    {
      this->enabled = enabled;
      repaint( NULL );
    }
    void addIcon( char *filename )
    {
      icons.add( new Icon( filename, &bmpGraphics ) );
    }
    virtual BOOL actionPerformed( AMouseEvent *mouseEvent ) {
      if( mouseEvent->getEvent() == AMouseEvent::CLICK && enabled )
      {
        setFocus();
        selectedIcon = (mouseEvent->getPos()->x+2) / (getHeight()-2) + leftPos;
        if( selectedIcon >= icons.getSize() )
          selectedIcon = icons.getSize() - 1;
        repaint( NULL );
      }
      return TRUE;
    }
    char *getSelectedIcon()
    {
      return ((Icon *)icons.getObjectAt(selectedIcon))->getFileName();
    }
    int getIconsInWindow() { return getWidth() / (getHeight()-2); }
    int getIcons() { return icons.getSize(); }
    int getSelectedPos() { return selectedIcon; }
    int getWindowStart() { return leftPos; }
    void selectIcon( char *filename );
    void setWindowStart( int start )
    {
      leftPos = start;
      repaint( NULL );
    }
    void paint( Rect *, Graphics *g );
  };
  IconWindow *iconWindow;
  int buttonType;
public:
  static int NEW, SPACE, COMMAND;
  ToolbarDialog( Window *frame, Window *parent, int type, char *title,
                 char *filename, char *command, char *description );
  virtual void size( int oldWidth, int oldHeight )
  {
    Dialog::size( oldWidth, oldHeight );
    // Set scrollbar thumb size
    scrollbar->setRatio( iconWindow->getIconsInWindow(), iconWindow->getIcons() );
    // Set range and thumb position
    setThumbPos();
  }
  void setThumbPos()
  {
    scrollbar->setRange( iconWindow->getWindowStart(), 0,
                         iconWindow->getIcons()-iconWindow->getIconsInWindow() );
  }
  virtual void actionPerformed( AScrollbarEvent *scrollbarEvent )
  {
    int pos = scrollbarEvent->getPos();
    int action = scrollbarEvent->getAction();

    if( action == Scrollbar::PAGELEFT )
      pos = iconWindow->getWindowStart() - iconWindow->getIconsInWindow();
    if( action == Scrollbar::PAGERIGHT )
      pos = iconWindow->getWindowStart() + iconWindow->getIconsInWindow();
    if( action == Scrollbar::LINELEFT )
      pos = iconWindow->getWindowStart()-1;
    if( action == Scrollbar::LINERIGHT )
      pos = iconWindow->getWindowStart()+1;

    if( pos < 0 ) pos = 0;
    if( pos >= iconWindow->getIcons() - iconWindow->getIconsInWindow() )
      pos = iconWindow->getIcons() - iconWindow->getIconsInWindow();

    // pos contains the new position
    iconWindow->setWindowStart( pos );
    // resize/move pos on scrollbar
    setThumbPos();
  }

  virtual void actionPerformed( AButtonEvent *buttonEvent );
  int getButtonType() { return buttonType; }
  char *getSelectedIcon() { return iconWindow->getSelectedIcon(); }
  ~ToolbarDialog()
  {
    WinProperties::updateProperty( name, x, y, width, height );
    delete name;
  }
  char *getCommand() { return commandCombo->getEditText(); }
  char *getDescription() { return descriptionField->getText(); }
};

class DockedErrorCollector : public EditControl, public BufferObserver
{
public:
  DockedErrorCollector( Window *parent, InfoBar *infobar );
  ~DockedErrorCollector() {}
  virtual void insertion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines );
};

#define MAXCOMMANDS 20

class MainWindow : public FrameWindow, public virtual AMouseListener,
                   public virtual AMenuListener,
                   public virtual ASelectionListener,
                   public virtual AToolbarListener,
                   public virtual AKeyListener,
                   public virtual AButtonListener,
                   public virtual AWinMoveListener,
                   public virtual ToolStateServer
{
  Toolbar *toolbar;
  Notebook *notebook;
  SComboBox *combo;
  Dialog *frameDialog;
  InfoBar *infoBar;
  DockedErrorCollector *errorCollector;
  HSeparator *separator;
  static char *command[MAXCOMMANDS];
  static int commands;

  BOOL actionPerformed( AKeyEvent *keyEvent );
  // No query, no check if MainWindow should close.
  void closeControl( EditControl *control );
	static BOOL toolbarModified, separatorMinimized;
	static int tbSize, separatorRestoreY, cmdButtonWidth, buttonWidth;
	void reReadToolbar();
  BOOL tempAutoClose, executingFocus;
  MutexSemaphore focusWait;
public:
  void waitFocus();
  void releaseFocus();
	static Vector<ToolButtonInfo> toolButtonInfo;
  MainWindow( Window *parent, char *title, int x, int y, int width, int height );

  Notebook *getNotebook() { return notebook; }

  virtual int execFromMainThread( int task, void * );
  void updateMenu()
  {
    addMenu( MainMenu::getMenu() );
    updateCaptionText();
  }

	void updateInfobar()
	{
		infoBar->repaint( NULL );
	}

	// Returns the created/already existing control
  EditControl *addFile( char *filename, BOOL threadedLoading, BOOL killScratchImmediately );

  void setTab( int tab )
  {
    notebook->setTab( tab );
  }

  void showBufferModified( EditControl *control, BOOL isModified );
  void filenameUpdated( EditControl *control );

  void selectControl( EditControl *control );

  // The notebook control will take care of bounds

  void nextTab();
  void prevTab();

  virtual void size( int oldWidth, int oldHeight );
  virtual void actionPerformed( AToolbarEvent *toolbarEvent )
  {
    ((EditControl *)notebook->getActiveWindow())->
      addCommand( toolbarEvent->getCommand() );
  }

  void updateCaptionText();

  virtual void actionPerformed( ASelectionEvent *selectionEvent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  virtual void actionPerformed( AWinMoveEvent *moveEvent );

  // Menu listener

  virtual void actionPerformed( AMenuEvent *menuEvent );

  virtual BOOL actionPerformed( AMouseEvent *mouseEvent ); 

  virtual int focus( BOOL newFocus );
  void focusCommandBar();
  virtual BOOL close();

  EditControl *getActiveControl()
  {
    return (EditControl *)notebook->getActiveWindow();
  }

  // Returns TRUE if the control was closed

  virtual BOOL closeControl( EditControl *control, BOOL queryOnModified );
  virtual void closeActiveWindow();
  virtual BOOL modifyButton( char **filename, char **command,
                             char **description );
	virtual void buttonDeleted( int id );
	virtual void buttonMoved();
	static void saveSettings( FILE *f );
	static void restoreSettings( FILE *f );
  void restoreMessages() { separator->sRestore();};
  void minimizeMessages() { separator->sMinimize();};
  virtual ~MainWindow();
  void adjustSeparatorPos( int yNow );
  void paint( Rect *, Graphics *g );
};

#endif 

