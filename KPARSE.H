//#define DEBUG

#if !defined KPARSE

typedef enum { SNoOp, SFor, SDo, SIf, SIfElse, SBinOp, SUnOp, SNumeric, SString,
               SIdentifier, SStmts, SExprns, SBreak, SContinue,
               SSubscript, SFunction, SArray, SReturn, SChar,
               SSwitch, SOverload } Statement;

struct TSyntax;

#if !defined K_PARSER
extern int Objects;
#endif

#include "kerr.h"

TSyntax *GetCopy( TSyntax *Expr );

void Kill( TSyntax **Obj );
void CopySyntax( TSyntax **Dest, TSyntax *Src );

struct TSyntax
{
  Statement Type;

  TSyntax( void )
  {
    Objects++;
#if defined DEBUG
    printf( "{+%d}", Objects );
#endif
  };
  ~TSyntax()
  {
    Objects--;
#if defined DEBUG
    printf( "{-%d} ", Objects );
#endif
  }
};

TSyntax **AddToList( TSyntax *List[], int *Size, TSyntax *Item, int Pos );

#define COPY      1
#define ORIGINAL  0

struct TConst : TSyntax
{
  int isCopy;
  TConst( int isCopy ) : TSyntax() { this->isCopy = isCopy; };
  ~TConst() {};
};

struct TNumeric : TConst
{
  int num;
  TNumeric( int num )  : TConst( ORIGINAL )
  {
    init( num );
  };
  TNumeric( int num, int isCopy )  : TConst( isCopy )
  {
    init( num );
  };

  void init( int num )
  {
#if defined DEBUG
    printf( "Create Numeric" );
#endif
    this->num = num;
    Type = SNumeric;
  };
  ~TNumeric()
  {
#if defined DEBUG
    printf( "Delete Numeric " );
#endif
  };
};

struct TString : TConst
{
  char *string;
  int CharacterPosition;  // This is != -1 if referring to a single character

  TString( char *string, int isCopy ) : TConst( isCopy )
  {
    init( string );
  };

  TString( char *string ) : TConst( ORIGINAL )
  {
    init( string );
  };

  TString( char *string, BOOL useParamString, int isCopy ) : TConst( isCopy )
  {
    CharacterPosition = -1;
    this->string = string;
    Type = SString;
  }

  void init( char *string )
  {
#if defined DEBUG
    printf( "Create string" );
#endif
    CharacterPosition = -1;
    this->string = (char *)malloc( strlen( string ) + 1 );
    strcpy( this->string, string );
    Type = SString;
  };

  void Concat( char *string )
  {
    this->string = (char *)realloc( (void *)this->string,
                                strlen( this->string) + strlen( string ) + 1 );
    strcat( this->string, string );
  };
  ~TString()
  {
    free( string );
#if defined DEBUG
    printf( "Delete string " );
#endif
  };
};

struct TChar : TSyntax
{
  char *c;
  TChar( char *c ) : TSyntax()
  {
    this->c = c;
    Type = SChar;
  };
  ~TChar() {};
};

typedef TSyntax  *TSyntaxPtr;

struct TArray : TConst
{
  TSyntaxPtr *Array;
  int Size, AllocatedSize;
  int KillElements;
  TArray( TSyntaxPtr *Array, int Size ) :
    KillElements(1), TConst(1)
  {
#if defined DEBUG
    printf( "Create Array " );
#endif

    this->Array = Array;
    this->Size = Size;
    AllocatedSize = Size;
    Type = SArray;
  };

  void InsertElement( TSyntax *Elem, int Pos )
  {
    if( Pos > Size )
      printf( "ERROR INSERT ELEMENT" );
    // else raise exception
    Array = AddToList( Array, &AllocatedSize, Elem, Pos );
    Size++;
  };

  TSyntax *GetElement( int Pos )
  {
    if( Pos < 0 || Pos >= Size )
      return NULL;
    return Array[Pos];
  };

  TSyntax **GetElementAddress( int Pos )
  {
    if( Pos < 0 || Pos >= Size )
      return NULL;
    return &Array[Pos];
  };

  void KeepElements( void ) { KillElements = 0; };
  void Concat( TArray *Add )
  {
    Concat( Add, ORIGINAL );
  };

  void Concat( TArray *Add, int MakeCopy );
  ~TArray();
};

struct TId : TSyntax
{
  char *string;
  int Reference; // If this is the name of a reference variable
  TId( char *string ) : TSyntax()
  {
#if defined DEBUG
    printf( "Create lexId" );
#endif

    Reference = 0;
    this->string = (char *)malloc( strlen( string ) + 1 );
    strcpy( this->string, string );
    Type = SIdentifier;
  };
  ~TId()
  {
#if defined DEBUG
    printf( "Delete lexId " );
#endif
    free( string );
  };
};

struct TNoOp : TSyntax
{
  TNoOp() : TSyntax() { Type = SNoOp; }
};

typedef enum { BIN_AND, BIN_OR, BIN_XOR, BIN_BOOL_OR, BIN_BOOL_AND, BIN_EQ, BIN_NEQ, 
				  BIN_GTEQ, BIN_LTEQ, BIN_LT, BIN_GT,  BIN_PLUS, BIN_MINUS, BIN_MUL,
          BIN_DIV, BIN_MOD, BIN_ASSIGN, BIN_STRCAT } BinOpDef;

struct TBinOp : TSyntax
{
  TSyntax *Left, *Right;
  //char *Operator;
  BinOpDef op;
  int forcePriority;
  TBinOp( BinOpDef op, TSyntax *Left, TSyntax *Right ) : TSyntax()
  {
#if defined DEBUG
    printf( "Create BinOp" );
#endif

//    this->Operator = (char *)malloc( strlen( Operator ) + 1 );
//    strcpy( this->Operator, Operator );
		this->op = op;
    this->Left = Left;
    this->Right = Right;
    forcePriority = FALSE;
    Type = SBinOp;
  };
  ~TBinOp()
  {
#if defined DEBUG
    printf( "Delete BinOp " );
#endif
    Kill( &Left );
    Kill( &Right );
//    free( Operator );
  };
};

typedef enum { UN_INC, UN_DEC, UN_NOT, UN_NEG } UnOpDef;

struct TUnOp : TSyntax
{
  TSyntax *Expr;
  UnOpDef op;
  TUnOp( UnOpDef op, TSyntax *Expr ) : TSyntax()
  {
  	this->op = op;
    this->Expr = Expr;
    Type = SUnOp;
  };
  ~TUnOp()
  {
#if defined DEBUG
    printf( "Delete UnOp " );
#endif
    Kill( &Expr );
  };
};

struct TExprns : TSyntax
{
  TSyntax *Expr, *Exprns;
  TExprns( TSyntax *Expr, TSyntax *Exprns ) : TSyntax()
  {
#if defined DEBUG
    printf( "Create Exprns" );
#endif

    this->Expr = Expr;
    this->Exprns = Exprns;
    Type = SExprns;
  };
  ~TExprns()
  {
#if defined DEBUG
    printf( "Delete Exprns " );
#endif
    Kill( &Expr );
    Kill( &Exprns );
  };
};

struct TStmts : TSyntax
{
  TSyntax *Stmt, *Stmts;
  TStmts( TSyntax *Stmt, TSyntax *Stmts ) : TSyntax()
  {
#if defined DEBUG
    printf( "Create Stmts" );
#endif

    this->Stmt = Stmt;
    this->Stmts = Stmts;
    Type = SStmts;
  };
  ~TStmts()
  {
#if defined DEBUG
    printf( "Delete Stmts " );
#endif
    Kill( &Stmt );
    Kill( &Stmts );
  };
};

struct TIf : TSyntax
{
  TSyntax *Arg, *Block;
  TIf( TSyntax *Arg, TSyntax *Block ) : TSyntax()
  {
    this->Arg = Arg;
    this->Block = Block;
    Type = SIf;
  };
  ~TIf()
  {
#if defined DEBUG
    printf( "Delete If " );
#endif
    Kill( &Arg );
    Kill( &Block );
  };
};

struct TIfElse : TSyntax
{
  TSyntax *Arg, *Block1, *Block2;
  TIfElse( TSyntax *Arg, TSyntax *Block1, TSyntax *Block2 ) : TSyntax()
  {
    this->Arg = Arg;
    this->Block1 = Block1;
    this->Block2 = Block2;
    Type = SIfElse;
  };
  ~TIfElse()
  {
#if defined DEBUG
    printf( "Delete IfElse " );
#endif
    Kill( &Arg );
    Kill( &Block1 );
    Kill( &Block2 );
  };
};

struct TFor : TSyntax
{
  TSyntax *Arg[3], *Block;
  TFor( TSyntax *Arg1, TSyntax *Arg2, TSyntax *Arg3, TSyntax *Block ) :
    TSyntax()
  {
    Type = SFor;
    Arg[0] = Arg1;
    Arg[1] = Arg2;
    Arg[2] = Arg3;
    this->Block = Block;
  };
  ~TFor()
  {
#if defined DEBUG
    printf( "Delete For " );
#endif
    Kill( &Arg[0] );
    Kill( &Arg[1] );
    Kill( &Arg[2] );
    Kill( &Block );
  };
};

struct TDo : TSyntax
{
  TSyntax *Arg, *Block;
  TDo( TSyntax *Arg, TSyntax *Block ) : TSyntax()
  {
    this->Arg = Arg;
    this->Block = Block;
    Type = SDo;
  };
  ~TDo()
  {
    Kill( &Arg );
    Kill( &Block );
  };
};

struct TBreak : TSyntax
{
  TBreak() : TSyntax() { Type = SBreak; };
  ~TBreak() {};
};

struct TContinue : TSyntax
{
  TContinue() : TSyntax() { Type = SContinue; };
  ~TContinue() {};
};

struct TReturn : TSyntax
{
  TSyntax *Expr;
  TReturn( TSyntax *Expr ) : TSyntax()
  {
    this->Expr = Expr;
    Type = SReturn;
  };
  ~TReturn()
  {
#if defined DEBUG
    printf( "Delete Return" );
#endif
    Kill( &Expr );
  };
};

struct TSubscript : TSyntax
{
  TSyntax *Array;
  TArray *Index;
  TSubscript( TSyntax *Array, TArray *Index ) : TSyntax()
  {
    this->Array = Array;
    this->Index = Index;
    Type = SSubscript;
  };
  ~TSubscript()
  {
#if defined DEBUG
    printf( "Delete Subscript " );
#endif
    Kill( &Array);
    Kill( (TSyntax **)&Index );
  };
};

struct TSwitch : TSyntax
{
  TSyntax *Stmt;
  TArray *Case, *Block;

  TSwitch( TSyntax *Stmt, TArray *Case, TArray *Block ) : TSyntax()
  {
    this->Stmt = Stmt;
    this->Case = Case;
    this->Block = Block;
    Type = SSwitch;
  };
  ~TSwitch()
  {
    Kill( &Stmt );
    Kill( (TSyntax **)&Case );
    Kill( (TSyntax **)&Block );
  };
};

struct TFunction : TSyntax
{
  char *Name;
  TArray *Args;
  int NumArgs;
  TFunction( char *Name, TArray *Args ) : TSyntax()
  {
    this->Name = (char*)malloc( strlen( Name ) + 1 );
    strcpy( this->Name, Name );
    this->NumArgs = (Args == NULL) ? 0 : Args->Size;
    this->Args = Args;
    Type = SFunction;
  };
  ~TFunction()
  {
#if defined DEBUG
    printf( "Delete Function " );
#endif
    free( Name );
    Kill( (TSyntax **)&Args );
  };
};

struct TOverload : TSyntax
{
  TFunction *function;
  TOverload( TFunction *function )
  {
    this->function = function;
    Type = SOverload;
  }
  ~TOverload()
  {
    Kill( (TSyntax **)&function );
  }
};

/*** Parser class ***/

class TModule;

#define MAXERR 25

class TParser
{
  TModule *Module;
  TErrorCollector *ErrorCollector;
  int Stop, Errors;
  TSyntax *reAssoc( TSyntax *expr );
public:
  TParser( TModule *Module, TErrorCollector *ErrorCollector ) :
    Module(Module), ErrorCollector( ErrorCollector ), Stop(0), Errors(0) {};
  void Error( int Column, int Line, char *lexWord, char *string );
  void AddToPostponed( TSyntax ***Postponed, TSyntax *Newpp );
#define BEFORE 1
#define AFTER  2
#define YES 1
#define NO  0
  void AddPostponed( TSyntax **Syntax, TArray **Postponed, int Location );
  TToken *ParseBinOp( TToken *Token, TSyntax **Expr, TArray *Postponed,
                      char *Op[], int n, int ExprI, int ExpandEq );
  TToken *ParseExpr( int Expri, TToken *Token, TSyntax **Expr,
                     TArray *Postponed );
  TToken *ParseList( TToken *Token, TArray *Array, TArray *Postponed );
  TToken *ParseExpr( TToken *Token, TSyntax **Expr, TArray *Postponed );
  TToken *ParseExprns( TToken *Token, TSyntax **Expr, TArray *Postponed );
  TToken *ParseStmt( TToken *Token, TSyntax **Syntax );
  TToken *ParseStmts( TToken *Token, TSyntax **Syntax );
  TToken *ParseStmtsUntilToken( TToken *Token, TSyntax **Syntax,
                                TToken *StopTokens );
  TToken *Parse( TToken *Token );
  int TokenIs( TToken *Token, int Type, char *string );
};

#define KPARSE

#endif
