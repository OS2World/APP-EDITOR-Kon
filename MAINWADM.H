/*****************************************************************************
 * The MainWindowAdmin singleton class maintains a list of all opened
 * MainWindows. It is used for making global updates to MainWindows, such
 * as global menu and toolbar updates.
 * It also handles various methods for objects that needs to use a specific
 * main window but does not have a handle to i
 */

#if !defined MAINWADM

#include <string.h>

class MainWindow;

#define MAINWADM

class MainWindowAdmin
{
	static MainWindow **mWindows; // Can't use Vector here due to inheritance
		                            // problem... Should solve that any year now.
	static int windowCount;
	static MainWindow *activeWindow;
public:
	static int ACTIVECONTROL, INFOBAR, BUTTONS, MAINWINDOWLAYOUT, MENU, FRAME;
  static void addWindow( MainWindow *mWindow )
  {
    mWindows = (MainWindow **)realloc( (void *)mWindows,
                                sizeof( MainWindow * ) * (windowCount + 1) );
    mWindows[windowCount] = mWindow;
    windowCount++;
  }

  static void removeWindow( MainWindow *mWindow );

  // Returns the currently active window, NULL if none is active or no
  // main windows exist

  static MainWindow *getActiveWindow()
  {
    return activeWindow;
  }

  static void setActiveWindow( MainWindow *window );

  // If window is an instance of a notebook, select prev or next tab

  static void nextTab( Window *window );
  static void prevTab( Window *window );

  // Identify the main window by the notebook control supplied.
  // If the supplied window is not a notebook, the method does nothing

  static void newFile( char *file, Window *window );
  static int getWindows() { return windowCount; }
	static void updateAllWindows( int parts );
};

#endif