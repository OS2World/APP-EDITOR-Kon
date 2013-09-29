// This is an automatically generated file.
// If the toolbar is manually changed, this file will be overwritten
// on program exit.
// Edit with caution.

setupToolbar()
{
  addToolButton( "loadFile( getScratchName(), TRUE, FALSE );", "NewDocument.bmp", "Open scratch buffer", 1 );
  addToolButton( "fileLoadDialog();", "LoadDocument.bmp", "Load file", 2 );
  addToolButton( "saveFile();", "SaveDocument.bmp", "Save file", 3 );
  addToolSpace();
  addToolButton( "cutBlockToClipboard();", "cut.bmp", "Cut selected text to clipboard", 4 );
  addToolButton( "copyBlockToClipboard();", "copy.bmp", "Copy selected text to clipboard", 5 );
  addToolButton( "pasteBlockFromClipboard();", "paste.bmp", "Paste text from clipboard", 6 );
  addToolSpace();
  addToolButton( "undo();", "undo.bmp", "Undo", 7 );
  addToolButton( "redo();", "redo.bmp", "Redo", 8 );
  addToolSpace();
  addToolButton( "searchDialog();", "FindText.bmp", "Search for text", 9 );
  addToolButton( "replaceDialog();", "ReplaceText.bmp", "Replace text string", 10 );
  addToolSpace();
  addToolButton( "indentBlock( getSetting( 'tabsize' ) );", "indent.bmp", "Indent selected text", 11 );
  addToolButton( "outdentBlock( getSetting( 'tabsize' ) );", "outdent.bmp", "Outdent selected text", 12 );
  addToolSpace();
  addToolButton( "nextView();", "viewToggle.bmp", "Switch view", 13 );
  addToolSpace();
  addToolButton( "savePos();", "SavePos.bmp", "Save position", 14 );
  addToolButton( "jumpToNextPos();", "NextPos.bmp", "Jump to next stored position", 15 );
  addToolButton( "jumpToPrevPos();", "PrevPos.bmp", "Jump to previous stored position", 16 );
  addToolButton( "clearSavedPositions();", "ClearPositions.bmp", "Remove all stored positions", 17 );
  addToolSpace();
  addToolButton( "recordMacro();", "Record.bmp", "Record a macro", 20 );
  addToolButton( "playMacro();", "Play.bmp", "Play macro", 21 );
}
