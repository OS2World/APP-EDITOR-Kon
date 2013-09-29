#include "bowl.h"
#include "kerrcoll.h"
#include "cntrladm.h"
#include "editctrl.h"
#include "multrep.h"
#include "mainwnd.h"
#include "mainwadm.h"
#include "repldlg.h"

BOOL MultipleFilesReplace::operateFile( char *filename, View *view, BOOL isVisible )
{
  BOOL result = FALSE;
  BOOL fileLoaded = FALSE;
  ControlInfo *control;
  BOOL continueSearch = TRUE;
  int queryResult = 1, replacements = 0;
  EditControl *editControl;
  MainWindow *parent;

  do
  {
    control = ControlAdmin::queryControlWithFilename( filename );

    if( control != NULL )
    {
      parent = control->parent;
      editControl = control->editControl;
      parent->setFocus();  // Set focus to the MainWindow, will destroy control
      parent->selectControl( editControl );
      view = editControl->getView();
      if( result < 0 )
        view->jumpToPosition( -queryResult - 1 );
    }

    result = view->replaceString( searchString, replaceString, options, 
                                  replacements, TRUE, queryResult );
    if( queryResult < 0 )
    {
      // Load the file into a visible window

      if( Application::executeFromMainThread( ControllerWindow::getInstance(),  
                                              5, filename ) == -1 )
      {
        KErrorCollector::getInstance()->message( Strings::get(114) );
        return FALSE;
      }
      else
        fileLoaded = TRUE;
    }
    else
    {
			if( queryResult == QueryReplaceDialog::CANCEL )
      	continueSearch = FALSE;
      else
			if( queryResult == QueryReplaceDialog::REPLACEINALLFILES )
      {
  			if( strchr( options, 'n' ) == NULL )
    			strcat( options, "n" ); // Don't query
      }
    }
  } while( queryResult < 0 );

  if( replacements > 0 )
  {
  	// Save the file

    view->saveFile();

    char *msg = new char[strlen( filename ) + strlen(Strings::get(113)) + 30];
  
    sprintf( msg, Strings::get(113), replacements, filename );
  
    KErrorCollector::getInstance()->message( msg );
    delete msg;
  }

  if( fileLoaded ) // Close the opened file
    Application::executeFromMainThread( ControllerWindow::getInstance(),  
                                        6, (void *)editControl );

  return continueSearch;
}

MultipleFilesReplace::MultipleFilesReplace( char *searchString, 
                                        char *replaceString, char *options ) :
  searchString(searchString), replaceString( replaceString )
{
  this->options = new char[strlen(options)+4];
  strcpy( this->options, options );

  strcat( this->options, "d" ); // Whole document
	strcat( this->options, "a" ); // Replace all
}