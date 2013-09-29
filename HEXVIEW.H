#include "view.h"
#include "buffer.h"
#include "syntax.h"

/******************************************************************************
 * HexView displays and allows the user to navigate the text as HEX and Hex
 */

class HexView : public View
{
  int spaceWidth;
  static int UNDEF;

public:
  struct Position
  {
    int position;
  };
private:
  Position  *pushed;
  int pushedPositions, pushedAllocated;
  enum Field { hexField, asciiField } currentField;
  int bytePos;
  int oldBytePos, oldUserPosition, oldCursorPosition;
  Field oldField;

	long getDefaultFontHeight( Graphics *g, int target );
public:
  HexView( ViewWindowHolder *vwHolder, TextBuffer *textBuffer,
             InfoBar *infoBar ) :
    View( vwHolder, textBuffer, infoBar ), 
    pushedPositions(0), pushedAllocated(0), currentField( asciiField ), 
    bytePos(0), oldCursorPosition(-1), spaceWidth(0)
  {
    viewType = VIEW_HEXVIEW;
    bufferLines = textBuffer->getSize() / 16 + 1;
  }

  void setDefaultFont( Graphics *g, int target );

  void switchField();

  virtual void updateGraphicCursor();

  virtual int getLineStart() { return userPosition - userPosition % 16; }
  virtual BOOL queryUpdateAllOnModification() { return TRUE; }
  virtual int getLineFromPosition( int position ) { return position / 16 + 1; }
  virtual int getPositionFromLine( int reader, int line )
  {
  	return line * 16 - 16;
  }

  virtual void insertion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines );
  virtual void deletion( TextBuffer *textBuffer, int reader, int position,
                         int size, int lines );

  virtual TSyntax *eval( char *name, TVariableList *vars, TEval *eval, 
	                       int index );

  void setMark( int &begin, int &end, int linestart, int lastPos,
                ColorValue fg, ColorValue bg );

  virtual int getTextlineHeight( int reader, int position, Graphics *g,
                                 int target );

  // Returns a DisplayText linked list with information for a given line

  virtual TextFont *getTextlineInfo( int reader, int &position, int &viewLine,  
                                     int &maxHeight,
                                     int &maxBaseline, Graphics *g,
                                     int target, int rightPixelMargin );
  virtual int getNewLineStartFrom( int reader, int lineStart, 
	                                 int &linesDelta, int &viewLine );

  virtual void storeCursorPos() 
  { 
    oldField = currentField;
    oldBytePos = bytePos;
    oldUserPosition = userPosition;
  }

  virtual BOOL cursorHasMoved() 
  { 
    return oldField != currentField || userPosition != oldUserPosition ||
           oldBytePos != bytePos || View::cursorHasMoved();;
  }

  virtual BOOL jumpToLine( int line );
  virtual BOOL lineDown();
  virtual BOOL lineUp();
  virtual BOOL charLeft();
  virtual BOOL charRight();
  virtual void cursorToColumn( int column );
  virtual void toLineEnd();
  virtual void toLineStart();

  virtual void pushPos( BOOL visible )
  {
    if( pushedAllocated == 0 )
    {
      pushed = (Position *)malloc( sizeof( Position ) );
      pushedAllocated = 1;
    }
    else
    if( pushedPositions >= pushedAllocated )
    {
      pushedAllocated = pushedPositions+1;
      pushed = (Position *)realloc( (void *)pushed,
                                    sizeof( Position ) * pushedAllocated );
    }
    pushed[pushedPositions].position = userPosition;
    pushedPositions++;
  }

  BOOL popPosP( Position *pos )
  {
    return popPosP( pos, FALSE );
  }

  virtual void popPos( BOOL moveCursor )
  {
  	Position pos;
    popPosP( &pos, FALSE );
		if( moveCursor )
			userPosition = pos.position;
  }


  BOOL peekPos( Position *pos )
  {
    return popPosP( pos, TRUE );
  }

  BOOL popPosP( Position *pos, BOOL peek )
  {
    if( pushedPositions == 0 )
    {
      PopupMessage popupMessage( Desktop::getInstance(), Strings::get(35),
                                 Strings::get(58 ), PopupMessage::error );
      return FALSE;
    }
    else
    {
      pushedPositions--;
      pos->position = pushed[pushedPositions].position;

      if( peek )
        pushedPositions++;
    }
    return TRUE;
  }

  virtual void setMark()
  {
    int oldBegin = markBegin, oldEnd = markEnd;
    markBegin = markEnd = userPosition;
    int reader = textBuffer->readRequest();
    vwHolder->updatePosition( this, reader, oldBegin, oldEnd );

    textBuffer->readRelease();

    if( (oldBegin == oldEnd && markBegin != markEnd) ||
        (oldBegin != oldEnd && markBegin == markEnd ) )
      markModified();

  }

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

  virtual BOOL jumpToPosition( int reader, int position );
  virtual void insertString( TextElem *string );
  virtual void deleteCharacter();
  virtual void copyBlockToClipboard();
	virtual void deleteBlock();
  virtual void cutBlockToClipboard();
  virtual BOOL pasteBlockFromClipboard();

  virtual void setCursorColumn( int pixelCol );
  virtual int getCursorColumn();
  virtual int getCursorWidth( int reader )
  {
    return getCursorWidth();
  }

  virtual int getCursorWidth()
  {
    return spaceWidth;
  }

  virtual void resetView()
  {
    userPosition = 0;
    currentColumn = 1;
    currentLine = 1;
  }
  virtual void getPositionString( char *pos )
  {
  	if( relativePos )
	    sprintf( pos, "%10d", userPosition - relativeStart );
		else
    	sprintf( pos, "%10d", userPosition );
  }
};

