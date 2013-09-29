/******************************************************************************
 * Colors are stored in RGB.
 */

typedef LONG ColorValue;

struct Color
{
  static ColorValue RED, DARKRED, GREEN, BLUE, PINK, CYAN, YELLOW, BROWN, 
  									WHITE, GRAY, LIGHTGRAY, BLACK, DEFAULT, BACKGROUND, 
                    BKGR, LIGHT, SHADOW, DARKCYAN, DARKGREEN, DARKBLUE, DARKPINK;
  static ColorValue createColor( int red, int green, int blue )
  {
    return (red << 16) + (green << 8) + blue;
  }
};

/******************************************************************************
 * Fonts
 *
 * Fonts should be selected by the user by allowing the user to choose
 * from the fonts retrieved by getFonts().
 */

struct FontSizes
{
  int pointSize;
  int X, Y;
	int devX, devY;
  LONG match;
};

struct Font
{
  char name[100];
  int sizes;          // the number of sizes in this font
  FontSizes *size;    // the sizes
  BOOL monoSpaced;
  static int BOLD, ITALIC, UNDERSCORE;
};

struct FontList
{
  int fonts;
  Font *font;
};

/******************************************************************************
 * Base graphics class
 */

struct AGraphics
{
  int codePage;
  static int fontOpaque;
protected:
  ColorValue fc, bc;
  void drawButtonShade( int x, int y, int width, int height, int up )
  {
    if( up )
      setColor( Color::LIGHT );
    else
      setColor( Color::SHADOW );
    drawLine( x, y + 1, x, y + height - 1 );
    drawLine( x, y + height - 1, x + width - 2, y + height - 1 );
    setColor( Color::BKGR );
    drawPixel( x + width - 1, y + height - 1 );
    if( up )
      setColor( Color::SHADOW );
    else
      setColor( Color::LIGHT );
    drawLine( x + width - 1, y + height - 2, x + width - 1, y );
    drawLine( x + width - 1, y, x + 1, y );
    setColor( Color::BKGR );
    drawPixel( x, y );
  }
  static BOOL unicode;
public:
  static int ACTIVE, INACTIVE, INACTIVESHADE;
  static BOOL useBigFont;
  virtual void setUnicode( BOOL use ) { unicode = use; }
  virtual void setColor( ColorValue color ) {}
  virtual void setBackColor( ColorValue color ) {}
  virtual void drawLine( int x1, int y1, int x2, int y2 ) {}
  virtual void drawPixel( int x, int y ) {}
  virtual void fillRect( Rect *rect ) {}
  virtual void drawButton( int x, int y, int width, int height, int up )
  {
    drawButtonShade( x, y, width, height, up );
    drawButtonShade( x+1, y+1, width-2, height-2, up );
    setBackColor( Color::BKGR );
    Rect rect( x + 2, y + 2, width - 4, height - 4 );
    fillRect( &rect );
  }
  virtual void drawRect( Rect *rect )
  {
    drawLine( rect->x, rect->y, rect->x, rect->y + rect->height );
    drawLine( rect->x, rect->y + rect->height,
              rect->x + rect->width, rect->y + rect->height );
    drawLine( rect->x + rect->width, rect->y + rect->height,
              rect->x + rect->width, rect->y );
    drawLine( rect->x + rect->width, rect->y, rect->x, rect->y );
  }
  virtual BOOL hasFontSelected() { return TRUE; };
};
