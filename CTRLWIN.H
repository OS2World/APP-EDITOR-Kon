#include "bowl.h"

#if !defined CONTROLLERWINDOW
#define CONTROLLERWINDOW

// The controller window closes other windows
class MainWindow;

class ControllerWindow : public Window
{
  static ControllerWindow *instance;
  static char *title;
public:
  ControllerWindow() : Window( Desktop::getInstance(), title, 0, 0, 0, 0 ) {}

  virtual int execFromMainThread( int task, void *obj );
  static char *getTitle() { return title; }
  static void createInstance()
  {
    instance = new ControllerWindow();
  }
  static ControllerWindow *getInstance() { return instance; }
	virtual void dataReceived( char *message );
  virtual int loadFile( MainWindow *window, char *filename, BOOL threadedLoading, BOOL killScratchImmediately, BOOL useExisting = FALSE, BOOL setFocus = FALSE ); 
};

#endif