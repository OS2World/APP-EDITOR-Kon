/******************************************************************************
 * Displays replace dialog
 * Call saveSettings to store settings to disk
 * Note! Uses SearchDialog static's for common options!
 */

#if !defined REPLACEDIALOG
#define REPLACEDIALOG

#define MAXSTRINGS  10
#define MAXSTRINGSIZE  200

#include "buffer.h"
#include "srchdlg.h"
//#include "viewwin.h"

class ReplaceDialog : public Dialog, public virtual AButtonListener
{
  char *name;
  Button *searchButton, *multSearchButton, *cancel1Button, *cancel2Button,
         *currentDirButton;
  SComboBox *searchCombo;
  CheckBox *caseCheck, *wordCheck, *grepCheck, *promptCheck, *replaceAllCheck, 
           *buffersCheck, *subdirCheck, *hexSearchCheck, *wrapAroundCheck;
  RadioButton *fromCursorButton, *entireDocButton, *fwdButton, *bkwdButton,
              *globalButton, *selectedButton;

	ListBox *openBuffersList;
  SComboBox *filesCombo, *pathsCombo;
  Label *lblF, *lblP, *lblB;
  SComboBox *replaceCombo;
  Dialog *singleDlg, *multiDlg;
  static int strings;
  static TextElem *string[MAXSTRINGS+1];
  static BOOL initialized, prompt, replaceAll;
  void doReplace( int result );
public:
  ReplaceDialog( Window *frame, Window *parent, TextElem *findString = NULL );
  void enableControls();
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  static int getReplaceStringLength()
  {
    return strlen( string[0] );
  }
  static int getFindStringLength()
  {
    return SearchDialog::getStringLength();
  }
  static void getSettings( char *options, TextElem *searchString, 
                           TextElem *replaceString );
  static void saveSettings( FILE *f );
  static void restoreSettings( FILE *f );
  ~ReplaceDialog();
};

/* Positions the dialog box above or below the found string */

class ViewWindow;

class QueryReplaceDialog : public Dialog, public virtual AButtonListener
{
	char *name;
	Button *replaceButton, *dontReplaceButton, *replaceAllButton, 
  			 *replaceInAllFilesButton, *skipRestButton, *cancelButton;
public:
	static int REPLACE, DONTREPLACE, REPLACEALL, REPLACEINALLFILES, SKIPREST, 
  					 CANCEL;
	QueryReplaceDialog( Window *frame, ViewWindow *viewWin, Window *parentFrame,
  											 char *foundString, char *replaceString, int position,
                         BOOL multiFile );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	~QueryReplaceDialog()
  {
    WinProperties::updateProperty( name, getX(), getY(),
                                   getWidth(), getHeight() );
		delete name;
  }
};

#endif