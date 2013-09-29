#include <stdio.h>

/******************************************************************************
 * Bitmap:
 * Holds a bitmap. It should be created from Graphics.readBitmap(..)
 */

class Bitmap : public AObject
{
  int width, height;
public:
  HBITMAP hbmBitmap;
  Bitmap( HBITMAP hbmBitmap, int width, int height )
  {
    this->hbmBitmap = hbmBitmap;
    this->width = width;
    this->height = height;
  }
  int getWidth() { return width; }
  int getHeight() { return height; }
  void switchBackgroundColor( ColorValue ) {}
};

/******************************************************************************
 * Graphics
 * Every window has a Graphics object which can be used for drawing in the
 * window.
 * Note that the Graphics object that is passed to the paint() function is
 * a new Graphics object.
 *
 * The user should not create or inherit from this class.
 */

class Graphics : public AGraphics
{
protected:
  static FontList *pfl;
  void freeFontList();

  struct SelectFont
  {
    int id;
    Font *font;
    int X, Y, points;
    int attr;
    SelectFont *next;
    int charHeight, aveCharWidth, baseline;
    LONG spaceWidth;
    SelectFont( int id, Font *font, int X, int Y, int points, int attr,
                SelectFont *next )
    {
      this->id = id;
      this->font = font;
      this->X = X;
      this->Y = Y;
      this->points = points;
      this->attr = attr;
      this->next = next;
    }
  } *selectedFont;

  SelectFont *currentFont;
  int currentId;

  static char *editorFont, *hexEditorFont;

  void collectFontMetrics();
	static ULONG getOSVER();
  static char *helvFont, *wsFont;
public:
  HPS hps;
  static int dialogDesignFontHeight, dialogDesignFontWidth;
  Graphics( HPS hps );
  Graphics() : /*pfl(NULL), */selectedFont(NULL),
               currentId(1), currentFont(NULL) {}
  void setColor( ColorValue color )
  {
    codePage = 850;
    fc = color;
    GpiSetColor( hps, fc );
  };
  void setBackColor( ColorValue color )
  {
    bc = color;
    GpiSetBackColor( hps, bc );
  };
  void drawString( int x, int y, STRING str );
  void drawString( int x, int y, Rect *rect, int options, STRING str );
  void drawString( int x, int y, int length, STRING str );
  void drawString( int x, int y, Rect *rect, int options, int length,
                   STRING str );
	// This one is needed for OS/2. Can't use virtual on this one. Better fix later..
  virtual void drawStringClip( int x, int y, Rect *rect, int options, int length,
                               STRING str )
	{
  	drawString( x, y, rect, options, length, str );
  }
  void drawLine( int x1, int y1, int x2, int y2 );
  void drawPixel( int x, int y );

  // The last position in the points vector contains the position immediately
  // after the last character, so the length of the vector should be length + 1

  void calcStringCharPos( int x, int y, int length, STRING str, Point *points );

  void fillRect( Rect *rect );

  void drawShadedBox( int x, int y, int width, int height,
                      ColorValue c1, ColorValue c2 )
  {
    setColor( c1 );
    drawLine( x, y, x, y + height - 1 );
    drawLine( x, y + height - 1, x + width - 1, y + height - 1 );
    setColor( c2 );
    drawLine( x + width - 1 , y + height - 1, x + width - 1, y );
    drawLine( x + width - 1, y, x, y );
  }

  void drawPolygon( int points, Point *point[] );
  void fillPolygon( int points, Point *point[] );

  FontList *getFonts();
  BOOL selectFont( Font *font, int pointSize, int X, int Y, int attr );
  BOOL selectFont( char *fontName, int pointSize, int attr ) {
    return selectFont( getFontFromName(fontName, attr), pointSize, -1, -1, attr );
  };
  BOOL selectFont( Font *font, int pointSize, int attr ) {
    return selectFont( font, pointSize, -1, -1, attr );
  };
  BOOL selectFont( Font *font, int X, int Y, int attr ) {
    return selectFont( font, -1, X, Y, attr );
  };
  Font *getFontFromName( char *name, int attr );

  void setSystemFont();
  void setDialogFont() { setSystemFont(); }
/*  void setSystemFont() {
    selectFont( getSystemFont(), getSystemFontSize(), 0 );
  }
  Font *getSystemFont() { return getFontFromName( "System Proportional", 0 ); }
  int getSystemFontSize() { return 8; }
  Font *getKonFont() { return getFontFromName( "Helv", 0 ); };
  int getKonFontSize() { return 10; }
  void setKonFont() {
    selectFont( getKonFont(), getKonFontSize(), 0 );
  };*/
  static char *getDialogFontName();
  static int getDialogFontSize();
  Font *getEditorFont() { return getFontFromName( editorFont, 0 ); }
  static char *getEditorFontName() { return editorFont; }
  static int getEditorFontSize() { return 8; }
  Font *getHexEditorFont() { return getFontFromName( hexEditorFont, 0 ); }
  static char *getHexEditorFontName() { return hexEditorFont; }
  static int getHexEditorFontSize() { return 8; }
  int getFontHeight();
  int getMaxFontHeight();
  int getFontBaseline();
  int getAverageCharWidth();
  int getSpaceWidth()
  {
    if( selectedFont )
      return selectedFont->spaceWidth;
    else
      return 0;
  }
  void setFontBold()
  {
    // See if a specialized bold font exists
    selectFont( currentFont->font, currentFont->points,
                currentFont->X, currentFont->Y, Font::BOLD );
  }
  void setFontNormal()
  {
    // See if a specialized bold font exists
    selectFont( currentFont->font, currentFont->points,
                currentFont->X, currentFont->Y, 0 );
  }
  void drawBitmap( Bitmap *bitmap, int flags, int x, int y, int width, int height );
  void drawInactiveBitmap( Bitmap *bitmap, int x, int y, int width, int height );

  // If an outline font is used, a text box should be used to ensure that
  // the background is covered

  BOOL queryStringBoxRecommended()
  {
    return currentFont->font->sizes == 0;
  }

  BOOL hasFontSelected() { return currentFont != NULL; };
  void freeSelectedFont( SelectFont *selectFont )
  {
    if( selectFont )
    {
      freeSelectedFont( selectFont->next );
      delete selectFont;
    }
  }

  virtual ~Graphics()
  {
    freeSelectedFont( selectedFont );
//    freeFontList();
  }
};

/******************************************************************************
 * BmpGraphics holds a presention space which is used for loading bitmaps and
 * doing operations which should not be drawn in a window
 */

class BmpGraphics : public Graphics
{
  HDC hdc;
public:
  BmpGraphics();
  Bitmap *readBitmap( char *fileName );
  virtual ~BmpGraphics()
  {
    GpiDestroyPS( hps );
    DevCloseDC( hdc );
  }
};
