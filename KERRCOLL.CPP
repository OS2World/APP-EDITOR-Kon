#include "kerrcoll.h"
#include "cntrladm.h"
#include "mainwnd.h"
#include "mainwadm.h"
#include "buffer.h"
#include "editctrl.h"

KErrorCollector *KErrorCollector::instance = NULL;

KErrorCollector::KErrorCollector( char *errTitle ) :
      Window( Desktop::getInstance(), errTitle, 20, 50, 600, 300 ),
      TErrorCollector(), visible(FALSE)
{
	TextBuffer *textBuffer = new TextBuffer( getBuffername() );

	messagesControl = new EditControl( this, textBuffer, NULL, NULL, 
	                           0, 0, 
                             getClientWidth(), getClientHeight(), TRUE );
	messagesControl->setVisible( FALSE );
	textBuffer->finishedLoading( FALSE, FALSE );

  int xx = getX(), yy = getY(), wwidth = getWidth(), hheight = getHeight();
  WinProperties::retrieveProperty( "ErrorCollector", xx, yy, wwidth, hheight );

  setPos( xx, yy );
  setSize( wwidth, hheight );
}

void KErrorCollector::show()
{
  MainWindowAdmin::getActiveWindow()->restoreMessages();
}

void KErrorCollector::hide()
{
  setVisible( FALSE );
  Settings::errorCollectorVisible = visible = FALSE;
}

BOOL KErrorCollector::isVisible() 
{ 
	return visible; 
}

BOOL KErrorCollector::close() 
{ 
  hide();
  return FALSE; 
}

int KErrorCollector::focus( BOOL newFocus )
{
  return 1;
}

void KErrorCollector::addString( char *str )
{
	char *msg, *fixedStr;

	fixedStr = KCommonCommands::getInstance()->fixString( str );

	msg = new char[strlen(fixedStr)*2+100];

	sprintf( msg, "toDocumentEnd();insertString( '%s\r\n' );toDocumentEnd();", fixedStr );

	messagesControl->runCommand( msg );
	
	delete msg;

	if( str != fixedStr ) delete fixedStr;
}

void KErrorCollector::Error( int column, int line, char *str )
{
  char *sstr;
  errors++;
  sstr = new char[50+strlen( str )];
  sprintf( sstr, "(Line: %d, Col: %d) : %s", line, column, str );
	addString( sstr );
  delete sstr;
  show();
}

void KErrorCollector::Error( char *file, int column, int line, char *str )
{
  char *sstr;
  errors++;
  sstr = new char[50+strlen(file)+strlen( str )];
  sprintf( sstr, "%s (%d, %d) : %s", file, line, column, str );
	addString( sstr );
  delete sstr;
  show();
}

void KErrorCollector::message( char *str )
{
	addString( str );
}


void KErrorCollector::clear()
{
	messagesControl->runCommand( "resetFPRegExp();clearAll();" );
}

KErrorCollector::~KErrorCollector()
{
	if( !isMinimized() && !isMaximized() )
	  WinProperties::updateProperty( "ErrorCollector", getX(), getY(),
  	                               getWidth(), getHeight() );
}

