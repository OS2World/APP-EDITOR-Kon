#include "buffer.h"

/*class Mess
{
public:
  static void msg( char *str );
};*/

struct Paren
{
  int start, end;
};

struct GrepSyntax
{
  GrepSyntax *next, *prev;
  GrepSyntax( GrepSyntax *prev )
  {
    this->prev = prev;
    next = NULL;
  }
  // Returns the rest of the unmatched string, -1 if match fail
  // if <= -2, then move forward (-retval-1) chars
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop ) { return pos; };
  virtual void printPattern() {}
  virtual void patternEnd( Paren *paren, int pos ) {}
  virtual void noreset() {}
  virtual ~GrepSyntax() { if( next ) delete next;}
};

struct GrepString : public GrepSyntax
{
  TextElem *matchString;
  int lastChar[256];
  int len;
  GrepString( GrepSyntax *prev, TextElem *string, int len );
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
  virtual void printPattern();
  virtual ~GrepString() { delete matchString; }
};

struct GrepZeroOrMore : public GrepSyntax
{
  GrepSyntax *testSyntax;
  GrepZeroOrMore( GrepSyntax *prev, GrepSyntax *testSyntax );
  BOOL resetOnNext;
  virtual void printPattern();
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
  virtual ~GrepZeroOrMore() { if( testSyntax ) delete testSyntax; }
};

struct GrepOneOrMore : public GrepZeroOrMore
{
  GrepOneOrMore( GrepSyntax *prev, GrepSyntax *testSyntax ) :
    GrepZeroOrMore( prev, testSyntax ) {}
  virtual void printPattern();
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
};

struct GrepParen : public GrepSyntax
{
  int parenNum;
  GrepSyntax *innerSyntax;
  BOOL resetNext;
  GrepParen( GrepSyntax *prev, GrepSyntax *innerSyntax, int parenNum );
  virtual void printPattern();
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
  virtual void patternEnd( Paren *paren, int pos );
  virtual void noreset() { resetNext = FALSE; }
  virtual ~GrepParen() { if( innerSyntax ) delete innerSyntax; }
};

struct GrepGroup : public GrepSyntax
{
  BOOL group[256];
  GrepGroup( GrepSyntax *prev, BOOL *group );
  virtual void printPattern();
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
};

struct GrepAnyChar : public GrepSyntax
{
  GrepAnyChar( GrepSyntax *prev ) : GrepSyntax(prev) {}
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
};

struct GrepChar : public GrepSyntax
{
	TextElem character;
  GrepChar( GrepSyntax *prev, TextElem character );
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
};

struct GrepOr : public GrepSyntax
{
  GrepOr( GrepSyntax *prev, GrepSyntax *next );
  virtual void printPattern();
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
  virtual ~GrepOr() { if( prev ) delete prev; }
};

struct GrepEOL : public GrepSyntax
{
  GrepEOL( GrepSyntax *prev ) : GrepSyntax(prev) {}
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
};


struct GrepBOL : public GrepSyntax
{
  GrepBOL( GrepSyntax *prev ) : GrepSyntax(prev) {}
  virtual int eval( Paren *paren, int reader, TextBuffer *buffer, 
  									TextElem *text, int pos, int stop );
};

class Grep
{
  static char *reserved;
  GrepSyntax *syntax;
  GrepSyntax *parse( TextElem *expr, int start, int end );
  Paren *paren;
  int parens;
  BOOL wholeWords, caseSensitive, wrapAround;
  char *delimitors;
	BOOL isCharCode( char *str );
public:
  Grep( char *expr, char *delimitors, BOOL caseSensitive, BOOL wholeWords, BOOL wrapAround );
  BOOL parseSuccess() { return syntax != NULL; }
  BOOL findForward( int reader, TextBuffer *buffer, TextElem *text, 
  										int &start, int &end );
  int getParens() { return parens; }
  int getParenStart( int parenNum ) { return paren[parenNum].start; }
  int getParenEnd( int parenNum ) { return paren[parenNum].end; }
  ~Grep();
};

