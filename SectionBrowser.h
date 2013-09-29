#if !defined SECTIONBROWSER
#define SECTIONBROWSER
#include "Buffer.h"
#include "Syntax.h"
#include "Asciview.h"

class EditControl;

struct Section
{
  char *name;
  int sectionStartPos, sectionEndPos;
  int line;
  ~Section() { delete name; }
};

class SectionBrowser : public Window, public virtual BufferObserver,
                       public virtual ASelectionListener,
                       public virtual ALineMoveListener,
                       public virtual AToolbarListener
{
  Section **section, **sortedPos;
  int sections;
  SComboBox *sectionCombo;
  EditControl *editControl;
  BOOL bufferModified, allowSelectEvent;
  void scanRegion( int reader, TextBuffer *buffer, Syntax *syntax, int startPos, int endPos );
  void fillCombo();
  int selectedIndex;
  Toolbar *toolbar;
  BOOL executingSearch, terminateSearch;
  MutexSemaphore searchSem;
public:
  SectionBrowser( EditControl *parent, int x, int y, int width, int height );
  virtual void insertion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines );
  virtual void deletion( TextBuffer *textBuffer, int reader, int position,
                         int size, int lines );
  virtual void modification( TextBuffer *textBuffer, int reader, int position,
                             int size );
  int getHeight();
  void paint( Rect *rect, Graphics *g );
  void size( int oldWidth, int oldHeight );
  void regenerate( BOOL force );
  virtual void actionPerformed( ASelectionEvent *selectionEvent );
  virtual void actionPerformed( ALineMoveEvent *moveEvent );
  virtual void actionPerformed( AToolbarEvent *toolbarEvent );
  Section *getSection( int index ) { return section[index]; }
  int findSectionAfterPos( int atPos );
  void terminate();
  virtual ~SectionBrowser();
};

#endif