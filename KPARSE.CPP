#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <os2.h>

#include "bowl.h"

#define K_PARSER

int Objects = 0;

#include "k.h"
#include "keval.h"

void CopySyntax( TSyntax **Dest, TSyntax *Src )
{
  if( Src != NULL )
  {
    switch( Src->Type )
    {
      case SNoOp : 
      {
        TNoOp *NoOp = new TNoOp();
        *Dest = (TSyntax *)NoOp;
        break;
      }
      case SFor :
      {
        TFor *For = new TFor( (TSyntax *)NULL, (TSyntax *)NULL,
                              (TSyntax *)NULL, (TSyntax *)NULL );
        for( int i = 0; i < 3; i++ )
          CopySyntax( &For->Arg[i], ((TFor *)Src)->Arg[i] );
        CopySyntax( &For->Block, ((TFor *)Src)->Block );
        *Dest = (TSyntax *)For;
        break;
      }
      case SDo :
      {
        TDo *Do = new TDo( (TSyntax *)NULL, (TSyntax *)NULL );
        CopySyntax( &Do->Arg, ((TDo *)Src)->Arg );
        CopySyntax( &Do->Block, ((TDo *)Src)->Block );
        *Dest = (TSyntax *)Do;
        break;
      }
      case SIf :
      {
        TIf *If = new TIf( (TSyntax *)NULL, (TSyntax *)NULL );
        CopySyntax( &If->Arg, ((TIf *)Src)->Arg );
        CopySyntax( &If->Block, ((TIf *)Src)->Block);
        *Dest = (TSyntax *)If;
        break;
      }
      case SIfElse :
      {
        TIfElse *IfElse = new TIfElse( (TSyntax *)NULL, (TSyntax *)NULL,
                                       (TSyntax *)NULL );
        CopySyntax( &IfElse->Arg, ((TIfElse *)Src)->Arg );
        CopySyntax( &IfElse->Block1, ((TIfElse *)Src)->Block1);
        CopySyntax( &IfElse->Block2, ((TIfElse *)Src)->Block2);
        *Dest = (TSyntax *)IfElse;
        break;
      }
      case SBinOp :
      {
        TBinOp *BinOp = new TBinOp( ((TBinOp *)Src)->op, (TSyntax *)NULL,
                                    (TSyntax *)NULL );
        CopySyntax( &BinOp->Left, ((TBinOp *)Src)->Left );
        CopySyntax( &BinOp->Right, ((TBinOp *)Src)->Right );
        *Dest = (TSyntax *)BinOp;
        break;
      }
      case SUnOp :
      {
        TUnOp *UnOp = new TUnOp( ((TUnOp *)Src)->op, (TSyntax *)NULL );
        CopySyntax( &UnOp->Expr, ((TUnOp *)Src)->Expr );
        *Dest = (TSyntax *)UnOp;
        break;
      }
      case SNumeric :
      {
        TNumeric *Numeric = new TNumeric( ((TNumeric *)Src)->num, 1 );	// 0
        *Dest = (TSyntax *)Numeric;
        break;
      }
      case SString:
      {
        TString *string = new TString( ((TString*)Src)->string, 1 ); // 0
        *Dest = (TSyntax *)string;
        break;
      }
      case SIdentifier :
      {
        TId *lexId = new TId(((TId *)Src)->string);
        *Dest = (TSyntax *)lexId;
        break;
      }
      case SStmts :
      {
        TStmts *Stmts = new TStmts( (TSyntax *)NULL, (TSyntax *)NULL );
        CopySyntax( &Stmts->Stmt, ((TStmts *)Src)->Stmt );
        CopySyntax( &Stmts->Stmts, ((TStmts *)Src)->Stmts );
        *Dest = (TSyntax *)Stmts;
        break;
      }
      case SExprns :
      {
        TExprns *Exprns = new TExprns( (TSyntax *)NULL, (TSyntax *)NULL );
        CopySyntax( &Exprns->Expr, ((TExprns *)Src)->Expr );
        CopySyntax( &Exprns->Exprns, ((TExprns *)Src)->Exprns );
        *Dest = (TSyntax *)Exprns;
        break;
      }
      case SBreak :
        *Dest = new TBreak();
        break;
      case SContinue :
        *Dest = new TContinue();
        break;
      case SReturn :
      {
        TReturn *Return = new TReturn( (TSyntax *)NULL );
        CopySyntax( &Return->Expr, ((TReturn *)Src)->Expr );
        *Dest = (TSyntax *)Return;
        break;
      }
      case SFunction :
      {
        TFunction *Function = new TFunction( ((TFunction *)Src)->Name, (TArray *)NULL );
        CopySyntax( (TSyntax **)&Function->Args, (TSyntax *)((TFunction *)Src)->Args );
        Function->NumArgs = (((TFunction *)Src)->Args == NULL) ? 0 :
                              ((TFunction *)Src)->Args->Size;
        *Dest = (TSyntax *)Function;
        break;
      }
      case SOverload :
      {
        TFunction *function;
        CopySyntax( (TSyntax **)&function,
                    (TSyntax *)((TOverload *)Src)->function );
        TOverload *Overload = new TOverload( function );
        *Dest = (TSyntax *)Overload;
        break;
      }
      case SSubscript :
      {
        TSubscript *Subscript = new TSubscript( NULL, NULL );
        CopySyntax( &Subscript->Array, ((TSubscript *)Src)->Array);
        CopySyntax( (TSyntax **)&Subscript->Index,
                    (TSyntax *)((TSubscript *)Src)->Index );
        *Dest = (TSyntax *)Subscript;
        break;
      }
      case SArray :
      {
        TArray *Array = new TArray( (TSyntaxPtr *)NULL, 0 );

        for( int i = 0; i < ((TArray *)Src)->Size; i++ )
        {
          TSyntax *Expr = NULL;
          CopySyntax( &Expr, ((TArray *)Src)->Array[i] );
          Array->InsertElement( Expr, i );
        }

				Array->KillElements = 1;
				Array->isCopy = TRUE;

        *Dest = (TSyntax *)Array;
        break;
      }
      case SSwitch :
      {
        TArray *Case = NULL, *Block = NULL;
        TSyntax *Stmt = NULL;
        CopySyntax( &Stmt, ((TSwitch *)Src)->Stmt );
        CopySyntax( (TSyntax **)&Case, ((TSwitch *)Src)->Case );
        CopySyntax( (TSyntax **)&Block, ((TSwitch *)Src)->Block );
        TSwitch *Switch = new TSwitch( Stmt, Case, Block );
        *Dest = (TSyntax *)Switch;
        break;
      }
      default : printf( "COULDN'T COPY! " );
    }
  }
  else
    *Dest = NULL;
}

void Kill( TSyntax **Obj )
{
  if( *Obj != NULL && Obj != NULL )
  {
    switch( (*Obj)->Type )
    {
      case SNoOp : delete (TNoOp *)*Obj; break;
      case SFor : delete (TFor *)*Obj; break;
      case SDo : delete (TDo *)*Obj; break;
      case SIf : delete (TIf *)*Obj; break;
      case SIfElse : delete (TIfElse *)*Obj; break;
      case SBinOp : delete (TBinOp *)*Obj; break;
      case SUnOp : delete (TUnOp *)*Obj; break;
      case SNumeric : delete (TNumeric *)*Obj; break;
      case SString : delete (TString *)*Obj; break;
      case SIdentifier : delete (TId *)*Obj; break;
      case SStmts : delete (TStmts *)*Obj; break;
      case SExprns : delete (TExprns *)*Obj; break;
      case SBreak : delete (TBreak *)*Obj; break;
      case SContinue : delete (TContinue *)*Obj; break;
      case SSubscript : delete (TSubscript *)*Obj; break;
      case SReturn: delete (TReturn*)*Obj; break;
      case SFunction : delete (TFunction *)*Obj; break;
      case SOverload : delete (TOverload *)*Obj; break;
      case SArray : delete (TArray *)*Obj; break;
      case SChar : delete (TChar *)*Obj; break;
      case SSwitch : delete (TSwitch *)*Obj; break;
      default : PopupMessage message( Desktop::getInstance(),
                                      "FATAL ERROR", "UNKNOWN OBJECT (KILL)!!!",
                                      PopupMessage::error );
                delete *Obj;
    }
    *Obj = NULL;
  }
}

int TParser::TokenIs( TToken *Token, int Type, char *string )
{
  if( Token != NULL )
    return Token->type == Type && !strcmp( Token->string, string );
  return 0;
}

// Send an error to the errorcollector

void TParser::Error( int Column, int Line, char *lexWord, char *string )
{
  char *str = string;

  if( Errors > MAXERR )
    Stop = 1;

  Errors++;

  if( lexWord != NULL )
  {
    str = new char[strlen ( string ) + strlen( lexWord ) + 1];
    sprintf( str, string, lexWord );
  }
  ErrorCollector->Error( (Module==NULL) ? "<internal>" : Module->getFilename(), 
                         Column, Line, str );
  if( lexWord != NULL )
    delete str;
}

// Add postponed expressions before or after to a statement

void TParser::AddPostponed( TSyntax **Syntax, TArray **Postponed,
                            int Location )
{
  if( *Postponed == NULL ) return;
  if( (*Postponed)->Size != 0 )  // There are postponed operations
  {
    TStmts *Stmts = new TStmts( (Location == AFTER) ? *Syntax : *Postponed,
                                (Location == AFTER) ? *Postponed : *Syntax );
    *Syntax = Stmts;
#if defined DEBUG
    printf( "Fixed postponed " );
#endif
  }
  else
    Kill( (TSyntax **)Postponed );
}

TSyntax *TParser::reAssoc( TSyntax *expr )
{
  if( expr->Type == SBinOp )
  {
    TBinOp *binOp = (TBinOp *)expr;
    if( (binOp->op == BIN_PLUS || binOp->op == BIN_MINUS) && 
        binOp->Right->Type == SBinOp )
    {
      TBinOp *R = (TBinOp *)binOp->Right;

      if( !R->forcePriority && (R->op == BIN_MINUS || R->op == BIN_PLUS) )
      {
        // Change association
        TSyntax *tmp = binOp->Left;
        binOp->Left = binOp->Right;
        binOp->Right = R->Right;
        R->Right = R->Left;
        R->Left = tmp;
        BinOpDef tmpOp = R->op;
        R->op = binOp->op;
        binOp->op = tmpOp;

        // reassociate down the tree

        binOp->Left = reAssoc( binOp->Left );
        binOp->Right = reAssoc( binOp->Right );
      }
    }
  }
  return expr;
}

// Parse a binary operation

TToken *TParser::ParseBinOp( TToken *Token, TSyntax **Expr, TArray *Postponed,
                             char *Op[], int n, int ExprI, int ExpandEq )
{
  if( Token == NULL || Stop ) return Token;

  TSyntax *Expr2 = NULL;
  TArray *pp = new TArray(NULL,0);
  TToken *NextToken = ParseExpr( ExprI+1, Token, &Expr2, pp );

  if( Expr2 != NULL && NextToken != NULL && NextToken != Token &&
      NextToken->type == lexSymbol )
  {
    int i;
    for( i = 0; i < n; i++ )
      if( !strcmp( NextToken->string, Op[i] ) ) break;

    if( i < n )
    {
      TSyntax *Expr3 = NULL;
      TToken *NextToken2 = ParseExpr( ExprI, NextToken->next, &Expr3, pp );

      if( Expr3 != NULL && NextToken2 != NextToken->next )
      {
        // If +=, -= etc. expand to a = a + b...
        if( ExpandEq && Op[i][1] != 0 )
        {
/*          char *Op2 = new char[2];
          sprintf( Op2, "%c", *Op[i] );*/

          BinOpDef op2;

          switch( *Op[i] )
          {
          	case '+' : op2 = BIN_PLUS; break;
          	case '-' : op2 = BIN_MINUS; break;
          	case '*' : op2 = BIN_MUL; break;
          	case '/' : op2 = BIN_DIV; break;
          	case '|' : op2 = BIN_OR; break;
          	case '@' : op2 = BIN_STRCAT; break;
          }

          TBinOp *BinOpI = new TBinOp( op2, Expr2, Expr3 );

//          delete Op2;
          TSyntax *Expr22;
          CopySyntax( &Expr22, Expr2 );
          TBinOp *BinOpO = new TBinOp( BIN_ASSIGN, Expr22, (TSyntax *)BinOpI );
          *Expr = BinOpO;
        }
        else
        {
        	BinOpDef op;
        	if( !strcmp( Op[i], "&&" ) )
          	op = BIN_BOOL_AND;
          else
        	if( !strcmp( Op[i], "||" ) )
          	op = BIN_BOOL_OR;
          else
        	if( !strcmp( Op[i], "|" ) )
          	op = BIN_OR;
          else
        	if( !strcmp( Op[i], "&" ) )
          	op = BIN_AND;
          else
        	if( !strcmp( Op[i], "^" ) )
          	op = BIN_XOR;
          else
        	if( !strcmp( Op[i], "=" ) )
          	op = BIN_ASSIGN;
          else
        	if( !strcmp( Op[i], "==" ) )
          	op = BIN_EQ;
          else
        	if( !strcmp( Op[i], "!=" ) )
          	op = BIN_NEQ;
          else
        	if( !strcmp( Op[i], ">=" ) )
          	op = BIN_GTEQ;
          else
        	if( !strcmp( Op[i], "<=" ) )
          	op = BIN_LTEQ;
          else
        	if( !strcmp( Op[i], ">" ) )
          	op = BIN_GT;
          else
        	if( !strcmp( Op[i], "<" ) )
          	op = BIN_LT;
          else
        	if( !strcmp( Op[i], "+" ) )
          	op = BIN_PLUS;
          else
        	if( !strcmp( Op[i], "-" ) )
          	op = BIN_MINUS;
          else
        	if( !strcmp( Op[i], "*" ) )
          	op = BIN_MUL;
          else
        	if( !strcmp( Op[i], "/" ) )
          	op = BIN_DIV;
          else
        	if( !strcmp( Op[i], "%" ) )
          	op = BIN_MOD;
          else
        	if( !strcmp( Op[i], "@" ) )
          	op = BIN_STRCAT;

          TBinOp *BinOp = new TBinOp( op, Expr2, Expr3 );

          *Expr = reAssoc( (TSyntax *)BinOp );

          //*Expr = BinOp;
        }
        Postponed->Concat( pp );
        pp->KeepElements();
        Kill( (TSyntax **)&pp );
        return NextToken2;
      }
      else
      {
				int line, col;

				if( NextToken2 == NULL )
				{
					line = NextToken->line;
					col = NextToken->column;
				}
				else
				{
					line = NextToken2->line;
					col = NextToken2->column;
				}

        Error( col, line,
               Op[i], "Missing right operand in '%s'" );
        Kill( &Expr2 );
        Kill( &Expr3 );
        Kill( (TSyntax **)&pp );
        return NextToken2;
      }
    }
  }

  Postponed->Concat( pp );
  pp->KeepElements();
  Kill( (TSyntax **)&pp );
  *Expr = Expr2;
  return NextToken;
}

// Parse a comma separated list

TToken *TParser::ParseList( TToken *Token, TArray *Array,
                            TArray *Postponed )
{
  if( Token == NULL || Stop ) return Token;

  int Pos = 0;
  TSyntax *Expr2 = NULL;
  TArray *pp = new TArray(NULL,0);

  TToken *NextToken = ParseExpr( Token, &Expr2, pp );

  if( NextToken != Token && Expr2 != NULL )
  {
    Array->InsertElement( Expr2, Pos++ );
    TToken *LastToken = Token;
    do
    {
      if( TokenIs( NextToken, lexSymbol, "," ) )
      {
        LastToken = NextToken = NextToken->next;
        Expr2 = NULL;
        NextToken = ParseExpr( NextToken, &Expr2, pp );
        Array->InsertElement( Expr2, Pos++ );
      }
      else
        break;
    } while( Expr2 != NULL );

    if( Expr2 != NULL )
    {
      Postponed->Concat( pp );
      pp->KeepElements();
    }
    else
    {
      Error( LastToken->column, LastToken->line, NULL,
             "Expression error in list" );
      // Array is handled by the caller
    }
  }
  else
    Kill( &Expr2 );

  Kill( (TSyntax **)&pp );
  return NextToken;
}

// Parse a single expression

TToken *TParser::ParseExpr( int ExprI, TToken *Token, TSyntax **Expr,
                            TArray *Postponed )
{
  if( Token == NULL || Stop ) return Token;

  switch( ExprI )
  {
    case 0 :
    {
      char *Op[] = { "=", "+=", "-=", "*=", "/=", "|=", "@=" };
      return ParseBinOp( Token, Expr, Postponed, Op, 7, 0, 1 );
    }
    case 1 :
    {
      char *Op[] = { "&&", "||", "|", "^", "&" };
      return ParseBinOp( Token, Expr, Postponed, Op, 5, 1, 0 );
    }
    case 2 :
    {
      char *Op[] = { "!=", "==", ">=", "<=", "<", ">" };
      return ParseBinOp( Token, Expr, Postponed, Op, 6, 2, 0 );
    }
    case 3 :
    {
      char *Op[] = { "+", "-", "@" };
      return ParseBinOp( Token, Expr, Postponed, Op, 3, 3, 0 );
    }
    case 4 :
    {
      char *Op[] = { "*", "/", "%" };
      return ParseBinOp( Token, Expr, Postponed, Op, 3, 4, 0 );
    }
    case 6 : // Unary operations ++, --,  !, -
    {
      if( Token->type == lexSymbol )
      {
        TSyntax *Expr2 = NULL;
        TToken *NextToken;

        int MissingExpr = 0;

        if( !strcmp( Token->string, "++" ) || !strcmp( Token->string, "--" ) )
        {
          NextToken = ParseExpr( 7, Token->next, &Expr2, Postponed );

          if( Expr2 != NULL && NextToken != Token->next )
          {
          	UnOpDef op;

            if( !strcmp( Token->string, "++" ) )
            	op = UN_INC;
            else
            	op = UN_DEC;

            TUnOp *UnOp = new TUnOp( op, Expr2 );
            *Expr = UnOp;
            return NextToken;
          }
          MissingExpr = 1;
        }
        else
        if( !strcmp( Token->string, "!" ) || !strcmp( Token->string, "-" ) )
        {
          NextToken = ParseExpr( 7, Token->next, &Expr2, Postponed );

          if( Expr2 != NULL && NextToken != Token->next )
          {
          	UnOpDef op;

            if( !strcmp( Token->string, "!" ) )
            	op = UN_NOT;
            else
            	op = UN_NEG;

            TUnOp *UnOp = new TUnOp( op, Expr2 );
            *Expr = UnOp;
            return NextToken;
          }
          MissingExpr = 1;
        }

        if( MissingExpr )
        {
          Kill( &Expr2 );
          Error( Token->column, Token->line, Token->string,
                 "Missing expression in '%s'" );
          return NextToken;
        }
      }
      else
      {
        TSyntax *Expr2 = NULL;

        TToken *NextToken = ParseExpr( 7, Token, &Expr2, Postponed );

        if( Expr2 != NULL && NextToken != Token )
        {
          if( NextToken != NULL && NextToken->type == lexSymbol &&
              (!strcmp( NextToken->string, "++" ) ||
               !strcmp( NextToken->string, "--" )) )
          {
          	UnOpDef op;

            if( !strcmp( NextToken->string, "++" ) )
            	op = UN_INC;
            else
            	op = UN_DEC;

/*            char op[3] = " ";
            op[0] = NextToken->string[0];*/
            *Expr = Expr2;
            TSyntax *Expr3;
            CopySyntax( &Expr3, Expr2 );

            TUnOp *UnOp = new TUnOp( op, Expr3 );

            Postponed->InsertElement( UnOp, 0 );
            return NextToken->next;
          }
          else
          {
            *Expr = Expr2;
            return NextToken;
          }
        }
        else
          Kill( &Expr2 );
      }
      return ParseExpr( 7, Token, Expr, Postponed );
    }
    case 7 :  // Subscript
    {
      TSyntax *Expr2 = NULL;

      TToken *NextToken = ParseExpr( 8, Token, &Expr2, Postponed );

      if( Expr2 != NULL && NextToken != Token && TokenIs( NextToken, lexSymbol, "[" ) )
      {
        // Parse indexes
        TArray *IndexList = new TArray( NULL, 0 );
        int ix = 0;
        TArray *pp = new TArray(NULL,0);
        while( TokenIs( NextToken, lexSymbol, "[" ) )
        {
          TSyntax *Index = NULL;
          NextToken = ParseExpr( 0, NextToken->next, &Index, pp );

          if( TokenIs( NextToken, lexSymbol, "]" ) )
          {
            IndexList->InsertElement( Index, ix++ );
            NextToken = NextToken->next;
          }
          else
          {
            Error( Token->column, Token->line, NULL,
                   "Missing subscripting ']'!" );
            Kill( &Expr2 );
            Kill( &Index );
            Kill( (TSyntax **)&IndexList );
            Kill( (TSyntax **)&pp );
            return NextToken;
          }
        }
        TSubscript *Subscript = new TSubscript( Expr2, IndexList );
        Postponed->Concat( pp );
        pp->KeepElements();
        Kill( (TSyntax **)&pp );
        *Expr = Subscript;
        return NextToken;
      }

      *Expr = Expr2;
      return NextToken;
    }
    case 8 :  // Constants, variable, array, paren and function
    {
      // CONST

      if( Token->type == lexNum )
      {
        TNumeric *Numeric = new TNumeric( atoi( Token->string ) );
        *Expr = Numeric;
        return Token->next;
      }

      if( Token->type == lexString )
      {
        TString *string = new TString( Token->string );
        *Expr = string;
        return Token->next;
      }

      if( TokenIs( Token, lexSymbol, ":" ) )
      {
        // Parse function
        TSyntax *Expr2 = NULL;
        TArray *pp = new TArray(NULL,0);
        TToken *nextToken = ParseExpr( 8, Token->next, &Expr2, pp );

        if( Expr2->Type != SFunction )
        {
          Error( Token->column, Token->line, NULL,
                 "Missing function in overload" );
          Kill( (TSyntax **)&Expr2 );
          Kill( (TSyntax **)&pp );
        }
        else
        {
          Postponed->Concat( pp );
          pp->KeepElements();
          Kill( (TSyntax **)&pp );
          *Expr = new TOverload( (TFunction *)Expr2 );
        }
        return nextToken;
      }

      if( TokenIs( Token, lexSymbol, "[" ) )
      {
        TArray *Array = new TArray( NULL, 0 ), *pp = new TArray(NULL,0);
        TToken *NextToken = ParseList( Token->next, Array, pp);

        if( TokenIs( NextToken, lexSymbol, "]" ) )
        {
          Postponed->Concat( pp );
          pp->KeepElements();
          Kill( (TSyntax **)&pp );
          *Expr = Array;
          return NextToken->next;
        }
        else
        {
          Error( Token->column, Token->line, NULL,
                 "Missing subscripting ']'!" );
          Kill( (TSyntax **)&Array );
          Kill( (TSyntax **)&pp );
          return NextToken;
        }
      }

      if( Token->type == lexId )
      {
        if( TokenIs( Token->next, lexSymbol, "(" ) )
        {
          TArray *Arglist = new TArray( NULL, 0 ), *pp = new TArray(NULL,0);
          TToken *NextToken = ParseList( Token->next->next, Arglist, pp );
					Arglist->isCopy = FALSE;

          if( TokenIs( NextToken, lexSymbol, ")" ) )
          {
            TFunction *Function = new TFunction( Token->string, Arglist );
            Postponed->Concat( pp );
            pp->KeepElements();
            Kill( (TSyntax **)&pp );
            *Expr = Function;
            return NextToken->next;
          }
          else
          {
            Error( Token->column, Token->line, Token->string,
                   "Missing ')' in function %s!" );
            Kill( (TSyntax **)&Arglist );
            Kill( (TSyntax **)&pp );
            return NextToken;
          }
        }
        else
        {
          TId *lexId = new TId( Token->string );
          *Expr = lexId;
          return Token->next;
        }
      }

      // (

      if( Token->type == lexSymbol )
      {
        if( !strcmp( Token->string, "(" ) )
        {
          TSyntax *Expr2 = NULL;
          TArray *pp = new TArray(NULL,0);
          TToken *NextToken = ParseExpr( Token->next, &Expr2, pp );

          if( TokenIs( NextToken, lexSymbol, ")" ) )
          {
						if( Expr2 != NULL )
						{
							if( Expr2->Type == SBinOp )
								((TBinOp *)Expr2)->forcePriority = TRUE;
						}

            *Expr = Expr2;
            Postponed->Concat( pp );
            pp->KeepElements();
            Kill( (TSyntax **)&pp );
            return NextToken->next;
          }
          else
          {
            Kill( &Expr2 );
            Kill( (TSyntax **)&pp );
            Error( Token->column, Token->line, NULL, "Missing ')'!" );
            return NextToken;
          }
        }
      }

      break;
    }
    default : return ParseExpr( ExprI+1, Token, Expr, Postponed );
  }

  return Token;
}

// Parse expression from prio 0

TToken *TParser::ParseExpr( TToken *Token, TSyntax **Expr, TArray *Postponed )
{
  if( Token == NULL || Stop ) return Token;

  return ParseExpr( 0, Token, Expr, Postponed );
}

// Parse several expresssions

TToken *TParser::ParseExprns( TToken *Token, TSyntax **Expr, TArray *Postponed )
{
  if( Stop ) return Token;

  TSyntax *Expr1 = NULL;
  TToken *NextToken = ParseExpr( Token, &Expr1, Postponed );

  if( Expr1 != NULL && TokenIs( NextToken, lexSymbol, "," ) )
  {
    TSyntax *Expr2 = NULL;
    NextToken = ParseExprns( NextToken->next, &Expr2, Postponed );
    TExprns *Exprns = new TExprns( Expr1, Expr2 );
    *Expr = (TSyntax *)Exprns;
  }
  else
    *Expr = Expr1;
  return NextToken;
}

// Parse several statements

TToken *TParser::ParseStmts( TToken *Token, TSyntax **Syntax )
{
  return ParseStmtsUntilToken( Token, Syntax, NULL );
}

// Parse several statements until any of the tokens in the StopTokens list
// is encountered

TToken *TParser::ParseStmtsUntilToken( TToken *Token, TSyntax **Syntax,
                                       TToken *StopTokens )
{
  if( Token == NULL || Stop )
    return Token;

  TToken *TestToken = StopTokens;

  while( TestToken != NULL )
  {
    if( TokenIs( Token, TestToken->type, TestToken->string ) )
      return Token;
    TestToken = TestToken->next;
  }

  TToken *LastToken = Token;
  TSyntax *Stmt = NULL;

  Token = ParseStmt( Token, &Stmt );

  if( Token != NULL && LastToken != Token )
  {
    TSyntax *StmtList = NULL;
    Token = ParseStmtsUntilToken( Token, &StmtList, StopTokens );
    if( StmtList != NULL )
    {
      TStmts *Stmts = new TStmts( Stmt, StmtList );

      *Syntax = Stmts;
    }
    else
      *Syntax = Stmt;
  }
  else
    *Syntax = Stmt;

  return Token;
}

// Parse a statement

TToken *TParser::ParseStmt( TToken *Token, TSyntax **Syntax )
{
  if( Token == NULL || Stop ) return Token;

  if( Token->type == lexWord )
  {
    // IF [..ELSE]

    if( !strcmp( Token->string, "if" ) )
    {
      if( TokenIs( Token->next, lexSymbol, "(" ) )
      {
        TSyntax *Expr = NULL, *Stmt = NULL;
        TArray *Postponed = new TArray(NULL,0);
        TToken *NextToken1 = ParseExpr( Token->next->next, &Expr, Postponed );

        if( Expr == NULL )
        {
          Kill( (TSyntax **)&Postponed );
          Error( Token->column, Token->line, NULL,
                 "Missing expression in 'if'!" );
          return NextToken1;
        }

        if( TokenIs( NextToken1, lexSymbol, ")" ) )
        {
          TToken *NextToken2 = ParseStmt( NextToken1->next, &Stmt );

          if( Stmt != NULL )
          {
            if( TokenIs( NextToken2, lexWord, "else" ) )
            {
              // ELSE
              TSyntax *Stmt2 = NULL;

              TToken *NextToken3 = ParseStmt( NextToken2->next, &Stmt2 );

              AddPostponed( &Stmt, &Postponed, BEFORE );
              AddPostponed( &Stmt2, &Postponed, BEFORE );
              TIfElse *IfElse = new TIfElse( Expr, Stmt, Stmt2 );
              *Syntax = IfElse;
              return NextToken3;
            }
            else
            {
              // IF

              if( Postponed->Size != 0 )
              {
                // We have to make it an if..else statement in order
                // to run the postponed statements if if fails

                AddPostponed( &Stmt, &Postponed, BEFORE );
                TIfElse *IfElse = new TIfElse( Expr, Stmt, Postponed );
                *Syntax = IfElse;
              }
              else
              {
                AddPostponed( &Stmt, &Postponed, BEFORE );
                TIf *If = new TIf( Expr, Stmt );
                *Syntax = If;
              }
              return NextToken2;
            }
          }
          else
          {
            Kill( &Expr );
            Kill( &Stmt );
            Kill( (TSyntax **)&Postponed );
            Error( Token->column, Token->line, NULL,
                   "Missing body in 'if'" );
            return NextToken2;
          }
        }
        else
        {
          Kill( (TSyntax **)&Postponed );
          Error( Token->column, Token->line, NULL,
                 "Right paranthesis missing in \"if\"!" );
          return NextToken1;
        }
      }
      else
      {
        Error( Token->column, Token->line, NULL,
               "Left paranthesis missing in \"if\"!" );
        return Token->next;
      }
    }
    else  // FOR
    if( !strcmp( Token->string, "for" ) )
    {
      if( TokenIs( Token->next, lexSymbol, "(" ) )
      {
        TSyntax *Arg[3] = {NULL, NULL, NULL};
        TArray *pp[3] = { new TArray(NULL,0), new TArray(NULL,0),
                          new TArray(NULL,0)};
        TToken *NextToken = Token->next->next;
				TToken *prevToken;
				
        int i;
        for( i = 0; i < 3; i++ )
        {
					prevToken = NextToken;
          NextToken = ParseExprns( NextToken, &Arg[i], pp[i] );
          if( i < 2 )
          {
            if( TokenIs( NextToken, lexSymbol, ";" ) )
              NextToken = NextToken->next;
            else
            {
              Error( prevToken->column, prevToken->line, NULL,
                     "Missing ';' in for-loop" );
              for( int n = 0; n < 3; n++ )
              {
                Kill( &Arg[n] );
                Kill( (TSyntax **)&pp[n] );
              }
              return NextToken;
            }
          }
        }

        if( TokenIs( NextToken, lexSymbol, ")" ) )
        {
          TSyntax *Stmts = NULL;
          NextToken = ParseStmt( NextToken->next, &Stmts );

          for( int i = 0; i < 3; i++ )
            AddPostponed( &Arg[2], &pp[i], AFTER );
          TFor *For = new TFor( Arg[0], Arg[1], Arg[2], Stmts );
          *Syntax = For;
          return NextToken;
        }
        else
        {
          for( int n = 0; n < 3; n++ )
          {
            Kill( &Arg[i] );
            Kill( (TSyntax **)&pp[i] );
          }
          Error( NextToken->column, NextToken->line, NULL,
                 "Missing ')' in for-loop" );
          return NextToken;
        }
      }
      else
      {
        Error( Token->column, Token->line, NULL,
               "Missing '(' in for-loop" );
        return Token->next;
      }
    }
    else  // WHILE
    if( !strcmp( Token->string, "while" ) )
    {
      if( TokenIs( Token->next, lexSymbol, "(" ) )
      {
        TSyntax *Expr = NULL;
        TArray *Postponed = new TArray(NULL,0);
        TToken *NextToken = ParseExpr( 0, Token->next->next, &Expr, Postponed );

        if( Expr != NULL )
        {
          if( TokenIs( NextToken, lexSymbol, ")" ) )
          {
            TSyntax *Block = NULL;
            NextToken = ParseStmt( NextToken->next, &Block );

            AddPostponed( &Block, &Postponed, AFTER );
            TFor *For = new TFor( NULL, Expr, NULL, Block );
            *Syntax = For;
            return NextToken;
          }
          else
          {
            Kill( &Expr );
            Kill( (TSyntax **)&Postponed );
            Error( NextToken->column, NextToken->line, NULL,
                   "Missing ')' in while-loop" );
            return NextToken;
          }
        }
        else
        {
          Error( NextToken->column, NextToken->line, NULL,
                 "Missing expresion in while" );
          return NextToken->next;
        }
      }
      else
      {
        Error( Token->column, Token->line, NULL, "Missing '(' in while-loop" );
        return Token->next;
      }
    }
    else  // DO..WHILE
    if( !strcmp( Token->string, "do" ) )
    {
      TSyntax *Block = NULL;
      TToken *NextToken = ParseStmt( Token->next, &Block );

      if( Block != NULL )
      {
        if( !strcmp( NextToken->string, "while" ) )
        {
          if( TokenIs( NextToken->next, lexSymbol, "(" ) )
          {
            TSyntax *Expr = NULL;
            TArray *Postponed = new TArray(NULL,0);
            NextToken = ParseExpr( 0, NextToken->next->next, &Expr, Postponed );

            if( Expr != NULL )
            {
              if( TokenIs( NextToken, lexSymbol, ")" ) )
              {
                AddPostponed( &Expr, &Postponed, AFTER );
                TDo *Do = new TDo( Expr, Block );
                *Syntax = Do;
                return NextToken->next;
              }
              else
              {
                Kill( &Block );
                Kill( &Expr );
                Kill( (TSyntax **)&Postponed );
                Error( NextToken->column, NextToken->line, NULL,
                       "Missing ')' in while-loop" );
                return NextToken;
              }
            }
            else
            {
              Kill( &Block );
              Error( NextToken->next->next->column, NextToken->next->next->line, NULL,
                     "Missing expresion in do..while" );
              return NextToken->next->next;
            }
          }
          else
          {
            Kill( &Block );
            Error( NextToken->next->column, NextToken->next->line, NULL,
                   "Missing '(' in while-loop" );
            return NextToken->next;
          }
        }
        else
        {
          Kill( &Block );
          Error( NextToken->column, NextToken->line, NULL,
                 "Missing while in do..while-loop" );
          return NextToken;
        }
      }
      else
      {
        Error( NextToken->column, NextToken->line, NULL,
               "Missing body in do..while-loop" );
        return NextToken;
      }
    }
    else  // SWITCH
    if( !strcmp( Token->string, "switch" ) )
    {
      TSyntax *Stmt = NULL;
      TArray *Case = new TArray( NULL, 0 ), *Block = new TArray( NULL, 0 ),
             *ppStmt = new TArray( NULL, 0 );
      int Cases = 0;

      TToken *NextToken = Token->next;

      if( TokenIs( NextToken, lexSymbol, "(" ) )
      {
        NextToken = NextToken->next;
        NextToken = ParseExpr( NextToken, &Stmt, ppStmt );

        TToken *DefaultToken = new TToken();
        DefaultToken->type = lexWord;
        DefaultToken->line = DefaultToken->column = 0;
        char *str = "default";
        DefaultToken->string = new char[strlen(str)+1];

        strcpy( DefaultToken->string, str );
        DefaultToken->next = NULL;

        TToken *CaseToken = new TToken();
        CaseToken->type = lexWord;
        CaseToken->line = CaseToken->column = 0;
        str = "case";
        CaseToken->string = new char[strlen(str)+1];

        strcpy( CaseToken->string, str );
        CaseToken->next = DefaultToken;

        if( Stmt != NULL )
        {
          if( TokenIs( NextToken, lexSymbol, ")" ) )
          {
            NextToken = NextToken->next;
            if( TokenIs( NextToken, lexSymbol, "{" ) )
            {
              NextToken = NextToken->next;
              while( 1 )
              {
                if( TokenIs( NextToken, lexWord, "case" ) )
                {
                  NextToken = NextToken->next;
                  TSyntax *Expr = NULL;
                  TArray *pp = new TArray( NULL, 0 );
                  NextToken = ParseExpr( NextToken, &Expr, pp );

                  if( Expr != NULL )
                  {
                    if( TokenIs( NextToken, lexSymbol, ":" ) )
                    {
                      NextToken = NextToken->next;
                      // Insert a case
                      TSyntax *Stmts = NULL;
                      NextToken = ParseStmtsUntilToken( NextToken, &Stmts,
                                                        CaseToken );

                      AddPostponed( &Stmts, &pp, BEFORE );

                      Case->InsertElement( Expr, Cases );
                      Block->InsertElement( Stmts, Cases );
                      Cases++;
                    }
                    else
                    {
                      Kill( &Expr );
                      Kill( (TSyntax **)&pp );
                      Error( NextToken->column, NextToken->line, NULL,
                             "Missing ':' in case" );
                      break;
                    }
                  }
                  else
                  {
                    Kill( (TSyntax **)&pp );
                    Error( NextToken->column, NextToken->line, NULL,
                           "Missing expression in case" );
                    break;
                  }
                }
                else
                if( TokenIs( NextToken, lexWord, "default" ) )
                {
                  NextToken = NextToken->next;
                  if( TokenIs( NextToken, lexSymbol, ":" ) )
                  {
                    NextToken = NextToken->next;
                    // Insert a case
                    TSyntax *Stmts = NULL;
                    NextToken = ParseStmtsUntilToken( NextToken, &Stmts,
                                                      CaseToken );

                    Case->InsertElement( NULL, Cases );
                    Block->InsertElement( Stmts, Cases );
                    Cases++;
                  }
                  else
                  {
                    Error( NextToken->column, NextToken->line, NULL,
                           "Missing ':' in default" );
                    break;
                  }
                }
                else
                {
                  if( !TokenIs( NextToken, lexSymbol, "}" ) )
                    Error( NextToken->column, NextToken->line, NextToken->string,
                           "Unrecognized word '%s' in switch" );
                  break;
                }
              } // while

              if( TokenIs( NextToken, lexSymbol, "}" ) )
              {

                AddPostponed( &Stmt, &ppStmt, AFTER );
                *Syntax = (TSyntax *)new TSwitch( Stmt, Case, Block );
								delete CaseToken;	// ********
				        delete DefaultToken;
                return NextToken->next;
              }
              else
                Error( NextToken->column, NextToken->line, NextToken->string,
                       "Missing '}' in switch" );
            }
            else
              Error( NextToken->column, NextToken->line, NULL,
                     "Missing '{' in switch" );
          }
          else
            Error( NextToken->column, NextToken->line, NULL,
                   "Missing ')' in switch" );
        }
        else
          Error( NextToken->column, NextToken->line, NULL,
                 "Missing expression in switch" );
        delete CaseToken;
        delete DefaultToken;
      }
      else
        Error( NextToken->column, NextToken->line, NULL,
               "Missing '(' in switch" );
      Kill( &Stmt );
      Kill( (TSyntax **)&Case );
      Kill( (TSyntax **)&Block );
      Kill( (TSyntax **)&ppStmt );
      return NextToken;
    }
    else
    if( !strcmp( Token->string, "continue" ) )  // CONTINUE
    {
      TContinue *Continue = new TContinue;
      *Syntax = Continue;
      if( TokenIs( Token->next, lexSymbol, ";" ) )
        return Token->next->next;
      return Token->next;
    }
    else
    if( !strcmp( Token->string, "break" ) ) // BREAK
    {
      TBreak *Break = new TBreak;
      *Syntax = Break;
      if( TokenIs( Token->next, lexSymbol, ";" ) )
        return Token->next->next;
      return Token->next;
    }
    else
    if( !strcmp( Token->string, "return" ) )  // RETURN
    {
      TToken *NextToken;
      TSyntax *Expr = NULL;
      TArray *Postponed = new TArray(NULL,0);

      NextToken = ParseExpr( 0, Token->next, &Expr, Postponed );

      AddPostponed( &Expr, &Postponed, AFTER );
      TReturn *Return = new TReturn( Expr );
      *Syntax = Return;
      if( TokenIs( NextToken, lexSymbol, ";" ) )
        return NextToken = NextToken->next;
      return NextToken;
    }
    else
    {
      Error( Token->column, Token->line, Token->string, "Misplaced %s!" );
      return Token->next;
    }
  }

  // BLOCK

  if( Token->type == lexSymbol )
  {
    if( !strcmp( Token->string, "{" ) )
    {
      TSyntax *Stmts = NULL;
      TToken *NextToken = ParseStmts( Token->next, &Stmts );

      if( TokenIs( NextToken, lexSymbol, "}" ) )
      {
        if( Stmts == NULL )
          Stmts = new TNoOp();
        *Syntax = Stmts;
        return NextToken->next;
      }
      else
      {
        Kill( &Stmts );
        Error( Token->column, Token->line, NULL, "Missing '}'!" );
        return NextToken;
      }
    }
  }

  // Parse expresions

  TToken *LastToken = Token;
  TArray *Postponed = new TArray(NULL,0);

  Token = ParseExprns( Token, Syntax, Postponed );

  if( TokenIs( Token, lexSymbol, ";" ) )
  {
    AddPostponed( Syntax, &Postponed, AFTER );
    return Token->next;
  }
  else
  {
    if( LastToken != Token )
      Error( LastToken->column, LastToken->line, NULL, "Missing ';'!" );
    Kill( (TSyntax **)&Postponed );
  }

  return Token;
}

// Parse a program including functions, function defs and global variables

TToken *TParser::Parse( TToken *Token )
{
  while( Token )
  {
    if( Stop ) break;
    if( Token->type == lexId ) // GLOBAL VARIABLE
    {
      if( TokenIs( Token->next, lexSymbol, "=" ) )
      {
        // Variable init

        TSyntax *Value = NULL;
        TArray *Postponed = new TArray(NULL,0);

        TToken *NextToken = ParseExpr( 0, Token->next->next, &Value, Postponed );

        if( Value != NULL )
        {
          // Add to globals
          AddPostponed( &Value, &Postponed, AFTER );

          // The value should be regarded as a copy in order to
          // be deleted on exit

          if( Value->Type == SNumeric || Value->Type == SString ||
              Value->Type == SArray )
            SetCopyFlag( Value, COPY );

          Global.Update( Token->string, &Value, Module, DONTKEEP );
        }
        else
        {
          Kill( (TSyntax **)&Postponed );
          Error( Token->next->next->column, Token->next->next->line,
                 Token->string, "Missing expression in assignment '%s'" );
          Token = Token->next;
        }

        Token = NextToken;
      }
      else
      if( TokenIs( Token->next, lexSymbol, "(" ) ) // FUNCTION
      {
        // Function

        TSyntax *Stmt = NULL;
        TId **ArgList = NULL;

        TToken *NextToken = Token->next->next;
        int error = 0, Args = 0, Reference = 0, Size = 0;

        while( 1 )
        {
          Reference = 0;
          if( NextToken->type == lexSymbol && !strcmp( NextToken->string, "&" ) )
          {
            Reference = 1;
            NextToken = NextToken->next;
          }

          if( NextToken->type == lexId )
          {
            TId *lexId = new TId( NextToken->string );
            lexId->Reference = Reference;
            ArgList = (TId **)AddToList( (TSyntax **)ArgList, &Size, (TSyntax *)lexId,
                                 Args );
            Args++;

            NextToken = NextToken->next;

            if( TokenIs( NextToken, lexSymbol, "," ) )
              NextToken = NextToken->next;

            continue;
          }

          if( !TokenIs( NextToken, lexSymbol, ")" ) )
          {
            for( int i = 0; i < Args; i++ )
              Kill( (TSyntaxPtr *)&ArgList[i] );
            free( ArgList );
            Error( NextToken->column, NextToken->line, Token->string,
                   "Missing ')' in '%s'" );
            Token = NextToken;
            error = 1;
            break;
          }
          else
          {
            NextToken = NextToken->next;
            break;
          }
        }

        if( error ) continue;

        if( TokenIs( NextToken, lexSymbol, ";" ) ) // FUNCTION DEFINITION
        {
          if( !FunctionList.Update( Token->string, Args, (TSyntaxPtr *)ArgList,
                                     NULL, Module, -1 ) )
          {
            for( int i = 0; i < Args; i++ )
              Kill( (TSyntaxPtr *)&ArgList[i] );
            free( ArgList );
            Error( Token->column, Token->line, Token->string,
                   "'%s' mismatch previous function definition!" );
          }
          Token = NextToken->next;
        }
        else  // FUNCTION
        {
          NextToken = ParseStmt( NextToken, &Stmt );

          if( Stmt != NULL )
          {
            // Assign function

            if( !FunctionList.Update( Token->string, Args, (TSyntaxPtr *)ArgList,
                                      Stmt, Module, -1 ) )
            {
              Error( Token->column, Token->line, Token->string,
                     "'%s' mismatch previous function definition!" );
              for( int i = 0; i < Args; i++ )
                Kill( (TSyntax **)&ArgList[i] );
              free( ArgList );
              Kill( &Stmt );
            }
            Token = NextToken;
          }
          else
          {
            for( int i = 0; i < Args; i++ )
              Kill( (TSyntax **)&ArgList[i] );
            free( ArgList );
            Error( Token->column, Token->line, Token->string,
                   "Expression error in '%s'" );
            Token = NextToken;
          }
        }
      }
      else
      {
        Error( Token->next->column, Token->next->line, Token->next->string,
               "Expression error in '%s'!" );
        Token = Token->next->next;
      }
    }
    else
    {
      Error( Token->column, Token->line, Token->string,
             "Unrecognized token '%s'!" );
      Token = Token->next;
    }

    if( TokenIs( Token, lexSymbol, ";" ) ) Token = Token->next;
  }
  return Token;
}

void TArray::Concat( TArray *Add, int MakeCopy )
{
  for( int i = 0; i < Add->Size; i++ )
  {
    if( MakeCopy )
      InsertElement( GetCopy(Add->Array[i]), Size );
    else
      InsertElement( Add->Array[i], Size );
  }
}

TArray::~TArray()
{
  if( Size > 0 )
  {
    if( KillElements )
      for( int i = 0; i < Size; i++ )
        Kill( &Array[i] );
    free( (void *)Array );
  }
};



