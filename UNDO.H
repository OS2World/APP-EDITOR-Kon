/******************************************************************************
 * The Undo class holds all undo information for a buffer.
 *
 * The TextBuffer uses addInsertion and addDeletion to add undo entries.
 * The view uses setRepeatMark to group undoes and undo or redo to retrieve undo
 * and redo information.
 * the UndoList internally holds pointers to the correct position in the list.
 */

#if !defined UNDO

#include <string.h>
#include "globals.h"

#define STARTREPEAT  (UndoPos *)1L
#define ENDREPEAT  (UndoPos *)2L
#define VALIDUNDOPOINTER (UndoPos *)3L

class UndoObserver : public AObject
{
public:
  virtual void undoAvailable( BOOL hasUndo, BOOL atUnmodified  ) {}
  virtual void redoAvailable( BOOL hasRedo, BOOL atUnmodified  ) {}
};

enum UndoType { INSERTION, DELETION };

/*
 * The UndoPos contains the actual undo information. This struct is returned
 * to the view.
 */

struct UndoPos
{
  UndoPos( int position, UndoType type, TextElem *text, int length ) :
    position(position), type(type), length(length)
  {
    this->text = new TextElem[length];
    memcpy( this->text, text, length );
  }
  int position, length;
  UndoType type;
  TextElem *text;
  ~UndoPos()
  {
    delete[] text;
  }
};

/*
 * The Undo class holds the list of all undo's
 */

class TextBuffer;

class Undo
{
	friend TextBuffer;
  //----------------------------
  struct UndoList
  {
    UndoPos *undo;
    UndoList *next, *prev;
		BOOL saved;

    UndoList( UndoPos *type ) :
      next(NULL), prev(NULL), undo(type), saved(FALSE) {}
    UndoList( UndoType type, int position, TextElem *text,
              int length ) : next(NULL), prev(NULL), undo(NULL), saved(FALSE)
    {
      undo = new UndoPos( position, type, text, length );
    }
    // No cascading delete
    ~UndoList()
    {
      if( undo >= VALIDUNDOPOINTER )
        delete undo;
      if( prev )
        prev->next = NULL;
      if( next )
        next->prev = NULL;
    }
  } *currentUndoElem, *lastUndoElem, *firstUndoElem;

  //----------------------------

  int allocated;
  BOOL atFirst, atEnd;

  void removeFromPos( UndoList *elem )
  {
    if( elem )
    {
      removeFromPos( elem->next );
      if( elem->undo >= VALIDUNDOPOINTER )
        allocated -= elem->undo->length;
      delete elem;
    }
  }

  void removeExceeding();

  void addElem( UndoList *elem );

  void notifyObservers( BOOL reportUndo, BOOL hasUndo,
                        BOOL reportRedo, BOOL hasRedo );

  Vector<UndoObserver> undoObservers;
  BOOL removedAtEnd, initDestroyed;
  BOOL hasUndo, hasRedo, reported, hasSavedPos;	// Used when reporting
	BOOL saved;
	BOOL atSavedPos();
protected:
	void setSavedMark();
public:
  Undo() : allocated(0), currentUndoElem(NULL), lastUndoElem(NULL),
           firstUndoElem(NULL), atFirst(TRUE), atEnd(TRUE),
           undoObservers( FALSE ), removedAtEnd(FALSE), initDestroyed(FALSE),
					 reported(TRUE), saved(FALSE) {}

  void addUndoObserver( UndoObserver *undoObserver )
  {
    undoObservers.add( undoObserver );
  }

  void removeUndoObserver( UndoObserver *undoObserver )
  {
    undoObservers.removeObject( undoObserver );
  }

  void addInsertion( int position, TextElem *text, int length )
  {
    addElem( new UndoList( INSERTION, position, text, length ) );
  }

  void addDeletion( int position, TextElem *text, int length )
  {
    addElem( new UndoList( DELETION, position, text, length ) );
  }

  void setStartRepeat()
  {
    addElem( new UndoList( STARTREPEAT ) );
  }

  void setEndRepeat()
  {
    addElem( new UndoList( ENDREPEAT ) );
  }

  BOOL canUndo() { return !atFirst; }
  BOOL canRedo() { return !atEnd; }

  UndoPos *undo()
  {
    if( !atFirst )
    {
			if( reported )
			{
				hasUndo = canUndo();
				hasRedo = canRedo();
				hasSavedPos = atSavedPos();
				reported = FALSE;
			}

      UndoPos *undoElem = currentUndoElem->undo;

      if( currentUndoElem->prev )
        currentUndoElem = currentUndoElem->prev;
      else
        atFirst = TRUE;

      atEnd = FALSE;

      return undoElem;
    }
    return NULL;
  }

	void reportUndoRedo()
	{
    notifyObservers( canUndo() != hasUndo || hasSavedPos != atSavedPos(), canUndo(),
                     canRedo() != hasRedo || hasSavedPos != atSavedPos(), canRedo() );
		reported = TRUE;
	}

  UndoPos *redo()
  {
    if( !atEnd )
    {
			if( reported )
			{
	      hasUndo = canUndo();
				hasRedo = canRedo();
				hasSavedPos = atSavedPos();
				reported = FALSE;
			}

      UndoPos *undoElem;

      if( !atFirst )
        undoElem = currentUndoElem->next->undo;
      else
        undoElem = currentUndoElem->undo;

      if( currentUndoElem->next && !atFirst )
        currentUndoElem = currentUndoElem->next;

      atEnd = (currentUndoElem->next == NULL);

      atFirst = FALSE;

      return undoElem;
    }
    return NULL;
  }

  void clearUndo();

//	void reportRedo()
//	{
//    notifyObservers( canUndo() != hasUndo, canUndo(),
//                     canRedo() != hasRedo, canRedo() );
//	}
};

#define UNDO

#endif

