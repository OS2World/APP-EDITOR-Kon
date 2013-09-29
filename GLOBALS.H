#if !defined GLOBALS

#include "lic.h"
#include <time.h>

#define GLOBALS

struct Settings
{
  static int tabSize, maxUndoSize, rightMargin, autosaveDelay, leftMarginChars;
  static ColorValue markFg, markBg, hiliteBg, hiliteFg, hexCursorFg, hexCursorBg, 
				 textFg, textBg, markerFg, markerBg, leftMarginBg, leftMarginFg;
  static char *standardDelim, *autosavePath;
  static BOOL columnBlock, CRLF, stripNameInNotebook, insertMode, backupFiles,
              persistentBlocks, showRightMargin, syntaxHighlight;
	static char *asciiFont, *hexFont;
	static int asciiFontSizeP, hexFontSizeP, asciiFontSizeX, hexFontSizeX,
	           asciiFontSizeY, hexFontSizeY;
	static BOOL asciiFixedWidthOnly, asciiBitmappedOnly, hexBitmappedOnly,
	            errorCollectorVisible, autoSave, useTab, optimalFill, 
							useSystemFileDialog, showLeftMarginInfo, showLineNumbers, 
							showSavedPos, linesInHex;
	static void saveSettings( FILE * f );
	static void restoreSettings( FILE * f );
	static void initSettings();
	static BOOL delimAllocated, autoCloseFrameWindow, showCommandBar, virtualSpace,
              showBytesInfo, reuseExistingBuffer, queryIfUnicode, showSectionBrowser,
              autogenerateSections, autoClearMessages, showToolbar, hideAscii32;
  static int  pulldownSize;
};

struct KonSystem
{
  static char *baseDir, *bmpDir, *kDir;
  static BOOL registered;
  static char regName[100], regKey[MAXREGCODELEN*2];
  static long buildNumber, major, minor;
  static time_t startupTime;
  static char *kClipboard;
};

#endif