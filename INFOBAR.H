/******************************************************************************
 * The InfoBar holds cursor information, buffer information, messages etc.
 */

#if !defined MINFOBAR

class InfoBar : public Window
{
  char posInfo[50], lastMessage[100];
  char *insertString, *overwriteString, *modifiedString, *recordString;
  BOOL modified, record;
  int modifiedX, insertX, recordX, busyX, lastMessageX;
  BOOL positionsSet, busyShowing;
public:
  InfoBar( Window *parent, int x, int y, int width, int height ) :
    modified(FALSE), record(FALSE), positionsSet( FALSE ), busyShowing( FALSE ),
    Window( parent, x, y, width, height )
  {
    *posInfo = 0, *lastMessage = 0;
    modifiedString = Strings::get(31);
    insertString = Strings::get(32);
    overwriteString = Strings::get(33);
    recordString = Strings::get(34);
  }
  void drawShade( Graphics *g, int x, int y, int width, int height, BOOL in );
  void paint( Rect *rect, Graphics *g );
  void updatePosition( char *posString );
  void message( char *message );
  void setModified( BOOL isModified );
  void setRecord( BOOL isRecording );
  void clearMessage();
  void showBusy( BOOL isBusy );
  static int getInset()
  {
    return 8;
  }
  void displayInfo( Graphics *g )
  {
  }
};

#define MINFOBAR

#endif