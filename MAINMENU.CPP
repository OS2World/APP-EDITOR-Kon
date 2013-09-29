#include "bowl.h"
#include "mainmenu.h"
#include "editctrl.h"
#include "cntrladm.h"
#include "bufadmin.h"
#include "winprop.h"

AMenu *MainMenu::mainMenu = NULL;
AMenu *MainMenu::windowMenu = NULL;
BOOL MainMenu::menuBuilt = FALSE;
Vector<IdCommand> MainMenu::idCommand(TRUE);
Vector<RecentFile> MainMenu::recentFile(TRUE);
AMenu *PopupMenu::popupMenu = NULL;
BOOL PopupMenu::menuBuilt = FALSE;
Vector<IdCommand> PopupMenu::idCommand(TRUE);
int MainMenu::windowMenuId = -1;
BOOL MainMenu::buildingWindowMenu = FALSE;

void MainMenu::addItem( AMenu *menu, char *string, int id, char *command )
{
	menu->addMenuItem( new AMenuItem( string, id, AMenuEntry::MENUITEM ) );

  for( int i = 0; i < idCommand.getSize(); i++ )
    if( idCommand.getObjectAt( i )->id == id )
    {
      idCommand.removeAt( i );
      break;
    }
  idCommand.add( new IdCommand( id, command ) );
}

void MainMenu::startSubmenuBuild( int id, BOOL removeItems )
{
	AMenuEntry *tempMenu;
	tempMenu = findId( (AMenuEntry *)mainMenu, id );
	if( id == 1000 ) windowMenu = (AMenu *)tempMenu;
  if( removeItems ) ((AMenu *)tempMenu)->removeAllItems();
}

void MainMenu::buildWindowMenu()
{
  // Run buildWindowMenu()

	KCommonCommands::getInstance()->runCommand( "buildWindowMenu();" );

  assignKbdShortcuts( windowMenu );
}

void MainMenu::checkItem( int id, BOOL check )
{
  AMenuEntry *menu = findId( (AMenuEntry *)mainMenu, id );

  if( menu != NULL )
  {
    TwoInts idCheck( id, check );
    menu->check( check );

    // Update the main menu of the active window

		Window *activeWindow = (Window *)MainWindowAdmin::getActiveWindow();

		if( activeWindow != NULL )
			Application::executeFromMainThread( activeWindow, 2, &idCheck );
  }
}

void MainMenu::enableItem( int id, BOOL enable )
{
  AMenuEntry *menu = findId( (AMenuEntry *)mainMenu, id );
  if( menu != NULL )
  {
    TwoInts idEnable( id, enable );
    menu->enable( enable );

    // Update the main menu of the active window
		Window *activeWindow = (Window *)MainWindowAdmin::getActiveWindow();

		if( activeWindow != NULL )
	    Application::executeFromMainThread( activeWindow, 3, &idEnable );
  }
}

AMenuEntry *MainMenu::findId( AMenuEntry *menu, int id )
{
  if( menu == NULL ) return NULL;

  if( menu->id == id ) return menu;

  if( menu->type == AMenuEntry::SUBMENU )
  {
    AMenu *mmenu = (AMenu *)menu;

    for( int i = 0; i < mmenu->items(); i++ )
      if( mmenu->getItemAt( i )->type == AMenuEntry::SUBMENU )
      {
        AMenuEntry *fMenu = findId( (AMenuEntry *)mmenu->getItemAt( i ), id );
        if( fMenu != NULL ) return fMenu;
      }
      else
      if( mmenu->getItemAt( i )->id == id )
        return mmenu->getItemAt( i );
  }
  return NULL;
}

BOOL MainMenu::addMenuFromId( int atId, char *string, int menuId )
{
  AMenuEntry *menu = findId( (AMenuEntry *)mainMenu, atId );

  if( menu == NULL ) return FALSE;

  if( menu->type != AMenuEntry::SUBMENU ) return FALSE;

  AMenu *mmenu = (AMenu *)menu;

  mmenu->addMenuItem( new AMenu( string, menuId ) );
  return TRUE;
}

BOOL MainMenu::addMenuCommandFromId( int atId, char *string, char *command, int menuId )
{
  AMenuEntry *menu = findId( (AMenuEntry *)mainMenu, atId );

  if( menu == NULL ) return FALSE;

  if( menu->type != AMenuEntry::SUBMENU ) return FALSE;

  AMenu *mmenu = (AMenu *)menu;

  addItem( mmenu, string, menuId, command );
  return TRUE;
}

BOOL MainMenu::addSeparatorFromId( int atId )
{
  AMenuEntry *menu = findId( (AMenuEntry *)mainMenu, atId );

  if( menu == NULL ) return FALSE;

  if( menu->type != AMenuEntry::SUBMENU ) return FALSE;

  AMenu *mmenu = (AMenu *)menu;

  mmenu->addMenuItem( new AMenuItem( "", 99, AMenuEntry::SEPARATOR) );
  return TRUE;
}

char *MainMenu::getCommandFromId( int id )
{
  for( int i = 0; i < idCommand.getSize(); i++ )
    if( idCommand.getObjectAt( i )->id == id )
      return idCommand.getObjectAt( i )->command;
  return NULL;
}

void MainMenu::assignKbdShortcuts( AMenu *menu )
{
  // Recursive search through the menu

  char keyString[100];

  for( int i = 0; i < menu->items(); i++ )
  {
    if( menu->getItemAt(i)->type == AMenuEntry::SUBMENU )
      assignKbdShortcuts( (AMenu *)menu->getItemAt(i) );
    else
    if( menu->getItemAt(i)->type == AMenuEntry::MENUITEM )
    {
      if( !KeyBindings::getInstance()->queryKeyString(
              getCommandFromId( menu->getItemAt(i)->id ), keyString ) )
        *keyString = 0;
      menu->getItemAt(i)->updateShortcut( keyString );
    }
  }
}

void MainMenu::addNewFileToRecent( char *file )
{
	if( BufferAdmin::isScratchName( file ) ) return;

	RecentFile *newFile = new RecentFile( file ), *found;

	found = recentFile.findObjectOnEq( newFile );

  if( found != NULL )
  	recentFile.removeObject( found );

	recentFile.insertAt( 0, newFile );
}

void MainMenu::saveSettings( FILE *f )
{
	long pos;
	if( !ClassProperties::savePropertyName( f, "MAINMENU001", &pos ) ) return;

	int s = recentFile.getSize();

	fwrite( &s, sizeof( int ), 1, f );

  for( int i = 0; i < recentFile.getSize(); i++ )
  {
  	char *filename = recentFile.getObjectAt(i)->name;

		s = strlen( filename ) + 1;

		fwrite( &s, sizeof( int ), 1, f );
    fwrite( filename, s, 1, f );
  }
	ClassProperties::savePropertySize( f, pos );
}

void MainMenu::restoreSettings( FILE *f )
{
	if( !ClassProperties::checkPropertyExists( f, "MAINMENU001" ) ) return;

	int size;
	fread( &size, sizeof( int ), 1, f );

  for( int i = 0; i < size; i++ )
  {
    int s;
		fread( &s, sizeof( int ), 1, f );
    char *filename = new char[s];

    fread( filename, s, 1, f );

    recentFile.add( new RecentFile( filename ) );
    delete filename;
  }
}

AMenuEntry *PopupMenu::findId( AMenuEntry *menu, int id )
{
  if( menu == NULL ) return NULL;

  if( menu->id == id ) return menu;

  if( menu->type == AMenuEntry::SUBMENU )
  {
    AMenu *mmenu = (AMenu *)menu;

    for( int i = 0; i < mmenu->items(); i++ )
      if( mmenu->getItemAt( i )->type == AMenuEntry::SUBMENU )
      {
        AMenuEntry *fMenu = findId( (AMenuEntry *)mmenu->getItemAt( i ), id );
        if( fMenu != NULL ) return fMenu;
      }
      else
      if( mmenu->getItemAt( i )->id == id )
        return mmenu->getItemAt( i );
  }
  return NULL;
}

void PopupMenu::addItem( AMenu *menu, char *string, int id, char *command )
{
  menu->addMenuItem( new AMenuItem( string, id, AMenuEntry::MENUITEM ) );
  for( int i = 0; i < idCommand.getSize(); i++ )
    if( idCommand.getObjectAt( i )->id == id )
    {
      idCommand.removeAt( i );
      break;
    }
  idCommand.add( new IdCommand( id, command ) );
}

BOOL PopupMenu::addMenuFromId( int atId, char *string, int menuId )
{
  AMenuEntry *menu = findId( (AMenuEntry *)popupMenu, atId );

  if( menu == NULL ) return FALSE;

  if( menu->type != AMenuEntry::SUBMENU ) return FALSE;

  AMenu *mmenu = (AMenu *)menu;

  mmenu->addMenuItem( new AMenu( string, menuId ) );
  return TRUE;
}

BOOL PopupMenu::addMenuCommandFromId( int atId, char *string, char *command, int menuId )
{
  AMenuEntry *menu = findId( (AMenuEntry *)popupMenu, atId );

  if( menu == NULL ) return FALSE;

  if( menu->type != AMenuEntry::SUBMENU ) return FALSE;

  AMenu *mmenu = (AMenu *)menu;

  addItem( mmenu, string, menuId, command );
  return TRUE;
}

BOOL PopupMenu::addSeparatorFromId( int atId )
{
  AMenuEntry *menu = findId( (AMenuEntry *)popupMenu, atId );

  if( menu == NULL ) return FALSE;

  if( menu->type != AMenuEntry::SUBMENU ) return FALSE;

  AMenu *mmenu = (AMenu *)menu;

  mmenu->addMenuItem( new AMenuItem( "", 99, AMenuEntry::SEPARATOR) );
  return TRUE;
}

char *PopupMenu::getCommandFromId( int id )
{
  for( int i = 0; i < idCommand.getSize(); i++ )
    if( idCommand.getObjectAt( i )->id == id )
      return idCommand.getObjectAt( i )->command;
  return NULL;
}
