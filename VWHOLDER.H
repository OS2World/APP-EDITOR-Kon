/******************************************************************************
 * EditControl should subclass this in order to make it possible for the view
 * to retrieve information about the view windows height, order paint the view,
 * and to be able to set the scrollbars
 */

#if !defined VIEWWINDOWHOLDER

class View;
class ViewWindow;

class ViewWindowHolder
{
public:
  virtual int getViewLines() { return 0; }
  virtual int getViewVidth() { return 0; }
  virtual int getLeftMargin() { return 0; }
  virtual void setHorzScrollbar( int pos, int visible, int total ) {}
  virtual void setVertScrollbar( int pos, int visible, int total ) {}
  virtual void repaintView( Rect *rect ) {}
  virtual void updatePosition( View *view, int reader, int start, int end ) {};
  // Use addUpdatePosition to add an update region when inside a bufferObserver
  // method. Using updatePosition will give a bad result.
  virtual void addUpdatePosition( View *view, int reader, int start, int end ) {};
  virtual void updateLines( View *view, int startLine, int endLine ) {};
  virtual void pageDown() {}
  virtual void pageUp() {}
  virtual BOOL viewActive() { return FALSE; }
  virtual void setViewActive( BOOL active ) {}
  virtual Graphics *getViewGraphics() { return NULL; }
  virtual void setCursorVisible() {}
  virtual void snapshot() {}
  virtual int queryReplaceDialog( char *foundString, char *replaceString, 
                                  int pos, int len, BOOL multiFile ) 
    { return 0; }
  virtual void setFocus() {}
  virtual View *getView() { return NULL; }
  virtual void addCommand( char *command ) {}
  virtual void setUpdate( BOOL ) {}
  virtual void setActive( BOOL ) {}
  virtual BOOL getUpdate( void ) { return FALSE; }
  virtual void popupMenu( Point *point ) {};
	virtual void userAsynchMessage( char * message ) {}
	virtual void userAsynchClearMessage() {}
  virtual void fileRenamed() {}
};
#define VIEWWINDOWHOLDER
#endif
