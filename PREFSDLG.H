/******************************************************************************
 * Displays preferences dialog
 */

#if !defined PREFSDIALOG
#define PREFSDIALOG

class FontPreviewWindow : public Window
{
	Font *font;
	int X, Y, points;
	ColorValue fg, bg;
public:
	FontPreviewWindow( Window *parent, int x, int y, int width, int height );
	void selectFont( Font *font, int X, int Y, int points );
	void setColor( ColorValue fg, ColorValue bg );
	virtual void paint( Rect *rect, Graphics *g );
};

class AsciiDialog : public Dialog, public virtual AButtonListener, 
									  public virtual AFocusListener, public virtual ASelectionListener
{
	SComboBox *fontCombo, *sizeCombo, *textItemCombo;
	CheckBox *fixedWidthOnlyCheck, *bitmapFontsCheck;
	RadioButton *fgButton, *bgButton;
	FontPreviewWindow *fontPreviewWindow;

	ColorValue textBg, textFg, markFg, markBg, hiliteFg, hiliteBg, 
	           markerFg, markerBg, leftMarginFg, leftMarginBg;
  int currentColorSet, currentCp;
	ValueSet *colorSet;
	static ColorValue colors[20];
  void setColor( ColorValue c );
  void setColorFromSelected();
public:
	AsciiDialog( Window *frame, Window *parent );
	virtual void actionPerformed( ASelectionEvent *focusEvent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	void saveSettings();
};

class Ascii2Dialog : public Dialog, public virtual AButtonListener
{
	CheckBox *lineNumbersCheck, *savedPositionsCheck;
	EntryField *leftMarginCharsEntry;
public:
	Ascii2Dialog( Window *frame, Window *parent );
	void saveSettings();
};

class HexDialog : public Dialog, public virtual AButtonListener, 
									public virtual AFocusListener, public virtual ASelectionListener
{
	SComboBox *fontCombo, *sizeCombo;
	CheckBox *bitmapFontsCheck, *numericInfoCheck, *linesHexCheck, *hideAscii32Check;
	FontPreviewWindow *fontPreviewWindow;

public:
	HexDialog( Window *frame, Window *parent );
	virtual void actionPerformed( ASelectionEvent *focusEvent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
	void saveSettings();
};

class EditorDialog : public Dialog, public virtual AButtonListener
{
	EntryField *maxUndoSizeEntry, *tabSizeEntry, *delimEntry;
	CheckBox *persistentBlocksCheck, *useTabCheck, *optimalFillCheck, *virtualSpaceCheck;
	RadioButton *CRLFButton, *LFButton;

public:
	EditorDialog( Window *frame, Window *parent );
	void saveSettings();
};

class DisplayDialog : public Dialog, public virtual AButtonListener
{
	EntryField *rightMarginEntry;
	CheckBox *showRightMarginCheck, *syntaxHighlightCheck, *showSectionsCheck, *autogenSectionsCheck;
public:
	DisplayDialog( Window *frame, Window *parent );
	void saveSettings();
};

class MiscDialog : public Dialog, public virtual AButtonListener
{
	EntryField *autosaveDelayEntry;
	CheckBox *autoSaveCheck, *backupFilesCheck, *useSystemFileDialogCheck, 
           *useBigFontCheck, *autoCloseFrameWindowCheck, *showCommandBarCheck,
           *reuseExistingBufferCheck, *queryUnicodeCheck, *showToolbarCheck;
public:
	MiscDialog( Window *frame, Window *parent );
	void saveSettings();
};

class PrefsDialog : public Dialog, public virtual AButtonListener
{
  char *name;
  Button *OKButton, *cancelButton;
	AsciiDialog *asciiDialog;
	Ascii2Dialog *ascii2Dialog;
	HexDialog *hexDialog;
	EditorDialog *editorDialog;
	DisplayDialog *displayDialog;
	MiscDialog *miscDialog;
  BOOL prevUseBigFont;
public:
  PrefsDialog( Window *frame, Window *parent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  ~PrefsDialog();
};


#endif