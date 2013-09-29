//#define DEBUG

// PLS DISREGARD THIS FILE! IT WORKS WELL, BUT IS A MISERABLE MESS..
// I WILL FIX ALL K FILES AT A LATER TIME

#if !defined K

#include "klexer.h"
#include "kerror.h"
#include "kparse.h"

TSyntaxPtr *AddToList( TSyntaxPtr *List, int *Size, TSyntax *Item, int Pos );

TSyntaxPtr *BuildArgList( TSyntaxPtr *ArgList, char *VarName, int Reference,
                          int NumArgs );

class TModule
{
  char *filename;
  typedef struct TVariableLevel
  {
    TVariableLevel *Next;
    TVariableLevel( TVariableLevel *Next )
    {
      this->Next = Next;
    };
  };
  int fail;
  BOOL preload;
public:
  TModule( char *data, int type, TErrorCollector *ErrorCollector, BOOL preload = FALSE );
  char *getFilename() { return filename; }
  int success() { return !fail; }
  BOOL getPreload() { return preload; }
  ~TModule();
};

class TLexer : public TBLexer 
{
public:
  TLexer( TStream *stream, TErrorCollector *errorCollector ) : 
    TBLexer( stream, errorCollector, 1 ) 
  {
    setupTokens();
  }
  virtual void setupTokens();
  virtual void includeFile( char *file );
};

#define KCODE   0
#define BUILTIN 1

struct TCommandOverload
{
  void *evaluator;
  int overloads;
};

struct TCommand
{
  char *Name;
  TSyntax *Code;
  int NumArgs;
  TSyntaxPtr *Args;
  TModule *Module;
  TCommandOverload *evalOverloaded;
  int overloads, overloadsAlloc;
	int index;
};

struct TVariable
{
  char *Name;
  int Reference;
  TSyntaxPtr *Value;
  TModule *Module;
};

#define KEEP      1
#define DONTKEEP  0

struct TVariableList
{
  TVariable *Variable;
  int Variables;

  int FindIndex( char *Name );

  TVariableList() : Variables(0), Variable(NULL) {};
  ~TVariableList();

  void Update( char *Name, TSyntaxPtr *Value, TModule *Module, int Reference );
  TSyntaxPtr *LookUp( char *Name );
  void Remove( char *Name );
  void EvaluateVariables( void );
};

struct TFunctionList
{
  int FindIndex( char *Name );
  // Update returns 0 if the update was unsuccessful.
  // If the function name already exists, the new function will be placed
  // first
  int updateOverloaded( void *evaluator, char *fncName, BOOL add );
  int Update( char *Name, int NumArgs, TSyntaxPtr *ArgList, TSyntax *Code,
              TModule *Module, int index );
  void Remove( char *Name );
  int getCount();
  TCommand *getFunction( int i );
};

void PrintSyntax( TSyntax *Syntax );

/*typedef struct
{
	char Name[30];
	int CodeNumber, ToolButton;
	char * Description;
  int ArgNum;
  char * ArgList[4];   // Max 4 arguments for built-in functions
} TBuiltInCommand;
*/
#if !defined K_MAIN
extern struct TVariableList Global;
extern struct TFunctionList FunctionList;
extern int Commands;
extern TCommand *Command;
//extern TBuiltInCommand StandardCommand[];
#endif

void BuildFunctionList( void );

#define K

#endif
