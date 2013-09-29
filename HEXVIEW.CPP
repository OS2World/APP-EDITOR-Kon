#include <string.h>
#include "bowl.h"
#include "hexview.h"
#include "globals.h"
#include "undo.h"
#include "print.h"

int HexView::UNDEF = -1;

void HexView::setDefaultFont( Graphics *g, int target )
{
	if( (target == SCREEN && (strcmp( defaultFontName, Settings::hexFont ) ||
	 		 defaultFontPoints != Settings::hexFontSizeP ||
	 		 defaultFontX != Settings::hexFontSizeX ||
			 defaultFontY != Settings::hexFontSizeY)) ||
      (target == PRINTER && (strcmp( defaultFontName, Print::printerFont ) ||
       defaultFontPoints != Print::printerFontSize)) )
	{
		delete defaultFontName;

    if( target == SCREEN )
    {
		  defaultFontName = new char[strlen(Settings::hexFont)+1];
		  strcpy( defaultFontName, Settings::hexFont );
		  defaultFontPoints = Settings::hexFontSizeP;
		  defaultFontX = Settings::hexFontSizeX;
		  defaultFontY = Settings::hexFontSizeY;
    }
    else
    {
		  defaultFontName = new char[strlen(Print::printerFont)+1];
		  strcpy( defaultFontName, Print::printerFont );
		  defaultFontPoints = Print::printerFontSize;
		  defaultFontX = -1;
		  defaultFontY = -1;
    }

		defaultFont = g->getFontFromName( defaultFontName, 0 );

		g->selectFont( defaultFont,
									 defaultFontPoints, 
									 defaultFontX, 
									 defaultFontY, 0 );

		defaultFontHeight = g->getMaxFontHeight();
	}
}

void HexView::updateGraphicCursor()
{
  int reader = textBuffer->readRequest();

  if( oldCursorPosition != userPosition )
    vwHolder->updatePosition( this, reader, 
                              oldCursorPosition, oldCursorPosition );

  oldCursorPosition = userPosition;    

  vwHolder->updatePosition( this, reader, userPosition, userPosition );
  textBuffer->readRelease();
}

void HexView::insertion( TextBuffer *textBuffer, int reader, int position,
                           int size, int lines )
{
	bufferLines = textBuffer->getSize() / 16 + 1;

  if( position < userPosition )
  	userPosition += size;

  int oldBegin = markBegin, oldEnd = markEnd;

  if( position <= markBegin )
    markBegin += size;

  if( position < markEnd )
    markEnd += size;

	for( int i = 0; i < 10; i++ )
	{
		if( position <= savedPos[i] )
			savedPos[i] += size;
	}

  if( (oldBegin == oldEnd && markBegin != markEnd) ||
      (oldBegin != oldEnd && markBegin == markEnd ) )
    markModified();

  vwHolder->addUpdatePosition( this, reader, position, textBuffer->getSize() );
}

void HexView::deletion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines )
{
	bufferLines = textBuffer->getSize() / 16 + 1;

  if( position < userPosition && position + size > userPosition )
  {
    // The user was located within the text that was destroyed. We need to
    // move to the position where the deletion took place

  	userPosition = position;
  }
  else
  if( position < userPosition  )
  {
    userPosition -= size;
  }

	for( int i = 0; i < 10; i++ )
	{
		if( position < savedPos[i] )
		{
			if( position + size > savedPos[i] )
				savedPos[i] = -1;
			else
				savedPos[i] -= size;
		}
	}

  // Update mark position

  int oldBegin = markBegin, oldEnd = markEnd;

  if( position + size < markBegin )
  {
    markBegin -= size;
    markEnd -= size;
  }
  else
  if( position < markBegin && position + size >= markBegin &&
      position + size < markEnd )
  {
    markBegin = position;
    markEnd -= size;
  }
  else
  if( position < markBegin && position + size >= markBegin &&
      position + size >= markEnd )
    markBegin = markEnd = position;
  else
  if( position < markEnd && position + size >= markEnd )
    markEnd = position;
  else
  if( position + size < markEnd )
    markEnd -= size;

  if( (oldBegin == oldEnd && markBegin != markEnd) ||
      (oldBegin != oldEnd && markBegin == markEnd ) )
    markModified();

  vwHolder->addUpdatePosition( this, reader, position, textBuffer->getSize() );
}

TSyntax *HexView::eval( char *name, TVariableList *vars, TEval *eval, 
                        int index )
{
	switch( index )
	{
	  case 82 : // switchField
  	{
   	 	currentField = (currentField == asciiField) ? hexField : asciiField;
    	return NULL;
  	}
		default :
	  	return View::eval( name, vars, eval, index );
	}
}

long HexView::getDefaultFontHeight( Graphics *g, int target )
{
	setDefaultFont(g, target);
	return defaultFontHeight;
}

int HexView::getTextlineHeight( int reader, int position, Graphics *g,
                                int target )
{
  return getDefaultFontHeight(g, target);
}

void HexView::switchField()
{
  bytePos = 0;

  if( currentField == asciiField )
    currentField = hexField;
  else
    currentField = asciiField;
}

/******************************************************************************
 * getTextlineInfo calculates which fonts and color attributes a line of text
 * should have.
 */

TextFont *HexView::getTextlineInfo( int reader, int &position, int &viewLine,  
                                    int &maxHeight,
                                    int &maxBaseline, Graphics *g,
                                    int target, int rightPixelMargin )
{
	static TextElem posText[9], hexText[3*16+3], asciiText[16+3], *blank = "  ";
  int hfcColor[17], hbcColor[17], afcColor[17], abcColor[17], i;

  if( position > textBuffer->getSize() )
    return NULL;
    
  if( !Settings::linesInHex )
  	sprintf( posText, "%8.8d  ", position );
  else
  	sprintf( posText, "%8.8x  ", position );
    
	// We always move to the next line. The viewLine should extend the 
	// actual lines in the file for the ViewWindow to stop processing.

	if( viewLine == -1 )
		viewLine = getLineFromPosition( position );
	else
		viewLine++;

	for( i = 0; i < 16; i++ )
  {
    if( position + i < textBuffer->getSize() )
    {
  	  TextElem c = textBuffer->getChar( reader, position + i );
      unsigned ic = (unsigned char)c & 0xff;
  	  sprintf( &hexText[i*3], "%2.2x ", ic );
  	  
  	  if( Settings::hideAscii32 ) c = (c < 32 ? '.' : c );
      asciiText[i] = c;
    }
    else
    {
  	  sprintf( &hexText[i*3], "   " );
      asciiText[i] = ' ';
    }

    if( hlBegin <= position + i && hlEnd > position + i )
    {
      hfcColor[i] = afcColor[i] = Settings::hiliteFg;
      hbcColor[i] = abcColor[i] = Settings::hiliteBg;
    }
    else
    if( markBegin <= position + i && markEnd > position + i )
    {
      hfcColor[i] = afcColor[i] = Settings::markFg;
      hbcColor[i] = abcColor[i] = Settings::markBg;
    }
    else
    {
      hfcColor[i] = afcColor[i] = Settings::textFg;
      hbcColor[i] = abcColor[i] = Settings::textBg;
    }

    if( position + i == userPosition )
    {
      if( currentField == asciiField )
      {
        hfcColor[i] = Settings::hexCursorFg;
        hbcColor[i] = Settings::hexCursorBg;
      }
      else
      {
        afcColor[i] = Settings::hexCursorFg;
        abcColor[i] = Settings::hexCursorBg;
      }
    }
  }

  if( Settings::hexFontSizeP != -1 )
    g->selectFont( g->getFontFromName(Settings::hexFont, 0), Settings::hexFontSizeP, 0 );
  else
    g->selectFont( g->getFontFromName(Settings::hexFont, 0), Settings::hexFontSizeX, Settings::hexFontSizeY, 0 );
  
  maxBaseline = g->getFontBaseline();
  maxHeight = getDefaultFontHeight(g, target);
    
  int charWidth = g->getAverageCharWidth();
  spaceWidth = charWidth;

  TextFont *lastFont = NULL, *firstFont = NULL;
  TextColor *textColor, *prevColor, *firstColor;

  // POS
  
	textColor = new TextColor( Settings::leftMarginFg, 
	                           Settings::leftMarginBg, posText, 0 );
  textColor->length = 8;
  textColor->lastX = charWidth * textColor->length;

  firstColor = prevColor = textColor;

  // BLANKS

  textColor = new TextColor( Settings::textFg, Settings::textBg, blank, 
                             prevColor->lastX);
  textColor->length = 2;
  textColor->lastX = charWidth * textColor->length + prevColor->lastX;

  prevColor->next = textColor;
  prevColor = textColor;

  // HEX

  for( i = 0; i < 16; )
  {
    int start = i;
    while( hfcColor[i] == hfcColor[start] &&
           hbcColor[i] == hbcColor[start] && i < 16 ) i++;

    textColor = new TextColor( hfcColor[start], hbcColor[start], &hexText[start*3], 
                               prevColor->lastX);
    textColor->length = (i-start)*3;
    textColor->lastX = charWidth * textColor->length + prevColor->lastX;

    prevColor->next = textColor;
    prevColor = textColor;
  }

  // BLANKS

  textColor = new TextColor( Settings::textFg, Settings::textBg, blank, 
                             prevColor->lastX);
  textColor->length = 2;
  textColor->lastX = charWidth * textColor->length + prevColor->lastX;

  prevColor->next = textColor;
  prevColor = textColor;

  // ASCII

  for( i = 0; i < 16; )
  {
    int start = i;
    while( afcColor[i] == afcColor[start] &&
           abcColor[i] == abcColor[start] && i < 16 ) i++;

    textColor = new TextColor( afcColor[start], abcColor[start], &asciiText[start], 
                               prevColor->lastX);
    textColor->length = (i-start);
    textColor->lastX = charWidth * textColor->length + prevColor->lastX;

    prevColor->next = textColor;
    prevColor = textColor;
  }

  // BLANKS

  textColor = new TextColor( Settings::textFg, Settings::textBg, blank, 
                             prevColor->lastX);
  textColor->length = 2;
  textColor->lastX = charWidth * textColor->length + prevColor->lastX;

  prevColor->next = textColor;
  prevColor = textColor;

  // FONT
  
	firstFont = new TextFont( NULL, firstColor, 0, maxHeight, maxBaseline,
  													0, g->getFontFromName(Settings::hexFont, 0), Settings::hexFontSizeP,
                            Settings::hexFontSizeX, Settings::hexFontSizeY );

  position += 16;

  if( position > textBuffer->getSize() )
    position = textBuffer->getSize();

  return firstFont;
}

int HexView::getNewLineStartFrom( int reader, int lineStart, 
                                  int &linesDelta, int &viewLine )
{
  int newLineStart = lineStart + linesDelta * 16;

  if( newLineStart < 0 )
  {
    newLineStart = 0;
    linesDelta = lineStart / 16;
  }
  else
  if( newLineStart >= textBuffer->getSize() )
  {
    newLineStart = textBuffer->getSize() - textBuffer->getSize() % 16;
    linesDelta = (newLineStart - lineStart) / 16;
  }

	viewLine += linesDelta;

  return newLineStart;
}

BOOL HexView::jumpToLine( int line )
{
  // Check that we're not outside limits

  if( line > bufferLines || line < 1 ) return FALSE;

  currentLine = line;
  userPosition = (currentLine-1) * 16 + currentColumn - 1;

  return TRUE;
}

BOOL HexView::jumpToPosition( int reader, int position )
{
  if( position < 0 || position > textBuffer->getSize() ) return FALSE;

  userPosition = position;

  currentLine = position / 16 + 1;
  currentColumn = position % 16 + 1;

  return TRUE;
}

BOOL HexView::lineDown()
{
  if( userPosition + 16 > textBuffer->getSize() ) return FALSE;
  userPosition += 16;
  currentLine++;
  return TRUE;
}

BOOL HexView::lineUp()
{
  if( userPosition - 16 < 0 ) return FALSE;
  userPosition -= 16;
  currentLine--;
  return TRUE;
}

BOOL HexView::charLeft()
{
  if( currentField == hexField && bytePos == 1 )
  {
    bytePos = 0;
    return TRUE;
  }
  if( userPosition - 1 < 0 ) return FALSE;
  userPosition--;
  currentLine = userPosition / 16 + 1;
  currentColumn = userPosition % 16 + 1;
  bytePos = 1;
  return TRUE;
}

BOOL HexView::charRight()
{
  if( currentField == hexField && bytePos == 0 )
  {
    bytePos = 1;
    return TRUE;
  }
  if( userPosition + 1 > textBuffer->getSize() ) return FALSE;
  userPosition++;
  currentLine = userPosition / 16 + 1;
  currentColumn = userPosition % 16 + 1;
  bytePos = 0;
  return TRUE;
}

void HexView::cursorToColumn( int column )
{
  if( (currentLine-1) * 16 + column - 1 > textBuffer->getSize() ) return;
  userPosition = (currentLine-1) * 16 + column - 1;
  currentColumn = column;
}

void HexView::toLineEnd()
{
  userPosition = (currentLine-1)*16 + 15;
  if( userPosition > textBuffer->getSize() ) 
    userPosition = textBuffer->getSize();
  currentColumn = userPosition % 16 + 1;
}

void HexView::toLineStart()
{
  userPosition = (currentLine-1)*16;
  currentColumn = 1;
}


void HexView::insertString( TextElem *string )
{
  if( userPosition > textBuffer->getSize() ) return;

  int writer = textBuffer->writeRequest(), len = strlen( string );
  TextElem str2[2];
  BOOL moveForward = TRUE;
  BOOL blockUndo = FALSE;

  if( !Settings::persistentBlocks && validMark() && !keepBlock() )
  {
  	textBuffer->startUndoRepeat();
  	blockUndo = TRUE;
  	textBuffer->deleteText( writer, markBegin, markEnd-markBegin, TRUE, TRUE );
  }

	// Only overwrite with single chars, and not at document end

	if( !Settings::insertMode && len == 1 &&
	    userPosition < textBuffer->getSize() &&
			currentField == asciiField)	
	{
		if( !blockUndo )
	  	textBuffer->startUndoRepeat();
		textBuffer->deleteText( writer, userPosition, 1, TRUE, TRUE );
		blockUndo = TRUE;
	}

  if( currentField == hexField && len == 1 )
  {
    int digit = -1;

    if( *string >= '0' && *string <= '9' ) digit = *string - '0';
    if( *string >= 'a' && *string <= 'f' ) digit = *string - 'a' + 10;
    if( *string >= 'A' && *string <= 'F' ) digit = *string - 'A' + 10;

    if( digit == -1 )
    {
      // Invalid digit

      textBuffer->writeRelease();
      if( blockUndo )
      	textBuffer->endUndoRepeat();
      return;
    }

    if( Settings::insertMode && bytePos == 0 )
      *str2 = 0;
    else
      *str2 = textBuffer->getChar( writer, userPosition );

    *str2 &= (bytePos == 0) ? 0x0f : 0xf0; 
    *str2 |= (TextElem)((bytePos == 0) ? (digit << 4) : digit);

    moveForward = (bytePos == 1);

    textBuffer->startUndoRepeat();

    if( (Settings::insertMode && bytePos == 1) || !Settings::insertMode )
      textBuffer->deleteText( writer, userPosition, 1, TRUE, TRUE );

    textBuffer->insertText( writer, userPosition, str2, 1, TRUE, TRUE );
    textBuffer->endUndoRepeat();

    bytePos = (bytePos == 0) ? 1 : 0;
  }
  else
    textBuffer->insertText( writer, userPosition, string, len, TRUE, TRUE );

  if( moveForward )
    jumpToPosition( writer, userPosition + len );

  if( blockUndo )
  	textBuffer->endUndoRepeat();
  textBuffer->writeRelease();
}

void HexView::deleteCharacter()
{
  if( userPosition >= textBuffer->getSize() ) return;

  int writer = textBuffer->writeRequest();

  if( !Settings::persistentBlocks && validMark() && !keepBlock() )
  	textBuffer->deleteText( writer, markBegin, markEnd-markBegin, TRUE, TRUE );
	else
  	textBuffer->deleteText( writer, userPosition, 1, TRUE, TRUE );
  textBuffer->writeRelease();
}

void HexView::markForward()
{
  int reader = textBuffer->readRequest(),
      oldMarkBegin = markBegin, oldMarkEnd = markEnd;

  Position pos;

  popPosP( &pos );

  if( pos.position != markBegin && pos.position != markEnd )
  {
    // Clear old mark
    markBegin = markEnd = pos.position;

    if( oldMarkBegin != -1 )
      vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );
  }

  if( (pos.position == markBegin && userPosition >= markEnd) ||
      pos.position == markEnd )
  {
    if( pos.position != markEnd )  // Wrap
    {
      markBegin = markEnd;
    }
    markEnd = userPosition;
  }
  else
  {
    markBegin = userPosition;
  }

  int startUpdate = pos.position, endUpdate = userPosition;

  vwHolder->updatePosition( this, reader, startUpdate, endUpdate );
  textBuffer->readRelease();

  if( (oldMarkBegin == oldMarkEnd && markBegin != markEnd) ||
      (oldMarkBegin != oldMarkEnd && markBegin == markEnd ) )
    markModified();
}

void HexView::markBackward()
{
  int reader = textBuffer->readRequest(),
      oldMarkBegin = markBegin, oldMarkEnd = markEnd;

  Position pos;

  popPosP( &pos );

  if( pos.position != markBegin && pos.position != markEnd )
  {
    // Clear old mark
    markBegin = markEnd = pos.position;

    if( oldMarkBegin != -1 )
      vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );
  }

  if( (pos.position == markEnd && userPosition <= markBegin) ||
      pos.position == markBegin )
  {
    if( pos.position != markBegin )  // Wrap
    {
      markEnd = markBegin;
    }
    markBegin = userPosition;
  }
  else
  {
    markEnd = userPosition;
  }

  int startUpdate = userPosition, endUpdate = pos.position;

  vwHolder->updatePosition( this, reader, startUpdate, endUpdate );
  textBuffer->readRelease();

  if( (oldMarkBegin == oldMarkEnd && markBegin != markEnd) ||
      (oldMarkBegin != oldMarkEnd && markBegin == markEnd ) )
    markModified();
}

void HexView::copyBlockToClipboard()
{
  if( !validMark() ) return;

  TextElem *text = new TextElem[markEnd-markBegin];

  int reader = textBuffer->readRequest(), size;

  size = markEnd - markBegin;
  for( int i = markBegin, t = 0; i < markEnd; i++, t++ )
    text[t] = textBuffer->getChar( reader, i );

  ClipBoard::setText( text, size );

  textBuffer->readRelease();
}

void HexView::deleteBlock()
{
  int writer = textBuffer->writeRequest();

  textBuffer->deleteText( writer, markBegin, markEnd - markBegin, TRUE, TRUE );

  markEnd = markBegin;

  textBuffer->writeRelease();
}

void HexView::cutBlockToClipboard()
{
  if( !validMark() ) return;

  copyBlockToClipboard();

	deleteBlock();
}

BOOL HexView::pasteBlockFromClipboard()
{
  TextElem *text;

  int writer = textBuffer->writeRequest();

  int len = ClipBoard::getText( &text );

  if( len )
  {
    BOOL blockUndo = FALSE;

    if( !Settings::persistentBlocks && validMark() && !keepBlock() )
    {
  	  textBuffer->startUndoRepeat();
  	  blockUndo = TRUE;
  	  textBuffer->deleteText( writer, markBegin, markEnd-markBegin, TRUE, TRUE );
    }

    int oldMarkBegin = markBegin, oldMarkEnd = markEnd;
     textBuffer->insertText( writer, userPosition, text, len, TRUE, TRUE );

		if( Settings::persistentBlocks )
		{
	    markBegin = userPosition;
		  markEnd = userPosition + len;
		}
		else
			markBegin = markEnd = userPosition;

    vwHolder->updatePosition( this, writer, oldMarkBegin, oldMarkEnd );
    vwHolder->updatePosition( this, writer, userPosition, userPosition + len );

    jumpToPosition( writer, userPosition + len );

    markModified();

    if( blockUndo )
  	  textBuffer->endUndoRepeat();
  }

  textBuffer->writeRelease();

  if( len )
    delete text;

  return len != 0;
}

void HexView::setCursorColumn( int pixelCol )
{
  if( pixelCol < 10 * spaceWidth )
  {
    currentField = hexField;
    currentColumn = 1;
    userPosition = (currentLine-1)*16;
    bytePos = 0;
    return;
  }

  if( pixelCol < (10+16*3)*spaceWidth )
  {
    pixelCol -= 10 * spaceWidth;
    currentField = hexField;
    currentColumn = pixelCol / (3*spaceWidth) + 1;
    userPosition = (currentLine-1)*16 + currentColumn-1;

    if( (pixelCol / spaceWidth) % 3 == 0 )
      bytePos = 0;
    else
      bytePos = 1;
    updateGraphicCursor();
    return;
  }

  pixelCol -= (10+3*16+2) * spaceWidth;

  currentField = asciiField;
  currentColumn = pixelCol / spaceWidth + 1;
  if( currentColumn < 1 ) currentColumn = 1;
  if( currentColumn > 16 ) currentColumn = 16;
  userPosition = (currentLine-1)*16 + currentColumn-1;
  updateGraphicCursor();
}

int HexView::getCursorColumn()
{
  return (12 + ((currentField == hexField) ? 
                (currentColumn-1) * 3 - 2 + bytePos : 
                (currentColumn-1) + 16*3)) * 
         spaceWidth;
}
