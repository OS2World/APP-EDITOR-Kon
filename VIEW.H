/******************************************************************************
 * View is a baseclass for views (for example Hex- or Ascii view).
 *
 * A View is line based, i.e. it should by possible to navigate the view by
 * specifying a line and a column. It should also be possible to navigate by
 * absolute positions.
 *
 * The view creates the line definitions which are used in the ViewWindow's
 * pait method. The View may ask the ViewWindow for positon information from
 * the ViewWindow since these are stored there.
 *
 * Note that a View may be created with no ViewWindow attached. Then
 * ViewWindowHolder is set to NULL.
 * The view then works with no wrapper class, and the view itself executes all
 * K commands.
 */

#include "buffer.h"
#include "vwholder.h"
#include "infobar.h"
#include "kcmdexec.h"
#include "kcommon.h"

// TextFont and TextColor describes the text to be drawn.
// TextFont lists all fonts. Within each TextFont structure a list of text
// attributes are listed

#if !defined VIEW

#define GR_LINE	1
#define GR_SOFTEOL	2

#define VIEW_ASCIIVIEW 0
#define VIEW_HEXVIEW 1

struct GraphicObject
{
	GraphicObject *next;
	int x, y, width, height;
	int type;
	ColorValue fg;
	GraphicObject( ColorValue fg, int type, int x, int y, int width, int height )
	{
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
		this->type = type;
		this->fg = fg;
		next = NULL;
	}
	~GraphicObject()
	{
		if( next )
			delete next;
	}
};

struct TextColor
{
  TextColor *next;
  ColorValue fg, bg;
  int length, xPos, lastX;
  TextElem *text;
  BOOL lastIsTab;
  BOOL locked;
  int xBeforeTab;

	// The text string should be a static variable. It is not deleted by
  // the caller of getTextLineInfo()!  

  TextColor( ColorValue fg, ColorValue bg, TextElem *text, int xPos)
  {
    this->fg = fg;
    this->bg = bg;
    this->text = text;
    this->xPos = xPos;
    lastIsTab = FALSE;
    next = NULL;
    lastIsTab = FALSE;
    locked = FALSE;
  }
  ~TextColor()
  {
    if( next )
      delete next;
  }
};

struct TextFont
{
  TextFont *next;
  TextColor *textColor;
	GraphicObject *graphicObject;
  int yPos, height, baseline, attr;
  Font *font;
  int points, X, Y;
  TextFont( TextFont *next, TextColor *textColor, int yPos,
            int height, int baseline, int attr,
            Font *font, int points, int X, int Y )
  {
    this->next = next;
    this->textColor = textColor;
    this->yPos = yPos;
    this->height = height;
    this->baseline = baseline;
    this->font = font;
    this->points = points;
    this->X = X;
    this->Y = Y;
    this->attr = attr;
		graphicObject = NULL;
  }
  ~TextFont()
  {
    if( next )
      delete next;
    if( textColor )
      delete textColor;
		if( graphicObject )
			delete graphicObject;
  }
};

/* previousSearchOperation holds info about the last search. Used for
   repeatFind() operation */

enum SearchOp { eSearch, eReplace, eUndef };

struct PreviousSearchOperation
{
  static SearchOp lastSearchOp;
  static char op[15];
  static char *searchString, *replaceString;
  static void setNewOperation( SearchOp searchOp, char *options, char *searchString,
                               char *replaceString );
};

class View : public BufferObserver, public virtual UndoObserver,
             public KCommandExecutor
{
protected:
  static PreviousSearchOperation previousSearchOperation;
  ViewWindowHolder *vwHolder;
  TextBuffer *textBuffer;
  int userPosition, bufferLines, currentLine, currentColumn;
  char *defaultFontName;
  int defaultFontHeight;
  Font *defaultFont;
  int defaultFontPoints, defaultFontX, defaultFontY;
  InfoBar *infoBar;
  Font *lastFont; // So we don't have to recreate the font if it's the same as
                  // the last one
  int markBegin, markEnd, hlBegin, hlEnd, mlLineStart, mlCursorPos;
  int _keepBlock;
  BOOL undoredo( BOOL isUndo );
  // Call when text has been marked/unmarked
  // Calls blockMarked() K function
  void markModified();
  int savedPos[10];
	int *savedPosSorted;
	int savedPositions;
	BOOL relativePos, positionViewChanged;
  int relativeStart;
  int viewType;
public:
  struct Position;
  static int PRINTER, SCREEN;
  View( ViewWindowHolder *vwHolder, TextBuffer *textBuffer, InfoBar *infoBar );

	void jumpToNumberedSavedPos( int mark );
  void saveNumberedPos( int mark );
	
	virtual void savePos();
  virtual void clearSavedPositions();
	virtual void jumpToNextPos();
	virtual void jumpToPrevPos();
	virtual int hasSavedPositions();

  virtual void undoAvailable( BOOL hasUndo, BOOL atUnmodified  );
  virtual void redoAvailable( BOOL hasRedo, BOOL atUnmodified  );

  char *translateHexCodes( char *hexCodes, int &len );
  BOOL findString( char *text, char *options, BOOL repeat, char **group[] = NULL, int *groups = 0);
  
	// If the view has no ViewWindow attached, and a query has to be done,
  // the metod will return the negative offset of the position in the file
  // where the string was found in queryResult. When the caller has opened a 
  // visible view,it only needs to position the cursor at this offset and 
  // restart the replace operation.
  // Otherwise replaceString returns:

  BOOL replaceString( char *sText, char *rText, char *options, int &replacements,
                      BOOL multiFile, int &queryResult );

  virtual void updateGraphicCursor() {}

  int getViewType() { return viewType; }

  // If the font is changed, presParamChanged is called which updates
  // internal variables

/*  void presParamChanged() {}
  {
    // Get the standard font height

    Graphics *g = vwHolder->getViewGraphics();
    setDefaultFont(g);
    g->selectFont( defaultFont, defaultFontPoints, 0 );
    defaultFontHeight = g->getMaxFontHeight();
  }*/

  int textRequest( MutexSemaphore *additionalSemaphore )
  {
    return textBuffer->readRequest( additionalSemaphore );
  }

  int textRequest( MutexSemaphore *additionalSemaphore, int timeout )
  {
    return textBuffer->readRequest( additionalSemaphore, timeout );
  }

  int textRequest()
  {
    return textBuffer->readRequest();
  }

  void textRelease()
  {
    textBuffer->readRelease();
  }
  
  virtual void requestScreenBuffer() {}
  virtual void releaseScreenBuffer() {}

/*  void setDefaultFont( char *font, int points )
  {
    setDefaultFont( font, points, -1, -1 );
  }
  void setDefaultFont( char *font, int X, int Y )
  {
    setDefaultFont( font, -1, X, Y );
  }*/

  int getTextlines()
  {
    return bufferLines;
  }

  int getTextSize()
  {
    return textBuffer->getSize();
  }

  // If the screen should be updated from the insertion/deletion point and
  // downwards, this method should return TRUE;

  virtual BOOL queryUpdateAllOnModification() { return FALSE; }
  virtual int getLineFromPosition( int position ) { return 0; }
  virtual int getPositionFromLine( int reader, int line ) { return 0; }

  int getPosition() { return userPosition; }

  virtual int getTextlineHeight( int reader, int position, Graphics *g,
                                 int target )
    { return 0; }

  // Returns a DisplayText linked list with information for a given line
  // The last color attribute should be extebded to the screen's right end!
	// viewLine will return the REAL line in the textbuffer for this view
	// at the next position. This may in the future not be the same as the corresponding
	// line on the screen.(Allows for text hiding).
	// The value in should correspont to the line at positon
	// If viewLine is -1 the line is initially calculated for this position.
  // If rightPixelMargin is -1, then the whole line length is calculated.

  virtual TextFont *getTextlineInfo( int reader, int &position, int &viewLine,
                                     int &maxHeight,
                                     int &maxBaseline, Graphics *g,
                                     int target,
                                     int rightPixelMargin )
  { return NULL; }

  virtual void storeCursorPos() {}
  virtual BOOL cursorHasMoved() 
  { 
  	BOOL hasChanged = positionViewChanged;
		positionViewChanged = FALSE;
  	return hasChanged;
  }

  // Return values of BOOL reports success or not of the command

  virtual BOOL jumpToLine( int line ) { return FALSE; }
  virtual BOOL jumpToPosition( int reader, int position ) 
  { 
    return jumpToPosition( reader, position );
  }
  BOOL jumpToPosition( int position )
  {
    int reader = textBuffer->readRequest();
    BOOL rc = jumpToPosition( reader, position );
    textBuffer->readRelease();
    return rc;
  }
  virtual BOOL lineDown() { return FALSE; }
  virtual BOOL lineUp() { return FALSE; }
  virtual BOOL charLeft() { return FALSE; }
  virtual BOOL charRight() { return FALSE; }
  virtual void cursorToColumn( int column ) {}
  virtual void cursorToColumn( int column, BOOL tempVirtualSpace ) { cursorToColumn( column );}
  virtual void toLineEnd() {}
  virtual void toLineStart() {}
  virtual void setMark() {}
  virtual void pushPos( BOOL visible ) {}
  virtual void popPos( BOOL moveCursor ) {}
  virtual BOOL undo() { return undoredo( TRUE ); }
  virtual BOOL redo() { return undoredo( FALSE ); }

	virtual void deleteBlock() {}
  virtual void copyBlockToClipboard() {}
  virtual void cutBlockToClipboard() {}
  virtual BOOL pasteBlockFromClipboard() { return FALSE; }
  virtual BOOL pasteBlockFromKClipboard();

  // use pushPos to save the current position. Move to the desired position
  // and call markForward/markBackward to mark the text.
  // markForward/markBackward will pop the position

	virtual void beginMark() { _keepBlock++; pushPos( FALSE ); }
  virtual void markForward() {};
  virtual void markBackward() {};
  virtual BOOL keepBlock() { return _keepBlock > 0; }
  virtual void discardBlock( BOOL discard )
  {
  	_keepBlock = !discard;
  }

  // markText selects markForward or markBackward

  virtual void endMark() {};
  void resetMark();
  
	virtual int getMarkBegin() { return markBegin; }
	virtual int getMarkEnd() { return markEnd; }
  virtual void shiftStateSet( int shiftstate ) {}

  virtual BOOL atTextEnd( int position )
  {          
    return position >= textBuffer->getSize();
  }
  virtual int getLine() { return currentLine; }
  virtual int getColumn() { return currentColumn; }
  virtual int getLineStart() { return 0; }
  virtual int getLineEnd() { return 0; }
  virtual void getLineString( TextElem str, int &len ) {}
  virtual void getLineString( int reader, TextElem str, int &len ) {}
	virtual void getText( TextElem *str, int position, int len, int reader = -1 );
  virtual TextBuffer *getTextBuffer() { return textBuffer; }
	BOOL validMark();
  virtual TextElem getChar( int position )
  {
    int reader = textRequest();
    TextElem c = textBuffer->getChar( reader, position );
    textRelease();
    return c;
  }
  virtual void insertString( TextElem *string ) {}
  virtual void deleteCharacter() {}
  // DEBUG BELOW
  TextElem getCharAt( int reader, int pos ) { return textBuffer->getChar( reader, pos ); }
  // Moves linesDelta lines up or down depending of the sign of linesDelta
  // If the linesDelta lines cannot be moved, linesDelta is updated with
  // the moved number fo lines.
  // The linestart of the new line is returned

  virtual int getNewLineStartFrom( int reader, int lineStart, 
	            int &linesDelta, int &viewLine ) {return 0;}

  // getCursorColumn returns the pixelbased cursor column

  virtual int getCursorColumn() {return 0;}
  virtual int getCursorWidth() { return 0; }
  virtual int getCursorWidth( int reader ) { return 0; }

  // setColumn sets the characterbased column based on the horizontal
  // pixel position

  virtual void setCursorColumn( int pixelCol ) {}
  virtual void setCursorColumn( int pixelCol, BOOL resetLockedColumn, BOOL tempVirtualSpace ) { setCursorColumn( pixelCol ); }

  // Replaces escape codes with character codes

  // Commands that are not evaluated here are chained down to the common
  // K command executor singleton object

  virtual TSyntax *eval( char *name, TVariableList *vars, TEval *eval, 
	                       int index );

  virtual void resetView() {};

  // Returns a string specifying the position in the file. This may be different
  // for different views. The pos string should be able to hold the number of
  // characters needed.

  virtual void getPositionString( char *pos ) {}

  // text should have place for a terminating zero (length+1 chars long)

  virtual void getCurrentLine( char *text, int &length ) {}

  virtual ~View()
  {
    delete defaultFontName;
  	textBuffer->removeBufferObserver( this );
    textBuffer->getUndoBuffer()->removeUndoObserver( this );
  }

  virtual void setHighlight( int begin, int end );
  virtual void resetHighlight();

	virtual int gethlBegin() { return hlBegin; }
	virtual int gethlEnd() { return hlEnd; }

  virtual void setLineMark( int lineStart, int cursorPos );
  virtual void resetLineMark();
  virtual int getLineMarkCursorPosWhenSet() { return mlCursorPos; }

	virtual void modified( BOOL isModified );
	virtual void filenameUpdated();
  BOOL isBusy() { return textBuffer->hasWriters(); }
	// returns 0 on success

  int saveFile();
};
#define VIEW
#endif
