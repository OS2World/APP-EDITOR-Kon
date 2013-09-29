#if !defined MAINMENUINC 

#define MAINMENUINC

/*****************************************************************************
 * The MainMenu singleton class holds and manipulates the main menu.
 * All MainWindows query their menu from this class.
 */

#include "mainwadm.h"

// IdCommand holds all menu Id's and their corresponding K command

class IdCommand
{
public:
  int id;
  char *command;
  IdCommand( int id, char *command )
  {
    this->id = id;
    this->command = new char[strlen(command)+1];
    strcpy( this->command, command );
  }
  ~IdCommand() { delete command; }
  virtual int equals( AObject *object )
  {
    return ((IdCommand *)object)->id = id;
  }
};

class RecentFile
{
public:
	char *name;
  RecentFile( char *name )
  {
  	this->name = new char[strlen(name)+1];
    strcpy( this->name, name );
  }
  ~RecentFile()
  {
  	delete name;
  }
  virtual int equals( AObject *object )
  {
  	return !strcmp( ((RecentFile *)object)->name, name );
  }
};

class MainMenu
{
  static AMenu *mainMenu;
  static AMenu *windowMenu;
  static BOOL menuBuilt;
  static Vector<IdCommand> idCommand;
  static Vector<RecentFile> recentFile;

  // Modify menu to display keyboard shortcuts for commands

  static void assignKbdShortcuts( AMenu *menu );

  // Create a list of all opened edit controls for quick navigation

  static void buildWindowMenu();
  static AMenuEntry *findId( AMenu *menu, int id );
  static void buildRecentFiles();
	static int windowMenuId;
	static BOOL buildingWindowMenu;
public:
  static void resetMenu()
  {
    menuBuilt = FALSE;
    if( mainMenu != NULL )
      delete mainMenu;
    mainMenu = new AMenu( "MAINMENU", 101 );
		windowMenu = NULL;
		buildingWindowMenu = FALSE;
  }
	static void windowMenuBuild( BOOL building )
	{
		buildingWindowMenu = building;
	}
  static void addItem( AMenu *menu, char *string, int id, char *command );
  static AMenu *getMenu()
  {
    if( menuBuilt ) return mainMenu;

    mainMenu = new AMenu( "MAINMENU", 101 );
    AMenu *fileMenu = new AMenu( "~File", 103 );
    windowMenu = new AMenu( "~Window", 104 );

    addItem( fileMenu,"~Open...", 105, "openFile();" );
    addItem( fileMenu,"~Save", 106, "saveFile();" );
    fileMenu->addMenuItem( new AMenuItem( "", 106, AMenuEntry::SEPARATOR) );
    addItem( fileMenu,"~Quit...", 107, "quit();" );

    mainMenu->addMenuItem( fileMenu );
    mainMenu->addMenuItem( windowMenu );

    assignKbdShortcuts( mainMenu );
    buildWindowMenu();

    menuBuilt = TRUE;

    return mainMenu;
  }

  static void endBuild()
  {
//    windowMenu = new AMenu( "~Window", 104 );
//    mainMenu->addMenuItem( windowMenu );
    menuBuilt = TRUE;
  }

	static void startSubmenuBuild( int id, BOOL removeItems );
  static void checkItem( int id, BOOL check );
  static void enableItem( int id, BOOL enable );

  static void updateWindowMenu()
  {
    buildWindowMenu();
    updateMenu();
  }

  static void updateMenu()
  {
    assignKbdShortcuts( mainMenu );
    MainWindowAdmin::updateAllWindows( MainWindowAdmin::MENU );
  }

  static AMenuEntry *findId( AMenuEntry *menu, int id );
  static BOOL addMenuFromId( int atId, char *string, int menuId );
  static BOOL addMenuCommandFromId( int atId, char *string, char *command, 
                                    int menuId );
  static BOOL addSeparatorFromId( int atId );

  static char *getCommandFromId( int id );

	static int getRecentFilesCount() { return recentFile.getSize(); }
  static char *getRecentFile( int pos ) 
  { 
  	return recentFile.getObjectAt(pos)->name;
  }
	static void addNewFileToRecent( char *file );
  static void saveSettings( FILE *f );
  static void restoreSettings( FILE *f );
};

class PopupMenu
{
  static AMenu *popupMenu;
  static BOOL menuBuilt;
  static Vector<IdCommand> idCommand;

public:
  static void resetMenu()
  {
    menuBuilt = FALSE;
    if( popupMenu != NULL )
      delete popupMenu;
    popupMenu = new AMenu( "POPUPMENU", 101 );
  }
  static void addItem( AMenu *menu, char *string, int id, char *command );
  static AMenu *getMenu()
  {
    if( menuBuilt ) return popupMenu;
    return NULL;
  }

  static void endBuild()
  {
    menuBuilt = TRUE;
  }

  static AMenuEntry *findId( AMenuEntry *menu, int id );
  static BOOL addMenuFromId( int atId, char *string, int menuId );
  static BOOL addMenuCommandFromId( int atId, char *string, char *command, 
                                    int menuId );
  static BOOL addSeparatorFromId( int atId );

  static char *getCommandFromId( int id );
};

#endif
