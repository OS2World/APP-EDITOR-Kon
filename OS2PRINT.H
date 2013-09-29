#if !defined PRINT

class PrinterGraphics : public Graphics
{
	static int queuesCount;
	static PBYTE  pBuf;
	static PPRJINFO2 prj2;
	static PDRIVDATA defaultJobProp;
	static PSZ defaultPrinter;
  static PSZ pszqueue;
	HDC hdc;
	static void getQueues();
	static PPRQINFO3 getINFO3( PSZ pszName );
	static void freeQueueMem();
	static void saveJobProp( PPRQINFO3 prq );
	BOOL printerSetup(DEVOPENSTRUC FAR *lpdos, PSZ Port );
	HPS getPrinterHPS(DEVOPENSTRUC FAR *lpdos, HDC FAR* lphDC, PSZ pszDocName,
										LONG lWidth, LONG lHeight );
  char documentName[MAXPATH+1];
public:
	PrinterGraphics( char *documentName, char *printer );
//  void startDocument();
  void setup();
  void formfeed();
  static void getPrinters( char ***list, int &count );
  static void setupPrinter( char *name );
  int getWidth();
  int getHeight();
  int getXRes();
  int getYRes();
  void terminateDocument();
  void endDocument();
  static void saveSettings( FILE *f );
  static void restoreSettings( FILE *f );
  virtual void drawStringClip( int x, int y, Rect *rect, int options, int length,
                   						 STRING str );
  ~PrinterGraphics();
};

#define PRINT
#endif