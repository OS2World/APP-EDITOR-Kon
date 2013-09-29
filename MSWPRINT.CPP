#include "bowl.h"
#include "winprop.h"

PRINTER_INFO_2 *PrinterGraphics::pInfo = NULL;
DWORD PrinterGraphics::printers = 0;
DEVMODE *PrinterGraphics::devmode = NULL;
int PrinterGraphics::devmodeSize = 0;
char PrinterGraphics::currentPrinter[300];

PrinterGraphics::PrinterGraphics( char *documentName, char *printer ) : Graphics()
{
  HANDLE handle;
  
  outsideDC = TRUE;

  // Find the printer

  int i;
  for( i = 0; i < printers; i++ )
  	if( !strcmp( printer, pInfo[i].pPrinterName ) ) break;

	// Setup the printer if we have a valid structure

	OpenPrinter( printer, &handle, NULL );

  if( strcmp( currentPrinter, printer ) || devmode == NULL )
  {
    long devmodeSize = DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, NULL, NULL, 0 );

  	devmode = (PDEVMODE)malloc( devmodeSize );

  	DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, devmode, NULL, DM_OUT_BUFFER );
	  DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, devmode, devmode, DM_IN_BUFFER|DM_OUT_BUFFER );
  }

  devmode->dmColor = DMCOLOR_COLOR;

	hdc = CreateDC( pInfo[i].pDriverName, pInfo[i].pPrinterName, NULL, (devmode!=NULL) ? devmode : NULL );

  width = getWidth();
  height = getHeight();

	if( !strcmp( currentPrinter, printer ) && devmode != NULL )
  {
  	// This is the same printer which we have device mode setup for
    // Setup printer
    DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, NULL, devmode, DM_IN_BUFFER );
 	}

  ClosePrinter( handle );

	DOCINFO docInfo;

  docInfo.cbSize = sizeof(DOCINFO);
  docInfo.lpszDocName = documentName;
  docInfo.lpszOutput = NULL;
  docInfo.lpszDatatype = NULL;
  docInfo.fwType = 0;

  StartDoc( hdc, &docInfo );
  StartPage( hdc );
}

void PrinterGraphics::getPrinters(char ***list, int &count )
{
  DWORD foo = 0, err, bufSize = 0, flags;
	BOOL res;
	printers = 0;

  flags = PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS;

  EnumPrinters( flags, NULL, 2, NULL, 0, &bufSize, &printers );

  if( bufSize == 0 ) 
  {
    count = 0;
    return;
  }

  if( pInfo != NULL ) GlobalFree( pInfo );

  pInfo = (PRINTER_INFO_2 *)GlobalAlloc( GPTR, bufSize );

  res = EnumPrinters( flags, NULL, 2, (LPBYTE)pInfo,
                bufSize, &foo, &printers );

	err = GetLastError();

  if( res == 0 )
  {
    count = 0;
    return;
  }
	
  *list = (char **)malloc( sizeof( char * ) * printers );

  count = printers;

	for( int i = 0; i < printers; i++ )
  {
  	(*list)[i] = (char *)malloc( strlen( pInfo[i].pPrinterName ) + 1 );
    strcpy( (*list)[i], pInfo[i].pPrinterName );
  }
}

void PrinterGraphics::setupPrinter( char *name )
{
	HANDLE handle;

  int i;
  for( i = 0; i < printers; i++ )
  	if( !strcmp( name, pInfo[i].pPrinterName ) ) break;

  if( i == printers ) return;

	BOOL res = OpenPrinter( pInfo[i].pPrinterName, &handle, NULL );

	if( devmode == NULL )
  {
    devmodeSize = DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, NULL, NULL, 0 );

  	devmode = (PDEVMODE)malloc( devmodeSize );

  	DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, devmode, NULL, DM_OUT_BUFFER );
 	}

	DocumentProperties( HWND_DESKTOP, handle, pInfo[i].pDriverName, devmode, devmode, DM_IN_BUFFER|DM_OUT_BUFFER|DM_PROMPT );

  strcpy( currentPrinter, pInfo[i].pPrinterName );
  
  ClosePrinter( handle );
}

void PrinterGraphics::formfeed()
{
	EndPage( hdc );
}

int PrinterGraphics::getWidth()
{
	return GetDeviceCaps( hdc, HORZRES );
}

int PrinterGraphics::getHeight()
{
	return GetDeviceCaps( hdc, VERTRES );
}

int PrinterGraphics::getXRes()
{
	return GetDeviceCaps( hdc, HORZSIZE );
}

int PrinterGraphics::getYRes()
{
	return GetDeviceCaps( hdc, VERTSIZE );
}

void PrinterGraphics::terminateDocument()
{
	AbortDoc( hdc );
}

void PrinterGraphics::endDocument()
{
	EndDoc( hdc );
}

void PrinterGraphics::saveSettings( FILE *f )
{
	long pos;
	if( !ClassProperties::savePropertyName( f, "PRNGRAPH001", &pos ) ) return;
	fwrite( currentPrinter, 300, 1, f );
	fwrite( &devmodeSize, sizeof(devmodeSize), 1, f );
  if( devmode != NULL )
  	fwrite( devmode, devmodeSize, 1, f );
	ClassProperties::savePropertySize( f, pos );
}

void PrinterGraphics::restoreSettings( FILE *f )
{
	if( !ClassProperties::checkPropertyExists( f, "PRNGRAPH001" ) ) return;
	fread( currentPrinter, 300, 1, f );
	fread( &devmodeSize, sizeof(devmodeSize), 1, f );
  if( devmodeSize != 0 )
  {
  	devmode = (DEVMODE *)malloc(devmodeSize);
  	fread( devmode, devmodeSize, 1, f );
  }
}

PrinterGraphics::~PrinterGraphics()
{
  DeleteDC( hdc );
}
