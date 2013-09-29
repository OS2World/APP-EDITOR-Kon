/*****************************************************************************
 * The edit control holds the view window, the scrollbars and the view(s).
 * It also holds the K Executer for this buffer.
 */

#if !defined EDITCTRL

#define EDITCTRL

#include "buffer.h"
#include "view.h"
#include "asciview.h"
#include "hexview.h"
#include "vwholder.h"
#include "viewwin.h"
#include "infobar.h"
#include "kcmdExec.h"
#include "cmdq.h"
#include "keybind.h"
#include "kerrcoll.h"
#include "cntrladm.h"
#include "bufadmin.h"
#include "bufhldr.h"
#include "bytesinfo.h"
#include "SectionBrowser.h"
#include "srchdlg.h"
#include "repldlg.h"

// QueryReplace is used by the query replace dialog

struct QueryReplace
{
  char *foundStr, *replaceStr;
  int pos, len;
  BOOL multiFile;
  QueryReplace( char *foundStr, char *replaceStr, int pos, int len, 
                BOOL multiFile ) :
    foundStr(foundStr), replaceStr(replaceStr), pos(pos), len(len),
    multiFile(multiFile) {}
};

/******************************************************************************
 * The EditControl holds the view for a specific buffer. The EditControl is
 * a window without frames, containing scrollbars and viewwindow.
 * There may be several EditControls (views) for one buffer.
 */

struct FileDialogInfo
{
	char *title;
	char *filename;
  int type;
  EncodingTypes encType;
};

class EditControl : public Window, public ViewWindowHolder,
                    public BufferHolder,
                    public virtual LoadObserver,
                    public virtual AScrollbarListener,
                    public virtual AKeyListener,
                    public virtual AMenuListener,
                    public virtual ASelectionListener,
                    public virtual KCommandExecutor,
                    public virtual APositionListener
{
  class ViewScrollbar : public Scrollbar
  {
  public:
    ViewScrollbar( Window *parent, int properties, int x, int y, int width,
                   int height ) :
      Scrollbar( parent, properties, x, y, width, height ) {}
    virtual void setFocus()
    {
      // Do not allow focus on this scrollbar
    }
  };

	struct QueryLineArg
	{
		char *title;
		char *query;
		char **initText;
	};

  int id;               // This id identifies the EditControl instance
  static int idCounter; // This is used as a counter for the above
  ViewScrollbar *south, *east;
  ViewWindow *viewWindow;
  View *currentView;
  AsciiView *asciiView;
  HexView *hexView;
  TextBuffer *textBuffer;
  Window *parent;
  BOOL viewInitialized;
  BmpGraphics bmpGraphics;
  Bitmap *backgroundBitmap;
  Syntax *syntax;
  BOOL scrollbarHandleDown;
  InfoBar *infoBar;
  BytesInfo *bytesInfo;
  SectionBrowser *sectionBrowser;
  CommandQueue *commandQueue;
  
  static SearchDialog *searchDialog;
  static ReplaceDialog *replaceDialog;

  struct ScrollbarData
  {
    int pos, total, visible;
  } vScrollbarData, hScrollbarData;
  BOOL isActive, isSimpleControl;
	void startControl( void );
public:
  EditControl( Window *parent) : 
     Window( parent, 0, 0, 100, 50 ), viewInitialized(FALSE),
     scrollbarHandleDown(FALSE), isActive( FALSE ), parent(parent),
     currentView(NULL)  {};
  EditControl( Window *parent, TextBuffer *textBuffer,
               Syntax *syntax, InfoBar *infoBar,
               int x, int y, int width, int height );

  EditControl( Window *parent, TextBuffer *textBuffer,
               Syntax *syntax, InfoBar *infoBar,
               int x, int y, int width, int height, BOOL isSimple );

  void startControl( TextBuffer *textBuffer,
                     Syntax *syntax, InfoBar *infoBar,
                     int x, int y, int width, int height, BOOL isSimple );

  virtual void setRunning( BOOL isRunning ) 
  { 
  	this->isRunning = isRunning; 
    asciiView->setRunning( isRunning );
    hexView->setRunning( isRunning );
  }
  void showBusy( BOOL isBusy ) { if( infoBar != NULL ) infoBar->showBusy( isBusy ); }
  virtual void snapshot() { viewWindow->snapshot();}
  virtual TextBuffer *getTextBuffer() { return textBuffer; }
  int getId() { return id; }

  virtual void activate( BOOL active );
  virtual void setUpdate( BOOL shouldUpdate )
  {
    viewWindow->setUpdate( shouldUpdate );
  }
  virtual BOOL getUpdate( void )
  {
    return viewWindow->getUpdate();
  }

  virtual int getViewHeight() { return viewWindow->getHeight(); }
  virtual int getViewVidth() { return viewWindow->getWidth(); }
  virtual void pageDown() { viewWindow->pageDown(); }
  virtual void pageUp() { viewWindow->pageDown(); }
  virtual Graphics *getViewGraphics() { return viewWindow->getGraphics(); }
  virtual void setCursorVisible();
  virtual void updatePosition( View *view, int reader, int start, int end )
  {
    if( view == currentView )
      viewWindow->updatePosition( reader, start, end );
  }
  virtual void addUpdatePosition( View *view, int reader, int start, int end )
  {
    if( view == currentView )
      viewWindow->addUpdatePosition( reader, start, end );
  }

  virtual void addCommand( char *command )
  {
    commandQueue->addCommand( command );
  }

  BOOL textModified() { return textBuffer->textModified(); }

  virtual BOOL actionPerformed( AKeyEvent *keyEvent );

  // This is the top level evaluator. Commands that are not processed here
  // is chained down to the View object

  virtual TSyntax *eval( char *name, TVariableList *vars, TEval *eval, 
	                       int index );

  virtual void size( int oldWidth, int oldHeight );

  // When the textbuffer is finished loading, this method will be called by the
  // buffer. Here we setup the window display

  virtual void textLoaded( BOOL isBinaryFile, BOOL reloaded );

	// returns 0 on success

  int saveFile();

  virtual int setVisible( BOOL visible )
  {
    viewWindow->setActive( visible );
    return Window::setVisible( visible );
  }

  virtual void setHorzScrollbar( int pos, int visible, int total );
  virtual void setVertScrollbar( int pos, int visible, int total )
  {
    if( !scrollbarHandleDown )
      east->setAll( pos, 1, total, visible, total );

    vScrollbarData.pos = pos;
    vScrollbarData.visible = visible;
    vScrollbarData.total = total;
  }

  virtual void actionPerformed( AScrollbarEvent *scrollbarEvent );
  virtual void actionPerformed( AMenuEvent *menuEvent );

  virtual int getLeftMargin() { return viewWindow->getLeftMargin(); }

  virtual void paint( Rect *, Graphics * );
  virtual void repaintView( Rect *rect )
  {
    size( width, height );
    viewWindow->reset( TRUE, FALSE );
    viewWindow->processPaint();
  }

  virtual void paintViewLines( int startLine, int endLine )
  {
    // ADD CODE FOR REPAINTING ONLY NECESSARY LINES!!
//    viewWindow->paintViewLines( startLine, endLine );
    viewWindow->processPaint();
  }

  virtual BOOL viewActive() { return viewWindow->isActive(); }
  virtual void setViewActive( BOOL active ) { viewWindow->setActive( active ); }
  virtual void setActive( BOOL active ) { isActive = active; }
  virtual void scrollHorz( int delta, int left, int right ) {}

  virtual int queryReplaceDialog( char *foundString, char *replaceString, int pos, 
                                  int len, BOOL multiFile );

  virtual int execFromMainThread( int task, void * );

  void screenToCursor()
  {
    viewWindow->screenToCursor();
  }

  virtual void setFocus()
  {
    viewWindow->setFocus();
  }

	virtual void closeControl()
	{
    commandQueue->close();
	}

	virtual void initFrameWindowClose()
	{
		commandQueue->closeParentFrame();
	}

	BOOL viewHasValidMark() { return viewInitialized ? currentView->validMark() : FALSE; }

  virtual ~EditControl();

  virtual View *getView() { return currentView; }
  virtual void popupMenu( Point *point );
	virtual void userAsynchMessage( char * message ) { if( infoBar != NULL ) infoBar->message( message ); }
	virtual void userAsynchClearMessage() { if( infoBar != NULL )infoBar->clearMessage(); }
  virtual void fileRenamed();
  Syntax *getSyntax() { if( viewInitialized ) return asciiView->getCurrentSyntax(); else return NULL; }
  virtual void actionPerformed( ASelectionEvent *selectionEvent );
  int getCodeSections( CodeSection **section );
  BytesInfo *getBytesInfo() { return bytesInfo; }
  virtual void actionPerformed( APositionEvent *positionEvent );
  void updateHexInfo();
};

#endif
