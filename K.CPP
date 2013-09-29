#define K_MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "bowl.h"

#include "k.h"

class TVariableList Global;
class TFunctionList FunctionList;

#include "keval.h"

int Commands = 0;
TCommand *Command = NULL;

void TLexer::setupTokens()
{
  char *orgSymbols = "<>=+-!*/%&|^~@";
  symbols = new char[strlen(orgSymbols)+1];
  strcpy( symbols, orgSymbols );
  char *orgSpecials = "(),;:[]{}";
  specials = new char[strlen(orgSpecials )+1];
  strcpy( specials, orgSpecials );
  char *orgWords[11] = { "if", "else", "while", "for", "break", "continue",
                         "do", "return", "switch", "case", "default" };

  wordsCount = 11;
  words = (char **)malloc( sizeof( char * ) * wordsCount );
  for( int i = 0; i < wordsCount; i++ )
  {
    words[i] = new char[strlen( orgWords[i] ) + 1];
    strcpy( words[i], orgWords[i] );
  }
}

void TLexer::includeFile( char *file )
{
  char *filename = file;

  if( *file != '\\' )
  {
    // Set up relative path
    filename = new char[strlen(stream->getFilename())+strlen(file)+1];
    strcpy( filename, stream->getFilename() );
    char *lastBackslash = strrchr( filename, '\\' );
    if( lastBackslash != NULL )
    {
      *(lastBackslash+1) = 0;
      strcat( filename, file );
    }
    else
      strcpy( filename, file );
  }

  TModule *module = new TModule( filename, FILESTREAM, errorCollector );
  if( !module->success() )
    delete module;

  if( filename != file ) delete filename;
}

// Insert an item in an array

TSyntaxPtr *AddToList( TSyntaxPtr *List, int *Size, TSyntax *Item, int Pos )
{
  int oldSize = *Size;
  if( *Size <= Pos )
  {
    (*Size) *= 2;
    if( *Size == 0 )
      *Size = Pos+1;
    List = (TSyntaxPtr *)realloc( (void *)List, sizeof( TSyntaxPtr ) * (*Size+1) );
  }
  if( oldSize-Pos > 0 )
    memmove( &List[Pos+1], &List[Pos], (oldSize-Pos) * sizeof( TSyntaxPtr ) );
  List[Pos] = Item;
  return List;
}

// Build an argumentlist for a function definition in commandlist

TSyntaxPtr *BuildArgList( TSyntaxPtr *ArgList, char *VarName, int Reference,
                          int NumArgs )
{
  ArgList = (TSyntaxPtr *)realloc( (void *)ArgList,
                                   sizeof( TSyntaxPtr ) * (NumArgs + 1) );
  TId *lexId = new TId( VarName );
  ArgList[NumArgs] = lexId;
  lexId->Reference = Reference;
  return ArgList;
}

void TVariableList::Update( char *Name, TSyntaxPtr *Value, TModule *Module,
                            int Reference )
{
  int i = FindIndex( Name );

  if( i == -1 ) // The variable should be added to the list
  {
    for( i = 0; i < Variables; i++ )
    {
      if( strcmp( Name, Variable[i].Name ) < 0 )
        break;
    }

    Variables++;

    Variable = (TVariable *)realloc( (void *)Variable,
                                     sizeof( TVariable ) * Variables );

    memmove( (void *)&Variable[i+1], (void *)&Variable[i],
             (Variables - i - 1) * sizeof( TVariable ) );

    Variable[i].Name = (char *)malloc( strlen( Name ) + 1 );

    strcpy( Variable[i].Name, Name );

    if( !Reference )
    {
      Variable[i].Value = new TSyntaxPtr;
      *Variable[i].Value = GetCopy( *Value );
    }
    else
      Variable[i].Value = Value;

    Variable[i].Reference = Reference;
    Variable[i].Module = Module;
  }
  else
  {
    Kill( Variable[i].Value );
    *Variable[i].Value = GetCopy( *Value );
  }

  if( !Reference && 
      ((*Variable[i].Value)->Type == SNumeric ||
       (*Variable[i].Value)->Type == SString) )
        // Reset so we always get copies of variables
      ((TConst *)(*Variable[i].Value))->isCopy = ORIGINAL;
}

int TVariableList::FindIndex( char *Name )
{
  int Pos = Variables / 2, Base = 0, Top = Variables, LastPos = -1, Res;

  if( Variables == 0 ) return -1;

  while( LastPos != Pos )
  {
    Res = strcmp( Name, Variable[Pos].Name );

    LastPos = Pos;

    if( Res != 0 && Base == Top ) return -1;

    if( Res < 0 )
    {
      Top = Pos;
      Pos = Base + (Pos - Base) / 2;
    }
    else
    if( Res > 0 )
    {
      Base = Pos;
      Pos += (Top - Pos) / 2;
    }
    else
      return Pos;
  }

  return -1;
}

TSyntaxPtr *TVariableList::LookUp( char *Name )
{
  int Index = FindIndex( Name );

  if( Index == -1 ) return NULL;

  return Variable[Index].Value;
}

void TVariableList::Remove( char *Name )
{
  int Index = FindIndex( Name );

  if( Index == -1) return;  // Not found

  if( !Variable[Index].Reference )
  {
    Kill( Variable[Index].Value );
    delete Variable[Index].Value;
  }

  free( Variable[Index].Name );

  memmove( (void *)&Variable[Index], (void *)&Variable[Index+1],
           (Variables - Index - 1) * sizeof( TVariable ) );

  Variables--;
}

TVariableList::~TVariableList()
{
  int v = Variables;
  for( int i = 0; i < v; i++ )
    Remove( Variable[0].Name );
	free( Variable );
}

int TFunctionList::FindIndex( char *Name )
{
  int Pos = Commands / 2, Base = 0, LastPos = -1, Top = Commands, Res;

  if( Commands == 0 ) return -1;

  while( LastPos != Pos )
  {
    Res = strcmp( Name, Command[Pos].Name );

    LastPos = Pos;

    if( Res != 0 && Base == Top ) 
      return -1;

    if( Res < 0 )
    {
      Top = Pos;
      Pos = Base + (Pos - Base) / 2;
    }
    else
    if( Res > 0 )
    {
      Base = Pos;
      Pos += (Top - Pos) / 2;
    }
    else
    {
      // Find the first occurence
      while( Pos >= 0 && !strcmp( Command[Pos].Name, Name ) ) Pos--;
      return Pos+1;
    }
  }

  return -1;
}

int TFunctionList::Update( char *Name, int NumArgs, TSyntaxPtr *ArgList,
                           TSyntax *Code, TModule *Module, int index )
{
  int i = FindIndex( Name );

  if( i == -1 ) // The function should be added to the list
  {
    for( i = 0; i < Commands; i++ )
    {
      if( strcmp( Name, Command[i].Name ) < 0 )
        break;
    }
  }
/*  else
  {
    // Rename the old function

    char *newName = (char *)malloc( strlen( Command[i].Name ) + 2 );
    strcpy( newName, "_" );
    strcat( newName, Command[i].Name );

    // add the renamed command

    Update( newName, Command[i].NumArgs, Command[i].Args,
            Command[i].Code, Command[i].Module );

    // Find the index of the old command again

    i = FindIndex( Name );

    free( (void *)Command[i].Name );

    // Place the command first in the list

    while( i > 0 && !strcmp( Command[i-1].Name, Name ) ) i--;
  }*/

  Commands++;

  Command = (TCommand *)realloc( (TCommand *)Command,
                                   sizeof( TCommand ) * Commands );

  memmove( (void *)&Command[i+1], (void *)&Command[i],
           (Commands - i - 1) * sizeof( TCommand ) );

  Command[i].Name = (char *)malloc( strlen( Name ) + 1 );

  strcpy( Command[i].Name, Name );

  Command[i].Code = Code;
  Command[i].Args = ArgList;
  Command[i].NumArgs = NumArgs;
  Command[i].Module = Module;
  Command[i].overloads = 0;
  Command[i].overloadsAlloc = 0;
  Command[i].evalOverloaded = NULL;
  Command[i].index = index;

  return 1;
}

// Returns the index of the function to execute
// Returns -1 if the function is not found
//         -2 if no function to overload exists
int TFunctionList::updateOverloaded( void *evaluator, char *fncName, BOOL add )
{
  int functionIdx = FindIndex( fncName );

  if( functionIdx == -1 )
    return -1;

  // Find evaluator
  BOOL found = FALSE;
  int evalPos = 0;

  for( evalPos = 0; evalPos < Command[functionIdx].overloads; evalPos++ )
  {
    if( Command[functionIdx].evalOverloaded[evalPos].evaluator == evaluator )
    {
      found = TRUE;
      break;
    }
  }

  if( add && !found )
  {
    // Try and find an empty slot
    for( int i = 0; i < Command[functionIdx].overloads; i++ )
    {
      if( Command[functionIdx].evalOverloaded[i].evaluator == NULL )
      {
        found = TRUE;
        evalPos = i;
        break;
      }
    }

    if( Command[functionIdx].overloads >= Command[functionIdx].overloadsAlloc && !found)
    {
      Command[functionIdx].overloadsAlloc = (Command[functionIdx].overloadsAlloc + 1) * 2;
      Command[functionIdx].evalOverloaded = (TCommandOverload *)realloc( Command[functionIdx].evalOverloaded, sizeof( TOverload * ) * Command[functionIdx].overloadsAlloc );
      evalPos = Command[functionIdx].overloads;
      Command[functionIdx].overloads++;
    }

    Command[functionIdx].evalOverloaded[evalPos].evaluator = evaluator;
    Command[functionIdx].evalOverloaded[evalPos].overloads = 0;
  }
  else
  if( !add && found )
  {
    Command[functionIdx].evalOverloaded[evalPos].overloads--;
    if( Command[functionIdx].evalOverloaded[evalPos].overloads == 0 )
      Command[functionIdx].evalOverloaded[evalPos].evaluator = NULL;
  }
  
  if( add )
  {
    // Update overload counter to reference forward in the list
    Command[functionIdx].evalOverloaded[evalPos].overloads++;

    // Check that there is an overloaded function

    if( strcmp( fncName,
                Command[functionIdx+Command[functionIdx].evalOverloaded[evalPos].overloads].Name ) )
      return -2;

    return functionIdx+Command[functionIdx].evalOverloaded[evalPos].overloads;
  }
  return -3;
}

void TFunctionList::Remove( char *Name )
{
  int Index = FindIndex( Name );

  if( Index == -1) return;  // Not found

  if( Command[Index].Code )
    Kill( &Command[Index].Code );

  free( Command[Index].Name );

  for( int i = 0; i < Command[Index].NumArgs; i++ )
    Kill( &Command[Index].Args[i] );

  memmove( (void *)&Command[Index], (void *)&Command[Index+1],
           (Commands - Index - 1) * sizeof( TCommand ) );

  Commands--;
}

int TFunctionList::getCount()
{
  return Commands;
}

TCommand *TFunctionList::getFunction( int i )
{
  return &Command[i];
}

TModule::TModule( char *data, int type, TErrorCollector *ErrorCollector, BOOL preload ) : fail(0)
{
  this->preload = preload;
  TStream *Stream;

  if( type == FILESTREAM )
  {
    filename = new char[strlen( data ) + 1];
    strcpy( filename, data );
    Stream = new TStream( data, FILESTREAM );
  }
  else
  {
    char *internal = "<internal>";
    filename = new char[strlen( internal ) + 1];
    strcpy( filename, internal );
    Stream = new TStream( data, DATASTREAM );
  }

  if( !Stream->openSuccess() )
  {
    char msg[255];
    sprintf( msg, "Failed to open '%s'", data );
    ErrorCollector->notice( msg );
    fail = 1;
    return;
  }
  TLexer *Lexer = new TLexer( Stream, ErrorCollector );
  TParser *Parser = new TParser( this, ErrorCollector );

  TToken *Token = Lexer->lexer();

#ifdef DEBUG
  FILE *f = fopen( "test2", "w" );

  Next = Token;

  while( Next != NULL )
  {
    fprintf( f, "%d,%d: '%s' ", Next->column, Next->line, Next->string );
    Next = Next->next;
  }

  fclose( f );
#endif

  Parser->Parse( Token );

  delete Parser;
  delete Lexer;
  delete Stream;
};

TModule::~TModule()
{
  // Remove all global variables belonging to this module

  int i = 0;

  while( i < Global.Variables )
  {
    if( Global.Variable[i].Module == this )
      Global.Remove( Global.Variable[i].Name );
    else
      i++;
  }

  // Remove all functions

  i = 0;

  while( i < Commands )
  {
    if( Command[i].Module == this )
      FunctionList.Remove( Command[i].Name );
    else
      i++;
  }
  delete filename;
}

/*void PrintSyntax( TSyntax *Syntax )
{
  if( Syntax == NULL ) return;
  switch( Syntax->Type )
  {
    case SNumeric :
    {
      printf( "%d", ((TNumeric *)Syntax)->num );
      break;
    }
    case SString:
    {
      printf( "\"%s\"", ((TString *)Syntax)->string );
      break;
    }
    case SIdentifier :
    {
      printf( ((TId *)Syntax)->string );
      break;
    }
    case SBinOp :
    {
      printf( "%s (", ((TBinOp *)Syntax)->Operator );
      PrintSyntax( ((TBinOp *)Syntax)->Left );
      printf( ") (" );
      PrintSyntax( ((TBinOp *)Syntax)->Right );
      printf( ")" );
      break;
    }
    case SUnOp :
    {
      printf( "%s (", ((TUnOp *)Syntax)->Operator );
      PrintSyntax( ((TUnOp *)Syntax)->Expr );
      printf( ")" );
      break;
    }
    case SExprns :
    {
      printf( "Exprns (" );
      PrintSyntax( ((TExprns *)Syntax)->Expr );
      printf( ") (" );
      PrintSyntax( ((TExprns *)Syntax)->Exprns );
      printf( ")" );
      break;
    }
    case SStmts :
    {
      printf( "Stmts (" );
      PrintSyntax( ((TStmts *)Syntax)->Stmt );
      printf( ") (" );
      PrintSyntax( ((TStmts *)Syntax)->Stmts );
      printf( ")" );
      break;
    }
    case SIf :
    {
      printf( "If (" );
      PrintSyntax( ((TIf *)Syntax)->Arg );
      printf( ") (" );
      PrintSyntax( ((TIf *)Syntax)->Block );
      printf( ")" );
      break;
    }
    case SIfElse :
    {
      printf( "IfElse (" );
      PrintSyntax( ((TIfElse *)Syntax)->Arg );
      printf( ") (" );
      PrintSyntax( ((TIfElse *)Syntax)->Block1 );
      printf( ") (" );
      PrintSyntax( ((TIfElse *)Syntax)->Block2 );
      printf( ")" );
      break;
    }
    case SFor :
    {
      printf( "For (" );
      PrintSyntax( ((TFor *)Syntax)->Arg[0] );
      printf( ") (" );
      PrintSyntax( ((TFor *)Syntax)->Arg[1] );
      printf( ") (" );
      PrintSyntax( ((TFor *)Syntax)->Arg[2] );
      printf( ") (" );
      PrintSyntax( ((TFor *)Syntax)->Block );
      printf( ")" );
      break;
    }
    case SDo :
    {
      printf( "Do (" );
      PrintSyntax( ((TDo *)Syntax)->Arg );
      printf( ") (" );
      PrintSyntax( ((TDo *)Syntax)->Block );
      printf( ")" );
      break;
    }
    case SBreak :
    {
      printf( "Break" );
      break;
    }
    case SReturn :
    {
      printf( "Return (" );
      PrintSyntax( ((TReturn *)Syntax)->Expr);
      printf( ")" );
      break;
    }
    case SSubscript :
    {
      printf( "Subscript (" );
      PrintSyntax( ((TSubscript *)Syntax)->Array);
      printf( ") (" );
      PrintSyntax( ((TSubscript *)Syntax)->Index);
      printf( ")" );
      break;
    }
    case SArray :
    {
      printf( "[");
      for( int i = 0; i < ((TArray *)Syntax)->Size; i++ )
      {
        PrintSyntax( ((TArray *)Syntax)->Array[i]);

        if( i != ((TArray *)Syntax)->Size - 1 )
          printf( ", " );
      }
      printf( "]" );
      break;
    }
    case SFunction:
    {
      printf( "Function (%s) (", ((TFunction *)Syntax)->Name );
      PrintSyntax( (TSyntax *)((TFunction *)Syntax)->Args);
      printf( ") " );
      break;
    }
    default : printf( "UNKNOWN (PRINTSYNTAX)" );
  }
}
*/
