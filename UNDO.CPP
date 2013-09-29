#include "bowl.h"
#include "buffer.h"
#include "undo.h"

BOOL Undo::atSavedPos()
{
	return (currentUndoElem && !atEnd && !(atFirst && atEnd)) ? currentUndoElem->saved : FALSE;
}

void Undo::setSavedMark()
{
	if( !atFirst || !atEnd )
	{
		saved = TRUE;
		UndoList *elem = firstUndoElem;
		while( elem )
		{
			elem->saved = FALSE;
			elem = elem->next;
		}
	}
	currentUndoElem->saved = TRUE;
}

void Undo::removeExceeding()
{
  // Remove elements first in list until we reach maxUndoSize
  while( allocated > Settings::maxUndoSize )
  {
		initDestroyed = TRUE;
    UndoList *nextElem = firstUndoElem->next;
    if( firstUndoElem->undo >= VALIDUNDOPOINTER )
      allocated -= firstUndoElem->undo->length;
    delete firstUndoElem;
    firstUndoElem = nextElem;
    removedAtEnd = TRUE;
    if( firstUndoElem == currentUndoElem ) break;
  }

	if( firstUndoElem == NULL )	// If we removed all elements
		atFirst = atEnd = TRUE;
}

void Undo::addElem( UndoList *elem )
{
  BOOL hasUndo = canUndo(), hasRedo = canRedo();

  if( atFirst )
  {
    removeFromPos( firstUndoElem );
    currentUndoElem = NULL;
  }
  else
    removeFromPos( currentUndoElem->next );

  lastUndoElem = elem;

  if( currentUndoElem )
    currentUndoElem->next = elem;

  elem->prev = currentUndoElem;

  currentUndoElem = elem;

  if( atFirst )
    firstUndoElem = elem;

  atFirst = FALSE;
  atEnd = TRUE;

  if( elem->undo >= VALIDUNDOPOINTER )
    allocated += elem->undo->length;

  removeExceeding();

  // Notify if undo/redo state has changed

  notifyObservers( canUndo() != hasUndo, canUndo(),
                   canRedo() != hasRedo, canRedo() );
}

void Undo::notifyObservers( BOOL reportUndo, BOOL hasUndo,
                            BOOL reportRedo, BOOL hasRedo )
{
	BOOL atFileInitLevel = !hasUndo && !initDestroyed;

  if( reportUndo )
    for( int i = 0; i < undoObservers.getSize(); i++ )
      undoObservers.getObjectAt(i)->undoAvailable( hasUndo, (atFileInitLevel && !saved) || atSavedPos() );

  if( reportRedo )
    for( int i = 0; i < undoObservers.getSize(); i++ )
      undoObservers.getObjectAt(i)->redoAvailable( hasRedo, atSavedPos() );
}

void Undo::clearUndo()
{
  removeFromPos( firstUndoElem );
  firstUndoElem = currentUndoElem = NULL;

  atFirst = atEnd = TRUE;
}
