/******************************************************************************
 * Events
 *
 * A class that wishes to listen for specific events inherits from the
 * listener classes it's interrested of. Note that inheritance should be
 * done with virtual listeners. Any number of listeners can be attached
 * to a specific window or controls.
 */

/* AEvent: Event base class */

class AEvent
{
protected:
  AObject *source;
public:
  AObject *getSource() { return source; }
};

/* AMouseEvent: Mouse event class */

class AMouseEvent : public AEvent
{
  Point *pos;
  int button, event;
public:
  static int LEFT, MIDDLE, RIGHT, NONE;
  static int DOWN, RELEASE, CLICK, MOVE, DOUBLECLICK, TRIPLECLICK, WHEEL;
  AMouseEvent( AObject *source, Point *pos, int button, int event )
  {
    this->source = source;
    this->pos = pos;
    this->button = button;
    this->event = event;
  }
  int getEvent() { return event; }
  Point *getPos() { return pos; }
  int getButton() { return button; }
};

/* Listener base class. */

class AListener
{
public:
  virtual int equals( AObject *) { return FALSE; }
};

/* AMouseListener: Mouse listener template */

class AMouseListener : public AListener
{
public:
	// Return TRUE if the window should get focus
  virtual BOOL actionPerformed( AMouseEvent *mouseEvent ) { return TRUE; };
};

/* ASelectionEvent: A selection event */

class ASelectionEvent : public AEvent
{
  int selectedItem;
public:
  ASelectionEvent( AObject *source, int selectedItem )
  {
    this->source = source;
    this->selectedItem = selectedItem;
  }

  /* getSelected:
      returns: Returns the position in the list.*/

  int getSelected() { return selectedItem; }
};

/* ASelectionListener: Listens to ListBoxes, ComboBoxes */

class ASelectionListener : public AListener
{
public:
  virtual void actionPerformed( ASelectionEvent *selectionEvent ) {};
};

/* AFocusEvent: Reacts to focus changes in a control, f.e. a listbox */

class AFocusEvent : public AEvent
{
  int focusItem;
public:
  AFocusEvent( AObject *source, int focusItem )
  {
    this->source = source;
    this->focusItem = focusItem;
  }

  /* getSelected:
      returns: Returns the position in the list.*/

  int getFocused() { return focusItem; }
};

/* AFocusListener: Listens to focus changes in ListBoxes, ComboBoxes */

class AFocusListener : public AListener
{
public:
  virtual void actionPerformed( AFocusEvent *focusEvent ) {};
};

/* AIdEvent: An event that returns an lexId */

class AIdEvent : public AEvent
{
  int id;
public:
  AIdEvent( AObject *source, int id )
  {
    this->source = source;
    this->id = id;
  }

  /* getSelected:
      returns: Returns the ID */

  int getId() { return id; }
};

/* AIdListener: Listens to controls which returns an lexId */

class AIdListener : public AListener
{
public:
  void actionPerformed( AIdEvent *idEvent ) {};
};

/* AMenuEvent: A Menu event */

class AMenuEvent : public AIdEvent
{
public:
  AMenuEvent( AObject *source, int id ) : AIdEvent( source, id ) {};
};

/* AMenuListener: Listens to Menues */

class AMenuListener : public AIdListener
{
public:
  virtual void actionPerformed( AMenuEvent *menuEvent )
  {
    AIdListener::actionPerformed( (AIdEvent *)menuEvent );
  }
};

/* AButtonEvent: A button event */

class AButtonEvent : public AIdEvent
{
public:
  AButtonEvent( AObject *source, int id ) : AIdEvent( source, id ) {};
};

/* AButtonListener: Listens to buttons */

class AButtonListener : public AIdListener
{
public:
  virtual void actionPerformed( AButtonEvent *buttonEvent ) {}
};

/* AScrollbarEvent: A scrollbar event
 * Note that pos is relative the range!
 */

class AScrollbarEvent : public AIdEvent
{
  int pos, action;
public:
  AScrollbarEvent( AObject *source, int id, int action, int pos ) :
    AIdEvent( source, id ) { this->pos = pos; this->action = action; }
  int getPos() { return pos; }
  int getAction() { return action; }
};

/* AButtonListener: Listens to buttons */

class AScrollbarListener : public AIdListener
{
public:
  virtual void actionPerformed( AScrollbarEvent *scrollbarEvent ) {}
};

/* AToolbarEvent: A toolbar event */

class AToolbarEvent : public AEvent
{
  char *command;
public:
  AToolbarEvent( AObject *source, char *command )
  {
    this->source = source;
    this->command = command;
  }
  char *getCommand() { return command; }
};

class Toolbutton;

/* AToolbarlistener :Listens to toolbars */

class AToolbarListener : public AListener
{
public:
  virtual void actionPerformed( AToolbarEvent *toolbarEvent ) {}
};

/* AKeyEvent: A key event */
class AKeyEvent : public AEvent
{
public:
  int modifiers, key;
  AKeyEvent( AObject *source, int modifiers, int key )
  {
    this->modifiers = modifiers;
    this->key = key;
  };
};

/* AKeyListener: Listens to buttons */
class AKeyListener : public AIdListener
{
public:
  // Return true if the key is processed
  virtual BOOL actionPerformed( AKeyEvent *keyEvent ) { return FALSE; }
};

/* AWinMoveEvent: Activates when a window is moved */
class AWinMoveEvent : public AEvent
{
  int oldX, oldY, newX, newY;
public:
  AWinMoveEvent( AObject *source, int oldX, int oldY, int newX, int newY )
  {
    this->source = source;
    this->oldX = oldX;
    this->oldY = oldY;
    this->newX = newX;
    this->newY = newY;
  }
  int getOldX() { return oldX; }
  int getOldY() { return oldY; }
  int getNewX() { return newX; }
  int getNewY() { return newY; }
};

/* AWinMoveListener: Listens to window moves */

class AWinMoveListener : public AListener
{
public:
  virtual void actionPerformed( AWinMoveEvent *moveEvent ) {};
};


/* APositionEvent: Activates when the cursor position is changed */
class APositionEvent : public AEvent
{
  long position;
public:
  APositionEvent( AObject *source, long position )
  {
    this->position = position;
  }
  int getPosition() { return position; }
};

/* APositionListener: Listens to cursor moves */

class APositionListener : public AListener
{
public:
  virtual void actionPerformed( APositionEvent *positionEvent ) {};
};


