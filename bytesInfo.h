#if !defined BYTESINFO
#define BYTESINFO

class BytesInfo : public Window
{
  unsigned char b1, b2, b3, b4;
public:
  BytesInfo( Window *parent, int x, int y, int width, int height ) : 
    b1(0), b2(0), b3(0), b4(0), Window( parent, x, y, width, height )
  {
  }
  void paint(Rect *rect, Graphics *g );
  void bytes( int b1, int b2, int b3, int b4 );
  static int getInset() { return 10;}
};

#endif