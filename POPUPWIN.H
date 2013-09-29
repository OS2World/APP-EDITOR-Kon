#if !defined POPUPWINDOW

#define POPUPWINDOW

class PopupWindow : public Window, public virtual AMouseListener
{
  char *text;
public:
  PopupWindow( Window *parent, char *text, int x, int y, BOOL belowCursor );
  virtual void paint( Rect *rect, Graphics *g );
	~PopupWindow()
	{
		delete text;
	}
};

#endif