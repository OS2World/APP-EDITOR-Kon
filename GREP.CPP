#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "bowl.h"
#include "grep.h"

// Identify C-function: ^[^:s#\x0d\x0a/@][^\x0d\x0a]*\([^\)]*\)$
// Ultra-edit fails if the function does not start at line start! for example
// typing "if(..)" at column 1 identifies if as a function!
// Correct in Kon (identifying C++ function name): ^[:m_]+:s[:m_]+\:\:([:m_]+)\([^)]*\)$
// DO NOT SEARCH IN COMMENTS!!!

char *Grep::reserved = "[]()*.+*^$|";

GrepString::GrepString( GrepSyntax *prev, TextElem *string, int len ) :
  GrepSyntax( prev )
{
  matchString = new TextElem[len + 1];
  int i, p;
  for( i = 0, p = 0; i < len; i++ )
  {
    if( string[i] == '\\' ) i++;
    matchString[p++] = string[i];
  }
  matchString[p] = 0;
  this->len = p;

  // Use Boyer-Moore search for strings

	for( i = 0; i < 256; i++ )
		lastChar[i] = -1;

  for( i = this->len - 1; i >= 0; i-- )
  {
    unsigned char c = (unsigned int)matchString[i];
    if( lastChar[c] == -1 ) lastChar[c] = i + 1;
  }
}

int GrepString::eval( Paren *paren, int reader, TextBuffer *buffer, 
  										TextElem *text, int pos, int stop )
{
  int newPos = -1;

  if( pos + len > stop ) return -1;

  pos += len - 1;

  TextElem c, strPos = 0;
  TextElem *sText = matchString + len - 1;

  do
  {
  	c = (text) ? text[pos - strPos] : buffer->getChar( reader, pos-strPos );

    if( *sText != c ) break;  // Mismatch

    sText--;
    strPos++;
  } while( strPos < len );

  if( strPos == len ) // String was found
    newPos = pos + 1;
  else
  {
    int pos = lastChar[(unsigned char)c], add;

    if( pos == -1 ) // Doesn't occur
      add = len - strPos;
    else	// Match char in buffer with char in string
      add = len - pos - strPos;

    if( add <= 0 ) add = 1;

    return -add - 1;
  }

  if( next )
    return next->eval( paren, reader, buffer, text, newPos, stop );
  else
    return newPos;
}

void GrepString::printPattern()
{
  printf( "GrepString: '%s'\n", matchString );
  if( next )
    next->printPattern();
}

GrepZeroOrMore::GrepZeroOrMore( GrepSyntax *prev, GrepSyntax *testSyntax ) :
  GrepSyntax( prev ), resetOnNext(TRUE)
{
  this->testSyntax = testSyntax;
}

void GrepZeroOrMore::printPattern()
{
  printf( "GrepZeroOrMore: {\n" );
  testSyntax->printPattern();
  printf( "}\n" );
  if( next )
    next->printPattern();
}

// aaaaabcd
int GrepZeroOrMore::eval( Paren *paren, int reader, TextBuffer *buffer, 
  												TextElem *text, int pos, int stop )
{
  // Find as long string as possible
  int currentPos = pos, longestPos, lastWasFound = -1, skip = 0, lastPosWhereFound = 0;
  BOOL firstLoop = TRUE;
  
  if( !resetOnNext ) testSyntax->noreset();
  resetOnNext = TRUE;
  
  while( currentPos < stop )
  {
    if( skip <= 0 )
    {
      if( next != NULL )
        longestPos = next->eval( paren, reader, buffer, text, currentPos, stop );
      else
        longestPos = -1;

      if( longestPos >= 0 ) 
      { 
        lastWasFound = longestPos;
        lastPosWhereFound = currentPos;
      }

      if( longestPos <= -2 )
        skip = -longestPos - 1;
    }

    skip--;

    if( !firstLoop ) testSyntax->noreset();

    currentPos = testSyntax->eval( paren, reader, buffer, text, currentPos, stop );

    if( currentPos < 0 )
    {
      if( next == NULL && lastWasFound == -1 && firstLoop )
        return pos;
      if( next != NULL )  
        testSyntax->patternEnd( paren, lastPosWhereFound );  
      return lastWasFound;
    }  
    else
    if( next == NULL )
      lastWasFound = currentPos;

    firstLoop = FALSE;      
  }

  if( next == NULL && lastWasFound >= 0 )
    return currentPos;

  testSyntax->patternEnd( paren, lastPosWhereFound );  
  return lastWasFound;
}

void GrepOneOrMore::printPattern()
{
  printf( "GrepOneOrMore: {\n" );
  testSyntax->printPattern();
  printf( "}\n" );
  if( next )
    next->printPattern();
}

int GrepOneOrMore::eval( Paren *paren, int reader, TextBuffer *buffer, 
  											 TextElem *text, int pos, int stop )
{
  int nextPos = testSyntax->eval( paren, reader, buffer, text, pos, stop );

  if( nextPos < 0 )
    return nextPos;
  
  resetOnNext = FALSE;
  return GrepZeroOrMore::eval( paren, reader, buffer, text, nextPos, stop );
}

GrepParen::GrepParen( GrepSyntax *prev, GrepSyntax *innerSyntax,
                      int parenNum ) :
  GrepSyntax( prev ), resetNext( TRUE )
{
  this->innerSyntax = innerSyntax;
  this->parenNum = parenNum;
}

void GrepParen::printPattern()
{
  printf( "GrepParen: {\n" );
  innerSyntax->printPattern();
  printf( "}\n" );
  if( next )
    next->printPattern();
}

int GrepParen::eval( Paren *paren, int reader, TextBuffer *buffer, 
  									 TextElem *text, int pos, int stop )
{
  int newPos = innerSyntax->eval( paren, reader, buffer, text, pos, stop );

  if( newPos < 0 ) { resetNext = TRUE; return -1; }
  
  if( resetNext )
    paren[parenNum].start = paren[parenNum].end = -1;

  resetNext = TRUE;
    
  // Build up paren. Then we can handle construction of type (.)*
  if( paren[parenNum].start == -1 )
    paren[parenNum].start = pos;
    
  paren[parenNum].end = newPos;

  if( next )
    return next->eval( paren, reader, buffer, text, newPos, stop );

  return newPos;
}

void GrepParen::patternEnd( Paren *paren, int pos )
{
  paren[parenNum].end = pos;
}

GrepGroup::GrepGroup( GrepSyntax *prev, BOOL *group ) :
  GrepSyntax( prev )
{
  memcpy( this->group, group, sizeof( BOOL ) * 256 );
}

void GrepGroup::printPattern()
{
  printf( "GrepGroup: \n" );
  if( next )
    next->printPattern();
}

int GrepGroup::eval( Paren *paren, int reader, TextBuffer *buffer, 
  									 TextElem *text, int pos, int stop )
{
  if( !group[(unsigned char)((text) ? text[pos] : buffer->getChar( reader, pos ))] ||
  		pos >= stop )
    return -1;

  if( next )
    return next->eval( paren, reader, buffer, text, pos+1, stop );
  else
    return pos + 1;
}

int GrepAnyChar::eval( Paren *paren, int reader, TextBuffer *buffer, 
  									   TextElem *text, int pos, int stop )
{
  if( pos >= stop ) return -1;

  if( next )
    return next->eval( paren, reader, buffer, text, pos+1, stop );
  else
    return pos + 1;
}

GrepChar::GrepChar( GrepSyntax *prev, TextElem character ) :
  GrepSyntax( prev )
{
	this->character = character;
}

int GrepChar::eval( Paren *paren, int reader, TextBuffer *buffer, 
  										TextElem *text, int pos, int stop )
{
  if( pos + 1 > stop ) return -1;

	TextElem c = (text) ? text[pos] : buffer->getChar( reader, pos );

	if( c == character ) 
		pos++;
	else
		return -1;

  if( next )
    return next->eval( paren, reader, buffer, text, pos, stop );
  else
    return pos;
}

GrepOr::GrepOr( GrepSyntax *prev, GrepSyntax *next ) : GrepSyntax(prev)
{
  this->next = next;
}

void GrepOr::printPattern()
{
  printf( "GrepOr: {\n" );
  if( prev )
    prev->printPattern();
  printf( "|||\n" );
  if( next )
    next->printPattern();
  printf( "}\n" );
}

int GrepOr::eval( Paren *paren, int reader, TextBuffer *buffer, 
  								TextElem *text, int pos, int stop )
{
  int newPos1 = -1, newPos2 = -1;

  if( prev )
    newPos1 = prev->eval( paren, reader, buffer, text, pos, stop );

  if( newPos1 >= 0 ) return newPos1;

  if( next )
    newPos2 = next->eval( paren, reader, buffer, text, pos, stop );

  // Return newPos2 if found, otherwise move forward the shortest distance

  return (newPos1 < newPos2) ? newPos2 : newPos1;
}

int GrepBOL::eval( Paren *paren, int reader, TextBuffer *buffer, 
  									   TextElem *text, int pos, int stop )
{
  if( pos >= stop ) return -1;

  if( text )
  {
  	if( pos > 0 && text[pos-1] != 0xa ) return -1;
  }
  else
  	if( pos > 0 && buffer->getChar(reader, pos-1) != 0xa ) return -1;

  if( next )
    return next->eval( paren, reader, buffer, text, pos, stop );
  else
    return pos;
}

int GrepEOL::eval( Paren *paren, int reader, TextBuffer *buffer, 
  									   TextElem *text, int pos, int stop )
{
	int EOLlen = 0;

  if( pos >= stop ) return -1;

  if( text )
  {
  	if( pos < stop )
    {
    	if( text[pos] == 0xa )
      	EOLlen = 1;
      else
    	if( text[pos] == 0xd && text[pos+1] == 0xa )
      	EOLlen = 2;
      else
      	return -1;
    }
  }
  else
  {
  	if( buffer->getChar(reader, pos) == 0xa )
    	EOLlen = 1;
    else
  	if( buffer->getChar(reader, pos) == 0xd && 
    		buffer->getChar(reader, pos+1) == 0xa )
    	EOLlen = 2;
    else
    	return -1;
  }

  if( next )
    return next->eval( paren, reader, buffer, text, pos+EOLlen, stop );
  else
    return pos+EOLlen;
}

Grep::Grep( char *expr, char *delimitors, BOOL caseSensitive, 
            BOOL wholeWords, BOOL wrapAround ) : parens(0)
{
  syntax = parse( expr, 0, strlen( expr ) );
  paren = new Paren[parens];
  this->wholeWords = wholeWords;
  this->caseSensitive = caseSensitive;
  this->delimitors = delimitors;
  this->wrapAround = wrapAround;
};

Grep::~Grep()
{
  if( paren ) delete paren;
  if( syntax ) delete syntax;
}

BOOL Grep::isCharCode( char *str )
{
	return *str == 'x' || (*str >= '0' && *str <= '9' && 
                         *(str+1) >= '0' && *(str+1) <= '9' &&
                         *(str+2) >= '0' && *(str+2) <= '9');
}

GrepSyntax *Grep::parse( TextElem *expr, int start, int stop )
{
  GrepSyntax *current = NULL, *first = NULL, *newSyn;

  for( int i = start; i < stop; )
  {
    switch( expr[i] )
    {
      case '*' :
      {
        if( current == NULL ) return NULL;
        newSyn = new GrepZeroOrMore( current->prev, current );
        current = current->prev;
        i++;
        break;
      }
      case '+' :
      {
        if( current == NULL ) return NULL;
        newSyn = new GrepOneOrMore( current->prev, current );
        current = current->prev;
        i++;
        break;
      }
      case '.' :
      {
        newSyn = new GrepAnyChar( current );
        i++;
        break;
      }
      case '|' :
      {
        GrepSyntax *nextSyntax =
          parse( expr, i+1, stop );

        if( nextSyntax )
          return new GrepOr( first, nextSyntax );
        else
        if( first != NULL ) 
          delete first;

        return NULL;
      }
      case ']' :
      case ')' :
      {
        if( first )
          delete first;
        return NULL;
      }
      case ':' :
      case '[' :
      {
        BOOL group[256], flag = TRUE, shortcut = FALSE;
        int tempStop = stop;
        
        if( expr[i] == ':' )
        {
          tempStop = i+2;
          shortcut = TRUE;
        }  
        else
          i++;

        if( expr[i] == '^' )
        {
          i++;
          flag = FALSE;
        }

        if( flag )
          memset( group, 0, 256 * sizeof( BOOL ) );
        else
          for( int f = 0; f < 256; f++ ) group[f] = 1;

        while( i < tempStop && expr[i] != ']' )
        {
          if( expr[i] == '\\' ) 
					{
            if( isCharCode( &expr[i+1] ) )
            {
              unsigned int character;
              if( expr[i+1] == 'x' ) // Hex
              {
                if( expr[i+3] <= '9' )
                  character = expr[i+3]-'0';
                else
                  character = toupper(expr[i+3])-'A'+10;

                if( expr[i+2] <= '9' )
                  character += (expr[i+2]-'0') * 16;
                else
                  character += (toupper(expr[i+2])-'A'+10)*16;
              }
              else
              {
                character = expr[i+3]-'0';
                character += (expr[i+2]-'0')*10;
                character += (expr[i+1]-'0')*100;
              }
              i += 4;
  	          group[character] = flag;
            }
            else
            {
  	          group[(unsigned char)expr[++i]] = flag;
	  					i++;
            }
						continue;
					}

          if( i >= tempStop ) break;

					if( expr[i] == ':' )  // Shortcut
					{
						++i;
						switch( expr[i++] )
						{
							case 'a' :
							{
								int a;
								for( a = 'a'; a <= 'z'; a++ ) group[a] = flag;
								for( a = 'A'; a <= 'Z'; a++ ) group[a] = flag;
								break;
							}
							case 'u' :
							{
								int a;
								for( a = 'A'; a <= 'Z'; a++ ) group[a] = flag;
								break;
							}
							case 'l' :
							{
								int a;
								for( a = 'a'; a <= 'z'; a++ ) group[a] = flag;
								break;
							}
							case 'n' :
							{
								int a;
								for( a = '0'; a <= '9'; a++ ) group[a] = flag;
								break;
							}
              case 'i' :
              {
                for( int a = 128; a < 256; a++ ) group[a] = flag;
              }
							case 'm' :
							{
								int a;
								for( a = 'a'; a <= 'z'; a++ ) group[a] = flag;
								for( a = 'A'; a <= 'Z'; a++ ) group[a] = flag;
								for( a = '0'; a <= '9'; a++ ) group[a] = flag;
								break;
							}
							case 's' :
							{
								group[32] = flag;
								group[9] = flag;
								break;
							}
						}
					}
					else
          if( i+1 < tempStop && expr[i] == '-' && i+1 < tempStop )  // range
          {
            i++;
            for( int n = expr[i-2]; n < expr[i]; n++ )
              group[n] = flag;
          }
					else
	          group[(unsigned char)expr[i++]] = flag;
        }

        if( expr[i] == ']' || shortcut )
        {
          newSyn = new GrepGroup( current, group );
          if( !shortcut ) i++;
        }
        else
        {
          if( first != NULL ) delete first;

          return NULL;
        }
        break;
      }
      case '(' :
      {
        int thisParen = parens;
        parens++;
        // Find end paren
        int match = 1, start = i;
        i++;

        while( i < stop && match > 0 )
        {
          if( expr[i] == '\\' ) i++;
          if( expr[i] == '(' ) match++;
          if( expr[i] == ')' ) match--;
          i++;
        }

        if( match == 0 )
        {
          GrepSyntax *innerSyntax =
            parse( expr, start + 1, i-1 );
          if( innerSyntax )
            newSyn = new GrepParen( current, innerSyntax, thisParen );
          else
          {
            if( first != NULL ) delete first;
            return NULL;
          }
        }
        else
        {
          if( first != NULL ) delete first;
          return NULL;
        }
        break;
      }
      case '^' :
      {
        newSyn = new GrepBOL( current );
        i++;
        break;
      }
      case '$' :
      {
        newSyn = new GrepEOL( current );
        i++;
        break;
      }
			case '\\' :
			{
				// Check that this is a decimal or hexadecimal code
				// Stricter check for hex may be a good idea later on...

				if( isCharCode( &expr[i+1] ) )
				{
					TextElem character;
					if( expr[i+1] == 'x' ) // Hex
					{
						if( expr[i+3] <= '9' )
							character = expr[i+3]-'0';
						else
							character = toupper(expr[i+3])-'A'+10;

						if( expr[i+2] <= '9' )
							character += (expr[i+2]-'0') * 16;
						else
							character += (toupper(expr[i+2])-'A'+10)*16;
					}
					else
					{
						character = expr[i+3]-'0';
						character += (expr[i+2]-'0')*10;
						character += (expr[i+1]-'0')*100;
					}
					i += 4;
					newSyn = new GrepChar( current, character );
					break;
				}
				// else continue as string
			}
      default :
      {
        int start = i, stringLength = 0;
        while( i < stop )
        {
          if( expr[i] == '\\' )
					{
						if( isCharCode( &expr[i+1] ) ) break;
            i++;
					}
          else
          {
            if( strchr( reserved, expr[i] ) != NULL ) break;
            stringLength++;
          }
          i++;
        }

        if( i != stop && stringLength > 1 &&
            (expr[i] == '*' || expr[i] == '+') )
        {
          // Must make a single char last for * or + to work

          newSyn = new GrepString( current, &expr[start], i - start - 1 );

          if( current )
            current->next = newSyn;
          else
            first = newSyn;

          current = newSyn;

          newSyn = new GrepString( current, &expr[i-1], 1 );
        }
        else
          newSyn = new GrepString( current, &expr[start], i - start );
      }
    }

    if( newSyn == NULL ) 
    {
      if( first != NULL ) delete first;
      return NULL;
    }

    if( current )
      current->next = newSyn;
    else
      first = newSyn;

    current = newSyn;
  }
  return first;
}

BOOL Grep::findForward( int reader, TextBuffer *buffer, char *text, 
                        int &start, int &end )
{
  if( syntax == NULL ) return FALSE;

  while( start < end )
  {
    for( int i = 0; i < parens; i++ )
      paren[i].start = paren[i].end = -1;

    int foundEnd = syntax->eval( paren, reader, buffer, text, start, end );

    if( foundEnd >= 0 )
    {
      if( wholeWords )
      {
        BOOL atStart = (start > 0) ?
                         (BOOL)strchr( delimitors,
                           (text) ? text[start-1] : 
                                    buffer->getChar( reader, start-1 ) ) :
                           TRUE;

        BOOL atEnd = (foundEnd < end) ?
                         (BOOL)strchr( delimitors,
                           (text) ? text[foundEnd] :
                                    buffer->getChar( reader, foundEnd ) ) :
                           TRUE;

        
        if( !atStart || !atEnd )
        {
          ++start;
          continue;
        }
      }
      end = foundEnd;
      return TRUE;
    }
    else
    if( foundEnd <= -2 )
      start += -foundEnd - 1;
    else
      ++start;
  }
  return FALSE;
}

/*void Mess::msg( char *str )
{
  FILE *f = fopen( "grep", "a+" );
  fprintf( f, "%s\n", str );
  fclose( f );
}*/