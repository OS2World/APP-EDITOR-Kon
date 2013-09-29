#include <stdio.h>
#include <time.h>
#include "lic.h"

#define TEXT \
  "%s,\n\n" \
  "Thank you for registering Kon!\n\n" \
  "This is your license information:\n\n" \
  "        Name: %s\n" \
  " License key: %s\n\n" \
  "Please don't hesitate to contact me if you have questions about Kon!\n\n" \
  "Also consider joining the support forum for Kon at:\n" \
  "   http://groups.yahoo.com/group/Kon-Editor\n\n" \
  "Best regards,\n" \
  "Björn Andersson\n"

int main( int argc, char *argv[] )
{
  char name[100];
  char result[MAXREGCODELEN*2];
  result[0] = 0;
  printf( "Enter name: " );
  gets( name );
  printf( "\n----------------------------\n\n" );
  scramble( name, result );
  printf( TEXT, name, name, result );

  FILE *f = fopen( "reg.log", "a+t" );
  time_t ltime;

  time( &ltime );
  fprintf( f, "%s\n   Name: %s\n    Key: %s\n\n------------------------\n", ctime( &ltime ), name, result );
  fclose( f );
}