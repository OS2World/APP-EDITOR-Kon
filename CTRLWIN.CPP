#include "ctrlwin.h"
#include "mainwadm.h"
#include "mainwnd.h"
#include "bufadmin.h"

ControllerWindow *ControllerWindow::instance = NULL;
char *ControllerWindow::title = "KonControllerWindow";

int ControllerWindow::execFromMainThread( int task, void *obj )
{
  BOOL threadLoading = TRUE, queryOnModified = TRUE;
  switch( task )
  {
    // Delete a main window and close the editor if this is the last one
    case 5 : threadLoading = FALSE;
    case 1:
    {
      return loadFile( NULL, (char *)obj, threadLoading, TRUE, Settings::reuseExistingBuffer );
    }
    case 2 :  // Set focus
    {
      ((Window *)obj)->setFocus();
      break;
    }
    case 6 : queryOnModified = FALSE;
    case 3 : // Close control
    {
      EditControl *control = (EditControl *)obj;
      int idx = ControlAdmin::getControlIndex( control );
      if( idx != -1 )
      {
        MainWindow *parent = ControlAdmin::getControl(idx)->parent;
        parent->closeControl( control, queryOnModified );
      }
      break;
    }
    case 4 :
    {
      delete (Window *)obj;
      break;
    }
    case 7 :
    {
      // Create a new MainWindow

      MainWindow *mainWin =
        new MainWindow( Desktop::getInstance(), "Kon", 10, 10, 600, 500 );
      mainWin->setVisible( TRUE );
			char filename[MAXPATH];
			BufferAdmin::getNextScratchName( filename );
      mainWin->addFile( filename, TRUE, TRUE );
      break;
    }
    case 30 :
    {
    	if( MainWindowAdmin::getActiveWindow() != NULL )
	    	return (int)MainWindowAdmin::getActiveWindow()->getHWNDFrame();
      else
      	return NULL;
    }
		case 8 :
		{
			// Close the frame window which contains this editControl

			MainWindow *mainWnd = ControlAdmin::getControl(ControlAdmin::getControlIndex((EditControl *)obj))->parent;
			
			if( mainWnd != NULL )
				mainWnd->close();
			break;
		}
		case 9 :  threadLoading = FALSE;
		case 10 :
		{
			// Check if the file is already loaded
      return loadFile( NULL, (char *)obj, threadLoading, TRUE, Settings::reuseExistingBuffer, TRUE );
		}
  }
  return 0;
}

void ControllerWindow::dataReceived( char *message )
{
  Application::executeFromMainThread( ControllerWindow::getInstance(), 1,
                                      message );
}

int ControllerWindow::loadFile( MainWindow *window, char *filename, BOOL threadedLoading, BOOL killScratchImmediately, BOOL useExisting, BOOL setFocus )
{
  int ID;
	char newFilename[MAXPATH];

	if( !strcmp( filename, "***" ) )
		BufferAdmin::getNextScratchName( newFilename );
	else
		strcpy( newFilename, filename );

  if( window == NULL ) window = MainWindowAdmin::getActiveWindow();

  if( window == NULL ) return -1;

	ControlInfo *control = ControlAdmin::queryControlWithFilename( newFilename );

	if( control == NULL || !useExisting)
	{
		// Load the file
		EditControl *ec = window->addFile( newFilename, threadedLoading, killScratchImmediately );
		ID = ec->getId();
		control = ControlAdmin::queryControlWithId( ID );
	}

  MainWindow *parent = control->parent;
  EditControl *editControl = control->editControl;

  ID = editControl->getId();

  if( setFocus ) parent->setFocus();  // Will destroy the controlInfo control
  // Jump to the correct position
  parent->selectControl( control->editControl );
  return ID;
}
