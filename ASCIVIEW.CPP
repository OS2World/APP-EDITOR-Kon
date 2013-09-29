#include <string.h>
#include "bowl.h"
#include "asciview.h"
#include "globals.h"
#include "undo.h"
#include "print.h"
#include "grep.h"

int AsciiView::UNDEF = -1;

// This is the "only" function which handles virtual-space turned off
void AsciiView::updateGraphicCursor()
{
  if( !Settings::virtualSpace && !tempVirtualSpace)
  {
    // Check the cursor pixel pos and see if it is beyond the line length
    int cursorPixel = getCursorColumn() - indentedPixels;

    calcCharacterPositions( -1, FALSE ); // Full length should not be needed

    if( !cursorBeyondEOL && cursorPixel > currentLineCharPos[currentLineLength].x )
    {
      cursorBeyondEOL = TRUE;
      virtualCursorPos = cursorPixel;
      setCursorColumn( currentLineCharPos[currentLineLength].x + indentedPixels );
    }
    else
    if( cursorBeyondEOL )
    {
      if( virtualCursorPos >= currentLineCharPos[currentLineLength].x )
        setCursorColumn( currentLineCharPos[currentLineLength].x + indentedPixels );
      else
      {
        cursorBeyondEOL = FALSE;
        setCursorColumn( virtualCursorPos + indentedPixels );
      }
    }
  }
}

int AsciiView::getCursorColumnSpaces(int reader, int charCol )
{
	if( vwHolder != NULL )
	{
    if( reader != -1 )
  		calcCharacterPositions(reader, TRUE );
    else
  		calcCharacterPositions(TRUE);

		if( charCol > currentLineLength )
			return (currentLineCharPos[currentLineLength].x) / spaceWidth +
						 (charCol - currentLineLength-1) + 1;

		return (currentLineCharPos[charCol-1].x/* + indentedPixels*/) / spaceWidth + 1;
	}
	else
	{
		int reader = textBuffer->readRequest( NULL );
		int col = 0;
		for( int i = currentLineStart; i < currentLineStart + charCol; i++ )
		{
			if( textBuffer->getChar(reader, i ) == 0x9 )
      {
        col += Settings::tabSize;
        col -= col % Settings::tabSize;
      }
			else
				col++;
		}
		textBuffer->readRelease();
		return col;
	}
}

void AsciiView::setDefaultFont( Graphics *g, int target )
{
	if( (target == SCREEN && (strcmp( defaultFontName, Settings::asciiFont ) ||
	 		 defaultFontPoints != Settings::asciiFontSizeP ||
	 		 defaultFontX != Settings::asciiFontSizeX ||
			 defaultFontY != Settings::asciiFontSizeY)) ||
      (target == PRINTER && (strcmp( defaultFontName, Print::printerFont ) ||
       defaultFontPoints != Print::printerFontSize)) ||
       !g->hasFontSelected() )
	{
		delete defaultFontName;

    if( target == SCREEN )
    {
		  defaultFontName = new char[strlen(Settings::asciiFont)+1];
		  strcpy( defaultFontName, Settings::asciiFont );
		  defaultFontPoints = Settings::asciiFontSizeP;
		  defaultFontX = Settings::asciiFontSizeX;
		  defaultFontY = Settings::asciiFontSizeY;
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

void AsciiView::setDelimitors( int *delims, int count )
{
	if( delimLine )
		delete delimLine;

	DelimitorLine *line = NULL;
	delimLine = NULL;

	for( int i = 0; i < count; i++ )
	{
		DelimitorLine *nline = new DelimitorLine( delims[i] );
		if( delimLine == NULL )
			delimLine = line = nline;
		else
		{
			line->next = nline;
			line = line->next;
		}
	}
}

void AsciiView::insertion( TextBuffer *textBuffer, int reader, int position,
                           int size, int lines )
{
  bufferLines += lines;

  adjustUserPosition();

  if( position < currentLineStart )
  {
     currentLineStart += size;
     currentLine += lines;
     userPosition += size;
  }
  else
  if( position >= currentLineStart &&
      position <= currentLineStart + currentLineLength )
  {
    if( position < userPosition )
    {
      currentLine += lines;
      cursorBeyondEOL = FALSE;
      tempVirtualSpace = FALSE;
      preferredPixelCol = UNDEF;
    }

    // Calculate the new linelength

    if( lines == 0 )
      currentLineLength += size;
    else
    {
      currentLineLength = userPosition;

      while( currentLineLength < textBuffer->getSize() &&
             textBuffer->getChar( reader, currentLineLength ) != 0xa )
        currentLineLength++;

      if( textBuffer->getChar( reader, currentLineLength ) == 0xa &&
          textBuffer->getChar( reader, currentLineLength-1 ) == 0xd )
      {
        currentLineLength--;
        if( textBuffer->getChar( reader, currentLineLength-1 ) == 0xd ) currentLineLength--;
      }

      // Find new linestart

      currentLineStart = userPosition;

      while( currentLineStart > 0 &&
             textBuffer->getChar( reader, currentLineStart - 1 ) != 0xa )
        currentLineStart--;

      currentLineLength -= currentLineStart;

      currentColumn = userPosition - currentLineStart + 1;
    }
    currentLineCharPosCalculated = -1;
  }

  if( position <= markBegin )
    markBegin += size;

  if( position < markEnd )
    markEnd += size;

  if( markBegin > markEnd ) markEnd = markBegin;
  
  int i;

	for( i = 0; i < pushedPositions; i++ )
	{
		if( position <= pushed[i].position )
		{
			pushed[i].position += size;
			pushed[i].line += lines;
			pushed[i].column = -1;
		}
	}

	for( i = 0; i < 10; i++ )
	{
		if( position <= savedPos[i] )
			savedPos[i] += size;
	}

	for( i = 0; i < savedPositions; i++ )
	{
		if( position <= savedPosSorted[i] )
			savedPosSorted[i] += size;
		else
			break;
	}

  if( syntax == NULL ) return;

  // Push all multilines forward, and check if we need to recalc due to
  // destroyed or inserted

  int pairIndex = findPairAfterPos( position, TRUE );
  int recalcStart = position-syntax->longestMultiline+1,
      recalcEnd = position+size+syntax->longestMultiline-1,
      startUpdatePos = -1, endUpdatePos = -1, n;

  int firstInvalidPosition=-1, lastInvalidPosition=-1;

  // Recalc to EOL in case an eol comment or pair (or set) was destroyed

  while( recalcEnd < textBuffer->getSize() &&
         textBuffer->getChar( reader, recalcEnd ) != 0xa )
    recalcEnd++;

  // Check if eol comment destroys a multiline start

  BOOL foo;

  int start = position - syntax->longestEolComment+1,
      end = position + size + syntax->longestEolComment-1;

  // new eol comments should be checked for only on the last line

  if( lines != 0 )
  {
    // start should be set to the first char on the last line
    int lns = lines;

    start = position + size;

    while( lns && start > 0 )
    {
      if( textBuffer->getChar( reader, start ) == 0xa )
        lns--;
      start--;
    }

    while( end < textBuffer->getSize() &&
           textBuffer->getChar( reader, end ) != 0xa )
      end++;
  }

  // Check eol comments
  for( n = start; n < end; n++ )
  {
    int firstPos = n;
    if( findEolComment( reader, n, textBuffer->getSize()) )
    {
      firstInvalidPosition = firstPos-size;
      lastInvalidPosition = n-size;
      break;
    }
  }

  // See if there's a (single line) pair enclosing position

  for( n = start; n < end; n++ )
  {
    int firstPos = 0;
    if( findPair( reader, syntax->pair, syntax->pairs, n, TRUE, foo ) != - 1 )
    {
      if( position >= firstPos && position < n )
      {
        if( firstInvalidPosition > firstPos - size )
          firstInvalidPosition = firstPos - size;
        if( lastInvalidPosition < n - size )
          lastInvalidPosition = n - size;
        break;
      }
    }
  }

  for( i = pairIndex; i < multilines; i++ )
  {
    // We're in the middle of a multiline, check if we typed a close pair

    if( position >= multiline[i].start + multiline[i].startLen &&
        position < multiline[i].end )
    {
      int closeLen = strlen( multiline[i].pair->closep );
      int start = position-closeLen+1,
          end = position+size+closeLen-1;

      if( findClosePair( reader, multiline[i].pair, start, end, FALSE ) )
      {
        if( recalcStart > multiline[i].start )
          recalcStart = multiline[i].start;

        if( recalcEnd < multiline[i].end + multiline[i].endLen )
          recalcEnd = multiline[i].end + multiline[i].endLen;
        continue;
      }
    }

    // See if the multiline was destroyed by inserting a character

    // Check if we destroyed a start or end tag or an eol comment or a pair
    // destroyed the multiline

    int checkPos = (position <= multiline[i].end) ?
                    multiline[i].end+size : multiline[i].end;

    BOOL literalDestroyedEnd =
      (textBuffer->getChar( reader, checkPos-1 ) == '\\' &&
       textBuffer->getChar( reader, checkPos-2 ) != '\\' );

    if( (position > multiline[i].start &&
         position <= multiline[i].start +
                     multiline[i].startLen) ||
        (position > multiline[i].end &&
         position <= multiline[i].end +
                     multiline[i].endLen) ||
        (multiline[i].start > firstInvalidPosition &&
         multiline[i].start <= lastInvalidPosition) ||
        literalDestroyedEnd )
    {
      if( position > multiline[i].end || literalDestroyedEnd )
      {
        // And endpair was destroyed. Scan forward to the next one
        int start = multiline[i].end;

        startUpdatePos = start;

        findClosePair( reader, multiline[i].pair, start, textBuffer->getSize(),
                       FALSE );

        multiline[i].end = start - multiline[i].endLen;

        // Remove overlapping pairs

        removeOverlappingMultiline( i, startUpdatePos, endUpdatePos, size );

        if( endUpdatePos < start )
          endUpdatePos = start;
      }
      else
      {
        // Remove the multiline, and set screen update limits
        startUpdatePos = multiline[i].start;

        endUpdatePos = multiline[i].end + multiline[i].endLen;

        recalcStart = startUpdatePos - syntax->longestMultiline+1;
        recalcEnd = endUpdatePos + syntax->longestMultiline-1;

        memmove( &multiline[i], &multiline[i+1],
                 (multilines-i-1) * sizeof( Multiline ) );
        multilines--;
        i--;
      }
      continue;
    }

    // Push multiline start

    if( position <= multiline[i].start )
      multiline[i].start += size;

    if( position <= multiline[i].end )
      multiline[i].end += size;
  }

  // Find multiline comments between recalcStart and recalcEnd

  if( recalcStart < 0 ) recalcStart = 0;
  if( recalcEnd > textBuffer->getSize() )
    recalcEnd = textBuffer->getSize();

  findMultilinePairs( reader, recalcStart, recalcEnd,
                      startUpdatePos, endUpdatePos );

  // Update screen
  if( startUpdatePos != - 1 )
    vwHolder->addUpdatePosition( this, reader, startUpdatePos, endUpdatePos );
}

void AsciiView::deletion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines )
{
  bufferLines -= lines;

  adjustUserPosition();

  BOOL userPositionRecalced = FALSE, moveColumn = FALSE;

  if( position < userPosition && position + size > userPosition )
  {
    // The user was located within the text that was destroyed. We need to
    // move to the position where the deletion took place

    if( lines == 0 )
    {
      userPosition = position;
      currentColumn = userPosition - currentLineStart + 1;
      cursorBeyondEOL = FALSE;
      tempVirtualSpace = FALSE;
      preferredPixelCol = UNDEF;
    }
    else
    {
      userPosition = 0;
      currentLine = 1;
      jumpToPosition( reader, position );
      userPositionRecalced = TRUE;
    }
  }
  else
  if( position < userPosition  )
  {
    currentLine -= lines;
    userPosition -= size;
    moveColumn = TRUE;
  }

  if( !userPositionRecalced )
  {
    if( position + size < currentLineStart )
       currentLineStart -= size;
    else
    if( (position < currentLineStart &&
         position + size >= currentLineStart + currentLineLength) ||
        (position < currentLineStart && position + size >= currentLineStart) ||
        (position + size > currentLineStart + currentLineLength && 
        position <= currentLineStart + currentLineLength))
    {
      // Current line was destroyed

      currentLineLength = userPosition;

      while( currentLineLength < textBuffer->getSize() &&
             textBuffer->getChar( reader, currentLineLength ) != 0xa )
        currentLineLength++;

      if( textBuffer->getChar( reader, currentLineLength ) == 0xa &&
          textBuffer->getChar( reader, currentLineLength-1 ) == 0xd )
      {
        currentLineLength--;
        if( textBuffer->getChar( reader, currentLineLength-1 ) == 0xd ) currentLineLength--;
      }

      currentLineStart = userPosition;

      while( currentLineStart > 0 &&
             textBuffer->getChar( reader, currentLineStart - 1 ) != 0xa )
        currentLineStart--;

      currentLineLength -= currentLineStart;

      currentColumn = userPosition - currentLineStart + 1;

      currentLineCharPosCalculated = -1;
    }
/*    else
    if( position + size > currentLineStart + currentLineLength && 
        position <= currentLineStart + currentLineLength )
    {
      while( currentLineLength < textBuffer->getSize() &&
             textBuffer->getChar( reader, currentLineLength ) != 0xa )
        currentLineLength++;

      if( textBuffer->getChar( reader, currentLineLength ) == 0xa &&
          textBuffer->getChar( reader, currentLineLength-1 ) == 0xd )
      {
        currentLineLength--;
        if( textBuffer->getChar( reader, currentLineLength-1 ) == 0xd ) currentLineLength--;
      }
    }*/
  }

  if( moveColumn )
    currentColumn = userPosition - currentLineStart+1;


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
    
  int i;  

	for( i = 0; i < pushedPositions; i++ )
	{
		if( position < pushed[i].position )
		{
			if( position + size > pushed[i].position )
			{
				memmove( &pushed[i].position, &pushed[i+1].position, pushedPositions - i - 1 );
				pushedPositions--;
			}
			else
			{
				pushed[i].position -= size;
				pushed[i].line -= lines;
				pushed[i].column = -1;
			}
		}
	}

	for( i = 0; i < 10; i++ )
	{
		if( position < savedPos[i] )
		{
			if( position + size > savedPos[i] )
				savedPos[i] = -1;
			else
				savedPos[i] -= size;
		}
	}

	for( i = 0; i < savedPositions; i++ )
	{
		if( position < savedPosSorted[i] )
		{
			if( position + size > savedPosSorted[i] )
			{
				memmove( &savedPosSorted[i], &savedPosSorted[i+1], 
				         savedPositions - i - 1 );
				savedPositions--;
			}
			else
				savedPosSorted[i] -= size;
		}
	}
  if( syntax == NULL ) return;

  // Pull all multilines backward, and check if we need to recalc due to
  // destroyed or inserted

  int pairIndex = findPairAfterPos( position, TRUE );
  int recalcStart = position-syntax->longestMultiline+1,
      recalcEnd = position+syntax->longestMultiline-1,
      startUpdatePos = -1, endUpdatePos = -1, n;

  int firstInvalidPosition=-1, lastInvalidPosition=-1;

  // Recalc to EOL in case an eol comment or pair (or set) was destroyed

  while( recalcEnd < textBuffer->getSize() &&
         textBuffer->getChar( reader, recalcEnd ) != 0xa )
    recalcEnd++;

  // Check if eol comment destroys a multiline start

  for( n = position - syntax->longestEolComment+1;
       n < position + syntax->longestEolComment-1; n++ )
  {
    int firstPos = n;
    if( findEolComment( reader, n, textBuffer->getSize()) )
    {
      firstInvalidPosition = firstPos+size;
      lastInvalidPosition = n+size;
      break;
    }
  }

  BOOL foo;

  // See if there's a (single line) pair enclosing position

  for( n = position - 2; n < position + 2; n++ )
  {
    int firstPos = 0;
    if( findPair( reader, syntax->pair, syntax->pairs, n, TRUE, foo ) != - 1 )
    {
      if( position >= firstPos && position < n )
      {
        if( firstInvalidPosition > firstPos + size )
          firstInvalidPosition = firstPos + size;
        if( lastInvalidPosition < n + size )
          lastInvalidPosition = n + size;
        break;
      }
    }
  }

  for( i = pairIndex; i < multilines; i++ )
  {
    int checkPos = multiline[i].end-size;

    BOOL literalDestroyedEnd = (position < multiline[i].end) ?
      (textBuffer->getChar( reader, checkPos-1 ) == '\\' &&
       textBuffer->getChar( reader, checkPos-2 ) != '\\' ) : FALSE;

    // We're in the middle of a multiline, check if we created a close pair
    if( position >= multiline[i].start + multiline[i].startLen &&
        position + size < multiline[i].end )
    {
      int closeLen = strlen( multiline[i].pair->closep );
      int start = position-closeLen+1,
          end = position+closeLen;    // Also find endPair which became valid
                                      // du to literalChar diabled

      if( findClosePair( reader, multiline[i].pair, start, end, FALSE ) )
      {
        if( recalcStart > multiline[i].start )
          recalcStart = multiline[i].start;

        if( recalcEnd < multiline[i].end + multiline[i].endLen + size )
          recalcEnd = multiline[i].end + multiline[i].endLen + size;
      }
      else
      if( position < multiline[i].end )
        multiline[i].end -= size;
    }
    else
    // See if a multiline was destroyed
    if( (position <= multiline[i].start &&
         position + size > multiline[i].start) ||
        (position < multiline[i].start + multiline[i].startLen &&
         position + size >= multiline[i].start + multiline[i].startLen) ||
        (multiline[i].start > firstInvalidPosition &&
         multiline[i].start < lastInvalidPosition) )
    {
      // multiline destroyed

      startUpdatePos = position;

      endUpdatePos = multiline[i].end + multiline[i].endLen - size;

      recalcStart = startUpdatePos - syntax->longestMultiline+1;
      recalcEnd = endUpdatePos + syntax->longestMultiline-1;

      memmove( &multiline[i], &multiline[i+1],
               (multilines-i-1) * sizeof( Multiline ) );
      multilines--;
      i--;
    }
    else
    if( (position < multiline[i].end + multiline[i].endLen &&
         position + size >= multiline[i].end + multiline[i].endLen) ||
        (position <= multiline[i].end &&
         position + size > multiline[i].end ) ||
        literalDestroyedEnd)
    {
      // multiline end was destroyed

      int start = position-multiline[i].endLen+1/*multiline[i].end*/;

      startUpdatePos = start;

      findClosePair( reader, multiline[i].pair, start, textBuffer->getSize(),
                     FALSE );

      multiline[i].end = start - multiline[i].endLen;

      // Remove overlapping pairs

      if( endUpdatePos < start )
        endUpdatePos = start;

      removeOverlappingMultiline( i, startUpdatePos, endUpdatePos, -size );
    }
    else
    {
      if( position < multiline[i].end )
        multiline[i].end -= size;

      if( position < multiline[i].start )
        multiline[i].start -= size;
    }
  }

  // Find multiline comments between recalcStart and recalcEnd

  if( recalcStart < 0 ) recalcStart = 0;
  if( recalcEnd > textBuffer->getSize() )
    recalcEnd = textBuffer->getSize();

  findMultilinePairs( reader, recalcStart, recalcEnd,
                      startUpdatePos, endUpdatePos );

  // Update screen
  if( startUpdatePos != - 1 )
    vwHolder->addUpdatePosition( this, reader, startUpdatePos, endUpdatePos );
}

TSyntax *AsciiView::eval( char *name, TVariableList *vars, TEval *eval,
                          int index )
{
	switch( index )
	{
		case 12 : // integrityCheck
		{
			textBuffer->integrityCheck( "out" );
			return NULL;
		}
		case 109 : // toggleColumnBlock
		{
			Settings::columnBlock = !Settings::columnBlock;
			int reader = textBuffer->readRequest();
			vwHolder->updatePosition( this, reader, markBegin, markEnd );
			textBuffer->readRelease();
			char cmd[100];
			sprintf( cmd, "columnBlockSet(%d);", Settings::columnBlock );
			vwHolder->addCommand( cmd );
			return NULL;
		}
		case 110 : // setDelimitorLines
		{
			TSyntax **lines = vars->LookUp( "lines" );

			if( lines == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*lines)->Type != SArray )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			// Create an array

			TArray *lineA = (TArray *)*lines;

			int *pos = new int[lineA->Size];

			for( int i = 0; i < lineA->Size; i++ )
			{
				if( lineA->Array[i]->Type == SNumeric )
					pos[i] = ((TNumeric *)lineA->Array[i])->num;
			}

			setDelimitors( pos, lineA->Size );

			vwHolder->repaintView( NULL );

			delete pos;

			return NULL;
		}
		case 96 : // indent
		{
			TSyntax **position = vars->LookUp( "position" ),
							**fromColumn = vars->LookUp( "fromColumn" ),
							**toColumn = vars->LookUp( "toColumn" );

			if( position == NULL || fromColumn == NULL  || toColumn == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*position)->Type != SNumeric || (*fromColumn)->Type != SNumeric || 
					(*toColumn)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			int writer = textBuffer->writeRequest();

			int startPos = ((TNumeric *)*position)->num;

			indent( writer, startPos, ((TNumeric *)*fromColumn)->num, 
							((TNumeric *)*toColumn)->num, FALSE, FALSE );

			textBuffer->writeRelease();

			return new TNumeric( startPos, COPY );
		}
		case 127 :	// getSyntaxName
		{
			if( syntax != NULL )
				return new TString( syntax->name, COPY );
			else
				return new TString( "", COPY );
		}
    case 132 : // setBlockStart
    {
			TSyntax **position = vars->LookUp( "position" );

			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
      setBlockStart( ((TNumeric *)*position)->num );
      return NULL;
    }
    case 133 : // setBlockEnd
    {
			TSyntax **position = vars->LookUp( "position" );

			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
      setBlockEnd( ((TNumeric *)*position)->num );
      return NULL;
    }
    case 155 : // setSyntax
    {
			TSyntax **sname = vars->LookUp( "name" );

			if( sname == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*sname)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
      syntax = SyntaxContainer::getSyntax(((TString *)*sname)->string );
      vwHolder->repaintView( NULL );
      return NULL;
    }
    case 166 : // getMarkBegin
      return new TNumeric(getMarkBeginColumn(), COPY); break;
    case 167 : // getMarkEnd
      return new TNumeric(getMarkEndColumn(), COPY); break;
		default :
			return View::eval( name, vars, eval, index );
	}
}

BOOL AsciiView::findClosePair( int reader, Pair *pair, int &start, int end,
                               BOOL checkEOL )
{
  BOOL found = FALSE;

  int closeLen = strlen( pair->closep );

  while( start < end )
  {
    if( checkEOL && textBuffer->getChar(reader, start ) == 0xa )
      return FALSE;

    if( textBuffer->getChar(reader, start) == syntax->literalChar )
    {
      start += 2;
      continue;
    }
    
    int l;

    for( l = 0; l < closeLen; l++ )
    {
      found = TRUE;
      if( textBuffer->getChar(reader, start+l) != pair->closep[l] )
      {
        found = FALSE;
        break;
      }
    }
    if( found )
    {
      start += l;
      return TRUE;
    }
    start++;
  }
  return FALSE;
}

// Returns -1 if no pair is found, or the index of the pair that is found

int AsciiView::findPair( int reader, Pair *pair, int pairs, int &pos,
                         BOOL checkEOL, BOOL &unterminated )
{
  BOOL found;
  int i;
  
  for( i = 0; i < pairs; i++ )
  {
    found = TRUE;
    int l;
    
    for( l = 0; l < strlen( pair[i].openp ); l++ )
    {
      if( textBuffer->getChar( reader, pos+l ) != pair[i].openp[l] )
      {
        found = FALSE;
        break;
      }
    }

    if( found )
    {
      pos += l;
      unterminated = !findClosePair( reader, &pair[i], pos,
                                     textBuffer->getSize(), checkEOL );
      return i;
    }
  }
  return -1;
}

// Returns the pair that immediately preceeds pos
// If pairEnd is true, check is made at the pairs end, otherwise at the pair
// start.

int AsciiView::findPairAfterPos( int atPos, int pairEnd )
{
  int pos = multilines / 2, base = 0, Top = multilines, lastPos = -1, Res;

  if( multilines == 0 ) return 0;

  while( 1 )
  {
    if( pairEnd )
      Res = atPos - (multiline[pos].end + multiline[pos].endLen);
    else
      Res = atPos - multiline[pos].start;

    if( lastPos == pos ) break;

    lastPos = pos;

    if( base == Top )
      break;

    if( Res < 0 )
    {
      Top = pos;
      pos = base + (pos - base) / 2;
    }
    else
    if( Res > 0 )
    {
      base = pos;
      pos += (Top - pos) / 2;
    }
    else
      break;
  }

  if( Res > 0 && pos < multilines && pairEnd )
    return pos+1;
  else
    return pos;
}

BOOL AsciiView::findEolComment( int reader, int &pos, int end )
{
  BOOL found = TRUE;

  for( int i = 0; i < syntax->eolComments; i++ )
  {
    found = TRUE;
    for( int l = 0; l < strlen( syntax->eolComment[i].string ); l++ )
    {
      if( textBuffer->getChar( reader, pos+l ) != syntax->eolComment[i].string[l] )
      {
        found = FALSE;
        break;
      }
    }

    if( found )
    {
      // skip eol-comment,
      while( textBuffer->getChar( reader, pos ) != 0xa && pos < end ) pos++;
      break;
    }
  }
  return found;
}

void AsciiView::removeOverlappingMultiline( int start, int &updateStart,
                                            int &updateEnd, int delta )
{
  for( int i = start + 1; i < multilines; i++ )
  {
    if( multiline[start].end > multiline[i].start + delta )
    {
      if( updateStart > multiline[i].start + delta )
        updateStart = multiline[i].start + delta;

      if( updateEnd < multiline[i].end + multiline[i].endLen + delta )
        updateEnd = multiline[i].end + multiline[i].endLen + delta;

      memmove( &multiline[i], &multiline[i+1],
               sizeof( Multiline ) * (multilines - i - 1) );
      multilines--;
      i--;
    }
  }
}

// Returns TRUE if a pair was found

BOOL AsciiView::findMultilinePairs( int reader, int start, int end,
                                    int &updateStart, int &updateEnd )
{
  if( syntax == NULL ) return FALSE;

  BOOL pairFound = FALSE;

  // Find linestart

  while( start > 0 && textBuffer->getChar( reader, start ) != 0xa ) start--;

  if( textBuffer->getChar( reader, start ) == 0xa ) start--;

  for( int pos = start; pos < end; )
  {
    // See if we have an ordinary pair or an eolComment

    // Check strings (pairs)

    BOOL foo;
    BOOL found = (findPair( reader, syntax->pair, syntax->pairs, pos,
                            TRUE, foo ) != -1);

    // Check comments

    if( !found )
      found = findEolComment( reader, pos, end );

    // Search for multiline comments

    int pairStart = pos;
    BOOL unterminated;

    int index = findPair( reader, syntax->multilineComment,
                          syntax->multilineComments, pos, FALSE,
                          unterminated );

    if( index != -1 )
    {
      // A multiline comment was found. Insert it at the correct position

      int pairIndex = findPairAfterPos( pairStart, TRUE );

      // See that the next pair in the list does not include this one
      if( pairIndex < multilines &&
          pairStart > multiline[pairIndex].start &&
          pairStart < multiline[pairIndex].end + multiline[pairIndex].endLen )
      {
        // Skip the found pair, and restart from the overlapping pair
        pos = multiline[pairIndex].start;
      }
      else
      {
        pairFound = TRUE;

        if( updateStart == -1 || updateStart > pairStart )
          updateStart = pairStart;

        if( updateEnd == -1 || updateEnd < pos )
          updateEnd = pos;

        multilines++;

        multiline = (Multiline *)realloc( multiline,
                                          sizeof( Multiline ) * multilines );
        memmove( &multiline[pairIndex+1], &multiline[pairIndex],
                 sizeof( Multiline ) * (multilines - pairIndex- 1) );
        multiline[pairIndex].pair = &syntax->multilineComment[index];
        multiline[pairIndex].start = pairStart;
        multiline[pairIndex].startLen =
            strlen(syntax->multilineComment[index].openp);
        if( unterminated )
          multiline[pairIndex].endLen = 0;
        else
          multiline[pairIndex].endLen =
              strlen(syntax->multilineComment[index].closep);
        multiline[pairIndex].end = pos - multiline[pairIndex].endLen;

        // Remove overlapping pairs

        removeOverlappingMultiline( pairIndex, updateStart, updateEnd, 0 );
      }
    }
    else
      pos++;
  }

  return pairFound;
}

AWord *AsciiView::findWord( TextElem *key, int keyLen, AWord *wordList,
                            int words )
{
  if( syntax == NULL ) return NULL;

  int pos = words / 2, base = 0, Top = words, lastPos = -1, Res;
	BOOL useWordLen = (keyLen == -1);

  if( words == 0 ) return NULL;

  while( lastPos != pos )
  {
		if( useWordLen ) keyLen = wordList[pos].len;
    if( syntax->caseInsensitive )
    {
			if( useWordLen )
				Res = strnicmp( key, wordList[pos].string, keyLen );
			else
				Res = stricmp( key, wordList[pos].string );
    }				
    else
		{
			if( useWordLen )
				Res = strncmp( key, wordList[pos].string, keyLen );
			else
				Res = strcmp( key, wordList[pos].string );
		}

    lastPos = pos;

    if( Res != 0 && base == Top ) return NULL;

			// Must have a trailing delimitor?

		if( useWordLen && Res == 0 && wordList[pos].delimEnd )
		{
			if( !strchr( syntax->delimitor, key[keyLen] ) )
				Res = 1;
		}

    if( Res < 0 )
    {
      Top = pos;
      pos = base + (pos - base) / 2;
    }
    else
    if( Res > 0 )
    {
      base = pos;
      pos += (Top - pos) / 2;
    }
    else
			return &wordList[pos];
  }

  return NULL;
}

void AsciiView::setAttr( AWord *word, int addFontAttr, int start, int end )
{
  for( int i = start; i < end; i++ )
  {
    if( !highlighted[i] )
    {
      colorAttribute[i].fg = (word->fg == Color::DEFAULT) ?
                              Settings::textFg : word->fg;
      colorAttribute[i].bg = (word->bg == Color::DEFAULT) ?
                              Settings::textBg : word->bg;
      fontAttribute[i].attr = word->fontAttr | addFontAttr;
    }
    highlighted[i] = TRUE;
  }
}

void AsciiView::setAttr( AWord *word, int start, int end )
{
	setAttr( word, 0, start, end );
}

void AsciiView::setMark( int &begin, int &end, int linestart, int lastPos,
                         ColorValue fg, ColorValue bg )
{
  if( (begin <= linestart && end > linestart) ||
      (begin <= lastPos && end >= lastPos) ||
      (begin >= linestart && begin < lastPos) ||
      (end >= linestart && end < lastPos) )
  {
    int first, last;

    if( begin <= linestart )
      first = 0;
    else
      first = begin - linestart;

    if( end > lastPos )
      last = lastPos-linestart;
    else
      last = end - linestart;

		if( last > MAXLINELEN ) last = MAXLINELEN;
    if( first > MAXLINELEN ) return;

    for( int i = first; i < last; i++ )
    {
      colorAttribute[i].fg = fg;
      colorAttribute[i].bg = bg;
    }
  }
}

void AsciiView::shiftStateSet( int shiftstate )
{
  static BOOL prevColBlock = -1;

  if( shiftstate & KeyDef::kALT )
  {
    if( prevColBlock == -1 ) 
    {
      prevColBlock = Settings::columnBlock;
    }
    Settings::columnBlock = TRUE;
  }
  else
  {
    if( prevColBlock != -1 ) 
    {
      Settings::columnBlock = prevColBlock;
		  int reader = textBuffer->readRequest();
		  vwHolder->updatePosition( this, reader, markBegin, markEnd );
		  textBuffer->readRelease();
    }
    prevColBlock = -1;
  }
}

int AsciiView::highLight( int reader, int linestart, int &lastPos,
                          int &trueLength, BOOL nonStandardHeightOnly, 
                          int target )
{
  if( nonStandardHeightOnly )
    return 0; // Syntax highlighting and selections doesn't alter heights

  // Fill the displayText array with the characters and fill the default
  // Color and Font attributes

  lastPos = linestart;

  softEOL = FALSE;
  int linelength = resetLine( reader, lastPos, softEOL ), indent = 0;
  trueLength = linelength - 1;  // Remove space at end

  if( syntax != NULL && Settings::syntaxHighlight )
  {
    TextElem *startPos = displayText, *endPos = displayText;
    TextElem *stopPos = displayText + linelength;
    BOOL *iHighlighted = highlighted;

    // Check multiline comments/strings

    // Find the multiline before linestart

    int multiIndex = findPairAfterPos( linestart, TRUE );

    // Loop through all multilines that extends between linestart and linelength

    for( int i = multiIndex; i < multilines &&
         multiline[i].start < linestart + linelength; i++ )
    {
      int fromPos = ((multiline[i].start > linestart) ?
                      multiline[i].start - linestart :
                      0),
          toPos = ((multiline[i].end+multiline[i].endLen <
                   linestart + linelength) ?
                   (multiline[i].end+multiline[i].endLen) - linestart :
                   linelength);
     	if( target == PRINTER )
	      setAttr( (AWord *)multiline[i].pair, Font::ITALIC, fromPos, toPos );
      else
  	    setAttr( (AWord *)multiline[i].pair, fromPos, toPos );
    }

    // Check strings and eol comments

    while( startPos < stopPos )
    {
      if( !*iHighlighted )
      {
        BOOL found = FALSE;

        // Check strings (pairs)
        
        int i;

        for( i = 0; i < syntax->pairs; i++ )
        {
          found = TRUE;
          int l;
          for( l = 0; l < strlen( syntax->pair[i].openp ); l++ )
          {
            if( *(startPos+l) != syntax->pair[i].openp[l] || *(iHighlighted+l) )
            {
              found = FALSE;
              break;
            }
          }

          if( found )
          {
            endPos = startPos + l;

            // Find the matching close

            while( endPos < stopPos )
            {
              // Check literalChar

              if( *endPos == syntax->literalChar )
              {
                endPos += 2;
                continue;
              }

              for( l = 0; l < strlen( syntax->pair[i].closep ); l++ )
              {
                found = TRUE;
                if( *(endPos+l) !=
                    syntax->pair[i].closep[l] || *(iHighlighted+l) )
                {
                  found = FALSE;
                  break;
                }
              }
              if( found )
              {
                endPos += l;
                break;
              }
              endPos++;
            }

            if( !found )  // Extend to EOL
              endPos = stopPos;

            setAttr( (AWord *)&syntax->pair[i], 
						         startPos - displayText,
                     endPos - displayText );
            iHighlighted += endPos - startPos;
            startPos = endPos-1;
            break;
          }
        }

        // Check comments

        if( !found )
        {
          for( int i = 0; i < syntax->eolComments; i++ )
          {
            found = TRUE;
            for( int l = 0; l < strlen( syntax->eolComment[i].string ); l++ )
            {
              if( *(startPos+l) !=
                  syntax->eolComment[i].string[l] || *(iHighlighted+l) )
              {
                found = FALSE;
                break;
              }
            }

            if( found )
            {
	          	if( target == PRINTER )
	              setAttr( (AWord *)&syntax->eolComment[i], Font::ITALIC, 
								         startPos - displayText,
  	                     linelength );
              else
	              setAttr( (AWord *)&syntax->eolComment[i], 
								         startPos - displayText,
  	                     linelength );
              stopPos = startPos;
              break;
            }
          }
        }
      }

      iHighlighted++;
      startPos++;
    }

    startPos = displayText;

		// Check noDelim
		if( syntax->noDelims != 0 )
		{
			while( startPos < stopPos )
			{
				// Skip whitespaces

				while( *startPos == ' ' || *startPos == 0x9 ) startPos++;

        AWord *word = findWord( startPos, -1, syntax->noDelim, syntax->noDelims );

        if( word != NULL )
          setAttr( word, startPos-displayText, 
					         startPos - displayText + word->len );

				startPos++;
			}
		}
		
		// Check greps
	  for( int g = 0; g < syntax->greps; g++ )
	  {
	    Grep *grep = new Grep(syntax->grep[g].string, syntax->delimitor, !syntax->caseInsensitive, syntax->grep[g].delimEnd, FALSE );
	    int startPos = 0, endPos = linelength;
	    
	    while( grep->findForward(-1, NULL, displayText, startPos, endPos ) )
	    {
	      startPos = endPos + 1;
	      endPos = linelength;
	      for( int p = 0; p < grep->getParens(); p++ )
          setAttr( &syntax->grep[g], grep->getParenStart(p), grep->getParenEnd(p) );
      }
      delete grep;
	  }

    startPos = displayText;

    // Check eol comments
    while( startPos < stopPos )
    {
      // Step through the words
      while( strchr( syntax->delimitor, *startPos ) && startPos < stopPos )
        startPos++;

      endPos = startPos;

      BOOL hasHighlight = FALSE;

      iHighlighted = highlighted + (startPos - displayText);

      while( !strchr( syntax->delimitor, *endPos ) && endPos < stopPos )
      {
        endPos++;
        if( *iHighlighted )
          hasHighlight = TRUE;
        iHighlighted++;
      }

      if( !hasHighlight )
      {
        // Search eolwords

        // For speed purposes, NULL terminate the keyword
        TextElem bakChar = *endPos;
        *endPos = 0;

        AWord *word = findWord( startPos, endPos-startPos, syntax->eolWord, syntax->eolWords );

        if( word != NULL )
          setAttr( word, startPos - displayText, 
					         linelength );
        else
        {
          // search keywords
          word = findWord( startPos, endPos-startPos, syntax->keyWord, syntax->keyWords );

          if( word != NULL )
          {
          	if( target == PRINTER )
            	setAttr( word, Font::BOLD, startPos - displayText, 
							         endPos - displayText );
            else
            	setAttr( word, startPos - displayText, 
							         endPos - displayText );
          }
        }

        *endPos = bakChar;
      }

      startPos = endPos;
    }
  }

  // Set block marks

  if( target != PRINTER )
  {
    if( Settings::columnBlock )
    {
      int start = -1, end = -1;

      if( markBegin < lastPos )
        start = markBeginColumn-1;

      if( markEnd >= linestart )
        end = markEndColumn-1;

      if( start > end )
      {
        int i = start;
        start = end;
        end = i;
      }

      if( start != -1 && end != -1 )
      {
        // We have to count tabs, in order for the marks to be as correct
        // as possible (not drifting away) Marks should not count characters,
        // but spaces
        // Note that column blocks will only work with fixed-width fonts

        int startPos = 0, endPos = 0, pos = 0, i;

        for( i = 0; i < end; i++ )
        {
          if( pos < start )
            startPos++;

          if( pos < end )
            endPos++;
          else
            break;

          if( i >= linelength )
          {
            if( pos < start )
              startPos += start-pos-1;
            endPos += end-pos-1;
            break;
          }

          if( displayText[i] == 0x9 )
          {
            pos += Settings::tabSize;
            pos -= pos % Settings::tabSize;
          }
          else
            pos++;
        }

        // Set mark color and add spaces if necessary

        for( i = startPos; i < endPos; i++ )
        {
          if( i >= linelength-1 )
          {
            for( int n = linelength; n <= i+1; n++ )
            {
              colorAttribute[n].fg = Settings::textFg;
              colorAttribute[n].bg = Settings::textBg;
              fontAttribute[n].font = defaultFont;
              fontAttribute[n].points = defaultFontPoints;
              fontAttribute[n].attr = 0;
              fontAttribute[n].X = defaultFontX;
              fontAttribute[n].Y = defaultFontY;
              displayText[n] = ' ';
            }
            linelength = i+2;
          }
          colorAttribute[i].fg = Settings::markFg;
          colorAttribute[i].bg = Settings::markBg;
        }
      }
    }
    else
      setMark( markBegin, markEnd, linestart, lastPos, Settings::markFg,
               Settings::markBg );

    setMark( hlBegin, hlEnd, linestart, lastPos, Settings::hiliteFg,
             Settings::hiliteBg );

    if( linestart == mlLineStart )
    {
      int uEnd = linestart+linelength;
      setMark( linestart, uEnd, linestart, lastPos, Settings::hiliteFg,
               Settings::hiliteBg );
    }
	}
  return linelength;
}

/******************************************************************************
 * resetLine fetches a line of text from the textbuffer and places it int the
 * displayText array.
 * !Note that it adds an extra space at the end of the text to terminate
 * keywords and strings. This is because the last color atttribute is extended
 * towars EOL when printing the text! This space should not be used when
 * calculating cursor width.
 */

#define setAttr \
      highlighted[lineLength] = FALSE; \
      colorAttribute[lineLength].fg = Settings::textFg; \
      colorAttribute[lineLength].bg = Settings::textBg; \
      fontAttribute[lineLength].font = defaultFont; \
      fontAttribute[lineLength].points = defaultFontPoints; \
      fontAttribute[lineLength].attr = 0; \
      fontAttribute[lineLength].X = defaultFontX; \
      fontAttribute[lineLength].Y = defaultFontY;

int AsciiView::resetLine( int reader, int &position, BOOL &softEOL )
{
  int lineLength = 0;
  softEOL = FALSE;
  while( 1 )
  {
    if( position >= textBuffer->getSize() ) break;

    TextElem tchar = textBuffer->getChar( reader, position++ );

    if( tchar == 0xd && textBuffer->getChar( reader, position ) == 0xa ||
        tchar == 0xa ||
	      tchar == 0xd && textBuffer->getChar( reader, position ) == 0xd && textBuffer->getChar( reader, position+1 ) == 0xa)
    {
      // End of line reached
      // Skip CR

      if( tchar == 0xd )
      {
	      if( textBuffer->getChar( reader, position ) == 0xd ) { softEOL = TRUE; position++; }
        position++;
      }
      break;
    }

    if( lineLength < MAXLINELEN - 1 )
    {
      setAttr
      displayText[lineLength++] = tchar;
    }
  }

  // Add a space character
  if( lineLength < MAXLINELEN - 1 )
  {
    setAttr
    displayText[lineLength++] = 32;
  }

  return lineLength;
}

long AsciiView::getDefaultFontHeight( Graphics *g, int target )
{
	setDefaultFont(g, target );
	return defaultFontHeight;
}

int AsciiView::getTextlineHeight( int reader, int position, Graphics *g,
                                  int target )
{
  int foo;

  requestScreenBuffer();
  setDefaultFont( g, target );

  int lineLength = highLight( reader, position, position, foo, TRUE, target );

  if( lineLength == 0 ) 
  {
    releaseScreenBuffer();
    return getDefaultFontHeight(g, target);
  }      

  // Loop through the fonts and find the largest height

  FontAttribute currentAttr;
  memset( &currentAttr, 0, sizeof( FontAttribute ) );
  int height = 0;

  for( int i = 0; i < lineLength; i++ )
  {
    if( memcmp( &currentAttr, &fontAttribute[i], sizeof( FontAttribute ) ) )
    {
      memcpy( &currentAttr, &fontAttribute[i], sizeof( FontAttribute ) );

      if( currentAttr.points == -1 )
        g->selectFont( currentAttr.font, currentAttr.X, currentAttr.Y,
                       currentAttr.attr );
      else
        g->selectFont( currentAttr.font, currentAttr.points, currentAttr.attr );

      if( g->getMaxFontHeight() > height )
        height = g->getMaxFontHeight();
    }
  }

  releaseScreenBuffer();
  return height;
}

/******************************************************************************
 * getTextlineInfo calculates which fonts and color attributes a line of text
 * should have.
 * It also calculates tab stops
 */

TextFont *AsciiView::getTextlineInfo( int reader, int &position, int &viewLine,  
                                      int &maxHeight,
                                      int &maxBaseline, Graphics *g,
                                      int target, int rightPixelMargin )
{
  if( position > textBuffer->getSize() )
    return NULL;

  requestScreenBuffer();

  int rightMargin = rightPixelMargin,
      charPixelPos = 0, charPos = 0, lineStart = position, trueLength;

	// We always move to the next line. The viewLine should extend the 
	// actual lines in the file for the ViewWindow to stop processing.

	if( viewLine == -1 )
		viewLine = 2;
	else
		viewLine++;

  // Syntax highlight
  setDefaultFont( g, target );

  int lineLength = highLight( reader, lineStart, position, trueLength, FALSE, target );

  if( lineLength == 0 )
  {
    maxHeight = getDefaultFontHeight( g, target );
    releaseScreenBuffer();
    return NULL;
  }

  if( lineLength > longestLineInText )
  {
    longestLineInText = lineLength;
    // Scan to very end of line and return info to the viewWindow for calculating the width of the scrollbar
    rightMargin = -1; 
  }

  // Create Font and Color list

  TextFont *lastFont = NULL, *firstFont = NULL;

	// If left margin info should be shown, add info

	if( Settings::showLeftMarginInfo && !isSimpleControl )
	{
		ColorValue fg = Settings::leftMarginFg, 
		           bg = Settings::leftMarginBg;

		// Check if we stand on a marker

		leftMarginText[19] = ' ';

		if( Settings::showSavedPos )
		{
		  int i;
			for( i = 0; i < pushedPositions; i++ )
			{
				if( pushed[i].position >= lineStart &&
						pushed[i].position < position &&
						pushed[i].visible )
				{
					fg = Settings::markerFg;
					bg = Settings::markerBg;
				}
			}

			for( i = 0; i < savedPositions; i++ )
			{
				if( savedPosSorted[i] >= lineStart &&
						savedPosSorted[i] < position )
				{
					fg = Settings::markerFg;
					bg = Settings::markerBg;
				}
				else
				if( savedPosSorted[i] >= position )
					break;
			}

			for( i = 0; i < 10; i++ )
			{
				if( savedPos[i] >= lineStart &&
						savedPos[i] < position )
				{
					fg = Settings::markerFg;
					bg = Settings::markerBg;
					leftMarginText[19] = i + '0';
				}
			}
		}

    //setDefaultFont( g, target );

    g->calcStringCharPos( 0, 0, 1, &leftMarginText[19],
                          charPositions );

    TextColor *textColor = new TextColor( fg, bg, 
		                                      &leftMarginText[19], 0 );

		textColor->length = 1;
    textColor->lastX = charPositions[1].x;
    textColor->locked = TRUE;

    TextColor *currentTC = textColor;
		           
		if( Settings::showLineNumbers )
		{
		  indentedPixels = charPixelPos = g->getAverageCharWidth() * 
		                   Settings::leftMarginChars;

		  sprintf( leftMarginText, "%u", viewLine - 1 );

		  // Right justify

      g->calcStringCharPos( 0, 0, strlen( leftMarginText ), 
		                        leftMarginText, charPositions );

			int indentX = indentedPixels - charPositions[strlen( leftMarginText )].x -
										g->getAverageCharWidth();

			currentTC->next = new TextColor( Settings::leftMarginFg, bg, 
																			 leftMarginText, indentX );

      currentTC = currentTC->next;

			currentTC->length = strlen( leftMarginText );
			currentTC->lastX = indentedPixels - g->getAverageCharWidth();
      currentTC->locked = TRUE;
		}
    else
      indentedPixels = charPixelPos = g->getAverageCharWidth();

    leftMarginText[18] = ' ';

		currentTC->next = new TextColor( Settings::leftMarginFg, bg, 
																		       &leftMarginText[18], indentedPixels );

    currentTC = currentTC->next;

		currentTC->length = 1;
		currentTC->lastX = indentedPixels;
    currentTC->locked = TRUE;

		firstFont = new TextFont( NULL, textColor, 0, 
															g->getMaxFontHeight(),
                              g->getFontBaseline(),
                              0,
                              defaultFont,
                              defaultFontPoints,
                              defaultFontX,
                              defaultFontY);
		lastFont = firstFont;
	}
	else
		indentedPixels = 0;
	
  while( (rightMargin == -1 || charPixelPos < rightMargin) && charPos < lineLength )
  {
    int startFont = charPos;

    TextColor *textColor = NULL;
    TextColor *firstColor = NULL, *lastColor = NULL;

    if( fontAttribute[startFont].X == -1 )
      g->selectFont( fontAttribute[startFont].font,
                     fontAttribute[startFont].points,
                     fontAttribute[startFont].attr );
    else
      g->selectFont( fontAttribute[startFont].font,
                     fontAttribute[startFont].X, fontAttribute[startFont].Y,
                     fontAttribute[startFont].attr );

    int fontHeight = g->getMaxFontHeight(),
        baseline = g->getFontBaseline();

    spaceWidth = g->getSpaceWidth();

    if( fontHeight > maxHeight)
      maxHeight = fontHeight;
    if( baseline > maxBaseline )
      maxBaseline = baseline;

    // Get all characters with the same font

    while( !memcmp( &fontAttribute[charPos], &fontAttribute[startFont],
                    sizeof( FontAttribute ) )&&
           charPos < lineLength )
    {
      int startColor = charPos;

      textColor = new TextColor( colorAttribute[startColor].fg,
                                 colorAttribute[startColor].bg,
                                 &displayText[startColor],
                                 charPixelPos );
      if( !firstColor )
        firstColor = textColor;

      // Get all characters with the same font and Color
      // When we hit a tab character, stop.
      // The tab character is included in the text list so this has
      // to be taken care of in the display routine so that the tab character
      // is not displayed. This is done by setting the lastIsTab flag in the
      // the TextColor struct, and xBeforeTab to the x pos immediately before
      // the tab character.

      while( !memcmp( &colorAttribute[charPos], &colorAttribute[startColor],
                      sizeof( ColorAttribute ) ) &&
             !memcmp( &fontAttribute[charPos], &fontAttribute[startFont],
                      sizeof( FontAttribute ) ) &&
             charPos < lineLength )
      {
        // See if we hit a tab
        if( displayText[charPos] == 0x9 )
        {
          textColor->lastIsTab = TRUE;
          charPos++;
          break;
        }

        charPos++;
      }

      textColor->length = charPos - startColor;

      if( lastColor != NULL )
        lastColor->next = textColor;
      lastColor = textColor;

      // Test print the string to find the last xPos

      g->calcStringCharPos( charPixelPos - indentedPixels, 0, textColor->length, textColor->text,
                            charPositions );

      if( textColor->lastIsTab )
      {
        textColor->xBeforeTab = charPositions[textColor->length-1].x + indentedPixels;

        // BORLAND OPTIMIZER BUG WORKAROUND BELOW USING TMP VARIABLE!

        int tmp = charPositions[textColor->length-1].x + 1;
        while( tmp % (spaceWidth*Settings::tabSize) ) tmp++;
        charPositions[textColor->length].x = tmp;
      }

      if( lineStart == currentLineStart )
      {
        // We may need this as we progress so fill upp, but only
        // use it if we really calculate the length of the whole line
        memcpy( &currentLineCharPos[startColor], charPositions,
                (textColor->length+1) * sizeof( Point ) );

        if( charPos + textColor->length >= trueLength )
        {
          currentLineCharPosCalculated = currentLineStart;
          currentLineLength = trueLength;
          currentLineLastSpaceWidth = spaceWidth;
        }
        else
          currentLineCharPosCalculated = -1;
      }

      textColor->lastX = charPixelPos = charPositions[textColor->length].x + indentedPixels;
    }

    // All colors in this font is done, create a new Font entry

    TextFont *textFont = new TextFont( NULL, firstColor, 0, fontHeight,
                                       baseline,
                                       fontAttribute[startFont].attr,
                                       fontAttribute[startFont].font,
                                       fontAttribute[startFont].points,
                                       fontAttribute[startFont].X,
                                       fontAttribute[startFont].Y );
    if( firstFont == NULL )
      firstFont = textFont;
    if( lastFont != NULL )
      lastFont->next = textFont;
    lastFont = textFont;
  }

	if( lastFont )
	{
		DelimitorLine *line = delimLine;
		GraphicObject *go;

		if( line )
		{
			lastFont->graphicObject = new GraphicObject( Color::LIGHTGRAY, GR_LINE, 
																								   spaceWidth * line->charPos + indentedPixels, 0, 0, maxHeight );
			go = lastFont->graphicObject;
			line = line->next;
		

			while( line )
			{
				go->next = new GraphicObject( Color::LIGHTGRAY, GR_LINE, 
							      									spaceWidth * line->charPos + indentedPixels, 0, 0, maxHeight );
				go = go->next;
				line = line->next;
			}
		}

    if( softEOL )
    {
      go = new GraphicObject( Color::DARKGREEN, GR_SOFTEOL, 
															charPixelPos - spaceWidth, 0, 0, maxHeight );
      go->next = lastFont->graphicObject;
      lastFont->graphicObject = go;
    }
	}

  releaseScreenBuffer();

  return firstFont;
}

int AsciiView::getNewLineStartFrom( int reader, int lineStart, 
                                    int &linesDelta, int &viewLine )
{
  if( linesDelta != 0 )
  {
    // It is possible that the lineStart is not really at the line start.
    // Move backwards if so

    while( lineStart > 0 && textBuffer->getChar( reader, lineStart-1 ) != 0xa )
      lineStart--;
    
    int delta = (linesDelta > 0) ? 1 : -1;
  
    int orgLinesDelta = linesDelta;
    linesDelta = 0;
  
    while( orgLinesDelta != linesDelta )
    {
      int oldLineStart = lineStart;
      if( delta > 0 )
      {
        // Go forward until we find an LF
  
        while( lineStart < textBuffer->getSize() &&
               textBuffer->getChar( reader, lineStart ) != 0xa )
          lineStart++;
  
        // Skip the LF
  
        if( lineStart < textBuffer->getSize() )
          lineStart++;
        else
          break;
      }
      else
      {
        // Skip the LF
  
        if( lineStart > 0 ) lineStart--;
  
        // Move backwards until we're at line start
  
        while( lineStart > 0 && textBuffer->getChar(reader, lineStart-1) != 0xa )
          lineStart--;
     }
  
      //  Break the loop if we have reached an endpoint
  
      if( lineStart == oldLineStart )
        break;
  
      linesDelta += delta;
    }
  }

  // A linestart shouldn't start at the textbuffer's end

  if( lineStart == textBuffer->getSize() )
  {
    while( lineStart > 0 && textBuffer->getChar(reader, lineStart-1) != 0xa )
      lineStart--;
  }

  if( linesDelta == 0 )
  {
    // We must force a move downwards (Even if linesDelta should be less than zero, 
    // the only time this var needs to be updates is downwards. (For the ViewWindow
    // to progress and find the end of the text.)
    viewLine++;
  }

	viewLine += linesDelta;

  return lineStart;
}

BOOL AsciiView::jumpToLine( int line )
{
  // Check that we're not outside limits
  int oldLine = currentLine;

  if( line > getTextlines() || line < 1 ) return FALSE;

  tempVirtualSpace = FALSE;

  int reader = textBuffer->readRequest();

  // Store the current column's pixel position

  preferredPixelCol = (preferredPixelCol != UNDEF) ? preferredPixelCol :
                       getCursorColumn();

  // Move to the new line

  int lines = line - currentLine, foo;
  currentLineStart = getNewLineStartFrom( reader, currentLineStart, lines, foo );
  currentLine = line;

  // Restore the column based on the last pixel position

  setCursorColumnKeepPreferred( preferredPixelCol );

  // Set the userPosition var

  adjustUserPosition();

  textBuffer->readRelease();
  
  performMoveEvent( oldLine, currentLine, currentLineStart, currentLineStart + currentLineLength );

  return TRUE;
}

void AsciiView::cursorToColumn( int column )
{
	setCursorColumnKeepPreferred( (column-1) * spaceWidth + indentedPixels );
}

void AsciiView::cursorToColumn( int column, BOOL tempVirtualSpace )
{
  this->tempVirtualSpace = tempVirtualSpace;
	cursorToColumn( column );
}

int AsciiView::getCursorWidth( int reader )
{
	if( Settings::insertMode )
		return 2;
	else
	{
		calcCharacterPositions( reader );

		if( currentColumn > currentLineLength )
			return 5;

		return currentLineCharPos[currentColumn].x -
					 currentLineCharPos[currentColumn-1].x;
	}
}

void AsciiView::setCursorColumnKeepPreferred( int pixelCol )
{
  calcCharacterPositions(FALSE);

  int x = indentedPixels;

  currentColumn = 1;

  int lastX = x;

  while( x < pixelCol )
  {
    lastX = x;
    if( currentColumn <= currentLineLength )
      x = currentLineCharPos[currentColumn].x + indentedPixels;
    else
      x += currentLineLastSpaceWidth;

    if( x <= pixelCol )
      currentColumn++;
  }

  if( x - pixelCol < pixelCol - lastX && x != pixelCol )
    currentColumn++;
  adjustUserPosition();
}

void AsciiView::getClosestStartingPoint( int toPosition, int &start, int &fromLine,
                                         BOOL useUserPosition )
{
  int delta, newLine = 1;

  delta = toPosition;

  start = 0;

  int nextDelta = abs( userPosition - toPosition );

  if( useUserPosition && nextDelta < delta )
  {
    delta = nextDelta;
    start = currentLineStart;
    newLine = currentLine;
  }

  nextDelta = abs( getTextSize() - toPosition );

  if( nextDelta < delta )
  {
    newLine = getTextlines();
    start = getTextSize();
  }
  fromLine = newLine;
}

BOOL AsciiView::jumpToPosition( int reader, int position )
{
  int oldPosition = userPosition;

  if( position < 0 || position > textBuffer->getSize() ) return FALSE;

  int lastLineStart = -1, lastLineEnd = -1, oldLine = currentLine;

  int start, end, delta;

  getClosestStartingPoint( position, userPosition, currentLine, TRUE );

  if( userPosition < position )
  {
    start = userPosition;
    end = position;
    delta = 1;
  }
  else
  {
    start = position;
    end = userPosition;
    delta = -1;
  }

  int lines = textBuffer->countLF( reader, start, end );

  userPosition = position;

  currentLine += lines * delta;

  if( oldPosition < currentLineStart ||
      oldPosition >= currentLineStart + currentLineLength ||
      currentLine != oldLine )
  {
    // Find the line start

    if( lastLineStart != -1 )
      currentLineStart = lastLineStart;
    else
    {
      currentLineStart = userPosition;

      while( currentLineStart > 0 &&
             textBuffer->getChar( reader, currentLineStart-1 ) != 0xa )
        currentLineStart--;
    }

    // Calc the line length

    if( lastLineEnd != -1 )
      currentLineLength = lastLineEnd;
    else
    {
      currentLineLength = userPosition;

      while( currentLineLength < textBuffer->getSize() &&
             textBuffer->getChar( reader, currentLineLength ) != 0xa )
        currentLineLength++;
      if( textBuffer->getChar( reader, currentLineLength-1 ) == 0xd )
        currentLineLength--;
      if( textBuffer->getChar( reader, currentLineLength-1 ) == 0xd )
        currentLineLength--;
    }

    currentLineLength -= currentLineStart;

    currentLineCharPosCalculated = -1;
  }
  tempVirtualSpace = FALSE;
  cursorBeyondEOL = FALSE;
  preferredPixelCol = UNDEF;
  currentColumn = userPosition - currentLineStart+1;

  performMoveEvent( oldLine, currentLine, currentLineStart, currentLineStart + currentLineLength );

  return TRUE;
}

int AsciiView::getLineFromPosition( int position )
{
  if( position < 0 || position > textBuffer->getSize() ) return 0;

  int start, end, delta, startPos, startLine;

  getClosestStartingPoint( position, startPos, startLine, TRUE );

  if( userPosition < position )
  {
    start = startPos;
    end = position;
    delta = 1;
  }
  else
  {
    start = position;
    end = startPos;
    delta = -1;
  }

	int reader = textBuffer->readRequest();
  int lines = textBuffer->countLF( reader, start, end );
  textBuffer->readRelease();

  return startLine + lines * delta;
}

BOOL AsciiView::indent( int writer, int &startPos, int fromColumn, int toColumn, BOOL inBlockUndo, BOOL startBlockUndo )
{
  int delta = toColumn - fromColumn;
	int start = 0;

  if( delta > 0 )
  {
 		if( !inBlockUndo && startBlockUndo )
			textBuffer->startUndoRepeat();

    TextElem *spaces = new TextElem[delta];

		if( Settings::optimalFill )
		{
			int lineLength = fromColumn - 1;
			while( lineLength + (Settings::tabSize - (lineLength + Settings::tabSize) % Settings::tabSize) < 
						 toColumn )
			{
				spaces[start++] = 0x9;
				lineLength += Settings::tabSize - (lineLength + Settings::tabSize) % Settings::tabSize;
			}
			delta = start + toColumn - lineLength - 1;
		}

    int i;
    for( i = start; i < delta; i++ ) spaces[i] = ' ';
    textBuffer->insertText( writer, startPos, spaces, i, TRUE, TRUE );
    startPos += i;
    delete spaces;
		return TRUE;
  }
	return FALSE;
}

void AsciiView::insertString( TextElem *string )
{
  BOOL blockUndo = FALSE;

  if( !Settings::persistentBlocks && validMark() && !keepBlock() )
  {
  	textBuffer->startUndoRepeat();
  	blockUndo = TRUE;
		deleteBlock();
  }

  int writer = textBuffer->writeRequest(), len = strlen( string );
	int startCol = getCursorColumnSpaces( writer, currentLineLength + 1), 
	    endCol = getCursorColumnSpaces( writer, currentColumn );

  blockUndo = 
	  (indent( writer, userPosition, startCol, endCol, blockUndo, TRUE )) ? 
	  TRUE : blockUndo;

	// Only overwrite with single chars, and not at line ends

	if( !Settings::insertMode && len == 1 && userPosition < currentLineStart + currentLineLength )
	{
		if( !blockUndo )
	  	textBuffer->startUndoRepeat();
		textBuffer->deleteText( writer, userPosition, 1, TRUE, TRUE );
		blockUndo = TRUE;
	}

  textBuffer->insertText( writer, userPosition, string, len, TRUE, TRUE );

  if( blockUndo )
  	textBuffer->endUndoRepeat();

  jumpToPosition( writer, userPosition + len );

  textBuffer->writeRelease();
}

void AsciiView::deleteCharacter()
{
  int reader = textBuffer->readRequest();
  int length;

  if( textBuffer->getChar( reader, userPosition ) == 0xd &&
                textBuffer->getChar( reader, userPosition+1 ) == 0xa )
    length = 2;
  else
  if( textBuffer->getChar( reader, userPosition ) == 0xd &&
          textBuffer->getChar( reader, userPosition+1 ) == 0xd  &&
          textBuffer->getChar( reader, userPosition+2 ) == 0xa )
    length = 3;
  else
    length = 1;


  textBuffer->readRelease();

  if( !Settings::persistentBlocks && validMark() && !keepBlock() )
    deleteBlock();
  else
  {
    int writer = textBuffer->writeRequest();
    textBuffer->deleteText( writer, userPosition, length, TRUE, TRUE );
    textBuffer->writeRelease();
  }
}

BOOL AsciiView::charRight()
{
  if( !Settings::virtualSpace && currentColumn > currentLineLength )
  {
    BOOL res = lineDown();
    if( res ) toLineStart();
    return res;
  }

  preferredPixelCol = UNDEF;
  tempVirtualSpace = FALSE;
  cursorBeyondEOL = FALSE;
  currentColumn++;
  adjustUserPosition();
  return TRUE;
}

BOOL AsciiView::charLeft()
{
  if( currentColumn == 1 ) 
  {
    if( !Settings::virtualSpace )
    {
      BOOL res = lineUp();
      if( res ) toLineEnd();
      return res;
    }
    else
      return FALSE;
  }

  preferredPixelCol = UNDEF;
  tempVirtualSpace = FALSE;
  cursorBeyondEOL = FALSE;
  currentColumn--;
  adjustUserPosition();
  return TRUE;
}

void AsciiView::toLineEnd()
{
  preferredPixelCol = UNDEF;
  tempVirtualSpace = FALSE;
  cursorBeyondEOL = FALSE;
  currentColumn = currentLineLength+1;
  adjustUserPosition();
}

void AsciiView::toLineStart()
{
  preferredPixelCol = UNDEF;
  tempVirtualSpace = FALSE;
  cursorBeyondEOL = FALSE;
  currentColumn = 1;
  adjustUserPosition();
}

void AsciiView::pushPos( BOOL visible )
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
  pushed[pushedPositions].column = currentColumn;
  pushed[pushedPositions].line = currentLine;
  pushed[pushedPositions].visible = visible;
  pushedPositions++;

	if( Settings::showLeftMarginInfo && visible && !isSimpleControl )
	{
		int reader = textBuffer->readRequest();
		vwHolder->updatePosition( this, reader, userPosition, userPosition );
		textBuffer->readRelease();
	}
}

BOOL AsciiView::popPosP( Position *pos )
{
  return popPosP( pos, FALSE );
}

void AsciiView::popPos( BOOL moveCursor )
{
	Position pos;
  if( !popPosP( &pos, FALSE ) ) return;

	if( Settings::showLeftMarginInfo && pos.visible && !isSimpleControl )
	{
		int reader = textBuffer->readRequest();
		vwHolder->updatePosition( this, reader, pos.position, pos.position );
		textBuffer->readRelease();
	}

	if( moveCursor )
	{
		int reader = textBuffer->readRequest();
		jumpToPosition( reader, pos.position );
		textBuffer->readRelease();

    if( pos.column != -1 )
    {
  		currentColumn = pos.column;
	  	currentLine = pos.line;
    }
	}

  if( pos.visible ) setLineMark( getLineStart(), getPosition() );
}

BOOL AsciiView::peekPos( Position *pos )
{
  return popPosP( pos, TRUE );
}

BOOL AsciiView::popPosP( Position *pos, BOOL peek )
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
    pos->column = pushed[pushedPositions].column;
    pos->line = pushed[pushedPositions].line;
    pos->visible  = pushed[pushedPositions].visible;

    if( peek )
      pushedPositions++;
  }
  return TRUE;
}

void AsciiView::setMark()
{
  int oldBegin = markBegin, oldEnd = markEnd;
  markBegin = markEnd = userPosition;
  markBeginColumn = markEndColumn = currentColumn;
  int reader = textBuffer->readRequest();
  vwHolder->updatePosition( this, reader, oldBegin, oldEnd );
  textBuffer->readRelease();

  if( (oldBegin == oldEnd && markBegin != markEnd) ||
      (oldBegin != oldEnd && markBegin == markEnd ) )
    markModified();
}

void AsciiView::markForward()
{
  int reader = textBuffer->readRequest(),
      oldMarkBegin = markBegin, oldMarkEnd = markEnd;

  Position pos;

  popPosP( &pos );

  if( pos.position != markBegin && pos.position != markEnd )
  {
    // Clear old mark
    markBegin = markEnd = pos.position;
    markBeginColumn = markEndColumn = getCursorColumnSpaces(reader, pos.column);

    if( oldMarkBegin != -1 )
      vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );
  }

  if( (pos.position == markBegin && userPosition >= markEnd) ||
      pos.position == markEnd )
  {
    if( pos.position != markEnd )  // Wrap
    {
      markBegin = markEnd;
      markBeginColumn = markEndColumn;
    }
    markEnd = userPosition;
    markEndColumn = getCursorColumnSpaces(reader, currentColumn);
  }
  else
  {
    markBegin = userPosition;
    markBeginColumn = getCursorColumnSpaces(reader, currentColumn);
  }

  int startUpdate = pos.position, endUpdate = userPosition;

  if( Settings::columnBlock && pos.column != currentColumn )
  {
    if( markBegin < startUpdate )
      startUpdate = markBegin;
    if( markEnd > endUpdate )
      endUpdate = markEnd;
  }

  vwHolder->updatePosition( this, reader, startUpdate, endUpdate );
  textBuffer->readRelease();

  if( (oldMarkBegin == oldMarkEnd && markBegin != markEnd) ||
      (oldMarkBegin != oldMarkEnd && markBegin == markEnd ) )
    markModified();
}

void AsciiView::markBackward()
{
  int reader = textBuffer->readRequest(),
      oldMarkBegin = markBegin, oldMarkEnd = markEnd;

  Position pos;

  popPosP( &pos );

  if( pos.position != markBegin && pos.position != markEnd )
  {
    // Clear old mark
    markBegin = markEnd = pos.position;
    markBeginColumn = markEndColumn = getCursorColumnSpaces(reader, pos.column);

    if( oldMarkBegin != -1 )
      vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );
  }

  if( (pos.position == markEnd && userPosition <= markBegin) ||
      pos.position == markBegin )
  {
    if( pos.position != markBegin )  // Wrap
    {
      markEnd = markBegin;
      markEndColumn = markBeginColumn;
    }
    markBegin = userPosition;
    markBeginColumn = getCursorColumnSpaces(reader, currentColumn);
  }
  else
  {
    markEnd = userPosition;
    markEndColumn = getCursorColumnSpaces(reader, currentColumn);
  }

  int startUpdate = userPosition, endUpdate = pos.position;

  if( Settings::columnBlock && pos.column != currentColumn )
  {
    if( markBegin < startUpdate )
      startUpdate = markBegin;
    if( markEnd > endUpdate )
      endUpdate = markEnd;
  }

  vwHolder->updatePosition( this, reader, startUpdate, endUpdate );
  textBuffer->readRelease();

  if( (oldMarkBegin == oldMarkEnd && markBegin != markEnd) ||
      (oldMarkBegin != oldMarkEnd && markBegin == markEnd ) )
    markModified();
}

void AsciiView::copyBlockToClipboard()
{
  if( !validMark() || 
      (Settings::columnBlock && markBeginColumn == markEndColumn)) return;

  TextElem *text = new TextElem[markEnd-markBegin];

  int reader = textBuffer->readRequest(), size;

  if( Settings::columnBlock )
  {
    size = 0;

    // Find linestart

    int i = markBegin;

    while( i > 0 && textBuffer->getChar( reader, i-1 ) != 0xa ) i--;

    // Get lines within columns

    int textSize = textBuffer->getSize();

    while( i < markEnd )
    {
      int column = 1;

      while( column < markBeginColumn && i < textSize )
      { 
        TextElem c = textBuffer->getChar( reader, i);
        if( c == 0xa ) break;
        column++;
        if( c == 0x9 )
          while( (column-1) % Settings::tabSize ) column++;
        i++; 
      }

      TextElem c;

      while( column < markEndColumn && i < textSize )
      {
        c = textBuffer->getChar( reader, i);
        if( c == 0xa ) break;
        column++;
        if( c == 0x9 )
          while( (column-1) % Settings::tabSize ) column++;
        text[size++] = c;
        i++;
      }

//      i++;

      // Remove CR if it got there

      if( size > 0 && ((c == 0xa && text[size-1] == 0xd) ||
                       (c == 0xd && textBuffer->getChar( reader, i) == 0xa)) )
        size--;

      if( Settings::CRLF )
        text[size++] = 0xd;

      text[size++] = 0xa;

      // Find the next LF

      while( i < textSize && textBuffer->getChar( reader, i-1 ) != 0xa ) i++;
    }
  }
  else
  {
    size = markEnd - markBegin;
    for( int i = markBegin, t = 0; i < markEnd; i++, t++ )
      text[t] = textBuffer->getChar( reader, i );
  }

  ClipBoard::setText( text, size );

  textBuffer->readRelease();
}

void AsciiView::deleteBlock()
{
  int writer = textBuffer->writeRequest();

  if( Settings::columnBlock )
  {
    textBuffer->startUndoRepeat();

    // Find linestart

    int i = markBegin;

    while( i > 0 && textBuffer->getChar( writer, i-1 ) != 0xa ) i--;

    // Get lines within columns

    while( i < markEnd )
    {
      int column = 1;

      int textSize = textBuffer->getSize(), lineStart = i;

      while( column < markBeginColumn && i < textSize )
      { 
        TextElem c = textBuffer->getChar( writer, i);
        if( c == 0xa ) break;
        column++;
        if( c == 0x9 )
          while( (column-1) % Settings::tabSize ) column++;
        i++; 
      }

      int startPos = i;

      while( column < markEndColumn && i < textSize )
      {
        TextElem c = textBuffer->getChar( writer, i);
        if( c == 0xa ) break;
        column++;
        if( c == 0x9 )
          while( (column-1) % Settings::tabSize ) column++;
        i++;
      }

      int endPos = i;

      // Remove CR if it got there

      if( i - startPos > 0 && ((textBuffer->getChar( writer, i-1) == 0xd &&
                                textBuffer->getChar( writer, i) == 0xa)) )
        endPos--;

      if( endPos - startPos > 0 )
        textBuffer->deleteText( writer, startPos, endPos - startPos, TRUE, TRUE );
      else
        vwHolder->addUpdatePosition( this, writer, lineStart, lineStart );

      i = startPos+1;

      // Find the next LF

      textSize = textBuffer->getSize();

      while( i < textSize && textBuffer->getChar( writer, i-1 ) != 0xa ) i++;
    }
    textBuffer->endUndoRepeat();
  }
  else
    textBuffer->deleteText( writer, markBegin, markEnd - markBegin, TRUE, TRUE );

  markEnd = markBegin;
  markEndColumn = markBeginColumn;

  textBuffer->writeRelease();

  markModified();
}

void AsciiView::cutBlockToClipboard()
{
  if( !validMark() || 
      (Settings::columnBlock && markBeginColumn == markEndColumn)) return;

  copyBlockToClipboard();

	deleteBlock();
}

BOOL AsciiView::pasteBlockFromClipboard()
{
  TextElem *text;

  int writer = textBuffer->writeRequest(), len = ClipBoard::getText( &text ),
      oldMarkBegin = markBegin, oldMarkEnd = markEnd, lastPos;

  if( len )
  {
    BOOL blockUndo = FALSE;

		int startCol = getCursorColumnSpaces( writer, currentLineLength + 1), 
			  endCol = getCursorColumnSpaces( writer, currentColumn );

	  blockUndo = 
		  (indent( writer, userPosition, startCol, endCol, blockUndo, TRUE )) ? 
		  TRUE : blockUndo;

	  if( !Settings::persistentBlocks && validMark() && !keepBlock() )
    {
  	  if( !blockUndo ) 
		    textBuffer->startUndoRepeat();
  	  blockUndo = TRUE;
  	  textBuffer->deleteText( writer, markBegin, markEnd-markBegin, TRUE, TRUE );
    }

    if( Settings::columnBlock )
    {
      int textPos = 0, insertPos = userPosition,
          textSize = textBuffer->getSize();

      markBeginColumn = markEndColumn = currentColumn;
      int column = userPosition - currentLineStart + 1;
      markBegin = markEnd = -1;

      // Update old block position
      vwHolder->updatePosition( this, writer, oldMarkBegin, oldMarkEnd );

      textBuffer->startUndoRepeat();

      while( textPos < len )
      {
        int insertLen, startPos = textPos;

        // Get the next line
        while( textPos < len && text[textPos] != 0xa ) textPos++;

        insertLen = textPos - startPos;

        if( textPos >= 1 && text[textPos-1] == 0xd ) insertLen--;

        textPos++;  // Move past LF

        // Insert linebreak if we're at end of text
        if( insertPos >= textSize )
        {
          TextElem crlf[] = {(TextElem)0xd, (TextElem)0xa};

          if( Settings::CRLF )
          {
            textBuffer->insertText( writer, insertPos, crlf, 2, TRUE, TRUE );
            textSize += 2;
          }
          else
          {
            textBuffer->insertText( writer, insertPos, &crlf[1], 1, TRUE, TRUE );
            textSize++;
          }
        }

        if( insertLen > 0 )
        {
          if( insertLen + markBeginColumn > markEndColumn )
            markEndColumn = insertLen + markBeginColumn;

          // insert the new line
          // Pad with spaces if necessary to get to the begin column
          if( column < markBeginColumn )
          {
            int len = markBeginColumn - column;
            TextElem *spaces = new TextElem[len];
            memset( spaces, 0x20, len );
            textBuffer->insertText( writer, insertPos, spaces, len, TRUE, TRUE );
            insertPos += len;
            textSize += len;
            delete spaces;
          }

          if( markBegin == -1 ) 
            markBegin = insertPos;
					else if( !Settings::persistentBlocks )
						markBegin = -1;

          textBuffer->insertText( writer, insertPos, &text[startPos],
                                  insertLen, TRUE, TRUE );
          textSize += insertLen;
          insertPos += insertLen;

					lastPos = insertPos;

          if( Settings::persistentBlocks )
						markEnd = insertPos;		
					else
						markEnd = -1;
        }
        else
        {
          // Update this line "manually"
          vwHolder->updatePosition( this, writer, insertPos, insertPos );
          insertPos++;
        }

        // Find the next linebreak
        while( insertPos < textSize &&
               textBuffer->getChar( writer, insertPos-1 ) != 0xa ) insertPos++;

        // Find the next column
        column = 1;

        while( column < markBeginColumn && insertPos < textSize &&
               textBuffer->getChar( writer, insertPos) != 0xa )
        {
          insertPos++;
          column++;
        }

        if( insertPos < textSize &&
            textBuffer->getChar( writer, insertPos) == 0xa &&
            textBuffer->getChar( writer, insertPos-1) == 0xd )
        {
          column--;
          insertPos--;
        }
      }

      textBuffer->endUndoRepeat();

      jumpToPosition( writer, lastPos );
    }
    else
    {
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
    }

    if( blockUndo )
  	  textBuffer->endUndoRepeat();
  }

  textBuffer->writeRelease();

  if( len )
    delete text;

  return len != 0;

  markModified();
}

void AsciiView::calcCharacterPositions( int reader, BOOL calcFullLength )
{
  requestScreenBuffer();
  if( currentLineCharPosCalculated != currentLineStart )
  {
    BOOL allocateReader = (reader == -1);

    if( allocateReader ) reader = textBuffer->readRequest();

    if( vwHolder != NULL )
    {
      // Recalc char starts using getTextlineInfo
      int foo = 0;
      int pos = currentLineStart;
      setDefaultFont( vwHolder->getViewGraphics(), SCREEN );
		  vwHolder->getViewGraphics()->getFontFromName( Settings::asciiFont, 0 );
      delete getTextlineInfo( reader, pos, foo, foo, foo,
                              vwHolder->getViewGraphics(), SCREEN,
                              calcFullLength ? -1 : vwHolder->getViewVidth() + vwHolder->getLeftMargin() );
    }
    else
    {
		  int col = 0;
      int i = currentLineStart;

      while( !atLineEnd( reader, i ) )
		  {
				col++;
        i++;
		  }
		  currentLineLength = col;
    }

    if( allocateReader ) textBuffer->readRelease();
  }
  releaseScreenBuffer();
}

BOOL AsciiView::atLineEnd( int reader, int pos )
{
  return pos >= textBuffer->getSize() ||
			   textBuffer->getChar( reader, pos ) == 0xa ||
			  (textBuffer->getChar( reader, pos ) == 0xd && 
				 textBuffer->getChar( reader, pos+1 ) == 0xa) ||
			  (textBuffer->getChar( reader, pos ) == 0xd && 
				 textBuffer->getChar( reader, pos+1 ) == 0xd && 
				 textBuffer->getChar( reader, pos+2 ) == 0xa);
}

int AsciiView::getLineEnd()
{
	int reader = textBuffer->readRequest();

  int i;
	for( i = getLineStart(); !atLineEnd( reader, i); i++ );

	textBuffer->readRelease();

	return i;
}

int AsciiView::getCursorColumn()
{
	if( vwHolder != NULL )
	{
		calcCharacterPositions(TRUE);

		if( currentColumn > currentLineLength )
			return currentLineCharPos[currentLineLength].x  +
						 (currentColumn - currentLineLength-1) * 
						  currentLineLastSpaceWidth + indentedPixels;

		return currentLineCharPos[currentColumn-1].x + indentedPixels;
	}
	else
		return currentColumn;
}

void AsciiView::getCurrentLine( char *text, int &length )
{
  if( currentLineLength < length )
    length = currentLineLength;

  int reader = textBuffer->readRequest();

  int i;
  for( i = 0; i < length; i++ )
    text[i] = textBuffer->getChar( reader, i + currentLineStart );

  text[i] = 0;

  textBuffer->readRelease();
}

int AsciiView::getPositionFromLine( int reader, int line )
{
  int lines = line - currentLine, foo;
  return getNewLineStartFrom( reader, currentLineStart, lines, foo );
}

void AsciiView::setBlockStart( int position )
{
  int reader = textBuffer->readRequest(),       
      oldMarkBegin = markBegin, oldMarkEnd = markEnd;

  pushPos( FALSE );
  jumpToPosition(reader, position );

  markBegin = position;
  markBeginColumn = getCursorColumnSpaces(reader, currentColumn);

  if( markBegin > markEnd )
  {
    markEnd = markBegin;
    markEndColumn = markBeginColumn;
  }

  if( oldMarkBegin != -1 )
    vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );

  vwHolder->updatePosition( this, reader, markBegin, markEnd );

  popPos(TRUE);  
  textBuffer->readRelease();

  if( oldMarkBegin != markBegin )
    markModified();
}

void AsciiView::setBlockEnd( int position )
{
  int reader = textBuffer->readRequest(),       
      oldMarkBegin = markBegin, oldMarkEnd = markEnd;

  pushPos( FALSE );
  jumpToPosition(reader, position );

  markEnd = position;
  markEndColumn = getCursorColumnSpaces(reader, currentColumn);

  if( markEnd < markBegin )
  {
    markBegin = markEnd;
    markBeginColumn = markEndColumn;
  }

  if( oldMarkBegin != -1 )
    vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );

  vwHolder->updatePosition( this, reader, markBegin, markEnd );

  popPos(TRUE);  
  textBuffer->readRelease();

  if( oldMarkEnd != markEnd )
    markModified();
}

int AsciiView::getCodeSections( CodeSection **section )
{
  int prevStart = 0;
  int sections = 0;
  *section = new CodeSection[multilines+1];
  
  for ( int i = 0; i < multilines; i++ )
  {
    (*section)[sections].startPos = prevStart;
    (*section)[sections].endPos = multiline[i].start;
    sections++;
    prevStart = multiline[i].end + multiline[i].endLen;
  }
  
  if( prevStart != textBuffer->getSize() )
  {
    (*section)[sections].startPos = prevStart;
    (*section)[sections].endPos = textBuffer->getSize();
    sections++;
  }
  
  return sections;
}

void AsciiView::performMoveEvent( int oldLine, int newLine, int lineStart, int lineEnd )
{
  if( vwHolder )
  { 
    if( vwHolder->getUpdate() )
      for( int i = 0; i < moveListener.getSize(); i++ )
      {
        ALineMoveEvent moveEvent( this, oldLine, newLine, lineStart, lineEnd );
        moveListener.getObjectAt(i)->actionPerformed( &moveEvent );
      }
  }
}

void AsciiView::requestScreenBuffer()
{
  lineInfoLock.request();
}

void AsciiView::releaseScreenBuffer()
{
 lineInfoLock.release();
}
