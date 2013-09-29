#include "print.h"
#include "printdlg.h"
#include "kerrcoll.h"
#include "winprop.h"
#include <time.h>

int Print::ODDPAGES = 1;
int Print::EVENPAGES = 2;
int Print::TWOUP = 4;
int Print::SELECTED = 8;
int Print::LINES = 16;
int Print::PRINTCOLOR = 32;
int Print::PRINTSYNTAXHL = 64;

BOOL Print::initialized = FALSE;
char Print::printerFont[MAXFONTNAMELEN];
char Print::headerFont[MAXFONTNAMELEN];
int Print::printerFontSize = 10;
int Print::headerFontSize = 8;

Print::Print( View *view, char *printer, int options, char *header, 
							char *footer, int fromLine, int toLine, int leftMargin,
              int rightMargin, int topMargin, int bottomMargin, char *docName ) :
              view(view), options(options), fromLine(fromLine), toLine(toLine),
              leftMargin(leftMargin), rightMargin(rightMargin), topMargin(topMargin),
              bottomMargin(bottomMargin)
{
  this->printer = new char[strlen(printer)+1];
  strcpy( this->printer, printer );
  this->header = new char[strlen(header)+1];
  strcpy( this->header, header );
  this->footer = new char[strlen(footer)+1];
  strcpy( this->footer, footer );
  this->docName = new char[strlen(docName)+1];
  strcpy( this->docName, docName );
}

void Print::addEscapeCodes( char *sourceString, char *targetString, int pageNumber )
{
	while( *sourceString )
	{
		switch( *sourceString )
		{
			case '&' :
				char str[MAXPATH];
				sourceString++;
				*targetString = 0;
				strcpy( str, "" );
				switch( *sourceString )
				{
					case 'p' :
						itoa( pageNumber, str, 10 );
						break;
					case 'd' :
						time_t aclock;
						struct tm *newtime;
						time( &aclock );
						newtime = localtime( &aclock );
						strcpy( str, "Date format error!" );
						char formatString[40];
						int n;

						n = 0;

						strcpy( formatString, "%Y-%m-%d %H:%M" );	// Default format

						if( *(sourceString + 1) == '[' ) // Has format string
						{
							while( n < 40 && *(sourceString + n) != ']' && 
							       *(sourceString + n) != 0 ) n++;

							if( *(sourceString + n) == ']' )
							{
								strcpy( formatString, sourceString+2 );
								formatString[n-2] = 0;
								sourceString += n;
							}
						}

						strftime( str, 40, formatString, newtime );
						break;
					case 'n' :
						strcpy( str, docName );
				}
				sourceString++;
				strcat( targetString, str );
				targetString += strlen( str );
				break;
			default :
				*targetString++ = *sourceString++;
		}
	}
	*targetString = 0;
}

int Print::setupPage( int pagenumber )
{
	int newLen = strlen( header ) + 1;
	char *str;

	if( strstr( header, "&p" ) != NULL ) newLen += 11;
	if( strstr( header, "&d" ) != NULL ) newLen += 40;	// Can be very long, don't know how long
	if( strstr( header, "&n" ) != NULL ) newLen += strlen( docName );

	str = new char[newLen];

	addEscapeCodes( header, str, pagenumber );

	// Draw header and footer
	prn->setColor( Color::BLACK );
	prn->setBackColor( Color::WHITE );
  prn->selectFont( headerFont, headerFontSize, Font::ITALIC );
  prn->drawString( leftMargin, prn->getHeight()-topMargin-prn->getFontHeight(), 
  								 strlen(str), str );

	delete str;
	newLen = strlen( footer );
	if( strstr( footer, "&p" ) != NULL ) newLen += 11;
	if( strstr( footer, "&d" ) != NULL ) newLen += 40;
	if( strstr( footer, "&n" ) != NULL ) newLen += strlen( docName );

	str = new char[newLen];

	addEscapeCodes( footer, str, pagenumber );

	prn->setColor( Color::BLACK );

  int yPos = prn->getHeight()-topMargin-prn->getMaxFontHeight() - 10;
  int tPos = yPos;
  prn->drawLine( leftMargin, yPos, prn->getWidth()-rightMargin, yPos );

  prn->drawString( leftMargin, bottomMargin, 
  								 strlen(str), str );

	delete str;

  yPos = bottomMargin+prn->getFontHeight() - 10;
  prn->drawLine( leftMargin, yPos, prn->getWidth()-rightMargin, yPos );

  if( options & TWOUP )
  	prn->drawLine( prn->getWidth() / 2, tPos, prn->getWidth() / 2, yPos );

  return prn->getFontHeight() * 2 + 10;
}

void Print::printPages()
{
	int reader = view->textRequest(), page = 1;
  int position, height, width = prn->getWidth(), maxBaseline, 
  		yPos = prn->getHeight()-topMargin, headerHeight, indent = 0, rightMost,
			viewLine = -1;

	headerHeight = setupPage(page);
	yPos -= headerHeight;

	position = view->getPositionFromLine( reader, fromLine );

  rightMost = width - rightMargin;
  indent = 0;

  int innerMargin = prn->getWidth() / 30;

 	if( options & TWOUP )
  {
  	width = (prn->getWidth() - leftMargin - innerMargin * 2 - rightMargin) / 2;
    rightMost = prn->getWidth() / 2 - innerMargin;
  }

	abortPrintDlg->setPage( page );

	for( int l = 0; l <= toLine-fromLine; l++ )
  {
  	height = maxBaseline = 0;

  	TextFont *textFont = NULL, *firstFont = NULL;

    firstFont = textFont =
      view->getTextlineInfo( reader, position, viewLine, height, maxBaseline, 
			                       prn, View::PRINTER, prn->getWidth() );
    yPos -= height;

    if( yPos < bottomMargin + headerHeight )
    {
    	yPos = prn->getHeight()-topMargin-headerHeight-height;
    	if( options & TWOUP )
      {
      	if( page % 2 != 0 )
        {
        	indent = prn->getWidth() / 2 - leftMargin + innerMargin;
          rightMost = prn->getWidth() - rightMargin;
          ++page;
        }
        else
        {
        	prn->formfeed();
        	setupPage(++page);
          indent = 0;
				  rightMost = width - rightMargin;
					abortPrintDlg->setPage( page );
        }
      }
      else
      {
      	prn->formfeed();
        setupPage(++page);
				abortPrintDlg->setPage( page );
			}
    }

    int lastX = 0;

    while( textFont )
    {
      if( textFont->X == -1 )
        prn->selectFont( textFont->font /*prn->getFontFromName("Courier New", textFont->attr)*/, textFont->points, textFont->attr );
      else
        prn->selectFont( textFont->font, textFont->X, textFont->Y,
                       textFont->attr );

      TextColor *textColor = textFont->textColor;

      while( textColor )
      {
        // Fill with the old color to the new pos if we have jumped forward
        // horizontally

        if( lastX != textColor->xPos )
        {
					int leftPos = lastX+leftMargin+indent;
          Rect rect( leftPos, yPos,
                     rightMost-leftPos, height );
          prn->fillRect( &rect );
        }

        if( options & PRINTCOLOR )
        {
	        prn->setColor( textColor->fg );
  	      prn->setBackColor( textColor->bg );
        }

        lastX = textColor->lastX;

        int lineLength = textColor->length;

        if( textColor->lastIsTab )
        {
          lastX = textColor->xBeforeTab;
          textColor->length--;
        }

        // We may have to  draw the text in a colored rectangle
        // to make sure all of the background is colored

        int baseline = (maxBaseline > textFont->baseline) ? maxBaseline :
                                                            textFont->baseline;
        int baselineDelta = baseline - textFont->baseline;

        int leftPos = textColor->xPos+leftMargin+indent;

        if( leftPos < rightMost )
        {
          Rect trect( leftPos, yPos, rightMost-leftPos,//lastX+leftMargin+indent,
                      height );

				  // Remove all tab characters. They mess up things

          for( int i = 0; i < lineLength; i++ )
        	  if( textColor->text[i] == 0x9 )
          	  textColor->text[i] = 0x20;

          prn->drawString( leftPos, yPos + baseline,
                         &trect, Graphics::fontOpaque,
                         lineLength, textColor->text );
        }
        textColor = textColor->next;
      }

      textFont = textFont->next;
    }

    if( firstFont != NULL )
      delete firstFont;
  }

  prn->formfeed();

  view->textRelease();
}

void Print::startPrint()
{
	prn = new PrinterGraphics( docName, printer );
	leftMargin = (prn->getWidth() * leftMargin) / prn->getXRes();
  rightMargin = (prn->getWidth() * rightMargin) / prn->getXRes();
  topMargin = (prn->getHeight() * topMargin) / prn->getYRes();
  bottomMargin = (prn->getHeight() * bottomMargin) / prn->getYRes();
  printPages();
}

void Print::abortPrint()
{
	prn->terminateDocument();
}

void Print::saveSettings( FILE * f )
{
	long pos;
	if( !ClassProperties::savePropertyName( f, "PRINT001", &pos ) ) return;
	fwrite( &initialized, sizeof( BOOL ), 1, f );
	fwrite( printerFont, MAXFONTNAMELEN, 1, f );
	fwrite( headerFont, MAXFONTNAMELEN, 1, f );
	fwrite( &printerFontSize, sizeof( int ), 1, f );
	fwrite( &headerFontSize, sizeof( int ), 1, f );
	ClassProperties::savePropertySize( f, pos );
}

void Print::restoreSettings( FILE * f )
{
	if( !ClassProperties::checkPropertyExists( f, "PRINT001" ) ) return;
	fread( &initialized, sizeof( BOOL ), 1, f );
	fread( printerFont, MAXFONTNAMELEN, 1, f );
	fread( headerFont, MAXFONTNAMELEN, 1, f );
	fread( &printerFontSize, sizeof( int ), 1, f );
	fread( &headerFontSize, sizeof( int ), 1, f );
}

Print::~Print()
{
	prn->endDocument();
	delete prn;
	delete printer;
 	delete header;
 	delete footer;
}

