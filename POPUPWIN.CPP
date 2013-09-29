#include "bowl.h"

#include "popupwin.h"

PopupWindow::PopupWindow( Window *parent, char *text, int x, int y, BOOL belowCursor ) :
  Window( parent, xlatProperties(POPUP), x, y, 1, 1 )
{
  this->text = new char[strlen(text)+1];
  strcpy( this->text, text );

	// WS_POPUP

	remapCoordinates( parent, Desktop::getInstance(), x, y);

  // Test print the text and calc dimensions of the window

  getGraphics()->setSystemFont();
  int nHeight = getGraphics()->getFontHeight() + 6;
  Point *pnt;

  pnt = new Point[strlen(text)+1];

  getGraphics()->calcStringCharPos( 0, 0, strlen( text ), text, pnt );

  setSize(pnt[strlen(text)].x + 10, nHeight );

	if( belowCursor )
	{
		int w, h;

		getCursorExtent( w, h );
		setPos(x+w, y-h-getHeight());
	}
	else
		setPos(x, y);

  delete pnt;
}
void PopupWindow::paint( Rect *rect, Graphics *g )
{
  g->setColor( Color::WHITE );
  g->setBackColor( Color::WHITE );
  g->fillRect( rect );

  Rect border( 0, 0, width-1, height-1 );

  g->setColor( Color::BLACK );

  g->drawRect( &border );

  g->setBackColor( Color::WHITE );
  g->setSystemFont();

  g->drawString( 3, 3 + g->getFontBaseline(), text );
}