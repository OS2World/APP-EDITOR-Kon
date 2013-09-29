filling = FALSE;

// Fills a selected column block with the string str. If repeat is TRUE
// the string is repeated from the left column to the right column for each 
// line in the block.
fillColumnBlock( str, repeat )
{
  if( queryView() != ASCIIVIEW ) return;

  filling = TRUE;

  persistentBlocks = getSetting( "persistentblocks" );
  insertMode = getSetting( "insertmode" );

  updateSetting( "persistentblocks", TRUE );
  updateSetting( "insertmode", TRUE );

  blockStartCol = getMarkBeginColumn();
  blockEndCol = getMarkEndColumn();

  insStr = "";

  if( repeat )
  {   
    loops = (blockEndCol - blockStartCol) / length(str);
    for( i = 0; i < loops; i++ )
      insStr @= str;
  }
  else
   insStr = str;

  loops = (blockEndCol - blockStartCol) - length(insStr);

  for( i = 0; i < loops; i++ )
    insStr @= " ";

  setScreenUpdate( FALSE );
  startUndoBlock();

  jumpToPosition( getBlockStart() );
  setCursorColumn( blockStartCol );

  while( getPosition() < getBlockEnd() )
  {
    insertPoint = getPosition();

    setCursorColumn( blockEndCol );

    deleteChars = getPosition() - insertPoint;

    setCursorColumn( blockStartCol );

    // Insert before delete to preserve block end

    insertString( insStr );

    deleteText( insertPoint + length( insStr ), deleteChars );

    if( !lineDown() ) break;

    setCursorColumn( blockStartCol );
  }

  endUndoBlock();
  setScreenUpdate( TRUE );

  jumpToPosition( getBlockEnd() ); 
  setCursorColumn( blockEndCol );

  updateSetting( "persistentblocks", persistentBlocks );
  updateSetting( "insertmode", insertMode );

  filling = FALSE;
}

insertString( str )
{
  if( filling || !hasValidBlockMark() || queryView() != ASCIIVIEW ||
      !getSetting( "columnblock") || getSetting( "persistentblocks" ))
    :insertString( str );
  else
    fillColumnBlock( str, TRUE );
}

fillColumnText()
{
  string = "";

  if( queryLineDialog( "Fill column", "Enter string:", string ) )
  {
    fillColumnBlock( string, FALSE );
  }
}