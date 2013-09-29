//#define LICENSED
#define INCL_DOSPROCESS
#include "bowl.h"
#include "syntax.h"
#include "infobar.h"
#include "keybind.h"
#include "kstartup.h"
#include "editctrl.h"
#include "mainwnd.h"
#include "mainwadm.h"
#include "ctrlwin.h"
#include "bufadmin.h"
#include "autosave.h"
#include "comdlg.h"
#include "lic.h"
#include <time.h>

#define PIPENAME "Kon2Server"

class PipeServer : public Thread
{
public:
  PipeServer() : Thread(TRUE) {}
  virtual void run();
};

void PipeServer::run()
{
  // Create the pipe

  NamedMessagePipe *pipe = new NamedMessagePipe();

  if( pipe->create( PIPENAME ) )
    return;

  while( 1 )
  {
    char msg[300];
    int maxLength;

    if( !pipe->waitForMessage() )
    {
      maxLength = MAXPATH;
      while( !pipe->readMessage( msg, maxLength ) )
      {
        if( maxLength == 0 ) break;

        // Check that the main window is not in a focus routine. 
        // Otherwise we may end up in an infinite loop.
        MainWindowAdmin::getActiveWindow()->waitFocus();

        Application::executeFromMainThread( ControllerWindow::getInstance(), 1,
                                            msg );
        maxLength = MAXPATH;

        MainWindowAdmin::getActiveWindow()->releaseFocus();
      }
      pipe->closeConnection();
    }
    else
			return;
  }
}

void fixFilename( char *orgPath, char *dirEntry, char *newFile )
{
	char filename[MAXPATH];

	strcpy( filename, orgPath );
	char *bPos = strchr( filename, '\\' );

	if( bPos != NULL )
		*(bPos+1) = 0;
	else
		*filename = 0;

	strcat( filename, dirEntry );
	System::getFullPath( filename, newFile, MAXPATH );
}

int konMain( int argc, char *argv[] )
{
	int filesArgStart = 1;  //Where the files start on the command line
  char fullPath[MAXPATH+1];

/*  char test[7], result[MAXREGCODELEN*2];
  test[0] = 1;
  test[1] = 1;
  test[2] = 1;
  test[3] = 1;
  test[4] = 1;
  test[5] = 1;
  test[6] = 0;
  char *reg = "Björn Andersson";

  result[0] = 0;

  scramble( reg, result );*/

  // See if an instance is running: If so, send files

  Window::popupWindow( ControllerWindow::getTitle() );	// This is only necessary on Win

	if( NamedMessagePipe::supportsPipes() )
	{
    NamedMessagePipe *pipe = new NamedMessagePipe();

    if( !pipe->connect( PIPENAME ) )
    {
      if( argc == 1 )
        pipe->sendMessage( "***" );
      else
      {
        for( int i = 1; i < argc; i++ )
        {
					if( strchr( argv[i], '*' ) != NULL || strchr( argv[i], '?' ) != NULL )
					{
						Dir dir( argv[i], DirEntry::ARCHIVED, 0 );

						for( int d = 0; d < dir.getEntries(); d++ )
						{
							fixFilename( argv[i], dir.getEntryAt(d)->getName(), fullPath );
		          pipe->sendMessage( fullPath );
						}
					}
					else
					{
	    		  System::getFullPath( argv[i], fullPath, MAXPATH );
	          pipe->sendMessage( fullPath );
					}
        }
        delete pipe;
      }
	    Window::popupWindow( ControllerWindow::getTitle() );	// This is only necessary on Win
      return 0;
    }

    delete pipe;
	}
	else
	{
    Window *wnd = new Window( Desktop::getInstance(), 0, 0, 10, 10 );
		BOOL res = TRUE;

    if( argc == 1 )
	    res = wnd->sendDataToWindow( ControllerWindow::getTitle(), "***" );
    else
			for( int i = 1; i < argc; i++ )
			{
				if( strchr( argv[i], '*' ) != NULL || strchr( argv[i], '?' ) != NULL )
				{
					Dir dir( argv[i], DirEntry::ARCHIVED, 0 );

					for( int d = 0; d < dir.getEntries(); d++ )
					{
						fixFilename( argv[i], dir.getEntryAt(d)->getName(), fullPath );
						res = wnd->sendDataToWindow( ControllerWindow::getTitle(), fullPath );
						if( !res ) break;
					}
				}
				else
				{
    			System::getFullPath( argv[i], fullPath, MAXPATH );
					res = wnd->sendDataToWindow( ControllerWindow::getTitle(), fullPath );
				}
				if( !res ) break;
			}

		if( res ) return 0;	// Exit, everything worked OK
    delete wnd;	// Didn't find a previous instance, continue
	}

  // Set up Kon base directory

  KonSystem::baseDir = argv[0];

  char *slashPos = strrchr( argv[0], '\\' );

  if( slashPos != NULL )
    *(slashPos+1) = 0;

  KonSystem::bmpDir = new char[strlen( argv[0] ) + 10];
  sprintf( KonSystem::bmpDir, "%s\\bmp\\", argv[0] );
  KonSystem::kDir = new char[strlen( argv[0] ) + 10];
  sprintf( KonSystem::kDir, "%s\\k\\", argv[0] );

  char filename[MAXPATH+1];

  sprintf( filename, "%s%s", KonSystem::baseDir, "english.prp" );

  if( Strings::loadStrings( filename ) )
  {
  	PopupMessage *message = 
    	new PopupMessage( Desktop::getInstance(),
                        "Startup error", "Couldn't load english.prp",
                        PopupMessage::error );
		message->getResult();
    return 1;
  }

	Settings::initSettings();

  sprintf( filename, "%s%s", KonSystem::baseDir, "win.prp" );
  WinProperties::loadProperties( filename );

  sprintf( filename, "%s%s", KonSystem::baseDir, "kon.prp" );
  ClassProperties::restoreProperties( filename );

  sprintf( filename, "%s%s", KonSystem::baseDir, "kon.syn" );

  SyntaxContainer::loadSyntaxes( filename );

  KeyBindings::createInstance();

  // Create common commands instance

  KCommonCommands::createInstance();

  // Create the errorCollector instance

  KErrorCollector::createInstance( Strings::get(54) );

  // Execute the startup script

	if( Settings::errorCollectorVisible )
		KErrorCollector::getInstance()->show();

  Kstartup *kStartup = new Kstartup();
  delete kStartup;

  ControllerWindow::createInstance();

  MainWindow *mainWin =
    new MainWindow( Desktop::getInstance(), "Kon", 10, 10, 600, 450 );
  mainWin->setVisible( TRUE );

  // Load files on commandline

	if( argc > 1 )
  {
  	for( int i = 1; i < argc; i++ )
		{
			if( strchr( argv[i], '*' ) != NULL || strchr( argv[i], '?' ) != NULL )
			{
				Dir dir( argv[i], DirEntry::ARCHIVED, 0 );

				for( int d = 0; d < dir.getEntries(); d++ )
				{
					fixFilename( argv[i], dir.getEntryAt(d)->getName(), fullPath );
	    		mainWin->addFile( fullPath, TRUE, TRUE );
				}
			}
			else
    		mainWin->addFile( argv[i], TRUE, TRUE );
		}
  }
  else
	{
		char filename[MAXPATH];
		BufferAdmin::getNextScratchName( filename );
   	mainWin->addFile( filename, TRUE, TRUE );
	}

	if( NamedMessagePipe::supportsPipes() )
	{
    // Start pipe server

    PipeServer *pipeServer = new PipeServer();
    pipeServer->start( 4096 );
	}

	AutosaveThread::createInstance();

  // Check license

  KonSystem::registered = FALSE;

  char regFile[MAXPATH];
  sprintf( regFile, "%s%s", KonSystem::baseDir, "kon.reg" );

  FILE *f = fopen( regFile, "rb");

  if( f != NULL )
  {
    fread( KonSystem::regName, sizeof(KonSystem::regName), 1, f );
    fread( KonSystem::regKey, sizeof(KonSystem::regKey), 1, f );

    for( int i = 0; i < strlen(KonSystem::regName); i++ )
      KonSystem::regName[i] ^= 0xef;

    for( i = 0; i < strlen(KonSystem::regKey); i++ )
      KonSystem::regKey[i] ^= 0xef;

    // Check if license key is correct

    char result[MAXREGCODELEN*2];
    result[0] = 0;
    int ok = scramble( KonSystem::regName, result );

    if( !ok || strcmp( KonSystem::regKey, result ) )
    {
      // Failed registration
  	  PopupMessage *message = 
    	  new PopupMessage( mainWin,
                          Strings::get(221), Strings::get(222),
                          PopupMessage::error );
		  message->getResult();
    }
    else
      KonSystem::registered = TRUE;
    fclose( f );
  }

  if( !KonSystem::registered )
  {
    AboutDialog *aboutDialog =
      new AboutDialog( mainWin );

    aboutDialog->setVisible( TRUE );

    delete aboutDialog;
  }

  Application::run();

  KErrorCollector::killInstance();

  sprintf( filename, "%s%s", KonSystem::baseDir, "win.prp" );
  WinProperties::saveProperties( filename );

  sprintf( filename, "%s%s", KonSystem::baseDir, "kon.prp" );
  ClassProperties::saveProperties( filename );

  if( !KonSystem::registered )
  {
    AboutDialog *aboutDialog =
      new AboutDialog( Desktop::getInstance() );

    aboutDialog->setVisible( TRUE );

    delete aboutDialog;
  }

  return 0;
}
