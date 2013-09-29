/*****************************************************************************
* The ControlAdmin singleton class maintains a list of all EditControl
* instances.
*
* Anyone can ask if there is already a control opened with a specific file,
* or retrieve EditControl instances.
*/

#if !defined CNTRLADMIN

#define CNTRLADMIN

//#include "editctrl.h"
#include <string.h>

class MainWindow;
class EditControl;

class ControlInfo : public AObject
{
public:
  MainWindow *parent;
  EditControl *editControl;

  ControlInfo( MainWindow *parent, EditControl *editControl )
  {
    this->parent = parent;
    this->editControl = editControl;
  }

  // returns TRUE if the filename matches for the textBuffers

/*  virtual int equals( AObject *object )
  {
    ControlInfo *eControl = (ControlInfo *)object;

    if( !stricmp( eControl->getTextBuffer()->getFilename,
                  editControl->getTextBuffer()->getFilename )
      return TRUE;

    return FALSE;
  }*/
};

class ControlAdmin
{
static Vector<ControlInfo> controls, exitList;
static MutexSemaphore lockExitList;
public:
  static void insertControl( MainWindow *parent, EditControl *editControl )
  {
    ControlInfo *newC = new ControlInfo( parent, editControl );
    controls.insertAt( 0, newC );
  }
  static ControlInfo *queryControlWithFilename( char *filename );
  static ControlInfo *queryControlWithId( int id );
  static int getControls() { return controls.getSize(); }
  static ControlInfo *getControl( int i ) { return controls.getObjectAt(i); }
  static void removeControl( int i )
  {
    controls.removeAt( i );
  }
  static int getControlIndex( EditControl *eControl );
  static void addToExitList( EditControl *control, BOOL closeMainWindow );
  static void processExitList();
};

#endif
