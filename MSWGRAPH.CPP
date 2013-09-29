#include "bowl.h"

#pragma option -Od

int Graphics::dialogDesignFontHeight = 17;
int Graphics::dialogDesignFontWidth = 6;
char *Graphics::editorFont = "Courier";
char *Graphics::hexEditorFont = "Courier";
FontList *Graphics::pfl = NULL;
BOOL Graphics::fontsEnumerated = FALSE;

int CALLBACK fontEnumCallback( ENUMLOGFONT FAR *lf, NEWTEXTMETRIC FAR *ntm, int type, LPARAM _fep )
{
	FontEnumParam *fep = (FontEnumParam *)_fep;
  FontList *pfl = fep->pfl;

  if( type == 0 ) return 1;

	if( ntm->tmDigitizedAspectX != fep->devX ||
	    ntm->tmDigitizedAspectY != fep->devY ) return 1;

  pfl->fonts++;

  pfl->font = (Font *)realloc( pfl->font, pfl->fonts * sizeof( Font ) );

  pfl->font[pfl->fonts-1].sizes = 0;
  pfl->font[pfl->fonts-1].monoSpaced = lf->elfLogFont.lfPitchAndFamily & FIXED_PITCH;

  strcpy( pfl->font[pfl->fonts-1].name, lf->elfLogFont.lfFaceName );

  return 1;
}

void Graphics::getDC()
{
  if( !outsideDC && hdcCounter == 0 )
		hdc = GetDC( hwnd );
  hdcCounter++;
}

void Graphics::releaseDC()
{
  hdcCounter--;
  if( !outsideDC && hdcCounter == 0 )
    ReleaseDC( hwnd, hdc );
}

HFONT Graphics::getCurrentFontHandle() 
{ 
	return currentFont->fontHandle; 
}
void Graphics::init()
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &os );
	isNT = (os.dwPlatformId == VER_PLATFORM_WIN32_NT);
	
  codePage = 850;

  Color::BKGR = WINBGR(GetSysColor( COLOR_BTNFACE ));
  Color::SHADOW = WINBGR(GetSysColor( COLOR_BTNSHADOW ));
  Color::LIGHT = WINBGR(GetSysColor( COLOR_BTNHILIGHT ));
}

Graphics::Graphics( HDC hdc, int width, int height ) : selectedFont(NULL), currentId(1), height(height), width(width),
          oldFont(NULL), outsideDC(TRUE), currentFont(NULL), systemFont(NULL), hdcCounter(0)
{
	init();
  this->hdc = hdc;
  currentFgBrush = CreateSolidBrush( (COLORREF)Color::BLACK );
  currentBgBrush = CreateSolidBrush( (COLORREF)Color::WHITE );
  currentPen = CreatePen( PS_SOLID, 0, (COLORREF)Color::BLACK );
  oldPen = (HPEN)SelectObject( hdc, currentPen );
  oldFgBrush = (HBRUSH)SelectObject( hdc, currentFgBrush );
}

Graphics::Graphics( HWND hwnd, int width, int height ) : selectedFont(NULL), currentId(1), height(height), width(width),
          oldFont(NULL), outsideDC(FALSE), currentFont(NULL), systemFont(NULL), hdcCounter(0)
{
	init();

  this->hwnd = hwnd;
}

void Graphics::setFontBold()
{
  // See if a specialized bold font exists
  if( currentFont->points == -1 && currentFont->X == -1 )
  {
    // Is a TrueType hopefully

    LOGFONT lf;

		getDC();

    if( GetObject( currentFont->fontHandle, sizeof(lf), &lf ) )
      selectFont( lf.lfFaceName, -MulDiv( 72, lf.lfHeight, GetDeviceCaps( hdc, LOGPIXELSY )), 
                  -1, -1, Font::BOLD );

		releaseDC();
  }
  else
    selectFont( currentFont->fontName, currentFont->points,
                currentFont->X, currentFont->Y, Font::BOLD );
}

void Graphics::setColor( ColorValue color )
{
  fc = WINBGR(color);
	if( outsideDC )
	{
		currentFgBrush = CreateSolidBrush( (COLORREF)fc );
		currentPen = CreatePen( PS_SOLID, 0, (COLORREF)fc );
		DeleteObject( SelectObject( hdc, currentPen ) );
		DeleteObject( SelectObject( hdc, currentFgBrush ) );
		SetTextColor( hdc, fc );
	}
};

void Graphics::setBackColor( ColorValue color )
{
  bc = WINBGR(color);
	if( outsideDC )
	{
		DeleteObject( currentBgBrush );
		currentBgBrush = CreateSolidBrush( (COLORREF)bc );
		SetBkColor( hdc, bc );
	}
};

FontList *Graphics::getFonts()
{
	if( !fontsEnumerated )
  {
		getDC();

  	pfl = (FontList *)malloc( sizeof( FontList ) );
  	pfl->fonts = 0;
  	pfl->font = NULL;
		struct FontEnumParam fep;
		fep.pfl = pfl;
		fep.devX = GetDeviceCaps( hdc, LOGPIXELSX );
		fep.devY = GetDeviceCaps( hdc, LOGPIXELSY );
  	int rc = EnumFontFamilies( hdc, NULL, (FONTENUMPROC)fontEnumCallback, (LPARAM)&fep );
    fontsEnumerated = TRUE;

		releaseDC();
  }

  return pfl;
}

Font *Graphics::getFontFromName( char *name, int )
{
  getFonts();

  for( int i = 0; i < pfl->fonts; i++ )
    if( !strcmp( pfl->font[i].name, name ) )
      return &pfl->font[i];

  return (Font *)0L;
}

void Graphics::setSystemFont()
{
  if( useBigFont )
    selectFont( GUI_FONT_NAME, GUI_FONT_SIZE, 0 );
  else
  {
    if( systemFont == NULL )
		{
	    HFONT fontHandle = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
      selectedFont = systemFont = new SelectFont( fontHandle, "", -1, -1, -1, 0, selectedFont );
		}

    currentFont = systemFont;

		getDC();

    if( oldFont == NULL )
      oldFont = (HFONT)SelectObject( hdc, systemFont->fontHandle );
    else
      SelectObject( hdc, systemFont->fontHandle );

    // Get font metrics for future reference

    collectFontMetrics();

	  if( !outsideDC )
	  {
      SelectObject( hdc, oldFont );
		  releaseDC();
		  oldFont = NULL;
	  }
  }
}

void Graphics::collectFontMetrics()
{
  TEXTMETRIC tm;

  GetTextMetrics( hdc, &tm );

  // ch

  currentFont->charHeight = tm.tmHeight - tm.tmExternalLeading;
  currentFont->maxCharHeight = tm.tmAscent + tm.tmDescent + 
	                             tm.tmExternalLeading;
  currentFont->aveCharWidth = tm.tmAveCharWidth;
  currentFont->baseline = tm.tmDescent;
  currentFont->overhang = tm.tmOverhang;

  INT size[1], fit;
  SIZE box;

  GetTextExtentExPoint( hdc, " ", 1, width, &fit, size, &box );
  currentFont->spaceWidth = size[0];
	
	if( !isNT )
		currentFont->spaceWidth -= currentFont->overhang;
}

BOOL Graphics::selectFont( char *fontName, int pointSize, int X, int Y, int attr )
{
  LONG RetVal = TRUE;
  BOOL newFont = TRUE, newSize = TRUE;
  HFONT fontHandle;

	getDC();

  // Check if the font has already been selected

  SelectFont *searchFont = selectedFont;

  while( searchFont )
  {
    if( !strcmp( searchFont->fontName, fontName) && searchFont->attr == attr )
    {
      if( searchFont->points == pointSize ||
				  (searchFont->X == X && searchFont->Y == Y && X != -1) )
      {
        newFont = FALSE;
        fontHandle = searchFont->fontHandle;
        currentFont = searchFont;
        currentFont->points = pointSize;
        currentFont->attr = attr;
        currentFont->X = X;
        currentFont->Y = Y;
        break;
      }
    }
    searchFont = searchFont->next;
  }

  if( newFont )
  {
    // Create the new font

    fontHandle = CreateFont( -MulDiv( pointSize, GetDeviceCaps( hdc, LOGPIXELSY ), 72), 0, 0, 0,
                             (attr&Font::BOLD) ? FW_BOLD : FW_NORMAL, (attr&Font::ITALIC) ? TRUE : FALSE,
                             FALSE, FALSE, DEFAULT_CHARSET, OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, fontName );

    // Create a new entry in the selected font's list

    selectedFont = new SelectFont( fontHandle, fontName, X, Y, pointSize, attr,
                                   selectedFont );

    currentFont = selectedFont;
  }

  // Select the font

  if( oldFont == NULL )
    oldFont = (HFONT)SelectObject( hdc, fontHandle );
  else
    SelectObject( hdc, fontHandle );

  // Get font metrics for future reference

  collectFontMetrics();

	if( !outsideDC )
	{
    SelectObject( hdc, oldFont );
		releaseDC();
		oldFont = NULL;
	}

	return RetVal;
}

void Graphics::drawString( int x, int y, int length, STRING str )
{
	if( outsideDC )
		ExtTextOut( hdc, x, height-y-currentFont->charHeight+currentFont->baseline, ETO_OPAQUE, NULL, str, length, NULL );
}

void Graphics::drawString( int x, int y, STRING str )
{
	if( outsideDC )
	  drawString( x, y, strlen( str ), str );
}

void Graphics::drawString( int x, int y, Rect *rect, int options, int length,
                           STRING str )
{
	if( !outsideDC ) return;
  RECT orect = {  rect->x, height - (rect->y + rect->height),
                  rect->x+rect->width, height - rect->y };
  ExtTextOut( hdc, x, height-y-currentFont->charHeight+currentFont->baseline, 
              ETO_CLIPPED | ETO_OPAQUE, &orect, str, length, NULL );
}

void Graphics::drawString( int x, int y, Rect *rect, int options, STRING str )
{
	if( outsideDC )
	  drawString( x, y, rect, options, strlen( str ), str );
}

int Graphics::getEditorFontSize() { return 10; }
int Graphics::getFontHeight()
{
  if( currentFont )
    return currentFont->charHeight;
  else
    return 0;
}

int Graphics::getAverageCharWidth()
{
  if( currentFont )
    return currentFont->aveCharWidth;
  else
    return 0;
}

int Graphics::getMaxFontHeight()
{
	return currentFont->maxCharHeight;
}

int Graphics::getFontBaseline()
{
  if( currentFont )
    return currentFont->baseline;
  else
    return 0;
}

void Graphics::fillPolygon( int points, Point *point[] )
{
	if( !outsideDC ) return;
  POINT *ppoint = new POINT[points];

  for( int i = 0; i < points; i++ )
  {
    ppoint[i].x = point[i]->x;
    ppoint[i].y = height - point[i]->y - 1;
  }

  Polygon( hdc, ppoint, points );
  delete ppoint;
}

void Graphics::drawPolygon( int points, Point *point[] )
{
	if( !outsideDC ) return;
  POINT *ppoint = new POINT[points];

  for( int i = 0; i < points; i++ )
  {
    ppoint[i].x = point[i]->x;
    ppoint[i].y = height - point[i]->y - 1;
  }

  Polyline( hdc, ppoint, points );
  delete ppoint;
}

void Graphics::freeFontList()
{
  if( pfl != NULL )
  {
    for( int i = 0; i < pfl->fonts; i++ )
      if( pfl->font[i].sizes != 0 )
        free( pfl->font[i].size);
    free( pfl );
  }                        
  pfl = NULL;
}

void Graphics::drawLine( int x1, int y1, int x2, int y2 )
{
	if( !outsideDC ) return;
  POINT old;

  MoveToEx( hdc, x1, height - y1 - 1, &old );

  if( x2 > x1 ) 
    x2++;
  else
  if( x2 < x1 ) 
    x2--;

  if( y2 > y1 ) 
    y2++;
  else
  if( y2 < y1 ) 
    y2--;

  LineTo( hdc, x2, height - y2 - 1 );
}

void Graphics::drawPixel( int x, int y )
{
	if( !outsideDC ) return;
  drawLine( x, height - y - 1, x, height - y - 1 ); // Just until I find out how to draw a single pixel in Windows...!!!
}

void Graphics::calcStringCharPos( int x, int y, int length, STRING str,
                                  Point *points )
{
	if( !outsideDC ) 
	{
		getDC();

		oldFont = (HFONT)SelectObject( hdc, currentFont->fontHandle );
	}

  INT size[2000];
  SIZE box;

  GetTextExtentExPoint( hdc, str, (length > 2000) ? 2000 : length, 0, NULL, size, &box );

/*  FILE *f = fopen( "font3", "a" );
  for( int n = 0; n < length; n++ )
    fprintf( f, "%c: %d\n", str[n], size[n] );
  fclose( f );*/

  // Cast the points array

  points[0].x = x;
  points[0].y = y;

  for( int i = 1; i <= length; i++ )
  {
    points[i].x = size[i-1] + x;
//    points[i].x = points[i-1].x + charWidths[str[i-1]];
    points[i].y = y;
  }

	if( !isNT )
		points[length].x -= currentFont->overhang;
/*  FILE *f = fopen( "text", "a" );
  for( i = 0; i < length; i++ )
    fprintf( f, "[%x(%c)]", str[i], str[i] );
  fprintf( f, "\n" );
  fclose( f );*/
	if( !outsideDC )
	{
		SelectObject( hdc, oldFont );
		releaseDC();
		oldFont = NULL;
	}
}

void Graphics::fillRect( Rect *rect )
{
	if( !outsideDC ) return;
  RECT interior;
  interior.left = rect->x;
  interior.top = height - (rect->height + rect->y);
  interior.right = rect->x + rect->width;
  interior.bottom = height - rect->y;
  FillRect( hdc, &interior, currentBgBrush );
}

void Graphics::drawBitmap( Bitmap *bitmap, int flags, int x, int y, int width,
                           int height )
{
	if( !outsideDC ) return;
  HDC hdcCompatSrc;
  HDC hdcCompat = CreateCompatibleDC( hdc );
	HBITMAP compatBmp;

	if( flags & INACTIVESHADE )
	{
		hdcCompatSrc = CreateCompatibleDC( hdc );

		SelectObject( hdcCompatSrc, bitmap->hbmBitmap );
		compatBmp = CreateCompatibleBitmap( hdc, bitmap->getWidth(), bitmap->getHeight() );
		SelectObject( hdcCompat, compatBmp );
		BitBlt(hdcCompat, 0, 0, bitmap->getWidth(), bitmap->getHeight(), hdcCompatSrc, 0, 0, SRCCOPY );

		for( int yy = bitmap->getHeight()-1; yy >= 0; yy-- )
			for ( int xx = 0; xx < bitmap->getWidth(); )
			{
				while( GetPixel( hdcCompat, xx, yy ) == WINBGR(Color::BKGR) && xx < bitmap->getWidth() ) xx++;
				while( GetPixel( hdcCompat, xx, yy ) != WINBGR(Color::BKGR) && xx < bitmap->getWidth() )
				{
					SetPixel( hdcCompat, xx, yy, WINBGR(Color::SHADOW));
					xx++;
					if( GetPixel( hdcCompat, xx, yy+1 ) == WINBGR(Color::BKGR) )
						SetPixel( hdcCompat, xx, yy+1, WINBGR(Color::LIGHT) );
				}
			}
	}
	else
	  SelectObject( hdcCompat, bitmap->hbmBitmap );

  StretchBlt( hdc, x, this->height - y - height, width, height, hdcCompat, 0, 0, bitmap->getWidth(), bitmap->getHeight(), SRCCOPY );

	if( flags & INACTIVE )
	{
		for( int yy = this->height - y - height; yy < this->height - y; yy++ )
			for( int xx = x + yy % 2; xx < x+width; xx += 2 )
				SetPixel( hdc, xx, yy, WINBGR(Color::GRAY)); 
	}

	if( flags & INACTIVESHADE )
	{
		DeleteObject(SelectObject(hdcCompat, NULL) );
		DeleteObject( compatBmp );
	  DeleteDC( hdcCompatSrc );
	}

  DeleteDC( hdcCompat );
}


void Graphics::freeSelectedFont( SelectFont *selectFont )
{
  if( selectFont )
  {
    freeSelectedFont( selectFont->next );
    DeleteObject( selectFont->fontHandle );
    delete selectFont;
  }
}

Graphics::~Graphics()
{
	if( outsideDC )
	{
		if( oldFont != NULL )
			SelectObject( hdc, oldFont );
	  DeleteObject( SelectObject( hdc, oldPen ) );  // currentPen
	//  freeFontList();

		DeleteObject( currentBgBrush );
		DeleteObject( SelectObject( hdc, oldFgBrush ) );  // currentFgBrush
	}

  freeSelectedFont( selectedFont );
}

Bitmap *BmpGraphics::readBitmap( char *filename )
{
  HBITMAP hBmp = (HBITMAP)LoadImage( NULL, filename, IMAGE_BITMAP,  0, 0, LR_LOADFROMFILE );

  if( hBmp == (HANDLE)-1 ) return NULL;

  BITMAP bmp;

  GetObject( hBmp, sizeof( BITMAP ), (LPSTR)&bmp );

  return new Bitmap( hBmp, bmp.bmWidth, bmp.bmHeight );
}

void Bitmap::switchBackgroundColor( ColorValue to )
{
  HBITMAP old;
	HDC hdc = CreateDC( "DISPLAY", NULL, NULL, NULL );
  HDC hdcCompat = CreateCompatibleDC( hdc );
  old = (HBITMAP)SelectObject( hdcCompat, hbmBitmap );

	ColorValue from = GetPixel( hdcCompat, 0, 0 );

  for( int xx = 0; xx < getWidth(); xx++ )
  	for( int yy = 0; yy < getHeight(); yy++ )
		{    	
    	if( GetPixel( hdcCompat, xx, yy ) == from ) // 0xcbcbcb
    		SetPixel( hdcCompat, xx, yy, WINBGR(to) );
    }

	hbmBitmap =	(HBITMAP)SelectObject( hdcCompat, old );
  DeleteDC( hdcCompat );
  DeleteDC( hdc );
}

Bitmap::~Bitmap()
{
	DeleteObject( hbmBitmap );
}
