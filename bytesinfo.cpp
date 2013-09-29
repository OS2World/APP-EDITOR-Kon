#include "bowl.h"
#include "bytesinfo.h"

void BytesInfo::paint(Rect *rect, Graphics *g )
{
  g->setBackColor( Color::BKGR );

  g->fillRect( rect );

  g->setSystemFont();

  g->setColor( Color::WHITE );
  g->drawLine( 0, 0, 0, getHeight()-1 );
  g->drawLine( 0, getHeight()-1, getWidth()-1, getHeight()-1 );
  g->setColor( Color::GRAY );
  g->drawLine( 0, 0, getWidth()-1, 0 );
  g->drawLine( getWidth()-1, 0, getWidth()-1, getHeight()-1 );
  g->setColor( Color::BLACK );
  char str[100];

  sprintf( str, "byte: %u   int16: %hd   uint16: %hu   int32: %ld   uint32: %lu",
           b1 & 0x000000ff,
           0xffff & (b2 << 8 | b1),
           0xffff & (b2 << 8 | b1), 
           b1 | (b2 << 8) | (b3 << 16) | (b4 << 24),
           b1 | (b2 << 8) | (b3 << 16) | (b4 << 24) );

  Rect *r = new Rect( 2, 2, getWidth() - 4, getHeight() - 4 );
  g->drawString(3, (getHeight() - g->getMaxFontHeight()) / 2, r, 0, str );
}

void BytesInfo::bytes(int b1, int b2, int b3, int b4 )
{
  this->b1 = b1;
  this->b2 = b2;
  this->b3 = b3;
  this->b4 = b4;
  repaint( NULL );
}