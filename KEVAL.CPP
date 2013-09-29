#define K_EVAL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bowl.h"
#include "mainwadm.h"

#include "k.h"

#include "keval.h"
#include "kcmdexec.h"

//#define DEBUG

void TEval::SendError( char *str, char *arg )
{
  if( ErrorOccured == 0 )  // Only report one error
  {
    if( arg == NULL )
      arg = "";
    char *errstr = new char[strlen(str)+strlen(arg)+1];
    sprintf( errstr, str, arg );
    Window *wnd = (Window *)MainWindowAdmin::getActiveWindow();
    if( wnd == NULL ) wnd = Desktop::getInstance();
    PopupMessage message( wnd, Strings::get(35), errstr,
                          PopupMessage::error );
    delete errstr;
  }
  ErrorOccured++;
}

TSyntax *EvalBuiltIn( char *Name, TVariableList *Vars, TEval *Eval )
{
  if( !strcmp( Name, "length" ) )
  {
    TSyntax **Value = Vars->LookUp( "array" );
    if( Value == NULL )
      Eval->SendError( Strings::get(47), Name );
    else
    if( (*Value)->Type == SArray )
      return new TNumeric( ((TArray *)(*Value))->Size, COPY );
    else
      Eval->SendError( Strings::get(36), Name );
  }
  else
  if( !strcmp( Name, "toInt" ) )
  {
    TSyntax **Value = Vars->LookUp( "data" );
    if( Value == NULL )
      Eval->SendError( Strings::get(47), Name );
    else
    if( (*Value)->Type == SString )
      return new TNumeric( atoi( ((TString *)*Value)->string), COPY );
    else
    if( (*Value)->Type == SNumeric )
      return new TNumeric( ((TNumeric *)*Value)->num, COPY );
    else
      Eval->SendError( Strings::get(36), Name );
  }
  else
  if( !strcmp( Name, "toString" ) )
  {
		char str[20];

    TSyntax **Value = Vars->LookUp( "data" );
    if( Value == NULL )
      Eval->SendError( Strings::get(47), Name );
    else
    if( (*Value)->Type == SString )
      return new TString( ((TString *)*Value)->string, COPY );
    else
    if( (*Value)->Type == SNumeric )
      return new TString( itoa( ((TNumeric *)*Value)->num, str, 10), COPY );
    else
      Eval->SendError( Strings::get(36), Name );
  }
  else
  if( !strcmp( Name, "debug" ) )
  {
/*    TSyntax **Value = Vars->LookUp( "data" );
    if( Value == NULL )
      Eval->SendError( Strings::get(47), Name );
    else
    {
      PrintSyntax( *Value );
    }*/
  }
  else
  {
    Eval->SendError( Strings::get(37), Name );
  }
  return NULL;
}

// Get memory address of the identfier or subscripting array

TSyntax **TEval::GetSlot( TSyntax *Syntax )
{
  TSyntax **Slot = NULL;

  if( Syntax->Type == SIdentifier )
  {
    Slot = LookUp( ((TId *)Syntax)->string );
    return Slot;
  }
  else
  if( Syntax->Type == SSubscript )
  {
    TSubscript *Subscript = (TSubscript *)Syntax;
    TArray *Array = (TArray *)Subscript->Array, *NextArray = NULL;

    for( int i = 0; i < Subscript->Index->Size; i++ )
    {
      int Kill = 0;
      // Get the index in order
      TSyntax *Index = Eval( Subscript->Index->GetElement( i ) );

      if( Index == NULL )
      {
        SendError( Strings::get(38), "" );
        return NULL;
      }

      if( Array->Type == SIdentifier )
      {
        TArray **PArray = (TArray **)LookUp( ((TId *)Array)->string );
        Array = *PArray;
      }
      else
      if( Array->Type != SArray )
      {
        Array = (TArray *)Eval( Array );
        Kill = 1;
      }

      if( Array->Type == SString )  // Strings are allowed in the last level
      {
        if( i == Subscript->Index->Size - 1 )
        {
          int iIndex = GetNum( Index );
          if( iIndex >= 0 && iIndex < strlen(((TString *)Array)->string)  )
          {
            TChar **Char;
            Char = new (TChar *);
            *Char = new TChar( &((TString *)Array)->string[iIndex] );
            KillCopy( Index );
            return (TSyntax **)Char;
          }
          else
            SendError( Strings::get(38), "" );
        }
        else
          SendError( Strings::get(39), NULL );
      }

      if( Array == NULL || Array->Type != SArray )
        SendError( Strings::get(40), NULL );

      if( ErrorOccured )
      {
        KillCopy( Index );
        if( Kill )
          KillCopy( Array );
      }

      Slot = Array->GetElementAddress( GetNum( Index ) );

      if( Slot == NULL )
        SendError( Strings::get(38), "" );
      else
      if( i < Subscript->Index->Size - 1)
      {
        NextArray = (TArray *)*Slot; // Get next array

        if( NextArray->Type != SArray)
        {
          if( i < Subscript->Index->Size - 1 )
            SendError( Strings::get(41), NULL );
          else
            NextArray = (TArray *)GetCopy( Eval( NextArray ) );
        }
      }

      if( Kill  )
        KillCopy( Array );

      Array = NextArray;

      KillCopy( Index );

      if( ErrorOccured )
        return NULL;
    }
    return Slot;
  }
  SendError( Strings::get(43), NULL );
  return (TSyntax **)-1;
}

TSyntax *TEval::execCommand( TSyntax *Syntax, int functionIdx )
{

  TArray *Args = ((TFunction *)Syntax)->Args;
  TSyntaxPtr *ArgsDef = Command[functionIdx].Args;

  if( Command[functionIdx].NumArgs != Args->Size )
  {
    SendError( Strings::get(47), ((TFunction *)Syntax)->Name );
    return NULL;
  }

  TVariableList *Arguments = new TVariableList();

  // Copy arguments.
  for( int i = 0; i < Args->Size; i++ )
  {
    if( ((TId *)ArgsDef[i])->Reference )
    {
      TSyntax **Slot = GetSlot( Args->Array[i] );

      if( Slot == (TSyntax **)-1 )
        SendError( Strings::get(43), NULL );

      if( Slot == NULL )
        SendError( Strings::get(42), NULL );

      if( ErrorOccured )
      {
        delete Arguments;
        return NULL;
      }

      Arguments->Update( ((TId *)ArgsDef[i])->string, Slot, Module, KEEP );
      continue;
    }

    TSyntax *Value = Eval(Args->Array[i]);

    if( ErrorOccured )
    {
      delete Arguments;
      KillCopy( Value );
      return NULL;
    }

    Arguments->Update( ((TId *)ArgsDef[i])->string,
                               &Value, Module, DONTKEEP );
  }

  // Move up one level in variable stack
  Calls++;

  if( Calls > VariablesAllocated )
  {
    VariablesAllocated *= 2;
    if( VariablesAllocated == 0 )
      VariablesAllocated = 1;
    Variable = (TVariableList **)realloc( (void *)Variable,
                                    sizeof( TVariableList *) * VariablesAllocated );
  }

  if( Calls < VariablesAllocated / 8 )
  {
    VariablesAllocated /= 8;
    if( VariablesAllocated == 0 )
      VariablesAllocated = 1;
    Variable = (TVariableList **)realloc( (void *)Variable,
                                    sizeof( TVariableList *) * VariablesAllocated );
  }

  Variable[Calls-1] = Arguments;

  // Evaluate function
  RetVal = NULL;

  if( Command[functionIdx].Code )
    KillCopy( Eval( Command[functionIdx].Code ) );
  else
    RetVal = commandExecutor->eval( ((TFunction *)Syntax)->Name,
                                    Variable[Calls-1], this, Command[functionIdx].index );

  Return = Continue = Break = 0;  // Reset all these

  Calls--;
  delete Variable[Calls];

  // We must always return something, so check if RetVal is NULL. Return FALSE if so.
  TSyntax *retCopy = ((RetVal == NULL) ? new TNumeric( 0, COPY ) : RetVal);
  RetVal = NULL;
  return retCopy;
}

TSyntax *TEval::Eval( TSyntax *Syntax )
{
  if( Return || Break || Continue || ErrorOccured ) return NULL;
  if( Syntax == NULL ) return NULL;
  switch( Syntax->Type )
  {
    case SNumeric :
      return Syntax;
    case SString :
      return Syntax;
    case SArray :
    {
      TArray *NewArray = new TArray( NULL, 0 ),
             *Array = (TArray *)Syntax;

      for( int i = 0; i < Array->Size; i++ )
      {
        TSyntax *Value = GetCopy(Eval( Array->Array[i] ));

        if( ErrorOccured )
        {
          KillCopy( NewArray );
          return NULL;
        }

        SetCopyFlag( Value, ORIGINAL );
        NewArray->InsertElement( Value, i );
      }

      return NewArray;
    }
    case SIdentifier :
    {
      TId *lexId = (TId *)Syntax;
      TSyntax **Value = LookUp( lexId->string );

      if( Value == NULL ) // EXCEPTION!
      {
        SendError( "Unknown variable %s!", lexId->string );
        return NULL;
      }

      return Eval( *Value );
    }
    case SSubscript :
    {
      TSubscript *Subscript = (TSubscript *)Syntax;
      TArray *Array = (TArray *)Subscript->Array, *NextArray;
      TSyntax *Value;

      for( int i = 0; i < Subscript->Index->Size; i++ )
      {
        int Kill = 1;
        // Get the index in order
        TSyntax *Index = Eval( Subscript->Index->GetElement( i ) );

        if( Index == NULL )
          SendError( Strings::get(38), "" );

        if( ErrorOccured )
          return NULL;

        if( Array->Type == SIdentifier )
        {
          TArray **PArray = (TArray **)LookUp( ((TId *)Array)->string );

          if( PArray ) // If no error
            Array = *PArray;
          Kill = 0;
        }
        else
        if( Array->Type != SArray )
          Array = (TArray *)Eval( Array );

        if( ErrorOccured )
        {
          KillCopy( Index );
          return NULL;
        }

        if( Array->Type == SString )
        {
          if( i == Subscript->Index->Size - 1 )
          {
            int iIndex = GetNum( Index );
            if( iIndex >= 0 && iIndex < strlen(((TString *)Array)->string) )
            {
/*              char str[] = { ((TString *)Array)->string[iIndex], 0 };
              TString *string = new TString( str, COPY );*/
              TNumeric *num = new TNumeric( (int)((TString *)Array)->string[iIndex], COPY );

              if( Kill  )
                KillCopy( Array );
              KillCopy( Index );
              return num;
//              return string;
            }
            else
              SendError( Strings::get(38), "" );

          }
          else
            SendError( Strings::get(39), NULL );
        }

        if( Array == NULL || Array->Type != SArray )
          SendError( Strings::get(41), NULL );

        if( ErrorOccured )
        {
          if( Kill  )
            KillCopy( Array );
          KillCopy( Index );
          return NULL;
        }

        TSyntax *Elem = Array->GetElement( GetNum( Index ) );

        if( Elem == NULL )
          SendError( Strings::get(38), "" );
        else
          NextArray = (TArray *)GetCopy( Eval( Elem ));

        if( Kill  )
          KillCopy( Array );

        Value = NextArray;

        Array = NextArray;

        KillCopy( Index );

        if( ErrorOccured )
          return NULL;
      }
      return Value;
    }
    case SNoOp : return NULL;
    case SUnOp :
    {
      TUnOp *UnOp = (TUnOp *)Syntax;
      TSyntax **Value, *MemVal = NULL;
      int iNum, Update = 0;

      if( UnOp->op == UN_INC || UnOp->op == UN_DEC )
      {
        Value = GetSlot( UnOp->Expr );

        if( Value == (TSyntax **)-1 )
          SendError( Strings::get(43), NULL );

        if( Value == NULL )
          SendError( Strings::get(42), NULL );
      }
      else
      {
        MemVal = Eval( UnOp->Expr );
        Value = &MemVal;
      }

      if( ErrorOccured )
      {
        KillCopy( MemVal );
        return NULL;
      }

      if( (*Value)->Type == SString )
        iNum = atoi( ((TString *)*Value)->string );
      else
      if( (*Value)->Type == SChar )
        iNum = *((TChar *)*Value)->c;
      else
        iNum = ((TNumeric *)*Value)->num;

      if( UnOp->op == UN_INC )
      {
        iNum++;
        Update = 1;
      }
      else
      if( UnOp->op == UN_DEC )
      {
        iNum--;
        Update = 1;
      }
      else
      if( UnOp->op == UN_NEG )
        iNum = -iNum;
      else
      if( UnOp->op == UN_NOT )
        iNum = !iNum;

      TNumeric *num = new TNumeric( iNum, COPY );

      if( Update )
      {
        if( (*Value)->Type == SChar )
        {
          *((TChar *)(*Value))->c = (char)iNum;
          Kill( Value );  // Doesn't hold any info
          delete Value;
          return num;
        }
        else
        {
          num->isCopy = ORIGINAL;
          Kill( Value );
          *Value = (TSyntax *)num;
          TSyntax *NumR;

          CopySyntax( &NumR, num );

          ((TConst *)NumR)->isCopy = COPY;

          return NumR;
        }
      }
      else
        KillCopy( *Value );
      return num;
    }
    case SBinOp :
    {
      TBinOp *BinOp = (TBinOp *)Syntax;
      TSyntax *Right = Eval( BinOp->Right );
      TSyntax *Left;

      if( Right == NULL )
      {
        SendError( "Missing right operand", NULL );
        KillCopy( Right );
        return NULL;
      }

      if( BinOp->op == BIN_ASSIGN )
      {
        TSyntax **Slot = GetSlot( ((TBinOp *)Syntax)->Left );

        if( ErrorOccured )
        {
          KillCopy( Right );
          return NULL;
        }

        if( Slot )  // Equal variables!
          if( *Slot == Right ) return *Slot;

        int KillRight = 0;

        if( Slot == NULL )
        {
          // Need to create the identifier
          Variable[Calls-1]->Update( ((TId *)BinOp->Left)->string, &Right,
                                     Module, DONTKEEP );
        }
        else
        if( (*Slot)->Type == SChar )
        {
          if( Right->Type == SString )
            *((TChar *)(*Slot))->c = ((TString *)Right)->string[0];
          else
            *((TChar *)(*Slot))->c = (char)((TNumeric *)Right)->num;

          Kill( Slot );
          delete Slot;  // Not needed anymore
          KillRight = 1; // Can be killed
        }
        else
        {
          Kill( Slot );

          Right = GetCopy( Right );
          SetCopyFlag( Right, ORIGINAL );

          *Slot = Right;
          KillRight = 1;
        }

        CopySyntax( &Left, Right );

        if( KillRight )
          KillCopy( Right );

        SetCopyFlag( Left, COPY );
        return Left;
      }

      Left = Eval( BinOp->Left );

      if( ErrorOccured )
      {
        KillCopy( Left );
        KillCopy( Right );
        return NULL;
      }

      if( BinOp->op == BIN_STRCAT )
      {
      	char *str1, *str2, cnvStr1[20], cnvStr2[20];

        if( Left->Type == SNumeric )
        	str1 = itoa( ((TNumeric *)Left)->num, cnvStr1, 10 );
				else
        if( Left->Type == SString )
        	str1 = ((TString *)Left)->string;
				else
  	      SendError( Strings::get(45), "<<BINOP>>" );

        if( Right->Type == SNumeric )
        	str2 = itoa( ((TNumeric *)Right)->num, cnvStr2, 10 );
				else
        if( Right->Type == SString )
        	str2 = ((TString *)Right)->string;
				else
  	      SendError( Strings::get(45), "<<BINOP>>" );

				char *concat = (char *)malloc(strlen(str1)+strlen(str2)+1);
        strcpy( concat, str1 );
        strcat( concat, str2 );

				KillCopy( Left );
				KillCopy( Right );

				TString *str = new TString( concat, TRUE, COPY );
        return str;
      }

      Harmonize( &Left, &Right ); // Does GetCopy(..) if needed

      BinOpDef op = BinOp->op;

      if( Left->Type == SArray )
      {
        if( op == BIN_PLUS )
        {
          if( Right->Type == SArray )
          {
            ((TArray *)Left)->Concat( (TArray *)Right, 0 );
            ((TArray *)Right)->KeepElements();
            delete (TArray *)Right;
            return Left;
          }
          else
          {
//            PrintSyntax( Right );
            Right = GetCopy( Right );
            SetCopyFlag( Right, ORIGINAL );
            ((TArray *)Left)->InsertElement( Right, ((TArray *)Left)->Size );
            return Left;
          }
        }
        SendError( Strings::get(46), "<<BINOP>>" );
      }
      else
      if( Left->Type == SNumeric && Right->Type == SNumeric )
      {
        int L = ((TNumeric *)Left)->num,
            R = ((TNumeric *)Right)->num;

        KillCopy( Left );
        KillCopy( Right );

        switch( op )
        {
        	case BIN_BOOL_AND :
	          return new TNumeric( L && R, COPY );
  				case BIN_BOOL_OR :
          	return new TNumeric( L || R, COPY );
          case BIN_OR :
	          return new TNumeric( L | R, COPY );
  				case BIN_AND :
	          return new TNumeric( L & R, COPY );
  				case BIN_XOR :
	          return new TNumeric( L ^ R, COPY );
  				case BIN_EQ :
	          return new TNumeric( L == R, COPY );
  				case BIN_NEQ :
	          return new TNumeric( L != R, COPY );
  				case BIN_GTEQ :
	          return new TNumeric( L >= R, COPY );
  				case BIN_LTEQ :
	          return new TNumeric( L <= R, COPY );
  				case BIN_LT :
	          return new TNumeric( L < R, COPY );
  				case BIN_GT :
	          return new TNumeric( L > R, COPY );
  				case BIN_PLUS :
	          return new TNumeric( L + R, COPY );
  				case BIN_MINUS :
	          return new TNumeric( L - R, COPY ); // TEMP!!
  				case BIN_MUL :
	          return new TNumeric( L * R, COPY );
  				case BIN_DIV :
	          return new TNumeric( L / R, COPY );
  				case BIN_MOD :
	          return new TNumeric( L % R, COPY );
        }

        SendError( Strings::get(44), "<<BINOP>>" );
      }
      else
      if( Left->Type == SString && Right->Type == SString )
      {
        TString *L = (TString *)Left, *R = (TString *)Right;

        if( op == BIN_PLUS )
        {
          L = (TString *)GetCopy( L );
          L->Concat( R->string );
          KillCopy( Right );
          return L;
        }

        if( op == BIN_GT || op == BIN_LT || op == BIN_GTEQ ||
        		op == BIN_LTEQ || op == BIN_NEQ || op == BIN_EQ )
        {
          int Res = strcmp( L->string, R->string );

          KillCopy( (TSyntax *)L );
          KillCopy( (TSyntax *)R );

					switch( op )
          {
          	case BIN_GT :
            	return (TSyntax *)new TNumeric( Res > 0, COPY );
            case BIN_LT :
	            return (TSyntax *)new TNumeric( Res < 0, COPY );
            case BIN_GTEQ :
            	return (TSyntax *)new TNumeric( Res >= 0, COPY );
            case BIN_LTEQ :
	            return (TSyntax *)new TNumeric( Res <= 0, COPY );
            case BIN_EQ :
	            return (TSyntax *)new TNumeric( Res == 0, COPY );
            case BIN_NEQ :
            	return (TSyntax *)new TNumeric( Res != 0, COPY );
          }
        }

        SendError( Strings::get(45), "<<BINOP>>" );
        return NULL;
      }
      else
      {
        KillCopy( Left );
        KillCopy( Right );
        SendError( Strings::get(48), NULL );
      }
      return NULL;
    }
    case SExprns :
    {
      TExprns *Exprn = (TExprns *)Syntax;
      KillCopy( Eval( Exprn->Expr ) );
      if( !ErrorOccured )
        Eval( Exprn->Exprns );
      return NULL;
    }
    case SIf :
    {
      TIf *If = (TIf *)Syntax;
      TSyntax *Arg = Eval( If->Arg );

      if( ErrorOccured )
      {
        KillCopy( Arg );
        return NULL;
      }

      int Res = GetNum( Arg );

      KillCopy( Arg );

      if( Res )
        return Eval( If->Block );

      return NULL;
    }
    case SIfElse :
    {
      TIfElse *IfElse = (TIfElse *)Syntax;
      TSyntax *Arg = Eval( IfElse->Arg );

      if( ErrorOccured )
      {
        KillCopy( Arg );
        return NULL;
      }

      int Res = GetNum( Arg );

      KillCopy( Arg );

      if( Res )
        return Eval( IfElse->Block1 );
      else
        return Eval( IfElse->Block2 );
    }
    case SReturn :
    {
      RetVal = GetCopy( Eval( ((TReturn *)Syntax)->Expr ) );

      Return = 1;

      return NULL;
    }
    case SContinue :
    {
      Continue = 1;
      return NULL;
    }
    case SBreak :
    {
      Break = 1;
      return NULL;
    }
    case SStmts :
    {
      KillCopy( Eval( ((TStmts *)Syntax)->Stmt ) );
      if( ErrorOccured || Break || Continue || Return ) return NULL;
      KillCopy( Eval( ((TStmts *)Syntax)->Stmts ) );
      return NULL;
    }
    case SFor :
    {
      TFor *For = (TFor *)Syntax;
      // Init

      KillCopy( Eval( For->Arg[0] ) );

      if( ErrorOccured )
        return NULL;

      while( 1 )
      {
        TSyntax *Test = Eval( For->Arg[1] );

        if( ErrorOccured )
        {
          KillCopy( Test );
          return NULL;
        }

        int i = GetNum( Test );
        KillCopy( Test );

        if( i )
        {
          // Execute body

          KillCopy( Eval( For->Block ) );

          if( Break || Return || ErrorOccured )
          {
            Break = 0;
            break;
          }

          Continue = 0;

          // Inc

          KillCopy( Eval( For->Arg[2] ) );
        }
        else
          break;
      }
      return NULL;
    }
    case SDo :
    {
      TDo *Do = (TDo *)Syntax;
      int Res;

      do
      {
        KillCopy( Eval( Do->Block ) );

        TSyntax *Arg = Eval( Do->Arg );

        if( ErrorOccured )
        {
          KillCopy( Arg );
          break;
        }

        Res = GetNum( Arg );

        KillCopy( Arg );
      } while( Res );
      return NULL;
    }
    case SSwitch :
    {
      TSwitch *Switch = (TSwitch *)Syntax;

      // Evaluate statement

      TSyntax *Test = Eval( Switch->Stmt );

      if( Test != NULL )
        for( int i = 0; i < Switch->Case->Size; i++ )
        {
          int Res;

          TSyntax *Case = Eval( Switch->Case->GetElement( i ) );

          if( Case == NULL ) // default
            Res = 1;
          else
          {
						// KillCopy on a BinOp deletes the expressions unconditionally. We have to keep
						// track of this ourselves
						// Case is a TConst efter evaluation

						int caseIsCopy = ((TConst *)Case)->isCopy;

            TSyntax *TestC = GetCopy( Test ), *CaseC = GetCopy( Case );

            SetCopyFlag( TestC, ORIGINAL );
            SetCopyFlag( CaseC, ORIGINAL );

            TBinOp *BinOp = new TBinOp( BIN_EQ, TestC, CaseC );

            TSyntax *Result = Eval( BinOp );

            Res = GetNum( Result );

            KillCopy( BinOp );
            KillCopy( Result );

						if( !caseIsCopy )
							KillCopy( Case );
          }

          if( Res )
          {
            // Execute all code blocks until a break

            for( int n = i; n < Switch->Case->Size; n++ )
            {
              KillCopy( Eval( Switch->Block->GetElement( n ) ) );
              if( Break ) break;
            }
            break;
          }
        }
      Break = 0;
      KillCopy( Test );
      return NULL;
    }
    case SOverload :
    {
      // Lookup the function

      char *name = ((TOverload*)(Syntax))->function->Name;

      int functionIdx = FunctionList.updateOverloaded( this, name, TRUE );

      if( functionIdx == -1 )
      {
        SendError( Strings::get(37), name );
        return NULL;
      }
      else
      if( functionIdx == -2 )
      {
        SendError( Strings::get(57), name );
        return NULL;
      }

      TSyntax *result =
        execCommand( ((TOverload*)(Syntax))->function, functionIdx );

      FunctionList.updateOverloaded( this, name, FALSE );

/*      int functionIdx = FunctionList.FindIndex( name );

      if( functionIdx == -1 )
      {
        SendError( Strings::get(37), name );
        return NULL;
      }

      // Update overload counter to reference forward in the list

      Command[functionIdx].overloaded++;

      // Check that there is an overloaded function

      if( strcmp( name,
                  Command[functionIdx+Command[functionIdx].overloaded].Name ) )
      {
        SendError( Strings::get(57), name );
        return NULL;
      }

      // Execute the command overloaded positions forward

      TSyntax *result =
        execCommand( ((TOverload*)(Syntax))->function,
                     functionIdx + Command[functionIdx].overloaded );

      // Find the index again. Commands may have beem inserted.

      functionIdx =
        FunctionList.FindIndex( name );

      Command[functionIdx].overloaded--;*/

      return result;
    }
    case SFunction :
    {
      // Lookup the function

      int FunctionIdx = FunctionList.FindIndex( ((TFunction *)Syntax)->Name );

      if( FunctionIdx == -1 )
      {
        SendError( Strings::get(37), ((TFunction *)Syntax)->Name );
        return NULL;
      }

      return execCommand( Syntax, FunctionIdx );
    }
    default :
    {
      Window *wnd = (Window *)MainWindowAdmin::getActiveWindow();
      if( wnd == NULL ) wnd = Desktop::getInstance();
      PopupMessage message( wnd,
                            "FATAL ERROR", "UNKNOWN STATEMENT!!!",
                            PopupMessage::error );
    }
  }
  return NULL;
}

int TEval::isNum( TString *Syntax )
{
  int Hex = !strnicmp( "0x", Syntax->string, 2 );
  for( int i = 0; i < strlen( Syntax->string ); i++ )
  {
    if( !((Syntax->string[i] >= '0' && Syntax->string[i] <= '9') ||
        (Hex && (toupper(Syntax->string[i]) >= 'A' &&
                  toupper( Syntax->string[i]) <= 'F' ))) )
      return 0;
  }
  return 1;
}

void TEval::Harmonize( TSyntax **Left, TSyntax **Right )
{
  int LeftNum = 0, RightNum = 0;

  // Both strings are left unconverted

  if( (*Left)->Type == SString && (*Right)->Type == SString ) return;

  // If one is an array, return

  if( (*Left)->Type == SArray || (*Right)->Type == SArray ) return;

  if( (*Left)->Type == SString )
    LeftNum = isNum( (TString *)*Left );

  if( (*Right)->Type == SString )
    RightNum = isNum( (TString *)*Right );

  if( (*Left)->Type == SChar && (*Right)->Type == SNumeric )
  {
    TNumeric *nLeft = new TNumeric( (int)((TChar *)*Left)->c, COPY );
    KillCopy( *Left );
    *Left = nLeft;
    return;
  }

  if( (*Left)->Type == SNumeric && (*Right)->Type == SChar )
  {
    TNumeric *nRight = new TNumeric( (int)((TChar *)*Right)->c, COPY );
    KillCopy( *Right );
    *Right = nRight;
    return;
  }

/*  if( (*Left)->Type == SString && (*Right)->Type == SString &&
      LeftNum && RightNum )
  {
    // Both are convertable strings
    TNumeric *LNum = new TNumeric( atoi( ((TString *)*Left)->string ), COPY ),
             *RNum = new TNumeric( atoi( ((TString *)*Right)->string ), COPY );
    KillCopy( *Left );
    KillCopy( *Right );
    *Left = LNum;
    *Right = RNum;
  }
  else*/
  if( (*Left)->Type == SString && (*Right)->Type == SNumeric )
  {
    if( LeftNum )
    {
      // Left is a convertable string and right is a number
      TNumeric *LNum = new TNumeric( atoi( ((TString *)*Left)->string ), COPY );
      KillCopy( *Left );
      *Left = LNum;
    }
    else
    {
      // Left is a nonconvertable string and right is a number
      char str[100];
      TString *string = new TString( itoa( ((TNumeric *)*Right)->num, str, 10 ), COPY );
      KillCopy( *Right );
      *Right = string;
    }
  }
  else
  if( (*Right)->Type == SString && (*Left)->Type == SNumeric )
  {
    if( RightNum )
    {
      // Right is a convertable string and left is a number
      TNumeric *RNum = new TNumeric( atoi( ((TString *)*Right)->string ), COPY );
      KillCopy( *Right );
      *Right = RNum;
    }
    else
    {
      // Right is a nonconvertable string and left is a number
      char str[100];
      TString *string = new TString( itoa( ((TNumeric *)*Left)->num, str, 10 ), COPY );
      KillCopy( *Left );
      *Left = string;
    }
  }

  // else both are numbers or both are alphastrings
  return;
}

int TEval::GetNum( TSyntax *Expr )
{
  if( Expr == NULL ) return 0;
  if( Expr->Type == SNumeric )
    return ((TNumeric *)Expr)->num;
  else
  if( Expr->Type == SString )
    return atoi( ((TString *)Expr)->string );
  else
    return 0;
}

TEval::~TEval()
{
	for( int i = 0; i < Calls; i++ )
		delete Variable[i];
	delete Variable;
}

void SetCopyFlag( TSyntax *Expr, int Flag )
{
  if( Expr->Type == SNumeric || Expr->Type == SString || Expr->Type == SArray )
    ((TConst *)Expr)->isCopy = Flag;
/*  else
  if( Expr->Type == SArray )
    for( int i = 0; i < ((TArray *)Expr)->Size; i++ )
      SetCopyFlag( ((TArray *)Expr)->Array[i], Flag );*/
}

TSyntax *GetCopy( TSyntax *Expr )
{
  if( Expr == NULL ) return NULL;
  if( Expr->Type == SNumeric || Expr->Type == SString || Expr->Type == SArray )
    if( ((TConst *)Expr)->isCopy == ORIGINAL )
    {
      TSyntax *Copy;

      CopySyntax( &Copy, Expr );
      SetCopyFlag( Copy, COPY );
//      ((TConst *)Copy)->isCopy = COPY;
      return Copy;
    }
  return Expr;
}

void KillCopy( TSyntax *Expr )
{
  if( Expr <= NULL ) return;

  if( Expr->Type == SNumeric || Expr->Type == SString || Expr->Type == SArray )
    if( ((TConst *)Expr)->isCopy == ORIGINAL )
      return; // This is not a copy
  Kill( &Expr );
}
