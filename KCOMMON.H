/******************************************************************************
 * KCommonCommands holds all K commands that are always valid in the editor.
 * It also initiates all other K commands.
 */

#if !defined KCOMMON

#include "bowl.h"
#include "k.h"
#include "keval.h"
#include "keybind.h"
#include "kcmdexec.h"
#include "kerrcoll.h"

class KCommonCommands : public KCommandExecutor
{
  static KCommonCommands *instance;
  enum BuildMenu { MAINMENU, POPUPMENU };
  BuildMenu buildMenu;
  BOOL errorOccured;
public:
  static void createInstance();

  static KCommonCommands *getInstance()
  {
    return instance;
  }

  // Support for
  //
  // bindKey( keySeq, command )
  // print( data )
  // TNumeric length( array )

  virtual TSyntax *eval( char *name, TVariableList *vars, TEval *eval, int index );

  // Replaces \ with \\ in a string. Returns a pointer to the new string
  // which should be deleted by the caller after use, unless no backslashes
  // are found: then fixString returns string.

  char *fixString( char *string );

  // Both returns FALSE if fail
  // returns in in param
  BOOL syntaxToInt( TSyntax *syntax, int &integer );

  // returns a newly allocated string in param, which does not need to be copied
  BOOL syntaxToString( TSyntax *syntax, char **string );
};

class CaptureThread : public Thread
{
  MessagePipe *pipe;
  int controlId;
public:
  CaptureThread( MessagePipe *pipe, int controlId ) : Thread(TRUE), pipe(pipe), controlId(controlId) {}
  virtual void run();
};

#define KCOMMON

#endif
