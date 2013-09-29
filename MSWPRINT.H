#if !defined PRINT

#define MAXPRINTERS	40

class PrinterGraphics : public Graphics
{
	static PRINTER_INFO_2 *pInfo;
  static DWORD printers;
  static DEVMODE *devmode;
  static int devmodeSize;
  static char currentPrinter[300];
public:
	PrinterGraphics( char *documentName, char *printer );
  static void getPrinters(char ***list, int &count );
  static void setupPrinter( char *name );
  void formfeed();
  int getWidth();
  int getHeight();
  int getXRes();
  int getYRes();
  void terminateDocument();
  void endDocument();
  static void saveSettings( FILE *f );
  static void restoreSettings( FILE *f );
  ~PrinterGraphics();
  
};

#define PRINT
#endif