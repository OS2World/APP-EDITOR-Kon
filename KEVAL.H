#if !defined KEVAL

class KCommandExecutor;

class TEval
{
  TSyntax *Syntax;
  int Calls, VariablesAllocated, Break, Return, Continue, ErrorOccured;
  TSyntax *RetVal;
  TVariableList **Variable;
  TModule *Module;
  KCommandExecutor *commandExecutor;
public:

  TEval( TModule *Module, KCommandExecutor *commandExecutor ) : Calls(1),
      VariablesAllocated(1), commandExecutor(commandExecutor), ErrorOccured(0),
      Variable(NULL)
  {
    this->Module = Module;
    Reset();
    // Create an empty variable list so we can add variables directly. We may
    // begin a program block directly, not just call a function.
    Variable = (TVariableList **)malloc( sizeof( TVariableList *) *
                                         VariablesAllocated );
    Variable[0] = new TVariableList();
  };

  TSyntaxPtr *LookUp( char *Name )
  {
    TSyntaxPtr *Value = NULL;

    if( Calls > 0 )
      Value = Variable[Calls-1]->LookUp( Name );

    if( Value != NULL )
      return Value;

    return Global.LookUp( Name );
  };

  void Reset( void )
  {
    Break = Continue = Return = 0;
  };

  void SendError( char *str, char *arg );

  TSyntax **GetSlot( TSyntax *Syntax );
  TSyntax *execCommand( TSyntax *Syntax, int functioIdx );
  TSyntax *Eval( TSyntax *Syntax );
  int isNum( TString *Syntax );
  void Harmonize( TSyntax **Left, TSyntax **Right );
  int GetNum( TSyntax *Expr );
  KCommandExecutor *getCommandExecutor() { return commandExecutor; }
  void haltExecution() {ErrorOccured = 1;}
	~TEval();
};

void SetCopyFlag( TSyntax *Expr, int Flag );
TSyntax *GetCopy( TSyntax *Expr );
void KillCopy( TSyntax *Expr );

#define KEVAL

#endif
