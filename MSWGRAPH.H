
#include <stdio.h>

#define WINBGR(a) ((a&0x00ff00) | ((a<<16) & 0xff0000) | ((a>>16) & 0xff))

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
  void switchBackgroundColor( ColorValue to );
	~Bitmap();
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

struct FontEnumParam
{
	FontList *pfl;
	int devX, devY;
};

class Dialog;

class Graphics : public AGraphics
{
	BOOL isNT;
  friend Dialog;
	friend Window;
  void getDC();
  void releaseDC();
  int hdcCounter;
protected:
  static FontList *pfl;
  static BOOL fontsEnumerated;
  void freeFontList();

  struct SelectFont
  {
    int id;
    char fontName[LF_FACESIZE];
    int X, Y, points;
    int attr;
    SelectFont *next;
    int charHeight, aveCharWidth, baseline, maxCharHeight;
    LONG spaceWidth, overhang;
    HFONT fontHandle;
    SelectFont( HFONT fontHandle, char *fontName, int X, int Y, int points, int attr,
                SelectFont *next )
    {
      strcpy( this->fontName, fontName );
      this->id = id;
      this->X = X;
      this->Y = Y;
      this->points = points;
      this->attr = attr;
      this->next = next;
      this->fontHandle = fontHandle;
    }
  } *selectedFont, *systemFont;

  SelectFont *currentFont;
  int currentId;

  BOOL selectFont( char *fontName, int pointSize, int X, int Y, int attr );
  static char *editorFont, *hexEditorFont;

  void collectFontMetrics();
  int width, height;

  HFONT getCurrentFontHandle();
	BOOL outsideDC;
	HWND hwnd;
public:
  HDC hdc;
  static int dialogDesignFontHeight, dialogDesignFontWidth;
  HBRUSH oldFgBrush, currentFgBrush, currentBgBrush;
  HPEN currentPen, oldPen;
  HFONT oldFont;
	void init();
  Graphics( HDC hdc, int width, int height );
  Graphics( HWND hwnd, int width, int height );
  Graphics() : selectedFont(NULL), systemFont(NULL),
               currentId(1), oldFont(NULL), currentFont(NULL), hdcCounter(0) {}
               
	static void reEnumerateFonts()
  {
  	fontsEnumerated = FALSE;
  }

  void setWidth( int width )
  {
    this->width = width;
  }
  void setHeight( int height )
  {
    this->height = height;
  }
  void setColor( ColorValue color );
  void setBackColor( ColorValue color );
  void drawString( int x, int y, STRING str );
  void drawString( int x, int y, int length, STRING str );
  void drawString( int x, int y, Rect *rect, int options, STRING str );
  void drawString( int x, int y, Rect *rect, int options, int length,
                   STRING str );
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
  BOOL selectFont( char *fontName, int pointSize, int attr ) {
    return selectFont( fontName, pointSize, -1, -1, attr );
  };
  BOOL selectFont( char *fontName, int X, int Y, int attr ) {
    return selectFont( fontName, -1, X, Y, attr );
  };

  BOOL selectFont( Font *font, int pointSize, int attr ) {
    return selectFont( font->name, pointSize, -1, -1, attr );
  };

  BOOL selectFont( Font *font, int pointSize, int X, int Y, int attr ) {
    return selectFont( font->name, pointSize, X, Y, attr );
  };

  BOOL selectFont( Font *font, int X, int Y, int attr ) {
    return selectFont( font->name, -1, X, Y, attr );
  };

  void setSystemFont();
  void setDialogFont() { setSystemFont(); }

  Font *getFontFromName( char *name, int attr );

  Font *getEditorFont() { return getFontFromName( editorFont, 0 ); }
  static char *getEditorFontName() { return editorFont; }
  static int getEditorFontSize();
  Font *getHexEditorFont() { return getFontFromName( hexEditorFont, 0 ); }
  static char *getHexEditorFontName() { return hexEditorFont; }
  static int getHexEditorFontSize() { return 10; }
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
  void setFontBold();
  void setFontNormal()
  {
    // See if a specialized bold font exists
    selectFont( currentFont->fontName, currentFont->points,
                currentFont->X, currentFont->Y, 0 );
  }
  void drawBitmap( Bitmap *bitmap, int flags, int x, int y, int width, int height );
/*  void drawInactiveBitmap( Bitmap *bitmap, int x, int y, int width, int height );
*/
  // If an outline font is used, a text box should be used to ensure that
  // the background is covered

  BOOL queryStringBoxRecommended()
  {
    return FALSE;
  }

  BOOL hasFontSelected() { return currentFont != NULL; };
  void freeSelectedFont( SelectFont *selectFont );
  ~Graphics();
};

/******************************************************************************
 * BmpGraphics holds a presention space which is used for loading bitmaps and
 * doing operations which should not be drawn in a window
 */

class BmpGraphics : public Graphics
{
public:
  Bitmap *readBitmap( char *fileName );
};
