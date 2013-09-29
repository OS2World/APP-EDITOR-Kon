// Do not alter these constants!

ASCIIVIEW = 0;
HEXVIEW = 1;
TRUE = 1;
FALSE = 0;

MBERROR = 1;
MBWARNING = 2;
MBMESSAGE = 3;
MBQUERY = 4;

MBRESOK = 5;
MBRESCANCEL = 6;
MBRESYES = 7;
MBRESNO = 8;

FILEDLGSAVE = 0;
FILEDLGOPEN = 1;

ENCODING_ANSI = 0;
ENCODING_UTF8 = 1;
ENCODING_UTF16 = 2;

// basic functions

atLineEnd( reader, pos )
{
return( rGetChar( reader, pos ) == 10 ||
(rGetChar( reader, pos ) == 13 && rGetChar( reader, pos+1) == 10) );
}

insertLine()
{
  if( queryView() != ASCIIVIEW ) return;

  iPos = getPosition();
  sPos = getLineStart();
  spaces = 0;
  tabSize = getSetting( "tabsize" );

  reader = readRequest();

  while( 1 )
  {
    c = rGetChar( reader, sPos++ );

    if( sPos > iPos )
    {
      spaces = 0;
      break;
    }

    if( c == 32 )
      spaces++;
    else
    if( c == 9 )
      while( ++spaces % tabSize );
    else
      break;
  }

  if( getSetting( "crlf" ) )
    iStr = "\r\n";
  else
    iStr = "\n";

  // Only indent if we're not at the line end
  isAtLineEnd = atLineEnd( reader, iPos );

  optimalFill = getSetting( "optimalfill" );

  if( !isAtLineEnd )
  {
    i = 0;
    while( i < spaces )
    {
      if( optimalFill )
      {
        if( i + tabSize <= spaces )
        {
          iStr += charToString(9);
          i += tabSize - 1;
        }
        else
          iStr += " ";
      }
      else
        iStr += " ";
      i++;
    }
  }

  // See if the line we're at is empty. If so, set column to current
  if( getLineStart() == getPosition() )
    spaces = getCursorColumn() - 1;

  readRelease();

  if( isAtLineEnd ) // Set the cursor column to line end, so insertString
    toLineEnd();    // won't fill up with spaces

  insertString( iStr );

  if( isAtLineEnd )
    setCursorColumnTmpDisVS( spaces + 1 );
}

// Returns the column where spaces ended
getLastSpaceColumn()
{
  reader = readRequest();

  while(1)
  {
    c = rGetChar( reader, getPosition() );
    if( c != 32 && c != 9 ) break;
    charRight();
  }

  readRelease();

  return getCursorColumn();
}

getPrevLastSpaceColumn()
{
  reader = readRequest();

  while(1)
  {
    if( !charLeft() ) break;
    c = rGetChar( reader, getPosition() );
    if( c != 32 && c != 9 ) 
    { 
      charRight(); 
      break; 
    }
  }

  readRelease();

  return getCursorColumn();
}

indentBlock( steps )
{
  if( queryView() != ASCIIVIEW ) return;

  keepBlock( TRUE );

  pushPos( FALSE );

  setScreenUpdate( FALSE );

  startUndoBlock();

  leftBlockColumn = getMarkBeginColumn();
  rightBlockColumn = getMarkEndColumn();

  jumpToPosition( getBlockStart() );
  columnBlock = getSetting( "columnblock" );

  while( getPosition() < getBlockEnd() )
  {
    if( columnBlock )
      setCursorColumn( leftBlockColumn );
    else
      toLineStart();

    reader = readRequest();

    isAtLineEnd = atLineEnd( reader, getPosition());

    readRelease();

    if( !isAtLineEnd )
    {
      startPos = getPosition();
      startCol = getCursorColumn();

      column = getLastSpaceColumn();

      deleteText( startPos, getPosition() - startPos );

      indent( startPos, startCol, column + steps );
    }

    if( !lineDown() ) break;
  }

  endUndoBlock();

  blockStart = getBlockStart();
  blockEnd = getBlockEnd();

  popPos();

  keepBlock( FALSE );

  if( columnBlock )
  {
    // Fix columns for block
    setBlockEnd(0);
    jumpToPosition( blockStart );
    setCursorColumn( leftBlockColumn + steps );
    beginMark();
    jumpToPosition( blockEnd );
    setCursorColumn( rightBlockColumn + steps );
    endMark();
  }

  setScreenUpdate( TRUE );
}

outdentBlock( steps )
{
  if( queryView() != ASCIIVIEW ) return;

  keepBlock( TRUE );

  pushPos( FALSE );

  setScreenUpdate( FALSE );

  startUndoBlock();

  leftBlockColumn = getMarkBeginColumn();
  rightBlockColumn = getMarkEndColumn();
  movedLeft = -1;
  movedRight = -1;

  jumpToPosition( getBlockStart() );
  columnBlock = getSetting( "columnblock" );

  while( getPosition() < getBlockEnd() )
  {
    if( columnBlock )
    {
      setCursorColumn( leftBlockColumn );
      getPrevLastSpaceColumn();
    }
    else
      toLineStart();

    startPos = getPosition();
    startCol = getCursorColumn();

    column = getLastSpaceColumn();

    if( column - startCol > 1 )
    {
      deleteText( startPos, getPosition() - startPos );

      toCol = column - steps;

      if( toCol < startCol ) toCol = startCol;

      if( movedLeft == -1 ) movedLeft = column - toCol;

      movedRight = column - toCol;

      indent( startPos, startCol, toCol );
    }
    else if( movedLeft == -1 )
    {
      // Find out if we're at virtual space.
      // If so, this line should be disregarded when calculating how far 
      // left we were able to move.
      toLineEnd();
      if( getCursorColumn() >= startCol )
        movedLeft = 0;
    }

    if( !lineDown() ) break;
  }

  endUndoBlock();

  blockStart = getBlockStart();
  blockEnd = getBlockEnd();

  popPos();

  keepBlock( FALSE );

  if( movedLeft == -1 ) movedLeft = 0;
  if( movedRight == -1 ) movedRight = 0;

  if( columnBlock )
  {
    // Fix columns for block
    setBlockEnd(0);
    jumpToPosition( blockStart );
    setCursorColumn( leftBlockColumn - movedLeft );
    beginMark();
    jumpToPosition( blockEnd );
    setCursorColumn( rightBlockColumn - movedRight );
    endMark();
  }

  setScreenUpdate( TRUE );
}

deletePreviousCharacter()
{
  if( !getSetting( "persistentblocks" ) &&
      hasValidBlockMark() )
  {
    deleteBlock();
    return FALSE;
  }

  deletedEOL = FALSE;

  // Check if we're at virtual space by storing the position
  charPos = getPosition();

  if( !charLeft() )
  {
    if( !lineUp() )
    {
      keepBlock( FALSE );
      return;
    }
    toLineEnd();
    deletedEOL = TRUE;
  }

  if( charPos != getPosition() )
    deleteCharacter();

  return deletedEOL;
}

isDelim( c )
{
  dLen = length( delim );
  for( i = 0; i < dLen; i++ )
    if( delim[i] == c )
      return 1;
  return 0;
}

nextWord()
{
  reader = readRequest();

  pos = getPosition();
  textLength = getTextLength();

  while( pos < textLength )
    if( isDelim( rGetChar( reader, pos++ ) ) )
    {
      while( pos < textLength &&
             isDelim( rGetChar( reader, pos ) ) ) pos++;
      break;
    }

  readRelease();
  jumpToPosition( pos );
}

markNextWord()
{
  beginMark();
  nextWord();
  endMark();
}

prevWord()
{
  reader = readRequest();

  pos = getPosition();

  while( pos > 0 )
    if( !isDelim( rGetChar( reader, --pos ) ) )
    {
      while( pos > 0 &&
             !isDelim( rGetChar( reader, pos-1 ) ) ) pos--;
      break;
    }

  readRelease();
  jumpToPosition( pos );
}

markPrevWord()
{
  beginMark();
  prevWord();
  endMark();
}

markLineDown()
{
  beginMark();
  res = lineDown();
  endMark();
  return res;
}

markLineUp()
{
  beginMark();
  res = lineUp();
  endMark();
  return res;
}

markCharLeft()
{
  beginMark();
  res = charLeft();
  endMark();
  return res;
}

markCharRight()
{
  beginMark();
  res = charRight();
  endMark();
  return res;
}

markToLineEnd()
{
  beginMark();
  toLineEnd();
  endMark();
}

markToLineStart()
{
  beginMark();
  toLineStart();
  endMark();
}

markPageDown()
{
  beginMark();
  pageDown();
  endMark();
}

markPageUp()
{
  beginMark();
  pageUp();
  endMark();
}

markToDocumentStart()
{
  beginMark();
  toDocumentStart();
  endMark();
}

markToDocumentEnd()
{
  beginMark();
  toDocumentEnd();
  endMark();
}

deleteLine()
{
  toLineStart();
  position = getPosition();
  lineDown();

  newPosition = getPosition();

  if( newPosition == position )
    newPosition = getTextLength();

  deleteText( position, newPosition-position );
}

deleteToEOL()
{
  position = getPosition();

  toLineEnd();

  deleteText( position, getPosition()-position );
}

deleteWord()
{
  reader = readRequest();

  pos = getPosition();

  while( pos < getTextLength() && isDelim( rGetChar( reader, pos )) ) pos++;
  while( pos < getTextLength() && !isDelim( rGetChar( reader, pos )) ) pos++;

  readRelease();

  deleteText( getPosition(), pos-getPosition() );
}

tabPressed()
{
  if( queryView() == HEXVIEW )
    switchField();
  else
  if( !getSetting( "persistentblocks" ) &&
      hasValidBlockMark() )
    indentBlock( getSetting( "tabsize" ) );
  else
  {
    if( getSetting( "usetab" ) )
      insertString( '\t' );
    else
    {
      tabSize = getSetting( "tabsize" );
      column = getCursorColumn();

      spaces = " ";

      while( column % tabSize )
      {
        column++;
        spaces += " ";
      }

      insertString( spaces );
    }
  }
}

cursorHome()
{
  // Store current column
  prevCol = getCursorColumn();

  toLineStart();

  // Find first non-space character
  reader = readRequest();

  pos = getPosition();
  textLength = getTextLength();

  while( pos < textLength &&
         (rGetChar( reader, pos ) == 32 || rGetChar( reader, pos ) == 9) )
  {
    pos++;
  }

  readRelease();

  jumpToPosition( pos );

  if( getCursorColumn() == prevCol )
  {
    toLineStart();
  }
}

markWord()
{
  keepBlock( TRUE );

  resetMark();

  if( getSetting( "persistentblocks" ) )
    pushPos( FALSE );

  reader = readRequest();

  pos = getPosition();

  while( pos > 0 && !isDelim( rGetChar( reader, pos )) ) pos--;
  if( isDelim(rGetChar(reader, pos)) ) pos++;
  jumpToPosition( pos );
  beginMark();
  while( pos < getTextLength() && !isDelim( rGetChar( reader, pos )) ) pos++;
  readRelease();
  jumpToPosition( pos );
  endMark();

  if( getSetting( "persistentblocks" ) )
    popPos();

  keepBlock( FALSE );
}

markLine()
{
  keepBlock( TRUE );

  resetMark();

  if( getSetting( "persistentblocks" ) )
    pushPos( FALSE );
  toLineStart();
  beginMark();
  position = getPosition();
  lineDown();

  if( getPosition() == position )
    toLineEnd();

  endMark();

  if( getSetting( "persistentblocks" ) )
    popPos();
  keepBlock( FALSE );
}

fileLoadDialog()
{
  filename = loadDialog( "Load file", "" );

  if( filename != "" )
    loadFile( filename, TRUE, getSetting("reuseExistingBuffer") );
}

fileSaveDialog()
{
  filename = getFilename();

  while( TRUE )
  {
    encoding = getEncoding();
    filename = saveDialog( "Save file", filename, encoding );

    if( fileExists( filename ) )
    {
      result = messageBox( "Warning!", "File " @ filename @ " already exists!" @
                           " Do you want to overwrite it?", MBQUERY );

      if( result == MBRESNO )
        continue;

      if( result == MBRESCANCEL )
        break;
    }

    if( filename != "" )
      saveFileAs( filename, encoding );
    break;
  }
}

macroSaveDialog()
{
  encoding = getEncoding();

  filename = saveDialog( "Save macro", "", encoding );

  if( filename != "" )
    saveMacro( filename );
}

jumpToLineDlg()
{
  line = toString( getLine() );

  queryLineDialog( "Jump to line", "Jump to line:", line );

  jumpToLine( toInt( line ) );

  highlightCurrentLine();
}

jumpToPosDlg()
{
  pos = toString( getPosition() );

  queryLineDialog( "Jump to position", "Jump to position:", pos );

  jumpToPosition( toInt( pos ) );

  highlightCurrentLine();
}

toggleLeftMargin()
{
  updateSetting( "showLeftMarginInfo", !getSetting( "showLeftMarginInfo" ) );
  resetViewWin( TRUE );
}

toggleMarksOnly()
{
  updateSetting( "showSavedPos", TRUE );
  updateSetting( "showLineNumbers", !getSetting( "showLineNumbers" ) );
  updateSetting( "showLeftMarginInfo", TRUE );
  resetViewWin( TRUE );
}

showLineNumbers()
{
  updateSetting( "showLineNumbers", TRUE );
  updateSetting( "showLeftMarginInfo", TRUE );
  resetViewWin( TRUE );
}

convertTabsToSpaces()
{
  setScreenUpdate( FALSE );
  spaceString = " ";

  for( i = 0; i < getSetting( "tabsize" ); i++ )
    spaceString = spaceString @ " ";

  toDocumentStart();

  startUndoBlock();

  while( findString( "\\009", "g" ) )
  {
    // Store current column
    afterCol = getCursorColumn();

    // Back one step to stand on tab and delete it
    charLeft();
    deleteCharacter();

    // Insert column delta as spaces
    insertString( leftString( spaceString, afterCol - getCursorColumn()) );
  }

  endUndoBlock();

  setScreenUpdate( TRUE );
}

recordMacro()
{
  if( isRecording() )
    endMacroRecord();
  else
    startMacroRecord();
}

playMacro()
{
  if( isRecording() )
    endMacroRecord();

  recordedMacro();
}

markAll()
{
  toDocumentStart();
  beginMark();
  toDocumentEnd();
  endMark();
}

clearAll()
{
  columnBlock = getSetting( "columnblock" );
  updateSetting( "columnblock", FALSE );
  markAll();
  deleteBlock();
  updateSetting( "columnblock", columnBlock );
}

launchProcessToMessages( exefile, arguments, positionRegExp, fileAndPosGroup )
{
  if( positionRegExp != "" ) addFPRegExp( positionRegExp, fileAndPosGroup );
  launchProcess(exefile, arguments, TRUE, getFileId( "*messages*"));
}

removeTrailingBlanks()
{
  replaceString( "[:s]+($)", "\\1", "dgan" );
}