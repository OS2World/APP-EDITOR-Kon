#include "bowl.h"
#include "editctrl.h"
#include "cntrladm.h"
#include "ctrlwin.h"

Vector<ControlInfo> ControlAdmin::controls(TRUE);
Vector<ControlInfo> ControlAdmin::exitList(TRUE);
MutexSemaphore ControlAdmin::lockExitList;

ControlInfo *ControlAdmin::queryControlWithFilename( char *filename )
{
  char fullPath[MAXPATH+1];

  if( BufferAdmin::isScratchName( filename ) )
    strcpy( fullPath, filename );
  else
    System::getFullPath( filename, fullPath, MAXPATH );

  for( int i = 0; i < controls.getSize(); i++ )
  {
    char *fn = controls.getObjectAt(i)->editControl->getTextBuffer()->
                   getFilename();
    if( !stricmp( controls.getObjectAt(i)->editControl->getTextBuffer()->
                   getFilename(),
                 fullPath ) )
      return controls.getObjectAt(i);
  }
  return NULL;
}

ControlInfo *ControlAdmin::queryControlWithId( int id )
{
  for( int i = 0; i < controls.getSize(); i++ )
    if( controls.getObjectAt(i)->editControl->getId() == id )
      return controls.getObjectAt(i);
  return NULL;
}

int ControlAdmin::getControlIndex( EditControl *eControl )
{
  for( int i = 0; i < controls.getSize(); i++ )
    if( controls.getObjectAt(i)->editControl == eControl )
      return i;
  return -1;
}

void ControlAdmin::addToExitList( EditControl *control, BOOL closeMainWindow )
{
	lockExitList.request();
	if( closeMainWindow )
	{
		exitList.insertAt( 0, 
		       new ControlInfo( getControl(getControlIndex(control))->parent, control ) );
	}
	else
		exitList.insertAt( 0, new ControlInfo( NULL, control ) );
	lockExitList.release();
}

void ControlAdmin::processExitList()
{
  while( exitList.getSize() > 0  )
  {
		// We may get into this from another thread, so lock the exit list
		// while we use it

		lockExitList.request();

		if( exitList.getSize() == 0 ) 
		{
			lockExitList.release();
			break;
		}
		MainWindow *parent = exitList.getObjectAt(0)->parent;
		EditControl *eControl = exitList.getObjectAt(0)->editControl;

    exitList.removeAt( 0 );

		lockExitList.release();
    Application::executeFromMainThread( ControllerWindow::getInstance(), 
																				(parent == NULL) ? 3 : 8, eControl );
  }
}
