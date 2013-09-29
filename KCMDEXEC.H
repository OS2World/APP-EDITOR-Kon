#if !defined KCMDEXEC

#include "k.h"
#include "keval.h"

/******************************************************************************
 * The KCommandExecutor executes a built in K command. Normally all EditControls
 * should inherit from this class.
 * The K evaluator has one instance of this class as the current
 * KCommandExecutor. As the program evaluates, this may be switched to a
 * different instance. When an EditControl receives a keystroke that should
 * execute a K program, it passes itself as the current KCommandExecutor.
 */

class KCommandExecutor
{
protected:
	BOOL isRunning;	// TRUE if in the eval method
public:
  // name is the ascii name of the function.
  // eval is the evaluator which called this method
  virtual TSyntax *eval( char *name, TVariableList *vars, TEval *eval, int index )
    { return NULL; }
  KCommandExecutor() : isRunning(FALSE) {}
  virtual ~KCommandExecutor() {}
  virtual BOOL inCommand() { return isRunning; }
  virtual void setRunning( BOOL isRunning ) { this->isRunning = isRunning; }
	virtual void switchExecutor( KCommandExecutor *newExecutor ) {}
	void runCommand( char *command )
	{
		TStream *stream = new TStream( command, DATASTREAM );
		TLexer *lexer = new TLexer( stream, KErrorCollector::getInstance() );
		TParser *parser = new TParser( NULL, KErrorCollector::getInstance() );

		// Pass command string through lexer

		TToken *token = lexer->lexer();

		TSyntax *code;

		// Parse the startup command and get the abstract syntax tree

		parser->ParseStmts( token, &code );
		delete stream;
		delete lexer;
		delete parser;

		// Evaluate the syntax tree

		TEval *eval = new TEval( NULL, this );

		KillCopy( eval->Eval( code ) );

		delete eval;

		Kill( &code );
	}
};

#define KCMDEXEC

#endif
