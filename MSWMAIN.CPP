#include "bowl.h"

int konMain( int argc, char *argv[] );

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdLine, int showState )
{
  int argc;
  char **argv, startupPath[300];
  LPTSTR cmdline = GetCommandLine();

  Application::initialize( hInstance );

  argv = (char **)malloc( sizeof( char *) );

  for( argc = 0;;argc++ )
  {
    while( *cmdline == ' ' ) cmdline++;

    argv = (char **)realloc( argv, sizeof( char *) * (argc+2) );

    if( !*cmdline ) break;

    if( *cmdline == '"' )
    {
      cmdline++;
      argv[argc] = cmdline;
      while( *cmdline && *cmdline != '"' ) cmdline++;
      if( *cmdline )
        *cmdline++ = 0;
    }
    else
    {
      argv[argc] = cmdline;
      while( *cmdline && *cmdline != ' ' ) cmdline++;
      if( *cmdline )
        *cmdline++ = 0;
    }
  }

  GetModuleFileName( NULL, startupPath, 299 );

  argv[0] = startupPath;

  return konMain( argc, argv );
}
