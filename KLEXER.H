#if !defined LEXER

#include "kerrcoll.h"

typedef enum { lexWord, lexConst, lexSymbol, lexId, lexSpace, lexAlpha, lexNum,
               lexSpecial, lexString, lexLiteral, lexCommentEOL } Tokentype;

struct TToken
{
  Tokentype type;
  int line, column;
  char *string;
  TToken *next;
  ~TToken()
  {
    if( string )
      delete string;
  }
};

#define DATASTREAM  1
#define FILESTREAM  2

/*** Stream class ***/

class TStream
{
  char *data;
  int line, column, lastCol, pos, size, type;
  FILE *f;
public:
  TStream( char *str, int type ) :
    line(1),
    column(1),
    lastCol(1),
    pos(0)
  {
    this->type = type;
    if( type == FILESTREAM )
    {
      f = fopen( str, "rb" );
      data = new char[strlen(str)+1];
      strcpy( data, str );
    }
    else
    {
      data = str;
      size = strlen( str );
    }
  };

  BOOL openSuccess() { return (type==FILESTREAM) ? (f!=NULL) : TRUE; }

  int eof( void )
  {
    if( type == FILESTREAM )
      return feof( f );
    else
      return pos >= size;
  };
  int getChar();
  void unGetChar( int c );
  int getLine() { return line; };
  int getColumn() { return column; };
  char *getFilename() { return (type==FILESTREAM) ? data : NULL; }
  ~TStream() { if( type == FILESTREAM ) {fclose(f); delete data;} };
};

/*** Lexer base class ***/

class TBLexer
{
  TToken *result;
  int usePreprocessor;
  Tokentype charIs( int );
  void deleteTokens();
protected:
  TStream *stream;
  char *symbols, *specials, **words;
  int symbolsCount, specialsCount, wordsCount;
  TErrorCollector *errorCollector;
public:
  TBLexer( TStream *stream, TErrorCollector *errorCollector, 
           int usePreprocessor )
  {
    result = NULL;
    this->usePreprocessor = usePreprocessor;
    this->stream = stream;
    this->errorCollector = errorCollector;
    setupTokens();
  };
  // If a #include "file" statement is encountered this method is called
  virtual void includeFile( char *file ) {}
  virtual void setupTokens() {}
  TToken *lexer( void );
  virtual ~TBLexer();
};

#define LEXER

#endif
