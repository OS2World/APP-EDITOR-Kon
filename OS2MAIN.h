#define INCL_DOSPROCESS
#include "bowl.h"

int konMain( int argc, char *argv[] );

int main( int argc, char *argv[] )
{
  Application::initialize();

  // DEBUG
//  argv[0] = new char[100];
//  strcpy( argv[0], "d:\\source\\kon\\kon2.exe" );
  // END DEBUG

  int retval = konMain( argc, argv );

  WinDestroyMsgQueue( Application::hmq );
  WinTerminate( Application::hab );

  return retval;
}
