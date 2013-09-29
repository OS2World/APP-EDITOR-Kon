inWordWrap = FALSE;
wordwrap = FALSE;
doubleCRLFHardEnd = FALSE;

softEOL = charToString( 13 ) @ charToString( 13 ) @ charToString( 10 );
hardEOL = charToString( 13 ) @ charToString( 10 );

atSoftEnd( textPos )
{
  reader = readRequest();
  res = atSoftEndR( reader, textPos );
  readRelease();
  return res;
}

atHardEnd( textPos )
{
  reader = readRequest();
  res = atHardEndR( reader, textPos );
  readRelease();
  return res;
}

atSoftEndR( reader, textPos )
{
  if( !doubleCRLFHardEnd )
  {
    return rGetChar( reader, textPos ) == 13 && 
           rGetChar( reader, textPos + 1 ) == 13 &&
           rGetChar( reader, textPos + 2 ) == 10;
  }
  else
  {
    return (rGetChar( reader, textPos ) == 13 && 
            rGetChar( reader, textPos + 1 ) == 10 &&
            rGetChar( reader, textPos + 2 ) != 13 && 
            rGetChar( reader, textPos + 3 ) != 10) ||
           (rGetChar( reader, textPos ) == 10 &&
            rGetChar( reader, textPos + 1 ) != 10);
  }
}

atHardEndR( reader, textPos )
{
  if( !doubleCRLFHardEnd )
  {
    return rGetChar( reader, textPos ) == 10 ||
           (rGetChar( reader, textPos ) == 13 && 
            rGetChar( reader, textPos + 1 ) == 10) ||
           textPos >= getTextLength();
  }
  else
  {
    return (rGetChar( reader, textPos ) == 10 && 
            rGetChar( reader, textPos + 1) == 10) ||
           (rGetChar( reader, textPos ) == 13 && 
            rGetChar( reader, textPos + 1 ) == 10 && 
            rGetChar( reader, textPos + 2 ) == 13 && 
            rGetChar( reader, textPos + 3 ) == 10) ||
           textPos >= getTextLength();
  }
}

getLineLength()
{
  pushPos( FALSE );
  toLineEnd();
  lineLength = getCursorColumn();
  popPos();
  return lineLength;
}

// Overload buildMainMenu to add word wrap menu items
buildMainMenu()
{
  :buildMainMenu();
  beginSubmenuUpdate( EDITMENUID, FALSE );
  addMenuSeparator( EDITMENUID );
  addMenu( EDITMENUID, "~Word wrap", 2000 );
    addMenuItem( 2000, "~Word wrap", "updateWordwrap();", 2001 );
  addMenuSeparator( 2000 );
    addMenuItem( 2000, "Word wrap ~paragraph", "wrapSmart(getPosition(), FALSE);", 2002 );
    addMenuItem( 2000, "Word wrap ~file", "wrapFile();", 2003 );
  addMenuSeparator( 2000 );
    addMenuItem( 2000, "Use ~soft line breaks", "doubleCRLFHardEnd = !doubleCRLFHardEnd;setMenuChecks();", 2004 );
  addMenuSeparator( 2000 );
    addMenuItem( 2000, "~Remove soft linebreaks (unwrap paragraphs)", "removeSoftEOLs();", 2005 );
    addMenuItem( 2000, "Replace soft linebreaks with ~hard linebreaks", "replaceSoftEOLs();", 2006 );
  endSubmenuUpdate();
}

// Overload setMenuChecks
setMenuChecks()
{
  :setMenuChecks();
  checkMenuItem( 2001, wordwrap );
  checkMenuItem( 2004, !doubleCRLFHardEnd );
}

wrapFile()
{
  // wrap whole text
  startUndoBlock();
  setScreenUpdate( FALSE );
  pushPos( FALSE );
  toDocumentStart();
  rightMargin = getSetting( "rightmargin" );
  while( lineDown() )
  {
    toLineEnd();
    if( getCursorColumn() >= rightMargin )
    {
      toLineStart();
      wrapSmart( getPosition(), FALSE );
    }
  }
  popPos();
  setScreenUpdate( TRUE );
  endUndoBlock();
}

updateWordwrap()
{
  wordwrap = !wordwrap;
  setMenuChecks();

  if( !doubleCRLFHardEnd )
  {
    if( wordwrap && getTextLength() < 100000 )
      wrapFile();
    else
    {
      setScreenUpdate( FALSE );
      pushPos( FALSE );
      removeSoftEOLs();
      popPos();
      setScreenUpdate( TRUE );
    }
  }
}

// Overloaded functions to catch inserts/deletes where wordwrapping should
// take place. This does not take care of every case, but most of them.
insertString( str )
{
  if( !inWordWrap && wordwrap ) startUndoBlock();
  :insertString( str );
  if( inWordWrap || !wordwrap ) return;
  pushPos( FALSE );
  wrapSmart( getPosition(), TRUE );
  popPos();
  endUndoBlock();
}

deleteText( position, size )
{
  if( !inWordWrap && wordwrap ) startUndoBlock();
  :deleteText( position, size );
  if( inWordWrap || !wordwrap ) return;
  pushPos( FALSE );
  wrapSmart( position, TRUE );
  popPos();
  endUndoBlock();
}

deleteCharacter()
{
  if( !inWordWrap && wordwrap ) startUndoBlock();
  :deleteCharacter();
  if( inWordWrap || !wordwrap ) return;
  pushPos( FALSE );
  wrapSmart( getPosition(), TRUE );
  popPos();
  endUndoBlock();
}

deleteBlock()
{
  if( !inWordWrap && wordwrap ) startUndoBlock();
  :deleteBlock();
  if( inWordWrap || !wordwrap ) return;
  pushPos( FALSE );
  wrapSmart( getBlockStart(), TRUE );
  popPos();
  endUndoBlock();
}

deletePreviousCharacter()
{
  // Don't start wordwrap in deleteCharacter
  inWordWrap = TRUE;

  if( wordwrap ) startUndoBlock();
  deletedEOL = :deletePreviousCharacter();

  if( !wordwrap ) 
  {
    inWordWrap = FALSE;
    return;
  }

  if( deletedEOL && 
      (getChar( getPosition() - 1) == 32 || getChar( getPosition() - 1) == 45) )
  {
    :deletePreviousCharacter();
  }

  inWordWrap = FALSE;

  pushPos( FALSE );
  wrapSmart( getPosition(), TRUE );
  popPos();
  endUndoBlock();
}

insertLine()
{
  if( doubleCRLFHardEnd && wordwrap)
  {
    enterAtColumn = getCursorColumn();

    if( enterAtColumn == 1 ) startUndoBlock();

    inWordWrap = TRUE;

    :insertLine();

    inWordWrap = FALSE;

    if( enterAtColumn == 1 )
    {
      pushPos( FALSE );
      wrapSmart( getPosition(), TRUE );
      popPos();
      endUndoBlock();
    }
  }
  else
    :insertLine();
}

removeSoftEOLs()
{
  // Replace everyone without space before EOL
  replaceString( "(^[:s-])\\x0d\\x0d\\x0a", "\\1 ", "dgan" );
  // Replace the rest
  replaceString( "\\x0d\\x0d\\x0a", "", "dgan" );
}

replaceSoftEOLs()
{
  // Replace everyone without space before EOL
  replaceString( "(^[:s-])\\x0d\\x0d\\x0a", "\\1 \\x0d\\x0a", "dgan" );
  // Replace the rest
  replaceString( "\\x0d\\x0d\\x0a", "\\x0d\\x0a", "dgan" );
}

// Wordwrapping routine.
wrapSmart( startPos, stopWhenNoUpdate )
{
  if( doubleCRLFHardEnd ) 
    softEndLen = 2;
  else
    softEndLen = 3;

  inWordWrap = TRUE;
  stopUpdate = -1;

  startUndoBlock();

  setScreenUpdate( FALSE );

  jumpToPosition( startPos );

  if( stopWhenNoUpdate ) 
  {
    stopUpdate = 2;
    if( lineUp() )
    {
      toLineEnd();
      if( doubleCRLFHardEnd )
      {
        if( getChar( getPosition() - 1 ) == 10 || getPosition() == 0)
          lineDown(); // At hard end
      }
      else
      if( !atSoftEnd( getPosition() ) )
        lineDown();
    }
  }

  rightMargin = getSetting( "rightmargin" );
  textLength = getTextLength();

  while( 1 )
  {
    EOLatHardEnd = FALSE;
    toLineEnd();

    lineLength = getCursorColumn() - 1;
    lastLineLength = 0;
    softEnds = [];

    reader = readRequest();

    if( lineLength >= rightMargin )
    {
      setCursorColumn( rightMargin + 1);
      lineLength = rightMargin;
    }
    else
      while( lineLength < rightMargin )
      {
        pos = getPosition();
        if( pos >= textLength || rGetChar( reader, pos ) == 13 ||
            rGetChar( reader, pos ) == 10 )
        {
          if( atSoftEndR( reader, pos ) )
          {
            softEnds += [pos];
            lineDown();
            toLineStart();
            lastLineLength = lineLength;
          }
          else
          if( atHardEndR(reader, pos) ) 
          {
            EOLatHardEnd = TRUE;
            break;
          }
        }
        charRight();
        lineLength = lastLineLength + getCursorColumn() - 1;
      }

     readRelease();

    insertedEOL = FALSE;
    softEndsLen = length( softEnds );

    if( (lineLength == rightMargin && !atSoftEnd(getPosition()) && 
         !EOLatHardEnd) || softEndsLen > 0 )
    {
      if( lineLength == rightMargin )
      {
        lineStart = getLineStart();

        reader = readRequest();
        while( getPosition() > lineStart &&
               rGetChar( reader, getPosition() - 1 ) != 32 &&
               rGetChar( reader, getPosition() - 1 ) != 45 ) charLeft();
        readRelease();

        if( getPosition() != lineStart )
        {
          noInsert = FALSE;

          if( softEndsLen > 0 )
          {
            if( getPosition() == softEnds[softEndsLen-1] )
            {
              softEndsLen--;
              noInsert = TRUE;
            }
          }

          if( !noInsert )
          {
            if( doubleCRLFHardEnd )
              insertString( hardEOL );
            else
              insertString( softEOL );
            insertedEOL = TRUE;
          }
        }
        else
        {
          if( softEndsLen > 0 )
            if( softEnds[softEndsLen-1] + softEndLen == lineStart ) 
              softEndsLen--;
        }
      }
      for( i = softEndsLen-1; i >= 0; i-- )
      {
        if( getChar( softEnds[i] - 1) != 32 && 
            getChar( softEnds[i] - 1) != 45 &&
            getChar( softEnds[i] - 1) != 10 &&
            getChar( softEnds[i] + softEndLen ) != 32 && 
            getChar( softEnds[i] + softEndLen) != 45 &&
            getChar( softEnds[i] + softEndLen) != 13 ) 
        {
          pushPos( FALSE );
          jumpToPosition( softEnds[i] + softEndLen );
          insertString( " " );
          popPos();
        }
        deleteText( softEnds[i], softEndLen );
      }

      textLength = getTextLength();
    }
    else
    if( atSoftEnd(getPosition()) ) 
      lineDown();

    if( softEndsLen == 0 && !insertedEOL && stopWhenNoUpdate ) stopUpdate--;
    if( EOLatHardEnd || !stopUpdate ) break;
  }

  endUndoBlock();
  setScreenUpdate( TRUE );
  inWordWrap = FALSE;
}

// Another variant of the word wrapping routine. Works quicker if the paragraph
// has not already been wordwrapped once.
wrapParagraph( startPos )
{
  jumpToPosition( startPos );
  rightMargin = getSetting( "rightmargin" );
  setScreenUpdate( FALSE );

  startUndoBlock();

  while( 1 )
  {
    reader = readRequest();

    while( 1 )
    {
      toLineEnd();
      if( getCursorColumn() < rightMargin )
      {
        if( atSoftEndR( reader, getPosition() ) )
        {
          readRelease();
          deleteCharacter();
          reader = readRequest();
        }
      }
      else
        break;

      if( atHardEnd( reader, getPosition() ) ) break;
    }

    lineStart = getLineStart();

    setCursorColumn( rightMargin );

    if( atHardEndR( reader, getPosition() ) ) break;

    while( getPosition() > lineStart &&
           rGetChar( getPosition() - 1 ) != 32 &&
           rGetChar( getPosition() - 1 ) != 45) charLeft();

    readRelease();

    if( getPosition() != lineStart )
      insertString( charToString( 13 ) @ charToString( 13 ) @ 
                    charToString( 10 ) );
    else
      lineDown();
  }
  toLineEnd();

  endUndoBlock();

  setScreenUpdate( TRUE );
}
