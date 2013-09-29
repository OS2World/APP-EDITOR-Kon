/***************************************************************************************
 * Print a file
 ***************************************************************************************/

#include "bowl.h"
#include "view.h"

#if !defined PRINTOBJECT
#define PRINTOBJECT

class AbortPrintDialog;

#define MAXFONTNAMELEN	300

class Print : public AObject
{
	int options, fromLine, toLine, leftMargin, rightMargin, topMargin, bottomMargin;
  char *printer, *header, *footer, *name, *docName;
  View *view;
  PrinterGraphics *prn;
  void printPages();
	void addEscapeCodes( char *sourceString, char *targetString, int pageNumber  );
  int setupPage( int pagenumber ); // return height of header/footer
	AbortPrintDialog *abortPrintDlg;
public:
	static char printerFont[MAXFONTNAMELEN], headerFont[MAXFONTNAMELEN];
	static int printerFontSize, headerFontSize;
	static BOOL initialized;
	static int ODDPAGES, EVENPAGES, TWOUP, SELECTED, LINES, PRINTCOLOR, 
  					 PRINTSYNTAXHL;
	Print( View *view, char *printer, int options, char *header, char *footer, 
  			 int fromLine, int toLine, int leftMargin, int rightMargin, 
         int topMargin, int bottomMargin, char *docName );
	void startPrint();
  void abortPrint();
	void setAbortPrintDialog( AbortPrintDialog *abortPrintDlg )
	{
		this->abortPrintDlg = abortPrintDlg;
	}
	static void saveSettings( FILE * f );
	static void restoreSettings( FILE * f );
	~Print();
};

#endif