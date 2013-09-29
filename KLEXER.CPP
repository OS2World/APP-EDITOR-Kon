#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "klexer.h"

int TStream::getChar()
{
  int c;
  if( type == DATASTREAM )
  {
    if( pos >= size ) return 0;
    c = data[pos++];
  }
  else
    c = fgetc( f );

  lastCol = column;
  if( c == 0xa )
  {
    line++;
    column = 1;
  }
  else
  if( c == 0x9 )
  {
    while( (column-1) % 8 != 0 ) column++;
  }
  else
    column++;

  return c;
}

void TStream::unGetChar( int c )
{
  if( type == DATASTREAM )
  {
    if( pos <= 0 ) return;
    pos--;
//    data[--pos] = c;
  }
  else
    ungetc( c, f );

  if( c == 0xa )
  {
    line--;
    column = lastCol;
  }
  else
  if( c == 0x9 )
    column = lastCol;
  else
    column--;
}

void TBLexer::deleteTokens()
{
  delete symbols;
  delete specials;
  for( int i = 0; i < wordsCount; i++ )
    delete words[i];
  free( words );
}

Tokentype TBLexer::charIs( int c )
{
  if( c == '\"' || c == '\'') return lexString;

  if( c == '\\' ) return lexLiteral;

  if( strchr( symbols, c ) ) return lexSymbol;

  if( strchr( specials, c ) ) return lexSpecial;

  if( c == ' ' || c == 0xd || c == 0xa || c == 0x9 ) return lexSpace;

  if( c >= '0' && c <= '9' ) return lexNum;

  return lexAlpha;
}

TToken *TBLexer::lexer( void )
{
#define MAXLINELEN  4000
  char token[MAXLINELEN+1];
  int start, c;

  TToken **current;
  result = NULL;

  current = &result;

  while( 1 )
  {
    int eof = 0;
    // Skip blanks

    while( 1 )
    {
      if( stream->eof() )
      {
        eof = 1;
        break;
      }
      if( charIs(c = stream->getChar()) != lexSpace ) break;
    }

    if( eof || c == EOF ) break;

    if( c == '/' )
    {
      char cc = stream->getChar();
      if( cc == '/' )
      {
        // EOL comment

        while( (cc = stream->getChar()) != 0xa && !stream->eof() );
        continue;
      }
      else
        stream->unGetChar(cc);
//     continue;
    }

    if( c == '#' && usePreprocessor )
    {
      char *teststr = "include";
      int i;
      for( i = 0; i < strlen( teststr ); i++ )
        if( teststr[i] != stream->getChar() ) break;

      if( i != strlen( teststr ) )
      {
        while( stream->getChar() != 0xa && !stream->eof() );
        errorCollector->Error( stream->getColumn(), stream->getLine(),
                               "Preprocessor error" );
      }

      while( charIs(c = stream->getChar()) == lexSpace && !stream->eof());

      if( c == '"' )
      {
        char file[255];
        i = 0;

        while( i < 255 && !stream->eof() )
        {
          file[i++] = (c=stream->getChar());
          if( c == '"' ) break;
        }
        if( c == '"' )
        {
          file[i-1] = 0;
          includeFile( file );
        }
        else
          errorCollector->Error( stream->getColumn(), stream->getLine(),
                                 "quote missing in preprocessor" );
      }
      else
      {
        errorCollector->Error( stream->getColumn(), stream->getLine(),
                               "quote missing in preprocessor" );
      }
      continue;
    }

    (*current) = new TToken;
    (*current)->next = NULL;
    (*current)->string = NULL;
    (*current)->line = stream->getLine();
    (*current)->column = stream->getColumn();

    start = 0;

    if( charIs(c) == lexString )
    {
      int done = 0, firstC = c, type = charIs(c);

      do
      {
        c = stream->getChar();

        switch( charIs(c) )
        {
          case lexLiteral :
          {
            c = stream->getChar();
            switch( c )
            {
              case 'n' : c = 0xa; break;
              case 'r' : c = 0xd; break;
              case 'l' : c = 0xa; break;
              case 't' : c = 0x9; break;
            }
            break;
          }
          case lexString  : if( c == firstC ) done = 1; break;
        }
        if( !done )
          token[start++] = c;
      } while( !done && !stream->eof());
      (*current)->type = lexString;
    }
    else
    if( charIs(c) == lexSpecial )
    {
      token[start++] = c;
      (*current)->type = lexSymbol;
    }
    else
    if( charIs(c) == lexSymbol )
    {
      do
      {
        token[start++] = c;
        c = stream->getChar();
        if( charIs(c) != lexSymbol )
        {
          stream->unGetChar(c);
          break;
        }
      } while( !stream->eof() );
      (*current)->type = lexSymbol;
    }
    else
    if( charIs(c) == lexNum )
    {
      do
      {
        token[start++] = c;

        if( stream->eof() ) break;

        c = stream->getChar();

        if( charIs( c ) != lexAlpha && charIs( c ) != lexNum )
        {
          stream->unGetChar(c);
          break;
        }
      } while( 1 );

      (*current)->type = lexNum;
    }
    else
    if( charIs(c) == lexAlpha || charIs(c) == lexLiteral )
    {
      do
      {
        token[start++] = c;

        if( stream->eof() ) break;

        c = stream->getChar();

        if( charIs( c ) != lexAlpha && charIs( c ) != lexNum &&
            charIs( c ) != lexLiteral  )
        {
          stream->unGetChar(c);
          break;
        }
      } while( 1 );

      token[start] = 0;

      int i;
      for( i = 0; i < wordsCount; i++ )
        if( !strcmp( words[i], token ) ) break;

      if( i < wordsCount )
        (*current)->type = lexWord;
      else
        (*current)->type = lexId;
    }

    token[start] = 0;

    (*current)->string = new char[strlen( token ) + 1];

    strcpy( (*current)->string, token );

    current = &(*current)->next;
  }
  return result;
}


TBLexer::~TBLexer()
{
  // Cascading delete is a bad idea, eats too much stack space here

  while( result )
  {
    TToken *next = result->next;
    delete result;
    result = next;
  }
  deleteTokens();
}
