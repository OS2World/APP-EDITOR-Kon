#include "prefs.k"
#include "util.k"
#include "base.k"
#include "menu.k"
#include "toolbar.k"
#include "signals.k"
#include "wordwrap.k"
#include "fillcol.k"
#include "modules.k"

columnDescription = [];

startKon()
{
  bindKey( "Ctrl+q-Ctrl+f", "searchDialog();" );
  bindKey( "Ctrl+q-Ctrl+a", "replaceDialog();" );
  bindKey( "F3", "repeatFind();" );
  bindKey( "Ctrl+l", "repeatFind();" );
  bindKey( "Ctrl+k-Ctrl+s", "saveFile();" );
  bindKey( "F2", "saveFile();" );
  bindKey( "Ctrl+s", "saveFile();" );
  bindKey( "Ctrl+Insert", "copyBlockToClipboard();" );
  bindKey( "Ctrl+c", "copyBlockToClipboard();" );
  bindKey( "Shift+Insert", "pasteBlockFromClipboard();" );
  bindKey( "Ctrl+v", "pasteBlockFromClipboard();" );
  bindKey( "Shift+Delete", "cutBlockToClipboard();" );
  bindKey( "Ctrl+x", "cutBlockToClipboard();" );
  bindKey( "Up", "lineUp();" );
  bindKey( "Down", "lineDown();" );
  bindKey( "Left", "charLeft();" );
  bindKey( "Right", "charRight();" );
  bindKey( "PageDown", "pageDown();" );
  bindKey( "PageUp", "pageUp();" );
  bindKey( "Home", "cursorHome();" );
  bindKey( "End", "toLineEnd();" );
  bindKey( "Ctrl+Up", "screenLineDown();" );
  bindKey( "Ctrl+Down", "screenLineUp();" );
  bindKey( "Ctrl+Home", "toDocumentStart();" );
  bindKey( "Ctrl+End", "toDocumentEnd();" );
  bindKey( "Shift+Down", "markLineDown();" );
  bindKey( "Shift+Up", "markLineUp();" );
  bindKey( "Shift+Left", "markCharLeft();" );
  bindKey( "Shift+Right", "markCharRight();" );
  bindKey( "Shift+PageDown", "markPageDown();" );
  bindKey( "Shift+PageUp", "markPageUp();" );
  bindKey( "Shift+End", "markToLineEnd();" );
  bindKey( "Shift+Home", "markToLineStart();" );
  bindKey( "Ctrl+Shift+Home", "markToDocumentStart();" );
  bindKey( "Ctrl+Shift+End", "markToDocumentEnd();" );
  bindKey( "Insert", "updateSetting( 'insertmode', !getSetting( 'insertmode'));" );
  bindKey( "Delete", "deleteCharacter();" );
  bindKey( "Backspace", "deletePreviousCharacter();" );
  bindKey( "Shift+Backspace", "deletePreviousCharacter();" );
  bindKey( "Enter", "insertLine();" );
  bindKey( "Return", "insertLine();" );
  bindKey( "Alt+PageUp", "prevTab();" );
  bindKey( "Alt+PageDown", "nextTab();" );
  bindKey( "Ctrl+Shift+Tab", "prevTab();" );
  bindKey( "Ctrl+Tab", "nextTab();" );
  bindKey( "Alt+b", 'print( "bind.." );bindKey( "Ctrl-b", "print( fibI(16) );" );' );
  bindKey( "Ctrl+f", "searchDialog();" );
  bindKey( "Ctrl+N", "loadFile( getScratchName(), TRUE, FALSE );" );
  bindKey( "Space", "insertString( ' ' );" );
  bindKey( "Tab", "tabPressed();" );
  bindKey( "Shift+Tab", "outdentBlock( getSetting( 'tabsize' ) );" );
  bindKey( "Ctrl+z", "undo();" );
  bindKey( "Alt+Backspace", "undo();" );
  bindKey( "Ctrl+y", "redo();" );
  bindKey( "Alt+Shift+Backspace", "redo();" );
  bindKey( "Ctrl+k-Ctrl+n", "toggleColumnBlock();" );
  bindKey( "Alt+l", "fileLoadDialog();" );
  bindKey( "Ctrl+F4", "closeActiveWindow();" );
  bindKey( "Ctrl+Right", "nextWord();" );
  bindKey( "Ctrl+Left", "prevWord();" );
  bindKey( "Ctrl+Shift+Right", "markNextWord();" );
  bindKey( "Ctrl+Shift+Left", "markPrevWord();" );
  bindKey( "Ctrl+k-Ctrl+d", "saveFile();closeActiveWindow();" );
  bindKey( "Ctrl+k-Ctrl+q", "closeActiveWindow();" );
  bindKey( "Ctrl+Shift+r", "recordMacro();" );
  bindKey( "Ctrl+Shift+p", "playMacro();" );
  bindKey( "Ctrl+x", "startUndoBlock();insertString( '\n' );insertString( '\n' );insertString( '\n' );endUndoBlock();" );
  bindKey( "Ctrl+d", "deleteLine();" );
  bindKey( "Ctrl+q-Ctrl+y", "deleteToEOL();" );
  bindKey( "Ctrl+t", "deleteWord();" );
  bindKey( "Ctrl+h", "nextView();" );
  bindKey( "Ctrl+k-0", "saveNumberedPos( 0 );" );
  bindKey( "Ctrl+k-1", "saveNumberedPos( 1 );" );
  bindKey( "Ctrl+k-2", "saveNumberedPos( 2 );" );
  bindKey( "Ctrl+k-3", "saveNumberedPos( 3 );" );
  bindKey( "Ctrl+k-4", "saveNumberedPos( 4 );" );
  bindKey( "Ctrl+k-5", "saveNumberedPos( 5 );" );
  bindKey( "Ctrl+k-6", "saveNumberedPos( 6 );" );
  bindKey( "Ctrl+k-7", "saveNumberedPos( 7 );" );
  bindKey( "Ctrl+k-8", "saveNumberedPos( 8 );" );
  bindKey( "Ctrl+k-9", "saveNumberedPos( 9 );" );
  bindKey( "Ctrl+q-0", "jumpToNumberedSavedPos( 0 );" );
  bindKey( "Ctrl+q-1", "jumpToNumberedSavedPos( 1 );" );
  bindKey( "Ctrl+q-2", "jumpToNumberedSavedPos( 2 );" );
  bindKey( "Ctrl+q-3", "jumpToNumberedSavedPos( 3 );" );
  bindKey( "Ctrl+q-4", "jumpToNumberedSavedPos( 4 );" );
  bindKey( "Ctrl+q-5", "jumpToNumberedSavedPos( 5 );" );
  bindKey( "Ctrl+q-6", "jumpToNumberedSavedPos( 6 );" );
  bindKey( "Ctrl+q-7", "jumpToNumberedSavedPos( 7 );" );
  bindKey( "Ctrl+q-8", "jumpToNumberedSavedPos( 8 );" );
  bindKey( "Ctrl+q-9", "jumpToNumberedSavedPos( 9 );" );
  bindKey( "F4", "createFrameWindow();" );
  bindKey( "Ctrl+k-Ctrl+i", "indentBlock( 1 );" );
  bindKey( "Ctrl+k-Ctrl+u", "outdentBlock( 1 );" );
  bindKey( "Shift+Space", "insertString(' ');" );
  bindKey( "Alt+m", "toggleMessageWindow();" );
  bindKey( "Ctrl+k-Ctrl+p", "printDialog();" );
  bindKey( "Ctrl+r", "toggleRelativePos();" );
  bindKey( "Ctrl+g", "jumpToLineDlg();" );
  bindKey( "Ctrl+p", "jumpToPosDlg();" );
  bindKey( "Ctrl+PageDown", "pushPos( TRUE );" );
  bindKey( "Ctrl+PageUp", "popPos();" );
  bindKey( "Ctrl+m-Ctrl+e", "executeKFromClipboard();" );
  bindKey( "Ctrl+m-Ctrl+k", "executeKFromKClipboard();" );
  bindKey( "Ctrl+m-Ctrl+c", "copyMacroToKClipboard();" );
  bindKey( "Ctrl+m-Ctrl+p", "pasteBlockFromKClipboard();" );
  bindKey( "Alt+i", "aboutDialog();" );
  bindKey( "F9", "toggleLeftMargin();" );
  bindKey( "F8", "toggleMarksOnly();" );
  bindKey( "Ctrl+Space", "savePos();" );
  bindKey( "Alt+Up", "jumpToPrevPos();" );
  bindKey( "Alt+Down", "jumpToNextPos();" );
  bindKey( "Ctrl+i", "showColumnInfo();" );
  bindKey( "Ctrl+A", "markAll();" );
  bindKey( "Ctrl+W-Ctrl+1", "updateSetting( 'virtualSpace', TRUE );" );
  bindKey( "Ctrl+W-Ctrl+2", "updateSetting( 'virtualSpace', FALSE );" );
  bindKey( "Alt+C", "focusCommandBar();" );

  bindKey( "Ctrl+O-Ctrl+W", "updateWordwrap();" );
  bindKey( "Ctrl+B", "wrapSmart(getPosition(), FALSE);" );
  bindKey( "Ctrl+U", "wrapSmart(getPosition(), TRUE);" );

  buildMainMenu();

  loadModules();

  print( "*** startup script is finished" );
}
