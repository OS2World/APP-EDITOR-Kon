#include "kerrcoll.h"
#include "kcommon.h"

/******************************************************************************
 * This class executes the startup script for Kon.
 *
 * Preconditions: The KeyBindings class should be initialized
 */

class Kstartup
{
public:
  Kstartup()
  {
    // Create a new module object and parse the startup.k file

    KErrorCollector::getInstance()->clearErrorCount();
    KErrorCollector::getInstance()->notice( Strings::get(49) );

    char filename[MAXPATH+1];
    sprintf( filename, "%s%s", KonSystem::kDir, "startup.k" );

    TModule *module = new TModule( filename, FILESTREAM, 
                                   KErrorCollector::getInstance() );

    if( module->success() )
    {
      // Create startup command

			KCommonCommands::getInstance()->runCommand( "startKon();" );
			KCommonCommands::getInstance()->runCommand( "setupToolbar();" );

/*      TStream *stream = new TStream( "startKon();", DATASTREAM );
      TLexer *lexer = new TLexer( stream, KErrorCollector::getInstance() );
      TParser *parser = new TParser( module, KErrorCollector::getInstance() );

      // Pass command string through lexer

      TToken *token = lexer->lexer();

      TSyntax *code;

      // Parse the startup command and get the abstract syntax tree

      parser->ParseStmts( token, &code );
      delete stream;
      delete lexer;
      delete parser;

      // Evaluate the syntax tree

      TEval *eval = new TEval( module, KCommonCommands::getInstance() );

      KillCopy( eval->Eval( code ) );

      delete eval;

      Kill( &code );*/
    }
    else
      KErrorCollector::getInstance()->notice( Strings::get(50) );
  }
};
