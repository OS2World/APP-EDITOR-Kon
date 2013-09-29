/******************************************************************************
 * Print dialog
 */

#if !defined COMMONDIALOGS
#define COMMONDIALOGS

class QueryLineDialog : public Dialog, public virtual AButtonListener
{
  char *name;
  Button *OKButton, *cancelButton;
  EntryField *textEntry;
public:
	static char text[1000];
  QueryLineDialog( Window *frame, Window *parent, char *title, char *query, char *initText );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  ~QueryLineDialog()
  {
    WinProperties::updateProperty( name, getX(), getY(),
                                   getWidth(), getHeight() );
		delete name;
  }
};

class AboutWindow : public Window
{
	Bitmap *bmp;
public:
	AboutWindow( Window *parent, int x, int y, int width, int height, Bitmap *bmp ) : 
	  Window( parent, x, y, width, height ), bmp(bmp) {}
	virtual void paint( Rect *rect, Graphics *g );
};

class AboutDialog : public Dialog, public virtual AButtonListener
{
	Bitmap *bmp;
  Button *OKButton, *registerButton;
public:
	AboutDialog( Window *parent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	~AboutDialog() { delete bmp; }
};

class RegisterDialog : public Dialog, public virtual AButtonListener
{
  Button *OKButton, *cancelButton;
  EntryField *nameEntry, *keyEntry;
public:
	RegisterDialog( Window *parent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
};

class UnregisteredWindow : public Window
{
	Bitmap *bmp;
public:
	UnregisteredWindow( Window *parent, int x, int y, int width, int height, Bitmap *bmp ) : 
	  Window( parent, x, y, width, height ), bmp(bmp) {}
	virtual void paint( Rect *rect, Graphics *g );
};

class UnregisteredDialog : public Dialog, public virtual AButtonListener
{
	Bitmap *bmp;
  Button *OKButton;
public:
	UnregisteredDialog( Window *parent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	~UnregisteredDialog() { delete bmp; }
};

class DirDialog : public Dialog, public virtual AButtonListener
{
  Button *OKButton, *cancelButton;
public:
	DirDialog( Window *parent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	~DirDialog() {}
};

#endif