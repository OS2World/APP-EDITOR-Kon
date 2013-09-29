#include "bowl.h"
#include "buffer.h"
#include "klexer.h"

#if !defined SYNTAX

struct SyntaxElem
{
  ColorValue fg, bg;
  char *fontName;
  int fontAttr;
};

struct AWord : SyntaxElem
{
	int len;
	BOOL delimEnd;
  TextElem *string;
  AWord() { string = NULL; }
  ~AWord() { if( string ) delete string; }
};

struct Pair : SyntaxElem
{
  TextElem *openp, *closep;
  Pair() { openp = closep = NULL; }
  ~Pair()
  {
    if( openp ) delete openp;
    if( closep ) delete closep;
  }
};

struct SectionDef
{
  char *regexp;
  int useParen;
  SectionDef( char *regexp, int useParen )
  {
    this->regexp = new char[strlen(regexp)+1];
    strcpy( this->regexp, regexp );
    this->useParen = useParen;
  }
  ~SectionDef() { delete regexp; }
};

class Extension : public AObject
{
public:
  char *extension;
  Extension( char *extension )
  {
    this->extension = new char[strlen(extension)+1];
    strcpy( this->extension, extension );
  }
  ~Extension()
  {
    delete extension;
  }
};

#if defined __BORLANDC__
extern int wordCmp( const void *word1, const void *word2 );
#elif defined _MSC_VER
extern int wordCmp( const void *word1, const void *word2 );
#else
extern int _Optlink wordCmp( const void *word1, const void *word2 );
#endif

#if defined __BORLANDC__
extern int pairCmp( const void *pair1, const void *pair2 );
#elif defined _MSC_VER
extern int pairCmp( const void *pair1, const void *pair2 );
#else
extern int _Optlink pairCmp( const void *pair1, const void *pair2 );
#endif

class Syntax : public AObject
{
public:
	BOOL requiresDelim;
  TextElem *delimitor;
  TextElem literalChar;
  int delimitors;
  BOOL caseInsensitive;
  AWord *keyWord, *previousWord, *eolWord, *eolComment, *noDelim, *grep;
  int keyWords, noDelims, previousWords, eolWords, eolComments, longestEolComment;
  Pair *pair, *multilinePair, *multilineComment;
  int pairs, multilinePairs, multilineComments, longestMultiline, greps;
  char **extensionList;
  int extensions;
  SectionDef **sectionList;
  int sections;
  char *name;
  BOOL usesStandardDelim;

  Syntax()
  {
    usesStandardDelim = FALSE; 
    extensions = delimitors = keyWords = previousWords = eolWords =
                 eolComments = pairs = multilinePairs = multilineComments =
                 longestMultiline = longestEolComment = noDelims = greps = 
                 sections = 0;
    noDelim = keyWord = previousWord = eolWord = eolComment = grep = NULL;
    pair = multilinePair = multilineComment = NULL;
    extensionList = NULL;
    sectionList = NULL;
    delimitor = NULL;
    name = NULL;
		requiresDelim = TRUE;
  }

  BOOL wrapUp();
  ~Syntax();
};

class ColorObject : public AObject
{
public:
  ColorValue color;
  char *name;
  ColorObject( char *name, ColorValue color )
  {
    this->name = name;
    this->color = color;
  }
};

class SyntaxContainer
{
  static Vector<Syntax> syntaxes;
  static Vector<ColorObject> colors;
  static ColorValue currentFg, currentBg;
  static int currentFontAttr;
	static BOOL delimEnd;
  static void insertSyntax( Syntax *syntax );
public:
  static void displayError( char *error, int line )
  {
    char str[200];

    sprintf( str, error, line );
    PopupMessage popupMessage( Desktop::getInstance(), Strings::get(13),
                               str, PopupMessage::error );
  }
  static void displayError( char *error, char *str, int line )
  {
    char str2[200];

    sprintf( str2, error, str, line );
    PopupMessage popupMessage( Desktop::getInstance(), Strings::get(13),
                               str2, PopupMessage::error );
  }
  static BOOL retrieveList( TToken **token, int &items, char ***itemList );
  static BOOL getWordList( TToken **token, int &words, AWord **wordList );
  static BOOL getPairList( TToken **token, int &pairs, Pair **pairList );
  static BOOL getRGBvalue( TToken **token, ColorValue &colorValue );
  static void loadSyntaxes( char *filename );
  static Syntax *retrieveSyntax( char *filename );
  static Syntax *getSyntax( char *name );
  static int getSyntaxCount() { return syntaxes.getSize(); }
  static Syntax *getSyntaxAt( int pos ) { return syntaxes.getObjectAt( pos );}
};

#define SYNTAX

#endif
