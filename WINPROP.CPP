#include "bowl.h"
#include "srchdlg.h"
#include "repldlg.h"
#include "printdlg.h"
#include "print.h"
#include "mainmenu.h"
#include "mainwnd.h"

Vector<WinProperty> WinProperties::properties;

int WinProperties::loadProperties( char *filename )
{
  FILE *f = fopen( filename, "rb" );
  if( f == NULL ) return 1;

  int items;
  fread( &items, sizeof( items ), 1, f );

  for( int i = 0; i < items; i++ )
  {
    int l;
    fread( &l, sizeof( l ), 1, f );

    char *name = new char[l];
    int x, y, width, height;

    fread( name, l, 1, f );
    fread( &x, sizeof( x ), 1, f );
    fread( &y, sizeof( y ), 1, f );
    fread( &width, sizeof( width ), 1, f );
    fread( &height, sizeof( height), 1, f );
    properties.add( new WinProperty(name, x, y, width, height ) );
    delete name;
  }
  fclose( f );
  return 0;
}

int WinProperties::retrieveProperty( char *name, int &x, int &y, int &width, int &height )
{
  WinProperty *winProperty = new WinProperty( name, 0, 0, 0, 0 );
  WinProperty *foundProperty =
    (WinProperty *)properties.findObjectOnEq( winProperty );
  delete winProperty;
  if( foundProperty == NULL ) return FALSE;

  x = foundProperty->getX();
  y = foundProperty->getY();
  width = foundProperty->getWidth();
  height = foundProperty->getHeight();

  if( y + height > Desktop::getInstance()->getHeight() )
  {
    y = Desktop::getInstance()->getHeight() - height;
    if( y < 0 )
    {
      y = 0;
      height = Desktop::getInstance()->getHeight();
    }
  }

  if( x + width > Desktop::getInstance()->getWidth() )
  {
    x = Desktop::getInstance()->getWidth() - width;
    if( x < 0 )
    {
      x = 0;
      width = Desktop::getInstance()->getWidth();
    }
  }
  return TRUE;
}

int WinProperties::saveProperties( char *filename )
{
  FILE *f = fopen( filename, "wb" );
  if( f == NULL ) return 1;

  int l = properties.getSize();
  fwrite( &l, sizeof( l ), 1, f );

  for( int i = 0; i < properties.getSize(); i++ )
  {
    WinProperty *winProp = properties.getObjectAt(i);
    l = strlen( winProp->getName() ) + 1;
    fwrite( &l, sizeof( l ), 1, f );
    fwrite( winProp->getName(), strlen( winProp->getName() ) + 1, 1, f );
    l = winProp->getX();
    fwrite( &l, sizeof( l ), 1, f );
    l = winProp->getY();
    fwrite( &l, sizeof( l ), 1, f );
    l = winProp->getWidth();
    fwrite( &l, sizeof( l ), 1, f );
    l = winProp->getHeight();
    fwrite( &l, sizeof( l ), 1, f );
  }
  fclose( f );
  return 0;
}

BOOL Strings::loadStrings( char *stringsFile )
{
  char input[1000];
  int current = 1;
  FILE *f = fopen( stringsFile, "r" );

  if( f == NULL )
    error = TRUE;
  else
    while( !feof( f ) )
    {
      if( fgets( input, 1000, f ) == NULL ) break;
      // Remove CR+LF
      int i = strlen( input ) - 1;
      while( i > 0 && (input[i] == 0xa || input[i] == 0xd) ) i--;
      input[i+1] = 0;

      // Get the string number

      for( i = 0; i < strlen(input); i++ )
        if( input[i] == ':' ) break;

      // See that we're on the correct number

      input[i] = 0;

      int shouldBe = atoi( input );

      if( shouldBe != current )
      {
        error = TRUE;
        break;
      }

      current++;
      i++;  // Move past ':'

      PropString *propString = new PropString( &input[i+1] );
      strings.add( propString );
    }
  fclose( f );

  if( error )
  {
    char str[100];

    sprintf( str, "string properties error at line %d", current );
    PopupMessage popupMessage( Desktop::getInstance(),
                               "Strings properties error",
                               str, PopupMessage::error );
    return TRUE;
  }
  return FALSE;
}

BOOL Strings::error = FALSE;
Strings  *Strings::stringsInstance = NULL;
Vector<PropString> Strings::strings;
char Strings::errString[50];

char *Strings::get( int number )
{
	if( number > strings.getSize() || number < 1 )
  {
  	sprintf( errString, "%d not found", number );
    return errString;
  }
  return strings.getObjectAt(number-1)->string;
};

BOOL ClassProperties::saveProperties( char *filename )
{
	FILE *f = fopen( filename, "wb" );

  if( f == NULL ) return FALSE;

  PrinterGraphics::saveSettings( f );
  SearchDialog::saveSettings( f );
  ReplaceDialog::saveSettings( f );
  PrintDialog::saveSettings( f );
  Print::saveSettings( f );
	MainMenu::saveSettings( f );
	Settings::saveSettings( f );
	MainWindow::saveSettings( f );

  fclose( f );

  return TRUE;
}

BOOL ClassProperties::restoreProperties( char *filename )
{
	FILE *f = fopen( filename, "rb" );

  if( f == NULL ) return FALSE;

  PrinterGraphics::restoreSettings( f );
  SearchDialog::restoreSettings( f );
  ReplaceDialog::restoreSettings( f );
  PrintDialog::restoreSettings( f );
  Print::restoreSettings( f );
	MainMenu::restoreSettings( f );
	Settings::restoreSettings( f );
	MainWindow::restoreSettings( f );
  fclose( f );

  return TRUE;
}

BOOL ClassProperties::savePropertyName( FILE *f, char *propNameAndVer, long *pos )
{
	char version[100];
	long size = 0;

	strcpy( version, propNameAndVer );
	fwrite( version, 100, 1, f );

	*pos = ftell( f );

	fwrite( &size, sizeof( long ), 1, f );

	return TRUE;
}

BOOL ClassProperties::savePropertySize( FILE *f, long pos )
{
	long size = 0, lastPos;

	lastPos = ftell( f );
	fseek( f, pos, SEEK_SET );

	size = lastPos - pos;
	fwrite( &size, sizeof( long ), 1, f );
	fseek( f, lastPos, SEEK_SET );
 	return TRUE;
}

BOOL ClassProperties::checkPropertyExists( FILE *f, char *propNameAndVer )
{
	char version[100];
	long pos, size;
	
	fread( version, 100, 1, f );

	if( strncmp( version, propNameAndVer, strlen( propNameAndVer ) - 3 ) )	// NOT EXISTING
	{
		pos = ftell( f );
		pos -= 100;
		fseek( f, pos, SEEK_SET );
		return FALSE;
	}

	fread( &size, sizeof( long ), 1, f );

	if( strcmp( version, propNameAndVer ) )	// WRONG VERSION
	{
		pos = ftell( f );
		pos += size;
		fseek( f, pos, SEEK_SET );
		return FALSE;
	}
	return TRUE;
}
