/******************************************************************************
 * Displays search dialogs
 * Call saveSettings to store settings to disk
 */

#if !defined SEARCHDIALOG
#define SEARCHDIALOG

#define MAXSTRINGS  10
#define MAXSTRINGSIZE  200

#include "buffer.h"

class ReplaceDialog;

class SearchDialog : public Dialog, public virtual AButtonListener
{
  friend ReplaceDialog;
  char *name;
  Button *searchButton, *multSearchButton, *cancel1Button, *cancel2Button,
  			 *currentDirButton;
  SComboBox *searchCombo;
  CheckBox *caseCheck, *wordCheck, *grepCheck, *buffersCheck, *subdirCheck,
           *hexSearchCheck, *clearCheck, *wrapAroundCheck;
  RadioButton *fromCursorButton, *entireDocButton, *fwdButton, *bkwdButton,
              *globalButton, *selectedButton;
	ListBox *openBuffersList;
  SComboBox *filesCombo, *pathsCombo;
  Label *lblF, *lblP, *lblB;
  Dialog *singleDlg, *multiDlg;
  static BOOL initialized, entireDoc, forward, selected,
              caseSensitive, wholeWords, grep, buffersOnly, subdirs,
              hexSearch, wrapAround;
  static int strings, buffersSelected, filesCount, pathsCount;
  static TextElem *string[MAXSTRINGS+1];
  static TextElem *files[MAXSTRINGS+1];
  static TextElem *paths[MAXSTRINGS+1];
  static char **buffers;
  void doSearch( int result );
public:
  static int CURRENTBUFFER, MULTIPLEFILES, CANCEL;
  SearchDialog( Window *frame, Window *parent, TextElem *findString = NULL );
  void enableControls();
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  static int getStringLength()
  {
    return strlen( string[0] );
  }
  static void getSettings( char *options, TextElem *searchString );
  static BOOL searchBuffersOnly() { return buffersOnly; }
  static char **getSelectedBuffers( void ) { return buffers; }
  static int getSelectedBuffersCount() { return buffersSelected; }
  static char *getPath() { return (pathsCount > 0) ? paths[0] : ""; }
  static char *getFiles() { return (filesCount > 0) ? files[0] : ""; }
  static void saveSettings( FILE *f );
  static void restoreSettings( FILE *f );
  ~SearchDialog();
};

#endif