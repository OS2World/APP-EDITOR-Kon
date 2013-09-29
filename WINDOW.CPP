// Window classes

#include "bowl.h"

int Iterator::iterator = 0;

int AMenuEntry::SEPARATOR = 1;
int AMenuEntry::MENUITEM = 2;
int AMenuEntry::SUBMENU = 3;

int AMouseEvent::NONE = 0;
int AMouseEvent::LEFT = 1;
int AMouseEvent::MIDDLE = 2;
int AMouseEvent::RIGHT = 3;
int AMouseEvent::DOWN = 0;
int AMouseEvent::RELEASE = 1;
int AMouseEvent::CLICK = 2;
int AMouseEvent::MOVE = 3;
int AMouseEvent::DOUBLECLICK = 4;
int AMouseEvent::TRIPLECLICK = 5;
int AMouseEvent::WHEEL = 6;

int AWindow::WINDOW = 1;
int AWindow::COMBOBOX = 2;
int AWindow::ENTRYFIELD = 3;
int AWindow::LISTBOX = 4;
int AWindow::PUSHBUTTON = 5;
int AWindow::RADIOBUTTON = 6;
int AWindow::DIALOG = 7;
int AWindow::GROUPBOX = 8;
int AWindow::CHECKBOX = 9;
int AWindow::LABEL = 10;
int AWindow::SCROLLBAR = 11;
int AWindow::COMBOLISTBOX = 12;
int AWindow::NOTEBOOK = 13;
int AWindow::INTERNALENTRYFIELD = 14;
int AWindow::SCOMBOBOX = 15;

ULONG AWindow::ADJUSTSIZE = 1;
ULONG AWindow::SYNCPAINT = 2;
ULONG AWindow::ISTABSTOP = 4;
ULONG AWindow::NEWGROUP = 8;
//ULONG AWindow::AUTOSCROLL= 16;
ULONG AWindow::INGROUP = 32;
ULONG AWindow::NOADJUSTSIZE = 64;
ULONG AWindow::ISMODAL = 128;
ULONG AWindow::NOTITLE = 256;
ULONG AWindow::CENTER = 512;
ULONG AWindow::BORDER = 1024;
ULONG AWindow::ISDEFAULT = 2048;
ULONG AWindow::CLIPSIBLINGS = 4096;
ULONG AWindow::ISMODALDONTDISABLEPARENT = 8192;
ULONG AWindow::MULTIPLESEL = 16;

// 8192-32768 is taken by the Label window

ULONG AWindow::NUMERIC = 65536;
ULONG AWindow::FILTERDIALOGARROWS = 131072;
ULONG AWindow::FILTERDIALOGTAB = 262144;
ULONG AWindow::PASSDIALOGKEYS = 524288;
ULONG AWindow::POPUP = 1048576;

int AWindow::MENU_CLOSE = -1;

int KeyDef::CTRL = 1;
int KeyDef::kALT = 2;
int KeyDef::SHIFT = 4;

int KeyDef::UP = 257;
int KeyDef::DOWN = 258;
int KeyDef::PAGEUP = 259;
int KeyDef::PAGEDOWN = 260;
int KeyDef::INSERT = 270;
int KeyDef::kDELETE = 280;
int KeyDef::HOME = 281;
int KeyDef::END = 282;
int KeyDef::ESCAPE = 283;
int KeyDef::F1 = 284;
int KeyDef::F2 = 285;
int KeyDef::F3 = 286;
int KeyDef::F4 = 287;
int KeyDef::F5 = 288;
int KeyDef::F6 = 289;
int KeyDef::F7 = 290;
int KeyDef::F8 = 291;
int KeyDef::F9 = 292;
int KeyDef::F10 = 293;
int KeyDef::F11 = 294;
int KeyDef::F12 = 295;
int KeyDef::ENTER = 296;
int KeyDef::BACKSPACE = 297;
int KeyDef::LEFT = 298;
int KeyDef::RIGHT = 299;
int KeyDef::TAB = 300;
int KeyDef::RETURN = 301;
int KeyDef::SPACE = 302;

void AWindow::setDialogFilter( ULONG properties )
{
	filterDialogKeys = 0;

	if( properties & FILTERDIALOGARROWS )
		filterDialogKeys |= 1;

	if( properties & FILTERDIALOGTAB )
		filterDialogKeys |= 2;

	if( properties & PASSDIALOGKEYS )
		filterDialogKeys |= 4;
}

AWindow *AWindow::getControl( int id )
{
  for( int i = 0; i < controls.getSize(); i++ )
    if( ((AWindow *)controls.getObjectAt(i))->id == id )
      return (AWindow *)controls.getObjectAt(i);
  return (AWindow *)NULL;
}

BOOL AWindow::performMouseEvent( Point *pos, int button, int event )
{
	int ret = FALSE;
  for( int i = 0; i < mouseListener.getSize(); i++ )
  {
    AMouseEvent mouseEvent( this, pos, button, event );
    if( !mouseListener.getObjectAt(i)->actionPerformed( &mouseEvent ) )
    	ret = TRUE;
  }
  return ret;
}

void AWindow::performSelectionEvent( int selectedItem )
{
  for( int i = 0; i < selectionListener.getSize(); i++ )
  {
    ASelectionEvent selectionEvent( this, selectedItem );
    selectionListener.getObjectAt(i)->actionPerformed( &selectionEvent );
  }
}

void AWindow::performMenuEvent( int id )
{
  for( int i = 0; i < menuListener.getSize(); i++ )
  {
    AMenuEvent menuEvent( this, id );
    menuListener.getObjectAt(i)->actionPerformed( &menuEvent );
  }
}

void AWindow::performButtonEvent( AWindow *button, int id )
{
  for( int i = 0; i < buttonListener.getSize(); i++ )
  {
    AButtonEvent buttonEvent( button, id );
    buttonListener.getObjectAt(i)->actionPerformed( &buttonEvent );
  }
}

void AWindow::performScrollbarEvent( int id, int action, int pos )
{
  for( int i = 0; i < scrollbarListener.getSize(); i++ )
  {
    AScrollbarEvent scrollbarEvent( this, id, action, pos );
    scrollbarListener.getObjectAt(i)->actionPerformed( &scrollbarEvent );
  }
}

BOOL AWindow::performKeyEvent( AWindow *window, int modifiers, int key )
{
  BOOL processed = FALSE;
  for( int i = 0; i < keyListener.getSize(); i++ )
  {
    AKeyEvent keyEvent( window, modifiers, key );
    if( keyListener.getObjectAt(i)->actionPerformed( &keyEvent ) )
      processed = TRUE;
  }
  return processed;
}

void AWindow::performFocusEvent( int focusItem )
{
  for( int i = 0; i < focusListener.getSize(); i++ )
  {
    AFocusEvent focusEvent( this, focusItem );
    focusListener.getObjectAt(i)->actionPerformed( &focusEvent );
  }
}

void AWindow::performMoveEvent( int oldX, int oldY, int newX, int newY )
{
  for( int i = 0; i < moveListener.getSize(); i++ )
  {
    AWinMoveEvent moveEvent( this, oldX, oldY, newX, newY );
    moveListener.getObjectAt(i)->actionPerformed( &moveEvent );
  }
}

void AWindow::setControlsVisible()
{
  for( int i = 0; i < controls.getSize(); i++ )
  {
    if( controls.getObjectAt( i )->getType() != COMBOLISTBOX )
      controls.getObjectAt( i )->setVisible( TRUE );
  }
}

BOOL AWindow::setSizeInit( BOOL setInit ) 
{ 
  BOOL prev = initSize; 
  initSize = setInit; 

  for( int i = 0; i < controls.getSize(); i++ )
    controls.getObjectAt(i)->setSizeInit( setInit );

  return prev; 
}
 
