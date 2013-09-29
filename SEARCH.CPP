#include <ctype.h>
#include "bowl.h"
#include "search.h"

Search::Search( TextBuffer *textBuffer, int reader, TextElem *text,
                int textLen, TextElem *delimitors, BOOL caseSensitive,
                BOOL wholeWords, BOOL wrapAround ) :
                textBuffer(textBuffer), reader(reader), text(text),
                delimitors(delimitors), textLen(textLen),
                caseSensitive(caseSensitive), wholeWords(wholeWords),
                wrapAround(wrapAround)
{
  if( !caseSensitive )
    strupr( text );

  // Build the last char list for both backwards and forward search

  int i;
	for( i = 0; i < 256; i++ )
		lastChar[i] = firstChar[i] = -1;

  // The last occurence of each char is set into the array

  for( i = textLen - 1; i >= 0; i-- )
  {
      TextElem c = text[i];
      if( lastChar[(unsigned char)c] == -1 ) lastChar[(unsigned char)c] = i + 1;
  }

  // First occurence

  for( i = 0; i < textLen; i++ )
  {
    TextElem c = text[i];
    if( firstChar[(unsigned char)c] == -1 ) firstChar[(unsigned char)c] = i;
  }
}

BOOL Search::findForward( int &position, int &len, int start, int end )
{
  position += textLen - 1;
  BOOL firstLoop = TRUE;

  do
  {
    if( position < start || position >= end ) 
    {
			if( !wrapAround || firstLoop == FALSE )
				break;
				
			if( position >= end ) 
				position = start;
			else
				break;
				
			firstLoop = FALSE;
		}

    int strPos = 0;

    TextElem *sText = text + textLen - 1;
    TextElem c;

    do
    {
      c = (caseSensitive) ? textBuffer->getChar( reader, position - strPos ) :
                                toupper( textBuffer->
                                         getChar( reader, position - strPos ) );

      if( *sText != c ) break;  // Mismatch

      sText--;
      strPos++;
    } while( strPos < textLen );

    if( strPos == textLen ) // String was found
    {
      // Do whole word check if needed

      if( wholeWords )
      {
        BOOL atStart = (position-textLen > 0) ?
                         (BOOL)strchr( delimitors,
                           textBuffer->getChar( reader, position-textLen ) ) :
                           TRUE;

        BOOL atEnd = (position + 1 < textBuffer->getSize()) ?
                         (BOOL)strchr( delimitors,
                           textBuffer->getChar( reader, position+1 ) ) :
                           TRUE;

        if( !atEnd || !atStart ) { position++; continue; }
      }

      len = textLen;
      position++;
      return TRUE;
    }
    else
    {
			int pos = lastChar[(unsigned char)c], add;

			if( pos == -1 ) // Doesn't occur
				add = textLen - strPos;
			else	// Match char in buffer with char in string
				add = textLen - pos - strPos;

			if( add <= 0 ) add = 1;

			position += add;
    }
  } while( 1 );

  return FALSE;
}

BOOL Search::findBackward( int &position, int &len, int start, int end )
{
  position -= textLen;
  BOOL firstLoop = FALSE;

  do
  {
    if( position < start || position >= end )
    {
			if( !wrapAround || firstLoop == FALSE )
				break;
				
			if( position < start ) 
				position = end;
			else
				break;
				
			firstLoop = FALSE;
		}

    int strPos = 0;

    TextElem *sText = text;
    int c;

    do
    {
      c = (caseSensitive) ? textBuffer->getChar( reader, position + strPos ) :
                                toupper( textBuffer->
                                         getChar( reader, position + strPos ) );

      if( *sText != c ) break;  // Mismatch

      sText++;
      strPos++;
    } while( strPos < textLen );

    if( strPos == textLen ) // String was found
    {
      // Do whole word check if needed

      if( wholeWords )
      {
        BOOL atStart = (position-1 >= 0) ?
                         (BOOL)strchr( delimitors,
                           textBuffer->getChar( reader, position-1 ) ) :
                           TRUE;

        BOOL atEnd = (position + textLen < textBuffer->getSize()) ?
                         (BOOL)strchr( delimitors,
                           textBuffer->getChar( reader, position+textLen ) ) :
                           TRUE;

        if( !atEnd || !atStart ) { position--; continue; }
      }

      len = textLen;
      return TRUE;
    }
    else
		{
			// Check if the character which doesn't match occur in the string
			int pos = firstChar[c], add;

			if( pos == -1 ) // Doesn't occur
				add = textLen - strPos;
			else	// Match char in buffer with char in string
				add = pos - strPos;

			if( add <= 0 ) add = 1;

			position -= add;
		}
  } while( 1 );

  return FALSE;
}
