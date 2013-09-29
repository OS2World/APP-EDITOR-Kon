#if !defined ASCIVIEW
#define ASCIVIEW
#include "view.h"
#include "buffer.h"
#include "syntax.h"

/******************************************************************************
 * AsciiView displays and allows the user to navigate the text as ASCII
 */

// Used for returning which sections in the file which are text, not
// comments etc.
struct CodeSection
{
  int startPos, endPos;
};

/* AWinMoveEvent: Activates when a window is moved */
class ALineMoveEvent : public AEvent
{
  int oldLine, newLine, lineStart, lineEnd;
public:
  ALineMoveEvent( View *source, int oldLine, int newLine, int lineStart, int lineEnd )
  {
    this->oldLine = oldLine;
    this->newLine = newLine;
    this->lineStart = lineStart;
    this->lineEnd = lineEnd;
  }
  int getOldLine() { return oldLine; }
  int getNewLine() { return newLine; }
  int getLineStart() { return lineStart; }
  int getLineEnd() { return lineEnd; }
};

/* AWinMoveListener: Listens to window moves */

class ALineMoveListener : public AListener
{
public:
  virtual void actionPerformed( ALineMoveEvent *moveEvent ) {};
};

class AsciiView : public View
{
	struct DelimitorLine
	{
		DelimitorLine *next;
		int charPos;
		DelimitorLine( int pos )
		{
			charPos = pos;
			next = NULL;
		}
		~DelimitorLine()
		{
			if( next )
				delete next;
		}
	} *delimLine;
  // A list containing the height and buffer psoitions for each line on the
  // screen

  struct LineInfo
  {
    int height,             // Height in pixels
        startPos, endPos;   // Start and endpos in characters
  };

  // holder for multilines
  //          1
  // 12345678901
  // /*       */
  //
  // start = 1, startLen = 2, end = 10, endLen = 2

  struct Multiline
  {
    Pair *pair;
    int start, startLen, end, endLen;
  } *multiline;

  int multilines;

#define MAXLINELEN 1000

  struct ColorAttribute
  {
    ColorValue fg, bg;
  } colorAttribute[MAXLINELEN];

  struct FontAttribute
  {
    Font *font;
    int X, Y, points, attr;
  } fontAttribute[MAXLINELEN];

  // If a position in highlighted is TRUE, it is already syntax highlighted
  // and should not be touched
  MutexSemaphore lineInfoLock;

  BOOL highlighted[MAXLINELEN];

  TextElem displayText[MAXLINELEN], leftMarginText[20]; // Used in getTextlineInfo
  Point charPositions[MAXLINELEN];  // -"-
  Point currentLineCharPos[MAXLINELEN]; // The character positions for the
                                        // current line
  int currentLineCharPosCalculated;     // Is set to currentLineStart
                                        // If this value doesn't correspond
                                        // to the current value of
                                        // currentLineStart, this value has
                                        // to be recalculated
  int currentLineCharPosMaxCalculated;  // The longest the current line has
                                        // been calculated.
  int longestLineInText;                // The longest found line in the text (in characters)

  Syntax *syntax;
  int currentLineStart;  // The linestart for the currentline
  int currentLineLength;
  int spaceWidth, currentLineLastSpaceWidth, indentedPixels;
  // PreferredPixelCol is set to the horizontal pixel column the cursor should
  // be set as close to as possible when moving between lines in the text
  // without moving the cursor horizontally.
  // When horizontal movement is done, preferredPixelCol should either be set
  // to UNDEF or to the new pixel column
  int preferredPixelCol;
  static int UNDEF;

  // adjustUserPosition sets the position in the text stream according to the
  // current column and line

  void adjustUserPosition()
  {
    userPosition = currentLineStart + currentColumn-1;
    if( userPosition > currentLineStart + currentLineLength )
      userPosition = currentLineStart + currentLineLength;
  }
  int markBeginColumn,
      markEndColumn;
  BOOL cursorBeyondEOL, tempVirtualSpace;  // Used when virtual space is off
  BOOL softEOL; // used in highlight. BAD BAD BAD
  long virtualCursorPos; // -"-
  Vector<ALineMoveListener> moveListener;
public:
  struct Position
  {
    int position;
    int column;
    int line;
		BOOL visible;
  };
private:
  Position  *pushed;
  int pushedPositions, pushedAllocated;

  // Returns the actual column on the screen, regardless of tabs.
  // This method only works with fixed-width fonts
  // If reader == -1 then a reader is allocated

  int getCursorColumnSpaces( int reader, int charCol );
  int oldColumn, oldUserPosition;
	long getDefaultFontHeight( Graphics *g, int target );
	BOOL isSimpleControl;
	void startControl( void )
	{
    viewType = VIEW_ASCIIVIEW;
		if( Settings::showRightMargin )
			delimLine = new DelimitorLine( Settings::rightMargin );
		else
			delimLine = NULL;
    bufferLines = textBuffer->getLFCountInserted() + 1;
    int foo, reader = textBuffer->readRequest();
    findMultilinePairs( reader, 0, textBuffer->getSize(), foo, foo );
    textBuffer->readRelease();
	}
public:
  AsciiView( ViewWindowHolder *vwHolder, TextBuffer *textBuffer,
             Syntax *syntax, InfoBar *infoBar, BOOL isSimple ) :
    View( vwHolder, textBuffer, infoBar ), multiline(NULL), multilines(0),
    currentLineCharPosCalculated(-1), currentLineStart(0), currentLineLength(0),
    preferredPixelCol(UNDEF), pushedPositions(0),
    pushedAllocated(0), oldColumn(-1), oldUserPosition(-1), indentedPixels(0),
		isSimpleControl( isSimple ), syntax(syntax), cursorBeyondEOL(FALSE),
    tempVirtualSpace(FALSE), moveListener(FALSE), longestLineInText(0)
  {
		startControl();
  }

  AsciiView( ViewWindowHolder *vwHolder, TextBuffer *textBuffer,
             Syntax *syntax, InfoBar *infoBar ) :
    View( vwHolder, textBuffer, infoBar ), multiline(NULL), multilines(0),
    currentLineCharPosCalculated(-1), currentLineStart(0), currentLineLength(0),
    preferredPixelCol(UNDEF), pushedPositions(0),
    pushedAllocated(0), oldColumn(-1), oldUserPosition(-1), indentedPixels(0),
		isSimpleControl( FALSE ), syntax(syntax), cursorBeyondEOL(FALSE),
    tempVirtualSpace(FALSE), moveListener(FALSE), longestLineInText(0)
  {
		startControl();
	}

  virtual void updateGraphicCursor();

  void setDefaultFont( Graphics *g, int target );

	void setDelimitors( int *delims, int count );

  virtual void insertion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines );
  virtual void deletion( TextBuffer *textBuffer, int reader, int position,
                         int size, int lines );

  virtual TSyntax *eval( char *name, TVariableList *vars, TEval *eval, 
	                       int index );

  AWord *findWord( TextElem *key, int keyLen, AWord *wordList, int words );

  BOOL findClosePair( int reader, Pair *pair, int &start, int end,
                      BOOL checkEOL );
  int findPair( int reader, Pair *pair, int pairs, int &pos,
                BOOL checkEOL, BOOL &unterminated );
  int findPairAfterPos( int atPos, int pairEnd );

  // pos will be set to the line end if a comment is found
  BOOL findEolComment( int reader, int &pos, int end );

  void removeOverlappingMultiline( int start, int &updateStart,
                                   int &updateEnd, int delta );

  // Find multiline comments between start and end. Remove overlapped
  // multiline comments and set updateStart and updateEnd to the endppoint
  // positions where the screen should be updated (in chars).

  BOOL findMultilinePairs( int reader, int start, int end, int &updateStart,
                           int &updateEnd );

  void setAttr( AWord *word, int start, int end );
  void setAttr( AWord *word, int addFontAttr, int start, int end );

  // Resets font and color attributes and returns the linelength

  int resetLine( int reader, int &position, BOOL &softEOL );

  void setMark( int &begin, int &end, int linestart, int lastPos,
                ColorValue fg, ColorValue bg );

  virtual void shiftStateSet( int shiftstate );

  // highLight is called from getTextlineInfo. It uses the displayText
  // array, and manipulates the colorAttribute and fontAttribute arrays
  // linestart is the position in the buffer where the current line starts.
  // If highLight is called with nonStandardHeightOnly, the displayText
  // is only filled with characters which are of non-standard font height
  // only. This can not be used to display text on the screen, but can be
  // used for calculating line heights.
  // The line length is returned. This may not be the actual number of
  // characters between linestart and line end. This length is returned in
  // trueLength

  virtual int highLight( int reader, int linestart, int &lastPos,
                         int &trueLength, BOOL nonStandardHeightOnly, 
                         int target );

  virtual int getTextlineHeight( int reader, int position, Graphics *g,
                                 int target );

  // Returns a DisplayText linked list with information for a given line

  virtual TextFont *getTextlineInfo( int reader, int &position, int &viewLine,  
                                     int &maxHeight,
	                                   int &maxBaseline, Graphics *g,
                                     int target, int rightPixelMargin );

  virtual void storeCursorPos() 
  { 
    oldColumn = currentColumn; 
    oldUserPosition = userPosition;
  }

  virtual BOOL cursorHasMoved() 
  { 
    return oldColumn != currentColumn || userPosition != oldUserPosition ||
    			View::cursorHasMoved();
  }

  BOOL atLineEnd( int reader, int pos );

  virtual int getNewLineStartFrom( int reader, int lineStart, 
	                                 int &linesDelta, int &viewLine );

  virtual BOOL jumpToLine( int line );

  virtual BOOL lineDown()
  {
    return jumpToLine( currentLine + 1 );
  }

  virtual BOOL lineUp()
  {
    return jumpToLine( currentLine - 1 );
  }

  virtual int getColumn() { return getCursorColumnSpaces(-1, currentColumn); }
  virtual void cursorToColumn( int column );
  virtual void cursorToColumn( int column, BOOL tempVirtualSpace );

  virtual BOOL charRight();
  virtual BOOL charLeft();
  virtual void toLineEnd();
  virtual void toLineStart();
  virtual void pushPos( BOOL visible );
  BOOL popPosP( Position *pos );
  virtual void popPos( BOOL moveCursor );
  BOOL peekPos( Position *pos );
  BOOL popPosP( Position *pos, BOOL peek );
  virtual void setMark();
  virtual void markForward();
  virtual void markBackward();

  virtual void endMark()
  {
    Position pos;

    peekPos( &pos );

    if( userPosition > pos.position )
      markForward();
    else
      markBackward();
		_keepBlock--;
  }

	virtual void deleteBlock();
  virtual void copyBlockToClipboard();
  virtual void cutBlockToClipboard();
  virtual BOOL pasteBlockFromClipboard();

  // calcCharacterPositions, getCursorColumn and getCursorWidth should not
  // be called if there is no viewWindow attached to this view. The latter
  // two is called outside of the View, but calcCharacterPositions may be called
  // from within this view. Check should therefore be done there so that it
  // is not called if no viewWindow exists.

  // Calculate the pixel positions for each character on the current line
  // if this hasn't already been done in a paint method

  void calcCharacterPositions( BOOL calcFullLength )
  {
    int reader = textBuffer->readRequest();
    calcCharacterPositions( reader, calcFullLength );
    textBuffer->readRelease();
  }

  void calcCharacterPositions( int reader, BOOL calcFullLength );

  virtual void requestScreenBuffer();
  virtual void releaseScreenBuffer();
  virtual int getLineStart() { return currentLineStart; }
  virtual int getLineEnd();

  virtual int getCursorColumn();

  virtual int getCursorWidth()
  {
    int reader = textBuffer->readRequest();
    int ret = getCursorWidth( reader );
    textBuffer->readRelease();
    return ret;
  }

  virtual int getCursorWidth( int reader );

  virtual void setCursorColumn( int pixelCol )
  {
    preferredPixelCol = UNDEF;
    setCursorColumnKeepPreferred( pixelCol );
  }

  virtual void setCursorColumn( int pixelCol, BOOL resetLockedColumn, BOOL tempVirtualSpace )
  {
    preferredPixelCol = UNDEF;
    cursorBeyondEOL = !resetLockedColumn;
    this->tempVirtualSpace = tempVirtualSpace;
    setCursorColumnKeepPreferred( pixelCol );
  }

  virtual void setCursorColumnKeepPreferred( int pixelCol );
  void getClosestStartingPoint( int toPosition, int &start, int &fromLine,
                                BOOL useUserPosition );
  virtual BOOL jumpToPosition( int reader, int position );
  BOOL indent( int writer, int &startPos, int fromColumn, int toColumn, BOOL inBlockUndo, BOOL startBlockUndo );	// Returns TRUE if blockUndo started
  virtual void insertString( TextElem *string );
  virtual void deleteCharacter();
  virtual void resetView()
  {
    currentLineStart = userPosition = 0;
    currentLine = 1;
    currentColumn = 1;
  }
  virtual void getCurrentLine( char *text, int &length );
  virtual int getLineFromPosition( int position );
  virtual int getPositionFromLine( int reader, int line );
  virtual void getPositionString( char *pos )
  {
  	if( relativePos )
	    sprintf( pos, "%10d", userPosition - relativeStart );
    else
	    sprintf( pos, "%4d:%d", getCursorColumnSpaces(-1, currentColumn), currentLine );
  }
  int getMarkBeginColumn() { return markBeginColumn; }
  int getMarkEndColumn() { return markEndColumn; }
  Syntax *getCurrentSyntax() { return syntax; }
  void setSyntax( Syntax *syntax ) { this->syntax = syntax; }
	~AsciiView()
	{
		if( pushedAllocated > 0 )
			free( pushed );
		if( multilines > 0 ) free( multiline );
		if( delimLine ) delete delimLine;
	}

  void setBlockStart( int position );
  void setBlockEnd( int position );
  int getCodeSections( CodeSection **section );
  void addMoveListener( ALineMoveListener *moveListener )
  {
    this->moveListener.add( moveListener );
  }
  void performMoveEvent( int oldLine, int newLine, int lineStart, int lineEnd );
};
#endif
