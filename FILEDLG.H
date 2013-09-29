/******************************************************************************
 * File dialog
 */

#define MAXFILES  10
#define MAXFILESIZE 255

class FileDialog : public Dialog, public virtual AButtonListener,
                   public virtual ASelectionListener,
                   public virtual AFocusListener,
                   public virtual AMouseListener,
                   public virtual AKeyListener
{
  Button *OKButton, *cancelButton;
  SComboBox *filesCombo;
  SComboBox *drivesCombo;
  ListBox *fileList, *dirList;
  SComboBox *dirsCombo;
  char *name;

  static int files, currentDrive, dirs;
  static char *filename[MAXFILES+1], currentDir[MAXPATH+1], *oldDir[MAXFILES+1], 
              filePattern[MAXPATH+1], *currentDirs['Z'-'A'+1];
  void insertFile( char *file );
  void insertDir( char *dir );
  void fillLists();
public:
  FileDialog( Window *frame, Window *parent, char *title, char *initFilename );
  virtual BOOL actionPerformed( AKeyEvent *keyEvent );
  virtual void actionPerformed( AButtonEvent *buttonEvent );
  virtual void actionPerformed( ASelectionEvent *selectionEvent );
  virtual void actionPerformed( AFocusEvent *focusEvent );
  virtual BOOL actionPerformed( AMouseEvent *mouseEvent );
/*  static int getFilenameLength()
  {
    return strlen( filename[0] );
  }*/

  static char *getFilename()
  {
    if( files )
      return filename[0];

    return NULL;
  }

  ~FileDialog()
  {
    WinProperties::updateProperty( name, getX(), getY(),
                                   getWidth(), getHeight() );
		delete name;
  }
};
