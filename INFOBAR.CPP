#include "bowl.h"
#include "infobar.h"
#include "globals.h"

void InfoBar::drawShade( Graphics *g, int x, int y, int width, int height,
                         BOOL in )
{
  if( in )
    g->setColor( Color::GRAY );
  else
    g->setColor( Color::WHITE );

  g->drawLine( x, y+1, x, y+height-1 );
  g->drawLine( x, y+height-1, x+width-2, y+height-1 );

  if( in )
    g->setColor( Color::WHITE );
  else
    g->setColor( Color::GRAY );

  g->drawLine( x+width-1, y+height-2, x+width-1, y );
  g->drawLine( x+width-1, y, x+1, y );
}

void InfoBar::paint( Rect *rect, Graphics *g )
{
  g->setBackColor( Color::BKGR );

  g->fillRect( rect );

  g->setSystemFont();

  if( !positionsSet )
  {
    Point points[50];
    char testString[50];

    // Calculate the positions for the vertical dividers

    strcpy( testString, "1000:1234" );

    g->calcStringCharPos( 0, 0, strlen( testString ), testString, points );

    modifiedX = 6 + points[strlen( testString )].x + 4;

    g->calcStringCharPos( 0, 0, strlen( modifiedString ), modifiedString,
                          points );

    insertX = modifiedX + points[strlen(modifiedString)].x + 3 + 4;

    g->calcStringCharPos( 0, 0, strlen( insertString ), insertString, points );

    int longest = points[strlen(insertString)].x;

    g->calcStringCharPos( 0, 0, strlen( overwriteString ), overwriteString, points );

    recordX = ((points[strlen(overwriteString)].x > longest) ?
               points[strlen(overwriteString)].x : longest) + insertX + 3 + 4;

    g->calcStringCharPos( 0, 0, strlen( recordString ), recordString,
                          points );

    busyX = recordX + points[strlen(recordString)].x + 3 + 4;

    lastMessageX = busyX + 14;
  }

  drawShade( g, 0, 0, width, height, FALSE );
  drawShade( g, 2, 2, modifiedX - 3, height-4, TRUE );
  drawShade( g, modifiedX, 2, insertX - modifiedX - 1, height-4, TRUE );
  drawShade( g, insertX, 2, recordX - insertX - 1, height-4, TRUE );
  drawShade( g, recordX, 2, busyX - recordX - 1, height-4, TRUE );
  drawShade( g, busyX, 2, lastMessageX - busyX - 1, height-4, TRUE );

  if( busyShowing )
  {
    Rect rc2( busyX+1, 2, lastMessageX - busyX - 3, height-4 );
    g->setBackColor( Color::RED );
    g->fillRect( &rc2 );
    g->setBackColor( Color::BKGR );
  }

  drawShade( g, lastMessageX, 2, width - lastMessageX - 2, height-4, TRUE );

  g->setColor( Color::BLACK );

  int baseline = g->getFontBaseline();

  g->drawString( 4, 4+baseline, posInfo );
  g->drawString( modifiedX + 2, 4+baseline, (modified) ? modifiedString : "" );
  g->drawString( insertX + 2, 4+baseline, (Settings::insertMode) ? insertString : "" );

  g->setColor( Color::RED );
  g->drawString( recordX + 2, 4+baseline, (record) ? recordString : "" );
  g->setColor( Color::BLACK );

  g->drawString( lastMessageX + 2, 4+baseline, lastMessage );
}

void InfoBar::updatePosition( char *posString )
{
  strcpy( posInfo, posString );
  Rect rect( 3, 3, modifiedX - 5, height - 6);
  repaint( &rect );
}

void InfoBar::message( char *message )
{
  if( !strcmp( message, lastMessage ) ) return;
  strcpy( lastMessage, message );
  Rect rect( lastMessageX + 1, 3, width - lastMessageX - 3, height - 6);
  repaint( &rect );
}

void InfoBar::clearMessage()
{
  message( "" );
}

void InfoBar::setModified( BOOL isModified )
{
	modified = isModified;
  Rect rect( modifiedX + 1, 3, width - insertX - 3, height - 6);
  repaint( &rect );
}

void InfoBar::setRecord( BOOL isRecording )
{
	record = isRecording;
  Rect rect( recordX + 1, 3, width - lastMessageX - 3, height - 6);
  repaint( &rect );
}

void InfoBar::showBusy( BOOL isBusy )
{
  if( busyShowing == isBusy ) return;
  busyShowing = isBusy;
  Rect rect( busyX + 1, 3, lastMessageX - busyX - 3, height - 6);
  repaint( &rect );
}
