#include "bowl.h"

void ClipBoard::setText( char *text, int length )
{
  OpenClipboard( NULL );

//  PCHAR dText, eText = NULL;
  HGLOBAL hdText, heText;
  char *dText, *eText = NULL;

  // Allocate a shared memory object for the text data.
  hdText = GlobalAlloc( GMEM_DDESHARE, length+1 );

  dText = (char *)GlobalLock( hdText );
  memcpy( dText, text, length );
  dText[length] = 0;
  GlobalUnlock( hdText );

  if( strlen( dText ) != length )
  {
    heText = GlobalAlloc( GMEM_DDESHARE, length+sizeof(int)+4 );

    eText = (char *)GlobalLock( heText );
    strncpy( eText, "KON", 3 );
    memcpy( &eText[3], &length, sizeof(int) );
    memcpy( &eText[sizeof(int)+3], text, length );
    GlobalUnlock( heText );
  }

  // Clear old data from the clipboard.
  EmptyClipboard();

  SetClipboardData( CF_TEXT, dText );

  if( eText != NULL )
    SetClipboardData( CF_DSPTEXT, eText );

  CloseClipboard();
}

int ClipBoard::getText( char **text )
{
  HGLOBAL hdata;
  char *data;

  if( !OpenClipboard( NULL ) ) return 0;  // Failed to open
  
  DWORD err = GetLastError();

  int len = 0;

  // Check if there's data available in native format
  if( IsClipboardFormatAvailable( CF_DSPTEXT ) )
  {
    hdata = GetClipboardData( CF_DSPTEXT );
    data = (char *)GlobalLock( hdata );

    if( !strncmp(data,"KON",3) )
    {
      memcpy( &len, data+3, sizeof(int) );
      *text = new char[len];
      memcpy( *text, &data[sizeof(int)+3], len );
      GlobalUnlock( hdata );
      CloseClipboard();
      return len;
    }
    GlobalUnlock( hdata );
  }

  err = GetLastError();

  hdata = GetClipboardData( CF_TEXT );
  err = GetLastError();

  if( hdata != NULL )
  {
    data = (char *)GlobalLock( hdata );
    len = strlen(data);
    *text = new char[len+1];
    strcpy( *text, data );
    GlobalUnlock( hdata );
  }

  CloseClipboard();

  return len;
}

