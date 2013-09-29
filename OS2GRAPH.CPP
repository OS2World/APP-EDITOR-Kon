#include "bowl.h"

#pragma option -Od

int Graphics::dialogDesignFontHeight = 17;
int Graphics::dialogDesignFontWidth = 6;
char *Graphics::editorFont = "Courier";
char *Graphics::hexEditorFont = "Courier";
char *Graphics::helvFont = "Helv";
char *Graphics::wsFont = "WarpSans";
FontList *Graphics::pfl = NULL;

ULONG Graphics::getOSVER()
{
	ULONG OSVER;

	DosQuerySysInfo( QSV_VERSION_MINOR, QSV_VERSION_MINOR, &OSVER, 
	                 sizeof( OSVER ) );
	return OSVER;
}

Graphics::Graphics( HPS hps ) : selectedFont(NULL), currentId(1), 
                                currentFont(NULL)
{
  codePage = 850;
  this->hps = hps;
  GpiCreateLogColorTable( hps, LCOL_PURECOLOR, LCOLF_RGB, 0, 0, NULL );
  Color::BKGR = WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0 );
  Color::SHADOW = WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONDARK, 0 );
  Color::LIGHT = WinQuerySysColor( HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0 );
}

FontList *Graphics::getFonts()
{
  HDC hdc;
  int iFace, iSize;
  LONG l, lFonts;
  PFONTMETRICS pfm;

  // Check for changed fonts

  if( !(QFA_PUBLIC & GpiQueryFontAction( hps, QFA_PUBLIC)) && pfl != NULL )
    return pfl;

  // Clean up old structure if necessary

  freeFontList();

  // Determine the number of fonts

  lFonts = 0;
  lFonts = GpiQueryFonts( hps, QF_PUBLIC, NULL, &lFonts, 0, NULL );

  if( lFonts == 0 ) return NULL;

  // Allocate memory for FONTMETRICS structure

  pfm = (PFONTMETRICS) calloc( lFonts, sizeof( FONTMETRICS ) );

  if( pfm == NULL ) return NULL;

  // Get all fonts

  GpiQueryFonts( hps, QF_PUBLIC, NULL, &lFonts, sizeof( FONTMETRICS ), pfm );

  // Allocate memory for FontList structure

  pfl = (FontList *)malloc( sizeof( FontList ) );
  pfl->fonts = 0;
  pfl->font = 0;

  // Loop through all fonts

  for( l = 0; l < lFonts; l++ )
  {
    // Loop through existing facenames

    for( iFace = 0; iFace < pfl->fonts; iFace++ )
      if( 0 == strcmp( pfl->font[iFace].name,
                       pfm[l].szFacename) )
        break;

    // If new face, reallocate FontList structure

    if( iFace == pfl->fonts )
    {
      pfl->fonts++;

      pfl->font = (Font *)realloc( pfl->font, pfl->fonts * sizeof( Font ) );

      strcpy( pfl->font[iFace].name, pfm[l].szFacename );

      pfl->font[iFace].sizes = 0;
      pfl->font[iFace].monoSpaced = pfm[l].fsType & FM_TYPE_FIXED;
      pfl->font[iFace].size = NULL;
    }

    if( !(pfm[l].fsDefn & FM_DEFN_OUTLINE) )
    {
      // Check so that this size isn't already in the list

/*      for( iSize = 0; iSize < pfl->font[iFace].sizes; iSize++ )
      {
        if( !(pfm[l].fsType & FM_TYPE_FIXED) )
        {
          if( pfl->font[iFace].size[iSize].pointSize ==
              pfm[l].sNominalPointSize / 10 )
            break;
        }
        else
        if( pfl->font[iFace].size[iSize].X == pfm[l].lAveCharWidth &&
            pfl->font[iFace].size[iSize].Y == pfm[l].lMaxBaselineExt )
          break;
      }

      if( iSize != pfl->font[iFace].sizes ) continue;*/

      iSize = pfl->font[iFace].sizes;

      pfl->font[iFace].sizes++;

      pfl->font[iFace].size = (FontSizes *)realloc( pfl->font[iFace].size,
        pfl->font[iFace].sizes * sizeof( FontSizes ) );

      // Store point size and lMatch value

      pfl->font[iFace].size[iSize].pointSize =
        pfm[l].sNominalPointSize / 10;

      pfl->font[iFace].size[iSize].X = pfm[l].lAveCharWidth;
      pfl->font[iFace].size[iSize].Y = pfm[l].lMaxBaselineExt;
      pfl->font[iFace].size[iSize].devX = pfm[l].sXDeviceRes;
      pfl->font[iFace].size[iSize].devY = pfm[l].sYDeviceRes;
      pfl->font[iFace].size[iSize].match = pfm[l].lMatch;
    }
  }
  // Clean up
  free( pfm );
  return pfl;
}

#pragma option -O2

Font *Graphics::getFontFromName( char *name, int attr )
{
	char altName[300];
	Font *foundFont = NULL;

	if( attr )
  {
    strcpy( altName, name );

    if( attr & Font::BOLD )
    	strcat( altName, " Bold" );

    if( attr & Font::ITALIC )
    	strcat( altName, " Italic" );
 	}

  getFonts();

  for( int i = 0; i < pfl->fonts; i++ )
  {
/*  	if( attr )
    {
	  	FILE *f = fopen( "Font", "a" );
  	  fprintf( f, "'%s', '%s'\n", pfl->font[i].name, altName );
    	fclose( f );
    }*/
    if( !strcmp( pfl->font[i].name, name ) )
    {
    	if( !attr )
	      return &pfl->font[i];
      else
      	foundFont = &pfl->font[i];
    }

		if( attr )
      if( !strcmp( pfl->font[i].name, altName ) )
	      return &pfl->font[i];
  }

  return (Font *)foundFont;
}

void Graphics::collectFontMetrics()
{
  FONTMETRICS	fontMetrics;

  GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fontMetrics );

  selectedFont->charHeight = fontMetrics.lMaxAscender+fontMetrics.lMaxDescender;

  if( fontMetrics.fsDefn & FM_DEFN_OUTLINE )
    selectedFont->charHeight += fontMetrics.lUnderscorePosition;

  selectedFont->aveCharWidth = fontMetrics.lAveCharWidth;

  selectedFont->baseline = fontMetrics.lMaxDescender;

  // Find out the width of the space character

  GpiQueryWidthTable( hps, 32, 1, &selectedFont->spaceWidth );

/*  FILE *f = fopen( "font", "a" );
  fprintf( f, "Height: %d\n", selectedFont->charHeight );
  fclose( f );*/
}

BOOL Graphics::selectFont( Font *font, int pointSize, int X, int Y, int attr )
{
  FATTRS fat;
  LONG RetVal = TRUE, xRes, yRes;
  BOOL newFont = TRUE, newSize = TRUE;
  int fontId;

  // Check if the font has already been selected

  SelectFont *searchFont = selectedFont;

/*  FILE *f = fopen( "font", "a" );
  fprintf( f, "Font.." );
  fclose( f );*/

  while( searchFont )
  {
    if( searchFont->font == font && searchFont->attr == attr )
    {
      if( searchFont->font->sizes == 0 && pointSize != searchFont->points )
      {
        newFont = FALSE;
        fontId = searchFont->id;
        currentFont = searchFont;
        currentFont->points = pointSize;
        break;
      }
      else
      if( searchFont->points == pointSize && searchFont->X == X &&
          searchFont->Y == Y && searchFont->font->sizes != 0 )
      {
        newFont = FALSE;
        newSize = FALSE;
        fontId = searchFont->id;
        currentFont = searchFont;
        break;
      }
    }
    searchFont = searchFont->next;
  }

/*  f = fopen( "font", "a" );
  fprintf( f, "close: " );
  fclose( f );*/

  // Determine font resolution

  HDC hdc = GpiQueryDevice( hps );

  DevQueryCaps( hdc, CAPS_HORIZONTAL_FONT_RES, 1, &xRes );
  DevQueryCaps( hdc, CAPS_VERTICAL_FONT_RES, 1, &yRes );

  if( newFont )
  {
    fat.usRecordLength = sizeof( FATTRS );

    if( strstr( font->name, " Bold" ) )
    	attr &= (Font::BOLD ^ 0xffff);

    if( strstr( font->name, " Italic" ) )
    	attr &= (Font::ITALIC ^ 0xffff);

    fat.fsSelection = ((attr & Font::BOLD) ? FATTR_SEL_BOLD : 0) |
                      ((attr & Font::ITALIC) ? FATTR_SEL_ITALIC : 0) |
                      ((attr & Font::UNDERSCORE) ? FATTR_SEL_UNDERSCORE : 0);
    fat.lMatch = 0;
    fat.szFacename[0] = '\0';
    fat.idRegistry = 0;
    fat.usCodePage = codePage;
    fat.lMaxBaselineExt = 0;
    fat.lAveCharWidth = 0;
    fat.fsType = 0;
    fat.fsFontUse = 0;

    // loop through sizes

    if( font->sizes == 0 )
    {
      strcpy( fat.szFacename, font->name );
      fat.fsFontUse = FATTR_FONTUSE_NOMIX | FATTR_FONTUSE_OUTLINE;
    }
    else 
    {
      for( int i = 0; i < font->sizes; i++ )
      {
/*				FILE *f = fopen( "font", "a" );
				fprintf( f, "%s, %d, %d, %d, %d, %d\n", font->name, pointSize, 
				         font->size[i].devX, xRes, font->size[i].devY, yRes );
				fclose( f );*/
        // If a size matches, setup FATTR structure

        if( (pointSize == font->size[i].pointSize &&
						 font->size[i].devX == xRes &&
						 font->size[i].devY == yRes) ||
            (X == font->size[i].X &&
             Y == font->size[i].Y) )
        {
          strcpy( fat.szFacename, font->name );
          fat.lMatch = font->size[i].match;
          fat.fsFontUse = FATTR_FONTUSE_NOMIX;
        }
      }
    }

    // Create a new entry in the selected font's list

    selectedFont = new SelectFont( currentId, font, X, Y, pointSize, attr,
                                          selectedFont );

/*    f = fopen( "font", "a" );
    fprintf( f, "New %d, %d, %d ", X, Y, pointSize );
    fclose( f );*/

    currentFont = selectedFont;

    // Create the font

    RetVal = GpiCreateLogFont( hps, NULL, currentId, &fat );

    fontId = currentId;
    currentId++;
    if( currentId > 254 ) currentId = 1;
  }
  else
  {
/*    f = fopen( "font", "a" );
    fprintf( f, "Old %d ", fontId );
    fclose( f );*/
  }

  GpiSetCharSet(hps, fontId);

  if( font->sizes == 0 )
	{
    if( newSize )
    {
      LONG Res[2];
      HDC hdc = GpiQueryDevice( hps );

      DevQueryCaps( hdc, CAPS_HORIZONTAL_FONT_RES, 2L, Res );

      SIZEF size = {
        MAKEFIXED( (pointSize * 10 * Res[0]) / 720,0),
        MAKEFIXED( (pointSize * 10 * Res[1]) / 720,0)
      };

      GpiSetCharBox( hps, &size );
    }
	}
	else
  {
		GpiSetMix( hps, BM_OVERPAINT );
    GpiSetBackMix( hps, BM_OVERPAINT );
  }

  // Get font metrics for future reference

  collectFontMetrics();

	return RetVal;
}

void Graphics::setSystemFont()
{
  if( useBigFont )
    selectFont( GUI_FONT_NAME, GUI_FONT_SIZE, 0 );
  else
		selectFont( getDialogFontName(), getDialogFontSize(), 0 );
}

char *Graphics::getDialogFontName() 
{ 
	return useBigFont ? GUI_FONT_NAME : 
	                    ((getOSVER() < 40 ) ? helvFont : wsFont); 
}

int Graphics::getDialogFontSize() 
{ 
	return useBigFont ? GUI_FONT_SIZE : (getOSVER() < 40 ) ? 12 : 9; 
}

void Graphics::drawString( int x, int y, int length, STRING str )
{
  POINTL ptl = {x,y};
  GpiSetColor( hps, fc );
  if( length > 512 ) length = 512;
  GpiCharStringAt( hps, &ptl, length, str );
}

void Graphics::drawString( int x, int y, STRING str )
{
  drawString( x, y, strlen( str ), str );
}

void Graphics::drawString( int x, int y, Rect *rect, int options, int length,
                           STRING str )
{
  RECTL orect = { rect->x, rect->y,
                  rect->x+rect->width, rect->y + rect->height };
  POINTL ptl = {x,y};
  GpiSetColor( hps, fc );
  GpiSetBackColor( hps, bc );
  int foptions = CHS_CLIP | ((options & fontOpaque) ? CHS_OPAQUE : 0);
  if( length > 512 ) length = 512;
  GpiCharStringPosAt( hps, &ptl, &orect, foptions, length, str, NULL );
}

void Graphics::drawString( int x, int y, Rect *rect, int options, STRING str )
{
  drawStringClip( x, y, rect, options, strlen( str ), str );
}

int Graphics::getFontHeight()
{
  if( selectedFont )
    return selectedFont->charHeight;
  else
    return 0;
}

int Graphics::getAverageCharWidth()
{
  if( selectedFont )
    return selectedFont->aveCharWidth;
  else
    return 0;
}

int Graphics::getMaxFontHeight()
{
	FONTMETRICS	fontMetrics;

	GpiQueryFontMetrics( hps, sizeof( FONTMETRICS ), &fontMetrics );

	int charHeight = fontMetrics.lMaxAscender+fontMetrics.lMaxDescender +
                   fontMetrics.lExternalLeading;

	if( fontMetrics.fsDefn & FM_DEFN_OUTLINE )
		charHeight += fontMetrics.lUnderscorePosition;

  return charHeight;
}

int Graphics::getFontBaseline()
{
  if( selectedFont )
    return selectedFont->baseline;
  else
    return 0;
}

void Graphics::fillPolygon( int points, Point *point[] )
{
  GpiBeginArea( hps, BA_BOUNDARY );

  drawPolygon( points, point );

  GpiEndArea( hps );
}

void Graphics::drawPolygon( int points, Point *point[] )
{
  POINTL ptl = { point[0]->x, point[0]->y };

  GpiMove( hps, &ptl );

  for( int i = 1; i < points; i++ )
  {
    POINTL ptl = { point[i]->x, point[i]->y };
    GpiLine( hps, &ptl );
  }
  GpiLine( hps, &ptl );
}

void Graphics::freeFontList()
{
  if( pfl != NULL )
  {
    for( int i = 0; i < pfl->fonts; i++ )
      free( pfl->font[i].size);
    free( pfl );
  }
}

void Graphics::drawLine( int x1, int y1, int x2, int y2 )
{
  POINTL pnt[2] = { x1, y1, x2, y2 };

  GpiMove( hps, &pnt[0] );
  GpiLine( hps, &pnt[1] );
}

void Graphics::drawPixel( int x, int y )
{
  POINTL pnt = {x, y};
  GpiSetPel( hps, &pnt );
}

void Graphics::calcStringCharPos( int x, int y, int length, STRING str,
                                  Point *points )
{
  POINTL pnt = {x,y};
  int testLength, pos = 0;

  while( length )
  {
    testLength = (length > 512) ? 512 : length;
    GpiQueryCharStringPosAt( hps, &pnt, 0, testLength, str, NULL,
                             (PPOINTL)&points[pos] );
    length -= testLength;
    pos += testLength;
    pnt.x = points[pos].x;
  }
}

void Graphics::fillRect( Rect *rect )
{
	POINTL left = { rect->x, rect->y };
	POINTL right = { rect->x + rect->width-1, 
	                 rect->y + rect->height-1 };
	GpiSetCurrentPosition( hps, &left );
  GpiSetColor( hps, bc );
  GpiSetBackColor( hps, bc );
	GpiBox( hps, DRO_OUTLINEFILL, &right, 0, 0 );
/*	RECTL interior;
	interior.xLeft = rect->x;
	interior.yTop = rect->y + rect->height;
	interior.xRight = rect->x + rect->width;
	interior.yBottom = rect->y;
	WinDrawBorder( hps, &interior, 0, 0, fc, bc, DB_INTERIOR );*/
}

void Graphics::drawBitmap( Bitmap *bitmap, int flags, int x, int y, int width,
                           int height )
{
  RECTL rect = {x, y, width+x, height+y};

	if( flags & INACTIVESHADE )
	{
		SIZEL    sizl = {0,0};
		DEVOPENSTRUC dop = { NULL
												 , "DISPLAY"
												 , NULL
												 , NULL
												 , NULL
												 , NULL
												 , NULL
												 , NULL
												 , NULL };

		HDC mhdc = DevOpenDC( Application::hab, OD_MEMORY, "*", 5L,
										 (PDEVOPENDATA)&dop, NULLHANDLE);
		HPS mhps = GpiCreatePS( Application::hab
													, mhdc
													, &sizl
													, PU_PELS | GPIA_ASSOC );

  	GpiCreateLogColorTable( mhps, LCOL_PURECOLOR, LCOLF_RGB, 0, 0, NULL );

		BITMAPINFOHEADER bmih;
		BITMAPINFOHEADER2 bmih2;

		GpiQueryBitmapParameters( bitmap->hbmBitmap, &bmih );

		memset (&bmih2,0, sizeof(BITMAPINFOHEADER2));
		bmih2.cbFix        = sizeof(BITMAPINFOHEADER2);
		bmih2.cx           = bmih.cx;
		bmih2.cy           = bmih.cy;
		bmih2.cPlanes      = bmih.cPlanes;
		bmih2.cBitCount = bmih.cBitCount;

		HBITMAP mbmp = GpiCreateBitmap( mhps, &bmih2, 0L, NULL, NULL );

		GpiSetBitmap( mhps, mbmp );
		
		RECTL r2 = {0, 0, bitmap->getWidth(), bitmap->getHeight() };

	  WinDrawBitmap( mhps, bitmap->hbmBitmap, NULL, (POINTL *)&r2, Color::WHITE, 0,
  	               DBM_NORMAL | DBM_STRETCH );

		POINTL ptl = { 0, 0 };

		LONG bColor = GpiQueryPel( mhps, &ptl );

		for( int yy = bitmap->getHeight()-1; yy >= 0; yy-- )
			for ( int xx = 0; xx < bitmap->getWidth(); )
			{
				ptl.x = xx;
				ptl.y = yy;
				while( GpiQueryPel( mhps, &ptl ) == bColor && xx < bitmap->getWidth() ) 
				{
					xx++;
					ptl.x = xx;
				}
				while( GpiQueryPel( mhps, &ptl ) != bColor && xx < bitmap->getWidth() )
				{
					GpiSetColor( mhps, Color::SHADOW );
					GpiSetPel( mhps, &ptl );
					xx++;
					ptl.y = yy + 1;
					ptl.x = xx;
					GpiSetColor( mhps, Color::LIGHT );
					if( GpiQueryPel( mhps, &ptl ) == bColor )
						GpiSetPel( mhps, &ptl );
					ptl.y = yy;
				}
			}

		POINTL aptl[4] = { x, y, width + x, height + y, 
		                   0, 0, bitmap->getWidth(), bitmap->getHeight() };

		GpiBitBlt( hps, mhps, 4, aptl, ROP_SRCCOPY, BBO_IGNORE );

		GpiSetBitmap( mhps, NULLHANDLE );
		GpiDeleteBitmap( mbmp );

		GpiDestroyPS( mhps );
		DevCloseDC( mhdc );
	}
	else
	  WinDrawBitmap( hps, bitmap->hbmBitmap, NULL, (POINTL *)&rect, Color::WHITE, 0,
  	               DBM_NORMAL | DBM_STRETCH |
    	             ((flags & (INACTIVE|INACTIVESHADE)) ? DBM_HALFTONE : 0) );
}

/******************************************************************************/
/* readBitmap: Loads a bitmap from a file.                                    */
/* Extracted from IBM example code                                            */

Bitmap *BmpGraphics::readBitmap( char *fileName )
{
   APIRET     rc;                                         /* API return code */
   BOOL       fRet = FALSE;                         /* Function return code. */
   PBITMAPFILEHEADER2 pbfh2;                   /* can address any file types */
   PBITMAPINFOHEADER2 pbmp2;                     /* address any info headers */
   PBYTE  pFileBegin = NULL;                /* beginning of bitmap file data */
   ULONG  cbRead;                        /* Number of bytes read by DosRead. */
   ULONG  cScans, cScansRet, width;   /* number of scan lines in bitmap (cy) */
   BITMAPINFOHEADER2 bmp2BitmapFile;
   PBITMAPINFOHEADER2 pbmp2BitmapFile = &bmp2BitmapFile;
   FILE *f = fopen( fileName, "rb" );
   HBITMAP hbmBitmap;

   if( f == NULL ) return NULL;

   /*
    *   Use Loop to avoid duplicate cleanup code.  If any errors, a break
    *   statement will jump directly to error cleanup code.
    */

   do
   {
      /*
       *   Find out how big the file is, allocate that much memory, and read
       *   in the entire bitmap.
       */

      fseek( f, 0, SEEK_END );
      int fileSize = ftell( f );
      fseek( f, 0, SEEK_SET );

      pFileBegin = (PBYTE)malloc( fileSize );

      cbRead = fread( (PVOID)pFileBegin, fileSize, 1, f );

      fclose( f );

      /*
       *   If it's a bitmap-array, point to common file header.  Otherwise,
       *   point to beginning of file.
       */

      pbfh2 = (PBITMAPFILEHEADER2) pFileBegin;
      pbmp2 = NULL;                   /* only set this when we validate type */

      switch (pbfh2->usType)
      {
         case BFT_BITMAPARRAY:

            /*
             *   If this is a Bitmap-Array, adjust pointer to the normal
             *   file header.  We'll just use the first bitmap in the
             *   array and ignore other device forms.
             */

            pbfh2 = &(((PBITMAPARRAYFILEHEADER2) pFileBegin)->bfh2);
            pbmp2 = &pbfh2->bmp2;    /* pointer to info header (readability) */
            break;

         case BFT_BMAP:

            pbmp2 = &pbfh2->bmp2;    /* pointer to info header (readability) */
            break;

         default:      /* these formats aren't supported; don't set any ptrs */
         case BFT_ICON:
         case BFT_POINTER:
         case BFT_COLORICON:
         case BFT_COLORPOINTER:

            break;

      }   /* end switch (pbfh2->usType) */

      if (pbmp2 == NULL)
             break;        /* File format NOT SUPPORTED: break out to error code */
      /*
       *   Check to see if BMP file has an old structure, a new structure, or
       *   Windows structure.  Capture the common data and treat all bitmaps
       *   generically with pointer to new format.  API's will determine format
       *   using cbFixed field.
       *
       *   Windows bitmaps have the new format, but with less data fields
       *   than PM.  The old strucuture has some different size fields,
       *   though the field names are the same.
       *
       *
       *   NOTE: bitmap data is located by offsetting the beginning of the file
       *         by the offset contained in pbfh2->offBits.  This value is in
       *         the same relatie location for different format bitmap files.
       */

      if (pbmp2->cbFix == sizeof(BITMAPINFOHEADER))           /* old format? */
      {
         width = (ULONG) ((PBITMAPINFOHEADER)pbmp2)->cx;
         cScans = (ULONG) ((PBITMAPINFOHEADER)pbmp2)->cy;
      }
      else                               /* new PM format, Windows, or other */
      {
         width = pbmp2->cx;
         cScans = pbmp2->cy;
      }

      memcpy(                      /* copy bitmap info into global structure */
         (PVOID) pbmp2BitmapFile,
         (PVOID) pbmp2,
         pbmp2->cbFix);      /* only copy specified size (varies per format) */

      hbmBitmap =
      GpiCreateBitmap(
         hps,                         /* presentation-space handle */
         pbmp2BitmapFile,            /* address of structure for format data */
         0L,                                                      /* options */
         NULL,                            /* address of buffer of image data */
         NULL);                 /* address of structure for color and format */

      if (!hbmBitmap)
         break;                        /* jump to error code outside of loop */

      if (GpiSetBitmap( hps, hbmBitmap) == (HBITMAP)BMB_ERROR)
         break;                        /* jump to error code outside of loop */

      /*
       *   Tell GPI to put the bits into the thread's PS. The function returns
       *   the number of scan lines of the bitmap that were copied.  We want
       *   all of them at once.
       */

      cScansRet =
      GpiSetBitmapBits(
         hps,                         /* presentation-space handle */
         0L,                                     /* index of first scan line */
         cScans,                                     /* number of scan lines */
         pFileBegin + pbfh2->offBits,              /* address of bitmap data */
         (PBITMAPINFO2) pbmp2);            /* address of bitmap header table */

      if (cScansRet != cScans)                       /* original # of scans? */
         break;                        /* jump to error code outside of loop */

      free( pFileBegin);
      return new Bitmap(hbmBitmap, width, cScans );

   } while (FALSE); /* fall through loop first time */

    /*
     *   Close the file, free the buffer space and leave.  This is an error exit
     *   point from the function.  Cleanup code is here to avoid duplicate code
     *   after each operation.
     */

    if (pFileBegin != NULL)
       free( pFileBegin);
    return NULL;                                         /* function failed */

// Remove "variable assigned a value that is never used" warning.
#pragma warn -aus
}   /* end ReadBitmap() */
#pragma warn +aus

BmpGraphics::BmpGraphics() : Graphics()
{
  SIZEL    sizl = {0,0};
  DEVOPENSTRUC dop = { NULL
                       , "DISPLAY"
                       , NULL
                       , NULL
                       , NULL
                       , NULL
                       , NULL
                       , NULL
                       , NULL };

  hdc = DevOpenDC( Application::hab, OD_MEMORY, "*", 5L,
                   (PDEVOPENDATA)&dop, NULLHANDLE);
  hps = GpiCreatePS( Application::hab
                        , hdc
                        , &sizl
                        , PU_PELS | GPIA_ASSOC/* | GPIT_MICRO */);

  this->hdc = hdc;
};

