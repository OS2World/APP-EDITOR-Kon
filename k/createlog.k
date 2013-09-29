fileCounter=1;
path = "c:\\test\\log\\";

createSQLFile( type )
{
  currentId = getCurrentControlId();
  writeFilename = path;

  if( type == 0 )
    writeFilename @= "br";
  else
    writeFilename @= "upd";

  writeFilename @= fileCounter @ ".SQL";

  keepBlock( TRUE );
  beginMark();
  findString( '-- T', 'ur' );
  toLineStart();
  endMark();
  copyBlockToClipboard();
  keepBlock( FALSE );
  lineDown();
  lineDown();
  lineDown();

  id = loadFile( writeFilename, FALSE, FALSE );

  switchExecutingControl( id );
  switchToControl( id );
  pasteBlockFromClipboard();
  saveFile();
  fileCounter++;

  switchExecutingControl( currentId );
  switchToControl( currentId );

  closeControl( id );

  id = loadFile( path @ "execorder.txt", FALSE, FALSE );

  switchExecutingControl( id );
  switchToControl( id );
  toDocumentEnd();

  if( type == 0 )
    insertString( "File.getTable( \"" @ writeFilename @ "\")\r\n");
  else
    insertString( "File.update( \"" @ writeFilename @ "\")\r\n");

  saveFile();

  switchExecutingControl( currentId );
  switchToControl( currentId );

  closeControl( id );
}

initModule()
{
  bindKey( "Ctrl+b", "createSQLFile( 0 );" );
  bindKey( "Ctrl+u", "createSQLFile( 1 );" );
}
