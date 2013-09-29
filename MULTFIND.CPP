#include "bowl.h"
#include "kerrcoll.h"
#include "cntrladm.h"
#include "editctrl.h"
#include "multfind.h"

BOOL MultipleFilesFind::operateFile( char *filename, View *view, BOOL isVisible )
{
  // Add whole document flag

  strcat( options, "d" );

  BOOL found = view->findString( searchString, options, FALSE );

  options[strlen(options)-1] = 0;

  if( found )
  {
    while( found )
    {
      char line[161];
      char *msg = new char[strlen( filename ) + 160 + 30];
      int len = 160;

      view->getCurrentLine( line, len );

      sprintf( msg, "%s (%d, %d) : %s", filename, view->getLine(),
                                        view->getColumn(), line );

      KErrorCollector::getInstance()->message( msg );
      delete msg;
      found = view->findString( searchString, options, FALSE );
    }
  }
/*  else
    KErrorCollector::getInstance()->message( filename );*/

  return TRUE;
}

MultipleFilesFind::MultipleFilesFind( char *searchString, char *options ) :
  searchString(searchString)
{
  this->options = new char[strlen(options)+2];
  strcpy( this->options, options );

  // Remove whole document flag

  char *d = strchr( this->options, 'd' );
  if( d )
    *d = '_';
}