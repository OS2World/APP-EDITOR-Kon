/******************************************************************************
 * Print dialog
 */

#include "print.h"

#if !defined PRINTDIALOG
#define PRINTDIALOG


#define MAXPRINTERNAMELEN	512
#define MAXHEADLEN	100

#define DALL	0
#define DSELECTED 1
#define DFROMTO 2

class PrintDialog : public Dialog, public virtual AButtonListener, 
									  public virtual ASelectionListener
{
  char *name;
  Button *OK1Button, *OK2Button, *OK3Button, *cancel1Button, *cancel2Button, 
  		   *cancel3Button, *setupButton;
  SComboBox *textFontCombo, *textFontSizeCombo, 
  				 *headerFontCombo, *headerFontSizeCombo, *printersCombo;
  CheckBox *printColorCheck, *printSyntaxHLCheck, *twoUpCheck, 
  				 *evenPagesCheck, *oddPagesCheck, *fixedWidthOnlyCheck;
  RadioButton *entireDocButton, *linesButton, *selectedButton;
  EntryField *leftMarginEntry, *rightMarginEntry, *bottomMarginEntry,
             *topMarginEntry, *fromLineEntry, *toLineEntry, *headerEntry,
             *footerEntry;
	Dialog *sourceDlg, *pageDlg, *headerDlg;
  int closing;  // Can't explain why I need this
public:
  static BOOL printColor, printSyntaxHL, twoUp, 
  	          evenPages, oddPages, fixedWidthOnly, initialized;
	static char header[MAXHEADLEN], footer[MAXHEADLEN], printer[MAXPRINTERNAMELEN];
	static int selection, from, to, leftMargin, rightMargin, bottomMargin,
             topMargin;

  PrintDialog( Window *frame, Window *parent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  virtual void actionPerformed( ASelectionEvent *selectionEvent );
	static void saveSettings( FILE * f );
	static void restoreSettings( FILE * f );
  ~PrintDialog();
};

class AbortPrintDialog : public Dialog, public virtual AButtonListener
{
  char *name;
	Print *print;
	Button *abortButton;
	Label *pageLabel;
public:
	AbortPrintDialog( Window *frame, Window *parent, Print *print );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	void setPage( int page );
};

#endif