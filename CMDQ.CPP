#include "bowl.h"
#include "kerrcoll.h"
#include "editctrl.h"

void ShellExecutor::switchExecutor( KCommandExecutor *newExecutor )
{
	executor->setRunning( FALSE );
	executor = newExecutor;
	executor->setRunning( TRUE );
}

void ShellExecutor::setStartExecRunning( BOOL isRunning ) 
{
	executor = originalExecutor;
  this->isRunning = isRunning;
  originalExecutor->setRunning( isRunning );
}

ShellExecutor::~ShellExecutor()
{
}

void CommandQueue::addCommand( char *command )
{
  if( commands >= MAXCOMMANDSINQUEUE )
    return;

  // Add the new command to the ring

  editCmdRingSem.request();

  int endPos = (atPos+commands) % MAXCOMMANDSINQUEUE;
  commands++;
  this->command[endPos] = new char[strlen(command)+1];

  strcpy( this->command[endPos], command );

  editCmdRingSem.release();

  if( executor->inCommand() && editControl != NULL )
    editControl->showBusy( TRUE ); // Visually show busy for the user

  if( ready )
    newCmdSem.post();
}

void CommandQueue::run()
{
  idleSem.post();
  while( !terminate )
  {
    // Wait for incoming command

    newCmdSem.wait();
    newCmdSem.reset();

    if( record && !recordingStarted )
    {
      recordingStarted = TRUE;
      if( macroCode ) free( macroCode );

//      char *init = "recordedMacro()\n{\n  startUndoBlock();\n";
			char *init = "  startUndoBlock();\n";
      macroCode = (char *)malloc( strlen( init ) + 1 );
      strcpy( macroCode, init );
      macroSize = strlen( init );
    }

    while( commands ) // execute commands in queue
    {
      if( terminate )
      {
        // delete all remaining commands

        while( commands )
        {
          delete command[atPos];
          commands--;

          atPos = (atPos+1) % MAXCOMMANDSINQUEUE;
        }

        break;
      }

      editCmdRingSem.request();

      if( record )
      {
        // Do not add commands that brings up search dialog or file dialogs
				// Do not add signals.
        // This may not always work, but it's the best we can do

        if( strstr( command[atPos], "searchDialog" ) == NULL &&
            strstr( command[atPos], "replaceDialog" ) == NULL &&
            strstr( command[atPos], "undoAvailable" ) == NULL &&
            strstr( command[atPos], "redoAvailable" ) == NULL &&
            strstr( command[atPos], "blockMarked" ) == NULL &&
            strstr( command[atPos], "columnBlockSet" ) == NULL &&
            strstr( command[atPos], "viewChanged" ) == NULL &&
            strstr( command[atPos], "copyMacroToClipboard" ) == NULL &&
            strstr( command[atPos], "fileDialog" ) == NULL )
        {
          lastCommandStart = macroSize;
          macroSize += strlen( command[atPos] ) + 3;
          macroCode = (char *)realloc( (void *)macroCode, macroSize + 1 );
          strcat( macroCode, "  " );
          strcat( macroCode, command[atPos] );
          strcat( macroCode, "\n" );
        }
      }

      // Start a K evaluator, and send the executor as argument

      KErrorCollector::getInstance()->clearErrorCount();
      TStream *stream = new TStream( command[atPos], DATASTREAM );

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

      delete command[atPos];
      commands--;

      atPos = (atPos+1) % MAXCOMMANDSINQUEUE;

      editCmdRingSem.release();

      // Evaluate the syntax tree if no parse error

      if( KErrorCollector::getInstance()->GetErrorCount() == 0 )
      {
        idleSem.reset();
      	executor->setStartExecRunning( TRUE );
        eval = new TEval( NULL, executor );

        KillCopy( eval->Eval( code ) );

      	executor->setStartExecRunning( FALSE );

        if( editControl != NULL )
          editControl->showBusy( FALSE );

        TEval *deleteEval = eval;	// For terminateCommand
        eval = NULL;

        delete deleteEval;
        idleSem.post();
      }

      Kill( &code );
    }

    Application::processMessages();
  }
  if( macroCode )
    free( macroCode );
  delete executor;
	if( editControl != NULL ) 
	{
		if( !frameClose )
			Application::executeFromMainThread( ControllerWindow::getInstance(), 4, editControl );
		else
			Application::executeFromMainThread( ControllerWindow::getInstance(), 5, editControl );
	}
}

void CommandQueue::terminateCommand()
{
  // Could this go wrong? Oh, yes. Could crash, so another semaphore might be
  // good to have here
  // CTrl+F4 would trigger the code below..
  //if( idleSem.check() == 0 )
  //  KErrorCollector::getInstance()->message( Strings::get(241));

  if( eval != NULL )
    eval->haltExecution();
}

void CommandQueue::endRecord()
{
  if( !record ) return;
  record = FALSE;
	macroCode = (char *)realloc( (char *)macroCode, macroSize + 30 );
  char *moduleCode = (char *)malloc( macroSize + 35 );

  // Remove last command (endMacroRecord or playMacro) or some other
  // code doing the same thing

  macroCode[lastCommandStart] = 0;

	strcat( macroCode, "  endUndoBlock();\n" );

	strcpy( moduleCode, "recordedMacro()\n{\n" );
	strcat( moduleCode, macroCode );
  strcat( moduleCode, "}\n" );

  if( macroModule )
    delete macroModule;

  macroModule = new TModule( moduleCode, DATASTREAM, KErrorCollector::getInstance() );

	delete moduleCode;

  macroModule = NULL;
}

