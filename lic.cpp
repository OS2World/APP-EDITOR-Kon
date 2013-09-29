#include <string.h>
#include <stdlib.h>
#include "lic.h"

int scramble( char *string, char *result )
{
  if( strlen( string ) < 6 ) return 0;

  unsigned left = 0, right = strlen( string ) - 1, toggle = 0;
  unsigned sum = 0;

  while( left < right )
  {
    int leftRes = string[left] ^ (!toggle ? 0xff : 0);
    int rightRes = string[right] ^ (toggle ? 0xff : 0);
    sum += (leftRes * rightRes);
    left++;
    right--;
  }
  sum = (sum << 1) | (sum >> 15);

  char sumstr[30];

  itoa( sum, sumstr, 10 );

  int rl = strlen( result );

  int pos = 0;
  int dashes = 0;
  int dashPos = rl;

  for( int i = 0; i < strlen(sumstr); i++, dashPos++ )
  {
    unsigned char b = sum >> ((i % 8) * 8);

    // Check if not A-Z, a-z, 0-9
    while( !((b >= 'a' && b <= 'z') || (b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9')) )
    {
      unsigned char lastVal = b;
      if( b < 'A' ) b = 'A' + b;
      if( b > 'z' ) b = b - (255 - 'z');
      if( b == lastVal )
        b = 'A';
    }

    pos = rl+i*2+dashes;

    if( dashPos > 0 && dashPos % 2 == 0 )
    {
      dashes++;
      result[pos] = '-';
      pos++;
    }

    result[pos] = b;
    result[pos+1] = sumstr[i];
  }
  result[pos+2] = 0;

  if( strlen( result ) < MAXREGCODELEN )
  {
    char rec[MAXREGCODELEN+1];
    strcpy( rec, result );
    scramble( rec, result );
  }
  result[MAXREGCODELEN] = 0;
  return 1;
}
