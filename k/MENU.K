// Do not alter these constants!

MAINMENUID = 101;
EDITMENUID = 300;
POPUPMENUID = 101;
WINDOWMENUID = 1000;

// Build the main menu
buildMainMenu()
{
  beginMainMenu();
  addMenu( MAINMENUID, "~File", 200 );
    addMenuItem( 200, "~Open file...", "fileLoadDialog();", 201 );
    addMenuItem( 200, "~Save file", "saveFile();", 202 );
    addMenuItem( 200, "Save file ~as...", "fileSaveDialog();", 203 );
    addMenuSeparator( 200 );
    addMenuItem( 200, "~New file", "loadFile( getScratchName(), TRUE, FALSE );", 204 );
    addMenuItem( 200, "New ~frame window", "createFrameWindow();", 205 );
    addMenuSeparator( 200 );
    addMenuItem( 200, "~Print...", "printDialog();", 206 );
    addMenuSeparator( 200 );
    addMenuItem( 200, "~Close buffer", "closeActiveWindow();", 207 );
    addMenuItem( 200, "Close ~window", "closeFrameWindow();", 208 );
    addMenuSeparator( 200 );

    for( i = 0; i < getRecentFilesCount() && i < 20; i++ )
    {
      if( i < 9 )
        shortcut = "~" @ (i + 1) @ ": ";
      else
      if( i == 9 )
        shortcut = "1~0: ";
      else
        shortcut = "   ";

      addMenuItem( 200, shortcut @ getRecentFile(i), 
                   "loadFile( \"" @ escapeChars(getRecentFile(i)) @ 
                   "\", TRUE, FALSE );", 209+i);
    }

  addMenu( MAINMENUID, "~Edit", 300 );
    addMenuItem( 300, "~Undo", "undo();", 301 );
    addMenuItem( 300, "~Redo", "redo();", 302 );
    addMenuSeparator( 300 );
    addMenuItem( 300, "Cu~t", "cutBlockToClipboard();", 303 );
    addMenuItem( 300, "~Copy", "copyBlockToClipboard();", 304 );
    addMenuItem( 300, "~Paste", "pasteBlockFromClipboard();", 305 );
    addMenuSeparator( 300 );
    addMenuItem( 300, "C~olumn selection", "toggleColumnBlock();", 306 );
    addMenuSeparator( 300 );
    addMenuItem( 300, "Convert ~tabs to spaces", "convertTabsToSpaces();", 307 );
    addMenuItem( 300, "Remove trailing ~blanks", "removeTrailingBlanks();", 308 );
    addMenuSeparator( 300 );
    addMenu( 300, "~Selection", 330 );
      addMenuItem( 330, "~Indent", "indentBlock( 1 );", 331 );
      addMenuItem( 330, "Outde~nt", "outdentBlock( 1 );", 332 );
      addMenuSeparator( 330 );
      addMenuItem( 330, "~Delete", "deleteBlock();", 333 );
      addMenuSeparator( 330 );
      addMenuItem( 330, "~Fill text", "fillColumnText();", 334 );
    addMenuSeparator( 300 );
    addMenu( 300, "M~acro", 340 );
      addMenuItem( 340, "~Start macro record", "startMacroRecord();", 341 );
      addMenuItem( 340, "~End macro record", "endMacroRecord();", 342 );
      addMenuItem( 340, "~Play macro", "playMacro();", 343 );
      addMenuItem( 340, "S~ave macro...", "macroSaveDialog();", 344 );
      addMenuSeparator( 340 );
      addMenuItem( 340, "~Copy macro to clipboard", "copyMacroToClipboard();", 345 );
      addMenuItem( 340, "E~xecute K code from clipboard", "executeKFromClipboard();", 346 );
      addMenuSeparator( 340 );
      addMenuItem( 340, "Copy macro to K-clipboard", "copyMacroToKClipboard();", 347 );
      addMenuItem( 340, "Execute K code from K-clipboard", "executeKFromKClipboard();", 348 );
      addMenuItem( 340, "Copy selected text to K-clipboard", "copyBlockToKClipboard();", 349 );
      addMenuItem( 340, "Paste from K-clipboard", "pasteBlockFromKClipboard();", 350 );

  addMenu( MAINMENUID, "~Search", 700 );
    addMenuItem( 700, "~Find (in files)...", "searchDialog();", 701 );
    addMenuItem( 700, "R~eplace (in files)...", "replaceDialog();", 702 );
    addMenuSeparator( 700 );
    addMenuItem( 700, "~Save position", "savePos();", 703 );
    addMenuItem( 700, "Jump to ~next position", "jumpToNextPos();", 704 );
    addMenuItem( 700, "Jump to ~previous position", "jumpToPrevPos();", 705 );
    addMenuItem( 700, "~Clear saved positions", "clearSavedPositions();resetViewWin( TRUE );", 706 );
    addMenuSeparator( 700 );
    addMenuItem( 700, "Pu~sh position", "pushPos( TRUE );", 707 );
    addMenuItem( 700, "P~op position", "popPos();", 708 );
    addMenuSeparator( 700 );
    addMenuItem( 700, "~Jump to line...", "jumpToLineDlg();", 709 );
    addMenuItem( 700, "Ju~mp to position...", "jumpToPosDlg();", 710 );

  addMenu( MAINMENUID, "~View", 500 );
    addMenuItem( 500, "~Hex view", "setView(HEXVIEW);", 501 );
    addMenuItem( 500, "~Ascii view", "setView(ASCIIVIEW);", 502 );
    addMenuSeparator( 500 );
    addMenu( 500, "~Syntax highlighting", 570 );
    for( i = 0; i < getSyntaxCount(); i++ )
      addMenuItem( 570, "~" @ getSyntaxNameAtPos(i), "if( queryView() == ASCIIVIEW ) setSyntax(\"" @ getSyntaxNameAtPos(i) @ "\");setSyntaxMenuCheck();", 571+i );
      addMenuSeparator( 570 );
      addMenuItem( 570, "~No syntax highlighting", "setSyntax( \"NONE\" );setSyntaxMenuCheck();", 571+getSyntaxCount() );
      addMenuSeparator( 570 );
      addMenuItem( 570, "~Edit kon.syn", "loadFile( getKonDir() @ \"\\\\kon.syn\", TRUE, TRUE );", 572+getSyntaxCount() );
    addMenuSeparator( 500 );
    addMenuItem( 500, "~Convert from UTF-8", "convertBufferFromUnicode(1);", 563 );
    addMenuItem( 500, "~Convert from UTF-16", "convertBufferFromUnicode(2);", 564 );
    addMenuSeparator( 500 );
    addMenuItem( 500, "~Reset", "setDelimitorLines( [80] );columnDescription = [];", 560 );
    addMenuItem( 500, "~Toggle left margin info", "toggleLeftMargin();", 561 );
    addMenuItem( 500, "~Toggle marks only visible", "toggleMarksOnly();", 562 );

  addMenu( MAINMENUID, "~Options", 400 );
    addMenuItem( 400, "~Preferences...", "prefsDialog();", 401 );
    addMenuItem( 400, "~Show messages", "togglemessageWindow();", 402 );

  // The window menu is built at a later time. Kon calls "buildWindowMenu"
  // for this task, but an empty menu with a proper ID must be placed in the 
  // main menu.

  addMenu( MAINMENUID, "~Window", WINDOWMENUID );
  addMenu( MAINMENUID, "~Help", 600 );
    addMenuItem( 600, "~Help...", "browse( \"file:///\" @ getKonDir() @ \"\\\\help\\\\index.html\" );", 601 );
    addMenuSeparator( 600 );
    addMenuItem( 600, "~Kon homepage...", "browse( \"http://www.koneditor.com\" );", 602 );
    addMenuSeparator( 600 );
    addMenuItem( 600, "~About...", "aboutDialog();", 603 );
  endMainMenu();
}

togglemessageWindow()
{
  messageWindowDisplay( !messageWindowVisible() );
  checkMenuItem( 402, messageWindowVisible() );
}

// Called whenever the checks or enables needs to be set on the menu
// This is called each time a new buffer is selected.
// Note that redoAvailable, undoAvailable and blockMarked are called
// separately from this function, so there's no need to call them from
// this function
setMenuChecks()
{
  checkMenuItem( 402, messageWindowVisible() );

  if( queryView() == ASCIIVIEW )
    checkMenuItem( 502, TRUE );
  else
    checkMenuItem( 501, TRUE );

  if( getSetting( "columnblock" ) )
    checkMenuItem( 306, TRUE );

  enableMenuItem( 704, hasSavedPositions() );
  enableMenuItem( 705, hasSavedPositions() );
  enableMenuItem( 706, hasSavedPositions() );

  setSyntaxMenuCheck();
}

setSyntaxMenuCheck()
{
  for( i = 0; i < getSyntaxCount(); i++ )
    checkMenuItem( 571 + i, getSyntaxName() == getSyntaxNameAtPos( i ) );
}

// This function is called whenever the window menu needs to be updated
// (A new window is opened, the user switches window)
buildWindowMenu()
{
  beginSubmenuUpdate( WINDOWMENUID, TRUE );
  addMenuItem( WINDOWMENUID, "~Next tab", "nextTab();", WINDOWMENUID+1 );
  addMenuItem( WINDOWMENUID, "~Previous tab", "prevTab();", WINDOWMENUID+2 );
  addMenuSeparator( WINDOWMENUID );
  for( i = 0; i < getWindowCount() && i < 20; i++ )
  {
    if( i < 9 )
      shortcut = "~" @ (i+1) @ ": ";
    else
    if( i == 9 )
      shortcut = "1~0: ";
    else
      shortcut = "   ";

    addMenuItem( WINDOWMENUID, shortcut @ getWindowFilename(i), 
                 "switchToControl( " @ getWindowId( i ) @ " );", 
                 WINDOWMENUID+3+i);
  }
  endSubmenuUpdate();
}

// Called when the user should bring up a popup-menu.
// position is the character position in the textfile, where the
// cursor was clicked.
// DO NOT MODIFY windowObject, position, x and y. They are for internal use.
buildPopupMenu( windowObject, position, x, y )
{
  beginPopupMenu();

  if( length( columnDescription ) > 0 )
  {
    addMenuItem( POPUPMENUID, "~Show column info", "showColumnInfo();", 204 );
    addMenuSeparator( POPUPMENUID );
  }

  // Check if the position is within a block

  if( position >= getBlockStart() && position < getBlockEnd() )
  {
    addMenuItem( POPUPMENUID, "Cu~t", "cutBlockToClipboard();", 201 );
    addMenuItem( POPUPMENUID, "~Copy", "copyBlockToClipboard();", 202 );
    addMenuSeparator( POPUPMENUID );
    addMenuItem( POPUPMENUID, "~Indent", "indentBlock(1);", 203 );
    addMenuItem( POPUPMENUID, "~Outdent", "outdentBlock(1);", 204 );
    addMenuSeparator( POPUPMENUID );
    addMenuItem( POPUPMENUID, "~Undo", "undo();", 205 );
    addMenuItem( POPUPMENUID, "~Redo", "redo();", 206 );
  }
  else
  {
    addMenuItem( POPUPMENUID, "~Paste", "pasteBlockFromClipboard();", 201 );
    addMenuSeparator( POPUPMENUID );
    addMenuItem( POPUPMENUID, "~Undo", "undo();", 202 );
    addMenuItem( POPUPMENUID, "~Redo", "redo();", 203 );
  }

  if( getFilename() == "*messages*" )
  {
    addMenuSeparator( POPUPMENUID );
    addMenuItem( POPUPMENUID, "Clear ~all", "resetFPRegExp();clearAll();", 210 );
  }

  endPopupMenu();
  popupMenu( windowObject, x, y );
}
