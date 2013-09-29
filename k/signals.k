// A list of grep expressions which is used to identify filename and position
// if the user doubleclicks in the message window. (For example after a 
// multifile search)
// When the buffer is cleared, this array is also cleared  to the default
// value as belwo

multfileSearchRegExp = "(.)+ \\((:n)+, (:n)+\\) \:";
filePosRegExp = [[multfileSearchRegExp,[0,1,2]]];

// Triggered when block is available/unavailable or
// when undo/redo is availabale/unavailable

blockMarked( blockAvailable )
{
  enableMenuItem( 303, blockAvailable );
  enableMenuItem( 304, blockAvailable );
  setToolActive( 4, blockAvailable );
  setToolActive( 5, blockAvailable );
  setToolActive( 11, blockAvailable );
  setToolActive( 12, blockAvailable );
}

undoAvailable( hasUndo )
{
  enableMenuItem( 301, hasUndo );
  setToolActive( 7, hasUndo );
}

redoAvailable( hasRedo )
{
  enableMenuItem( 302, hasRedo );
  setToolActive( 8, hasRedo );
}

columnBlockSet( set )
{
  checkMenuItem( 306, set );
}

viewChanged( newView )
{
  if( newView == ASCIIVIEW )
  {
    checkMenuItem( 501, FALSE );
    checkMenuItem( 502, TRUE );
  }
  else
  {
    checkMenuItem( 501, TRUE );
    checkMenuItem( 502, FALSE );
  }
}

savePos()
{
  :savePos();
  setMenuChecks();
  enableToolbarItems();
}

savePos()
{
  :savePos();
  setMenuChecks();
  enableToolbarItems();
}

clearSavedPositions()
{
  :clearSavedPositions();
  setMenuChecks();
  enableToolbarItems();
}

enableToolbarItems()
{
  setToolActive( 15, hasSavedPositions() );
  setToolActive( 16, hasSavedPositions() );
  setToolActive( 17, hasSavedPositions() );
}

mouseDoubleClick()
{
  // Check if this is the messages window. In that case double cliok
  // may be done on a file which should be opened on a specific line and 
  // column. (Multiple files search)
  // Otherwise, mark the word.
  if( getFilename() == "*messages*" )
  {
    toLineEnd();
    setBlockStart( getLineStart() );
    markToLineStart();

    for( rPos = 0; rPos < length(filePosRegExp); rPos++ )
    {
      fileAndPos = [];

      if( findStringGrep( filePosRegExp[rPos][0], "ds", fileAndPos ) )
      {
        controlId = loadFile( fileAndPos[filePosRegExp[rPos][1][0]], FALSE, TRUE );
        switchExecutingControl( controlId );
        jumpToLine( toInt(fileAndPos[filePosRegExp[rPos][1][1]]) );
        setCursorColumn( toInt(fileAndPos[filePosRegExp[rPos][1][2]]) );
        return;
      }
    }
  }
  markWord();
}

mouseTripleClick()
{
  markLine();
}

resetFPRegExp()
{
  filePosRegExp = [];
  addFPRegExp(multfileSearchRegExp, [0,1,2]);
}

addFPRegExp( regExp, fileAndPosGroup )
{
  filePosRegExp += [[regExp,fileAndPosGroup]];
}