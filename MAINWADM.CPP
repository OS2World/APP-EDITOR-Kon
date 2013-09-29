#include "bowl.h"
#include "mainwadm.h"
#include "mainwnd.h"

MainWindow **MainWindowAdmin::mWindows = NULL;
int MainWindowAdmin::windowCount = 0;
MainWindow *MainWindowAdmin::activeWindow = NULL;

int MainWindowAdmin::ACTIVECONTROL = 1;
int MainWindowAdmin::INFOBAR = 2;
int MainWindowAdmin::BUTTONS = 4;
int MainWindowAdmin::MAINWINDOWLAYOUT = 8;
int MainWindowAdmin::MENU = 16;
int MainWindowAdmin::FRAME = 32;

void MainWindowAdmin::removeWindow( MainWindow *mWindow )
{
  for( int i = 0; i < windowCount; i++ )
    if( mWindow == mWindows[i] )
    {
      memmove( &mWindows[i], &mWindows[i+1], sizeof( mWindows ) * (windowCount-i-1) );
      windowCount--;
      break;
    }

	// If the active window is the one that is removed,
  // then, set the activeWindow to any other window.
  // If the next window focused is a Kon window, the activeWindow
  // will be correctly set again

  if( mWindow == activeWindow )
  	activeWindow = mWindows[0];
}

void MainWindowAdmin::nextTab( Window *window )
{
  for( int i = 0; i < windowCount; i++ )
    if( mWindows[i]->getNotebook() == window )
    {
      mWindows[i]->nextTab();
      break;
    }
}

void MainWindowAdmin::prevTab( Window *window )
{
  for( int i = 0; i < windowCount; i++ )
    if( mWindows[i]->getNotebook() == window )
    {
      mWindows[i]->prevTab();
      break;
    }
}


void MainWindowAdmin::newFile( char *file, Window *window )
{
  for( int i = 0; i < windowCount; i++ )
    if( mWindows[i]->getNotebook() == window )
    {
      ControllerWindow::getInstance()->loadFile( mWindows[i], file, TRUE, TRUE, Settings::reuseExistingBuffer );
//      mWindows[i]->addFile( file, TRUE, TRUE );
      break;
    }
}

void MainWindowAdmin::setActiveWindow( MainWindow *window )
{
  activeWindow = window;
}

void MainWindowAdmin::updateAllWindows( int parts )
{
  for( int i = 0; i < windowCount; i++ )
	{
    //mWindows[i]->repaint( NULL, TRUE );
    if( parts & FRAME )
      mWindows[i]->size( -1, -1 );

		if( parts & ACTIVECONTROL )
			mWindows[i]->getActiveControl()->repaintView(NULL);

		if( parts & INFOBAR )
			mWindows[i]->updateInfobar();
		if( parts & MENU )
	    mWindows[i]->updateMenu();
	}
}
