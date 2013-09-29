/******************************************************************************
 * Holds the properties for a window with a specific name
 * It is held in the singleton WinProperties
 */

#if !defined PROPERTIES
#define PROPERTIES

class WinProperty : public AObject
{
  char *name;
  int x, y, width, height;
public:
  WinProperty( char *name, int x, int y, int width, int height )
  {
    this->name = new char[strlen( name ) + 1];
    strcpy( this->name, name );
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
  }
  char *getName() { return name; }
  int getX() { return x; }
  int getY() { return y; }
  int getWidth() { return width; }
  int getHeight() { return height; }
  void setProperties( int x, int y, int width, int height )
  {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
  }
  virtual int equals( AObject *object )
  {
    return !strcmp( ((WinProperty *)object)->getName(), name );
  }
  ~WinProperty()
  {
    delete name;
  }
};

/* Holds properties for all windows in the system (which register).
 * This makes it easy for a window to restore it's last position and size
 */

class WinProperties : public AObject
{
  static Vector<WinProperty> properties;
public:
  static int loadProperties( char *filename );
  // Add or update a window property
  static void updateProperty(  char *name, int x, int y, int width, int height )
  {
    WinProperty *winProperty = new WinProperty( name, x, y, width, height );
    WinProperty *oldProperty;

    // Check if the name is already in the list

    if( (oldProperty =
          (WinProperty *)properties.findObjectOnEq( winProperty )) == NULL )
      properties.add( winProperty );  // No add
    else
    {
      // Yes, update
      delete winProperty;
      oldProperty->setProperties( x, y, width, height );
    }
  }

  // Retrieve properties for a certain window
  // Note that the parameters are not modified if the property is not found

  static int retrieveProperty( char *name, int &x, int &y, int &width, int &height );
  static int saveProperties( char *filename );
  static void clearProperties()
  {
    properties.removeAll();
  }
};

/******************************************************************************
 * Holds strings. Strings are loaded from a specific resource using loadStrings
 *
 * The strings file is built up like:
 *
 * 1: string number 1
 * 2: string number 2
 * .
 * .
 * n: string number n
 *
 * Note: that none of the number may be skipped. This results in an error.
 *       that strings should start at 1.
 */

#include <stdio.h>

class PropString : public AObject
{
public:
  char *string;
  PropString( char *string )
  {
    this->string = new char[strlen(string)+1];
    strcpy( this->string, string );
  }
	~PropString() { delete string; }
};

class Strings : public AObject
{
  static Strings *stringsInstance;
  static BOOL error;
  static Vector<PropString> strings;
  static char errString[50];
public:
  // Returns FALSE if something went wrong
  static BOOL loadStrings( char *stringsFile );
  static char *get( int number );
};

class ClassProperties : public AObject
{
public:
	// Last three digits in propNameAndVer must be version number
	static BOOL savePropertyName( FILE *f, char *propNameAndVer, long *pos );
	static BOOL savePropertySize( FILE *f, long pos );
	static BOOL checkPropertyExists( FILE *f, char *propNameAndVer );
	static BOOL saveProperties( char *file );
  static BOOL restoreProperties( char *file );
};

#endif