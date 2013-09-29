#include "bowl.h"

void ClipBoard::setText( char *text, int length )
{
  WinOpenClipbrd( Application::hab );

  PCHAR dText, eText = NULL;

  /* Allocate a shared memory object for the text data. */
  DosAllocSharedMem( (PPVOID)&dText, NULL, length+1,
                     PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE );

  memcpy( dText, text, length );
  dText[length] = 0;

  if( strlen( dText ) != length )
  {
    DosAllocSharedMem( (PPVOID)&eText, NULL, length+1+sizeof(int),
                       PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE );

    memcpy( eText, &length, sizeof(int) );
    memcpy( &eText[sizeof(int)], text, length );
  }

  /* Clear old data from the clipboard. */
  WinEmptyClipbrd( Application::hab );

  WinSetClipbrdData( Application::hab, (ULONG)dText, CF_TEXT, CFI_POINTER);

  if( eText != NULL )
    WinSetClipbrdData( Application::hab, (ULONG) eText, CF_DSPTEXT,
                       CFI_POINTER);

  WinCloseClipbrd( Application::hab );

  DosFreeMem( dText );

  if( eText != NULL )
    DosFreeMem( eText );
}

int ClipBoard::getText( char **text )
{
  WinOpenClipbrd( Application::hab );

  // Check if there's data available in native format

  PCHAR data = (PCHAR)WinQueryClipbrdData( Application::hab, CF_DSPTEXT );

  int len = 0;

  if( data != 0 )
  {
    memcpy( &len, data, sizeof(int) );
    *text = new char[len];
    memcpy( *text, &data[sizeof(int)], len );
  }
  else
  {
    data = (PCHAR)WinQueryClipbrdData( Application::hab, CF_TEXT );
    if( data != 0 )
    {
      len = strlen(data);
      *text = new char[len+1];
      strcpy( *text, data );
    }
  }

  WinCloseClipbrd( Application::hab );

  return len;
}

