/*****************************************************************************
 * The viewWindow holds the text for each view. Only one view in the view
 * window can be active at one time. This window does not contain any
 * scrollbars.
 *
 * The ViewWindow receives keystrokes from the keyboard.
 */

#if !defined VIEWWINDOW
#define VIEWWINDOW

#include "buffer.h"
#include "view.h"
#include "popupwin.h"

#define MAXSCREENLINES  300

#define MOVE_SCREEN_TO_CURSOR	1
#define MOVE_PAGE	2

class ViewWindow : public Window, public virtual AMouseListener,
                   public virtual BufferObserver
{
  View *view;
  ViewWindowHolder *viewHolder;
  int windowLines, windowStartLine, windowStart, leftMargin;

  static int UNDEF, PAGEUP, PAGE, LINES, UP, DOWN;

  struct LinePos  // Contains the positions of each line on screen
  {
    int yPos,     // if yPos == UNDEF, no pos is calculated
        height;
    int lineStart;
  } linePos[MAXSCREENLINES];

  struct UpdatePos : public AObject
  {
    int startpos, endpos;
    UpdatePos( int startpos, int endpos ) : startpos(startpos), endpos(endpos)
    {}
  };

  Vector<UpdatePos> updatePos;

  struct UpdateRect : public AObject
  {
  	Rect rect;
    UpdateRect( Rect *rect )
    {
    	this->rect.x = rect->x;
    	this->rect.y = rect->y;
    	this->rect.width = rect->width;
    	this->rect.height = rect->height;
    }
  };

  Vector<APositionListener> positionListener;
  Vector<UpdateRect> updateRect;
  MutexSemaphore updateRectLock;

  BOOL linePosIsInvalid;  // TRUE if the linepos array should be recalculated
                          // Note that windowLines must be set to -1 then also!
                          // linePosInvalid implies that no entry in the
                          // linePos array is correct, but windowLines == -1
                          // implies that the positions that are not UNDEF
                          // are correct.
                          // This means that if we insert lines in the text
                          // it is safe to set windowLines to -1, but not if
                          // we remove text, but don't clear all entries from
                          // the removed part and forwards in the linePos array.

  InfoBar *infoBar;
  BOOL cursorVisible,
       active,            // TRUE if this view is active and visible
       refreshWindowWhenAfterUpdate;  // TRUE if window should be refreshed when the window 
                                      // has been modified during period of hideUpdates > 0
  int hideUpdates;				// = 0 if the view should be updated on screen
  int linePosLocked;
  MutexSemaphore pointerLock,
                  linePosLock; // Due to that paint and scroll exists in
                               // different threads, collisions can occur
                               // when updating the linePos array.
                               // This is avoided by using this mutex.
                               // Note that scrolls can (should) only be done
                               // within one thread, so linePos is locked
                               // using lockLinePos and releaseLinePos
                               // which only locks once, subsequent calls
                               // only increments linePosLocked.

  // old* variables are used to determine if the scrollbars should be updated
  int oldLeftMargin, oldWindowStartLine, oldTextLines, oldWindowLines, leftMarginLock;
  BOOL mouseButtonDown;
  int mouseButton;
  BOOL hasFocus;
  PopupWindow *popupWin;
  int popupX, popupY;
  int visibleText, longestLineInText;
private:
  void pScreenToCursor();
	void pMovePage( int dir );
public:
  ViewWindow(  Window *parent, ViewWindowHolder *viewHolder, InfoBar *infoBar,
               int x, int y, int width, int height ) : windowLines(-1),
               view(NULL), windowStartLine(1), windowStart(0), leftMargin(0),
               active( FALSE), cursorVisible( FALSE ), linePosLocked(0),
               Window( parent, x, y, width, height ), linePosIsInvalid( TRUE ),
               oldLeftMargin(-1), oldWindowStartLine(0), oldTextLines(0),
               oldWindowLines(0), mouseButtonDown( FALSE ), hasFocus( FALSE ),
               hideUpdates(0), leftMarginLock(0), popupWin( NULL ), visibleText(0),
               longestLineInText(0), positionListener(FALSE)
  {
		acceptDrop( TRUE );
    addMouseListener( this );
    resetLineDefs();
    this->viewHolder = viewHolder;
    this->infoBar = infoBar;
    setPointer( PTR_TEXT );
  }

  void updateScrollbars();
  virtual void insertion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines );
  virtual void deletion( TextBuffer *textBuffer, int reader, int position,
                         int size, int lines );
  virtual void noWriter() { processUpdateRects(); }
  void snapshot();

  // lockLinePos and releaseLinePos are called by everyone modifying
  // the linePos array, except the paint method which uses the linePosLock
  // mutex directly.
  void lockLinePos();
  void releaseLinePos();

  void popupWindow( char *text );

  int nextLine( int reader, int position, int &viewLine );

  virtual void paint( Rect *rect, Graphics *g );

  virtual void focusSet( BOOL newFocus )
  {
    if( !newFocus )
      hideCursor();
    else
      showCursor( FALSE );
    hasFocus = newFocus;
  }

  BOOL isUpdating() { return !hideUpdates; }
  virtual BOOL inFocus()
  {
    return hasFocus;
  }

  void setView( View *view, BOOL doRepaint )
  {
    this->view = view;
    if( doRepaint ) repaint( NULL );
  }

  virtual void size( int, int )
  {
    resetLineDefs();
  }

  BOOL isActive() { return active; }
  void setActive( BOOL active ) { this->active = active; }
  void setUpdate( BOOL shouldUpdate );
  BOOL getUpdate( void );
  int getLeftMargin() { return leftMargin; }
  int getPositionVertPixel( int position, BOOL top );
  void resetLineDefs();

  void removeLines( int reader, int fromLine, int lines );

  void moveStart( int reader, int &startPos, int &atLine, int lines );

  void getClosestStartingPoint( int &start, int &toLine,
                                BOOL useUserPosition,
                                BOOL useWindowStart );

  // Upwards scroll is always from the first line on the screen

  virtual void scrollUp( int reader, int lines );
  virtual void scrollDown( int reader, int lines, int fromLine, int type );
  virtual void screenLineDown();
  virtual void screenLineUp();
  virtual void screenLinesDown( int lines );
  virtual void screenLinesUp( int lines );
  virtual void screenPageUp();
  virtual void screenPageDown();
  virtual void setScreenStart( int line );
  virtual void setHorzMargin( int margin )
  {
    if( margin < 0 ) margin = 0;
    int delta = leftMargin - margin;
    leftMargin = margin;
    if( delta != 0 )
      scrollHorz( delta, leftMarginLock, getWidth() );
  }

  virtual void scrollColumn( int delta )
  {
    setHorzMargin( leftMargin + delta );
  }

  virtual void showCursor( BOOL linePosLocked )
  {
    showCursor( linePosLocked, -1 );
  }

  virtual void showCursor( BOOL linePosLocked, int reader );

  virtual void hideCursor()
  {
    cursorVisible = FALSE;
    Window::hideCursor();
  }

  BOOL getCursorVisibility() { return cursorVisible; }

  void screenToCursor();
  void cursorToScreen();

  int getWindowLines() { return windowLines; }
  int getWindowStartLine() { return windowStartLine; }

  // CalcLinePos should be used restrictively since it's essentially doing
  // the same job as the paint method.
  // This method calculates windowLines and fills the linePos array
  // If useExisiting is not set, no previously entered info in the linePos
  // array will be used. If is set, all lines to toLine will be considered
  // valid entries.

  void calcLinePos( int reader, BOOL useExisting, int toLine );

  void pageUp()
  {
    movePage( UP );
  }

  void pageDown()
  {
    movePage( DOWN );
  }

  void movePage( int dir );

  virtual BOOL actionPerformed( AMouseEvent *mouseEvent );
  virtual void paintLines( int reader, int startLine, int endLine );
  // Update all lines between char positions start and end
  void updatePosition( int reader, int start, int end );
  void addUpdatePosition( int reader, int start, int end )
  {
    updatePos.add( new UpdatePos( start, end ) );
  }

  virtual void scrollVert( int delta, int bottom, int top );
  virtual void scrollHorz( int delta, int left, int right );

  void processUpdatePositions( int reader );
  void processUpdateRects();
	virtual void filesDropped( char **files, int count );

  void reset( BOOL doRepaint, BOOL moveWindowStarts )
  {
    resetLineDefs();
    oldWindowLines = -1;

		if( moveWindowStarts )
		{
			windowStartLine = 1;
			windowStart = 0;
			oldLeftMargin = -1;
			leftMargin = 0;
		}
    if( doRepaint )
      repaint( NULL );
  }
	virtual int execFromMainThread( int task, void *obj );
  virtual void addPositionListener( APositionListener *positionListener )
  {
    this->positionListener.add( positionListener );
  }
  virtual void removePositionListener( APositionListener *positionListener )
  {
    this->positionListener.removeObject( positionListener );
  }
  void performPositionEvent( long position );
};

#endif