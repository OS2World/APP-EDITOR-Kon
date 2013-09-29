#if !defined KERRCOLL

#define KERRCOLL

#include "bowl.h"
#include "kerror.h"

class EditControl;	// Forward definition 

class KErrorCollector : public TErrorCollector, public Window
{
  BOOL visible;
	EditControl *messagesControl;
  static KErrorCollector *instance;
	void addString( char *str );
public:
  KErrorCollector( char *errTitle );
  void show();
  void hide();
  virtual int focus( BOOL newFocus );
  BOOL isVisible();
  virtual BOOL close();
  virtual void Error( int column, int line, char *str );
  virtual void Error( char *file, int column, int line, char *str );
  virtual void message( char *str );
  static char *getBuffername() { return "*messages*";}

  static void createInstance( char *errTitle )
  {
    instance = new KErrorCollector( errTitle );
  }

  static KErrorCollector *getInstance()
  {
    return instance;
  }

  static void killInstance()
  {
    delete instance;
  }

  virtual void notice( char *note )
  {
  }
  
  virtual void clear();

  virtual ~KErrorCollector();
};

#endif