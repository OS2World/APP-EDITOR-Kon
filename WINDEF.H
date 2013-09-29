struct Rect
{
  int x, y, width, height;
  Rect() {}
  Rect( int x, int y, int width, int height )
  {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
  };
};

struct Point
{
  int x, y;
  Point() {}
  Point( int x, int y )
  {
    this->x = x;
    this->y = y;
  };
};

struct TwoInts
{
  int i1, i2;
  TwoInts( int i1, int i2 )
  {
    this->i1 = i1;
    this->i2 = i2;
  }
};

struct KeyDef
{
  static int CTRL, kALT, SHIFT, UP, DOWN, LEFT, RIGHT, PAGEUP, PAGEDOWN, INSERT,
             kDELETE, HOME, END, ESCAPE, F1, F2, F3, F4, F5, F6, F7, F8, F9,
             F10, F11, F12, ENTER, BACKSPACE, TAB, RETURN, SPACE;
};

