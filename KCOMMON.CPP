#include "kcommon.h"
#include "cntrladm.h"
#include "mainwnd.h"
#include "multfind.h"
#include "multrep.h"
#include "multfile.h"
#include "syntax.h"
#include "kmodules.h"
#include <time.h>

KCommonCommands *KCommonCommands::instance = NULL;

void KCommonCommands::createInstance()
{
  instance = new KCommonCommands();

  TSyntaxPtr *argList = NULL;

  // Initiate K commands

	/*D Display About dialog */
	/*R Nothing */
  FunctionList.Update( "aboutDialog", 0, NULL, NULL, NULL, 137 );

  /*D Add a sub menu to a menu. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "atId", 0, 0 ); /* The menu id where the sub-menu 
                                                      should be placed under */
  argList = BuildArgList( argList, "string", 0, 1 ); /* Menu title */
  argList = BuildArgList( argList, "menuId", 0, 2 ); /* Id of menu */
  FunctionList.Update( "addMenu", 3, argList, NULL, NULL, 19 );

  /*D Add a menu item to a menu */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "atId", 0, 0 ); /* The id of them menu where the menu item should
                                                      be placed. */
  argList = BuildArgList( argList, "string", 0, 1 ); /* Menu title */
  argList = BuildArgList( argList, "command", 0, 2 ); /* A valid K-statement to execute
                                                         when this menu item is selected. */
  argList = BuildArgList( argList, "menuId", 0, 3 );  /* Id of menu item */
  FunctionList.Update( "addMenuItem", 4, argList, NULL, NULL, 20 );

  /*D Add a menu separator to a menu. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "atId", 0, 0 ); /* The id of them menu where the separator should
                                                      be placed. */
  FunctionList.Update( "addMenuSeparator", 1, argList, NULL, NULL, 21 );

  /*D Adds a tool to the toolbar */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "command", 0, 0 ); /* A valid K-statement */
  argList = BuildArgList( argList, "bitmap", 0, 1 ); /* Full path to the bitmap which should be used as the toolbar icon. */
  argList = BuildArgList( argList, "description", 0, 2 ); /* Tool description */
  argList = BuildArgList( argList, "id", 0, 3 ); /* Tool ID */
  FunctionList.Update( "addToolButton", 4, argList, NULL, NULL, 119 );

  /*D Adds a space delimitor to the toolbar */
  /*R Nothing */
  FunctionList.Update( "addToolSpace", 0, NULL, NULL, NULL, 120 );

	/*D Checks if there is a line end at the specified position. */
	/*R TRUE if there is a line end at the position */
	// e
	// e e e e "reader" /* Read handler */
	// e e e e "pos" /* Position to check */
	// FunctionList.Update( "atLineEnd", 2, )

  /*D Start building then main menu. Menu build must be ended with an endMainMenu.
     See menu.k for an example of how to build the main menu. */
	/*R Nothing */
  FunctionList.Update( "beginMainMenu", 0, NULL, NULL, NULL, 16 );

  /*D Start marking text. */
  /*R Nothing */
  FunctionList.Update( "beginMark", 0, NULL, NULL, NULL, 78 );

  /*D Start building a popup menu. See menu.k for an example of how to use this function. */
	/*R Nothing */
  FunctionList.Update( "beginPopupMenu", 0, NULL, NULL, NULL, 18 );

  /*D Start building or adding to a drop-down menu in main menu. 
      See menu.k for an example of how to use this function */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* Menu id for the sub menu */
  argList = BuildArgList( argList, "removeItems", 0, 1 ); /* Menu id for the sub menu */
  FunctionList.Update( "beginSubmenuUpdate", 2, argList, NULL, NULL, 17 );

  /*D Binds a key sequence to a K-command */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "keySeq", 0, 0 ); /* Describes the key sequence.<BR>
                                                        <BR>
                                                        Form a key sequence as:<BR>
                                                        Modifer1+Key1-Modifer2+Key2...ModiferN+KeyN<BR>
                                                        <BR>
                                                        Example: Ctrl+q-Ctrl+f<BR>
                                                        <BR>
                                                        Recognized key constants:<BR>
                                                        Ctrl, Alt, Shift, Left, Right, 
                                                        Up, Down, PageUp, PageDown,
                                                        Home, End, Insert, Delete, 
                                                        Escape, Enter, Backspace, 
                                                        Return, Tab, Space,
                                                        F1-F12, any alphanumeric character.*/
  argList = BuildArgList( argList, "command", 0, 1 ); /* Any valid K-statement */
  FunctionList.Update( "bindKey", 2, argList, NULL, NULL, 1 );

  /*D Callback function. Called when the active textbuffer has a block marked. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "isMarked", 0, 0 ); /* TRUE if block is marked */
  FunctionList.Update( "blockMarked", 1, argList, NULL, NULL, 32 );

  /*D Starts the web browser with an URL. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "URL", 0, 0 ); /* The URL */
  FunctionList.Update( "browse", 1, argList, NULL, NULL, 152 );

  /*D Move one character left */
  /*R TRUE if successful */
  FunctionList.Update( "charLeft", 0, NULL, NULL, NULL, 43 );

  /*D Move one character right */
  /*R TRUE if successful */
  FunctionList.Update( "charRight", 0, NULL, NULL, NULL, 42 );

  /*D Converts a character code to a string. Returns the string. */
  /*R String */
  argList = NULL;
  argList = BuildArgList( argList, "data", 0, 0 );  /* Integer representing a character code */
  FunctionList.Update( "charToString", 1, argList, NULL, NULL, 7 );

  /*D Set or remove a check mark next to a menu item.*/
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* Menu id where the mark should set or reset */
  argList = BuildArgList( argList, "checked", 0, 1 ); /* TRUE to set, FALSE to remove */
  FunctionList.Update( "checkMenuItem", 2, argList, NULL, NULL, 22 );

	/*D Removes all text from the whole document. */
	/*R Nothing */
	// e
	// FunctionList.Update( "clearAll", 0, )

  /*D Removes all saved positions. */
  /*R Nothing */
  FunctionList.Update( "clearSavedPositions", 0, NULL, NULL, NULL, 143 );

  /*D Close the active control (window). */
  /*R Nothing */
  FunctionList.Update( "closeActiveWindow", 0, NULL, NULL, NULL, 13 );

  /*D Close a control (window). */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* Id of control to be closed */
  FunctionList.Update( "closeControl", 1, argList, NULL, NULL, 157 );

  /*D Close the main window. Will close all controls within it. */
  /*R Nothing */
  FunctionList.Update( "closeFrameWindow", 0, NULL, NULL, NULL, 14 );

  /*D Converts the text in the current buffer from Unicode to Ansi representation.
      This conversion cannot be undone! */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "encoding", 0, 0 ); /* 1 (UTF8), 2 (UTF16) */
  FunctionList.Update( "convertBufferFromUnicode", 1, argList, NULL, NULL, 160 );

  /*D Copy selected text to the clipboard */
  /*R Nothing */
  FunctionList.Update( "copyBlockToClipboard", 0, NULL, NULL, NULL, 57 );

  /*D Copy the currently recorded macro to the clipboard */
  /*R Nothing */
  FunctionList.Update( "copyMacroToClipboard", 0, NULL, NULL, NULL, 124 );
  
  /*D Copy the currently recorded macro to the K-clipboard */
  /*R Nothing */
  FunctionList.Update( "copyMacroToKClipboard", 0, NULL, NULL, NULL, 175 );
  
  /*D Copy text to clipboard */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "text", 0, 0 ); /* Text to copy */
  FunctionList.Update( "copyTextToClipboard", 1, argList, NULL, NULL, 136 );

  /*D Copy selected text to the K-clipboard */
  /*R Nothing */
  FunctionList.Update( "copyBlockToKClipboard", 0, NULL, NULL, NULL, 176 );
  
  /*D Create a new main window */
  /*R Nothing */
  FunctionList.Update( "createFrameWindow", 0, NULL, NULL, NULL, 15 );

  /*D Cut selected text to the clipboard */
  /*R Nothing */
  FunctionList.Update( "cutBlockToClipboard", 0, NULL, NULL, NULL, 58 );

  /*D Debug memory. Internal use only */
  /*R Nothing */
  FunctionList.Update( "debug", 0, NULL, NULL, NULL, 11 );

	/*D Delete the selected block. */
	/*R Nothing */
	FunctionList.Update( "deleteBlock", 0, NULL, NULL, NULL, 148 );

  /*D Delete the current character */
  /*R Nothing */
  FunctionList.Update( "deleteCharacter", 0, NULL, NULL, NULL, 52 );

	/*D Deletes the line at cursor. */
	/*R Nothing */
	// e
	// FunctionList.Update( "deleteLine", 0, )

	/*D Delete the previous character from cursor position. */
	/*R Nothing */
	// e
	// FunctionList.Update( "deletePreviousCharacter", 0, )

  /*D Delete text */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 ); /* Start position */
  argList = BuildArgList( argList, "size", 0, 1 ); /* Length of text to delete */
  FunctionList.Update( "deleteText", 2, argList, NULL, NULL, 95 );

	/*D Deletes to end of line. */
	/*R Nothing */
	// e
	// FunctionList.Update( "deleteToEOL", 0, )

	/*D Deletes the current word. */
	/*R Nothing */
	// e
	// FunctionList.Update( "deleteWord", 0, )

  /*D Enable or disable a menu item */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* Menu id which should be enabled/disabled */
  argList = BuildArgList( argList, "enabled", 0, 1 ); /* TRUE to enable, FALSE to disable */
  FunctionList.Update( "enableMenuItem", 2, argList, NULL, NULL, 23 );

  /*D End recording the macro. */
  /*R Nothing */
  FunctionList.Update( "endMacroRecord", 0, NULL, NULL, NULL, 74 );

  /*D End building the main menu */
	/*R Nothing */
  FunctionList.Update( "endMainMenu", 0, NULL, NULL, NULL, 26 );

  /*D End marking text. */
  /*R Nothing */
  FunctionList.Update( "endMark", 0, NULL, NULL, NULL, 79 );

  /*D End building a popup menu */
	/*R Nothing */
  FunctionList.Update( "endPopupMenu", 0, NULL, NULL, NULL, 24 );

  /*D End updating a submenu */
	/*R Nothing */
  FunctionList.Update( "endSubmenuUpdate", 0, NULL, NULL, NULL, 25 );

  /*D Ends the current undo block. */
  /*R Nothing */
  FunctionList.Update( "endUndoBlock", 0, NULL, NULL, NULL, 77 );

  /*D Execute K-code residing in the clipboard */
  /*R Nothing */
  FunctionList.Update( "executeKFromClipboard", 0, NULL, NULL, NULL, 123 );

  /*D Execute K-code residing in the K-clipboard */
  /*R Nothing */
  FunctionList.Update( "executeKFromKClipboard", 0, NULL, NULL, NULL, 178 );

  /*D Check if a file exists in the file system*/
  /*R TRUE if the specified filename exists */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* Filename to test */
  FunctionList.Update( "fileExists", 1, argList, NULL, NULL, 129 );

	/*D Brings up a file dialog and loads the selected file. */
	/*R Nothing */
	// e
	// FunctionList.Update( "fileLoadDialog", 0, )

	/*D Brings up a file dialog and saves the selected file. */
	/*R Nothing */
	// e
	// FunctionList.Update( "fileSaveDialog", 0, )

	/*D Fills a selected column block with a string. */
	/*R Nothing */
	// e
	// e e e e "str" /* String to fill columnblock */
	// e e e e "repeat" /* TRUE if the string should be repeated on each line to fill up between column end points */
	// FunctionList.Update( "fillColumnBlock", 0, )

  /*D Find a string in the text */
  /*R TRUE if string was found */
  argList = NULL;
  argList = BuildArgList( argList, "string", 0, 0 ); /* String to search for */
  argList = BuildArgList( argList, "options", 0, 1 ); /* Options string:<BR><BR>
                                                         'd': Global search. (Not set: Selected text) <BR>
                                                         'b': Backward search. (Not set: Forward search) <BR>
                                                         's': Search selected text. (Not set: Search whole file) <BR>
                                                         'u': Case insensitive search. <BR>
                                                         'w': Find whole words only. <BR>
                                                         'g': Use regular expressions.*/
  FunctionList.Update( "findString", 2, argList, NULL, NULL, 103 );

  /*D Find a string in multiple files and display the results in the message window. */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "searchString", 0, 0 ); /* String to search for */
  argList = BuildArgList( argList, "options", 0, 1 ); /* Options string:<BR><BR>
                                                         'd': Global search. (Not set: Selected text) <BR>
                                                         'b': Backward search. (Not set: Forward search) <BR>
                                                         's': Search selected text. (Not set: Search whole file) <BR>
                                                         'u': Case insensitive search. <BR>
                                                         'w': Find whole words only. <BR>
                                                         'g': Use regular expressions. <BR>
                                                         'r': Search sub directories.*/
  argList = BuildArgList( argList, "path", 0, 2 ); /* Path where to start searching */
  argList = BuildArgList( argList, "files", 0, 3 ); /* Files to search, separated by ';'. Wildcards allowed. */
  FunctionList.Update( "findStringFiles", 4, argList, NULL, NULL, 104 );

  /*D A specialized function for finding string using regular expressions. The difference between this function
      and 'findString' is that this will also return an array consisting of the substrings specified using the
      grouping function in the regular expression. */
  /*R TRUE if string was found */
  argList = NULL;
  argList = BuildArgList( argList, "string", 0, 0 ); /* String to search for */
  argList = BuildArgList( argList, "options", 0, 1 ); /* Options string:<BR><BR>
                                                         'd': Global search. (Not set: Selected text) <BR>
                                                         'b': Backward search. (Not set: Forward search) <BR>
                                                         's': Search selected text. (Not set: Search whole file) <BR>
                                                         'u': Case insensitive search. <BR>
                                                         'w': Find whole words only.*/
  argList = BuildArgList( argList, "substrings", 1, 2 ); /* An array of found substrings.*/
  FunctionList.Update( "findStringGrep", 3, argList, NULL, NULL, 170 );

  /*D Focus the command bar int eh active main window. */
  /*R Nothing */
  FunctionList.Update( "focusCommandBar", 0, NULL, NULL, NULL, 163 );

  /*D Returns the marked block end. */
  /*R Block end */
  FunctionList.Update( "getBlockEnd", 0, NULL, NULL, NULL, 85 );

  /*D Returns the marked block start. */
  /*R Block start */
  FunctionList.Update( "getBlockStart", 0, NULL, NULL, NULL, 84 );

  /*D Return a character */
  /*R Character code */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 );  /* Position in the text */
  FunctionList.Update( "getChar", 1, argList, NULL, NULL, 102 );

  /*D Returns the ID for the currently executing control. */
  /*R Column */
  FunctionList.Update( "getCurrentControlId", 0, NULL, NULL, NULL, 6 );

  /*D Returns the current column for the cursor. */
  /*R Column */
  FunctionList.Update( "getCursorColumn", 0, NULL, NULL, NULL, 83 );

 /*D Returns the encoding for the active buffer. */
  /*R 0 (ANSI), 1 (UTF8), 2 (UTF16) */
  FunctionList.Update( "getEncoding", 0, NULL, NULL, NULL, 159 );

  /*D Returns the id for a buffer with a named file */
  /*R Id of control */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 );  /* Position in the text */
  FunctionList.Update( "getFileId", 1, argList, NULL, NULL, 169 );

  /*D Returns the filename for the current file */
  /*R Filename */
  FunctionList.Update( "getFilename", 0, NULL, NULL, NULL, 122 );

  /*D Returns the current line in the text. */
  /*R Current line */
  FunctionList.Update( "getLine", 0, NULL, NULL, NULL, 69 );

  /*D Returns the Directory where the Kon program file resides. */
  /*R Full path where the exe file resides (excluding the exe file itself)*/
  FunctionList.Update( "getKonDir", 0, NULL, NULL, NULL, 153 );

  /*D Returns the line start for the current line. */
  /*R Character position for linestart */
  FunctionList.Update( "getLineStart", 0, NULL, NULL, NULL, 70 );

  /*D Returns the line end for the current line. <BR>
	    <I>Note that there must be no writers on the buffer when calling this function
			   or the call will block.</I> */
  /*R Character position for linestart */
  FunctionList.Update( "getLineEnd", 0, NULL, NULL, NULL, 150 );

  /*D Returns the column where the marked block starts. Only valid in ASCII-view. */
  /*R A column */
  FunctionList.Update( "getMarkBeginColumn", 0, NULL, NULL, NULL, 166 );

  /*D Returns the column where the marked block ends. Only valid in ASCII-view. */
  /*R A column */
  FunctionList.Update( "getMarkEndColumn", 0, NULL, NULL, NULL, 167 );

  /*D Returns the cursor position in the text. */
  /*R Character position */
  FunctionList.Update( "getPosition", 0, NULL, NULL, NULL, 68 );

  /*D Returns the a scratch buffer name. No buffer is opened, this
	    has to be done using loadFile. */
  /*R filename */
  FunctionList.Update( "getScratchName", 0, NULL, NULL, NULL, 139 );

  /*D Returns the marked block end. */
  /*R Block end */
  FunctionList.Update( "getScreenUpdate", 0, NULL, NULL, NULL, 164 );

  /*D Returns the recently opened file at a certain position in the list. */
  /*R Filename */
  argList = NULL;
  argList = BuildArgList( argList, "pos", 0, 0 ); /* Position in the list */
  FunctionList.Update( "getRecentFile", 1, argList, NULL, NULL, 115 );

  /*D Returns the number of files in the recently opened files list */
  /*R Number of files */
  FunctionList.Update( "getRecentFilesCount", 0, NULL, NULL, NULL, 114 );

  /*D Query setting */
  /*R Returns the setting */
  argList = NULL;
  argList = BuildArgList( argList, "setting", 0, 0 ); /* One of the following strings:<BR><BR>
                                                         'autosave': TRUE if auto save should be done.<BR>
                                                         'backupfiles': TRUE if .BAK file should be created on file save.<BR>
                                                         'columnblock': Columnblock marking (TRUE/FALSE).<BR>
                                                         'crlf': TRUE if CR+LF line end sequence should be used, FALSE if LF only.<BR>
                                                         'hexcursorbg': Colorvalue(RGB) of hex cursor background.<BR>
                                                         'hexcursorfg': Colorvalue(RGB) of hex cursor.<BR>
                                                         'hilitebg': Colorvalue(RGB) of highlight background.<BR>
                                                         'hilitefg': Colorvalue(RGB) of highligt foreground.<BR>
                                                         'insertmode': TRUE if insert mode, FALSE if overwrite mode.<BR>
                                                         'leftMarginbg': Colorvalue(RGB) of left margin background.<BR>
                                                         'leftMarginfg': Colorvalue(RGB) of left margin.<BR>
                                                         'markerbg': Colorvalue(RGB) of marked position background.<BR>
                                                         'markerfg': Colorvalue(RGB) of marked position.<BR>
                                                         'markbg': Colorvalue(RGB) of mark background.<BR>
                                                         'markfg': Colorvalue(RGB) of mark foreground.<BR>
                                                         'maxUndoSize': Max number of undo steps.<BR>
                                                         'optimalfill': TRUE to fill long space runs whith tab character (0x9) when possible..<BR>
                                                         'persistentblocks': TRUE if persistent blocks should be used.<BR>
                                                         'reuseExistingBuffer : TRUE if an existing buffer and control should be used instead of reloading when a file is opened.<BR>
                                                         'rightmargin': The right margin.<BR>
                                                         'showLeftMarginInfo' : TRUE to show the left margin info.<BR>
                                                         'showLineNumber' : TRUE to show line numbers in the left margin.<BR>
                                                         'showSavedPos' : TRUE to show saved position in the left margin. 
                                                         'standarddelim': Standard delimitor string. (When no syntax highlighting is defined.)<BR>
                                                         'showrightmargin': TRUE if the right margin should be shown in the text window.<BR>
                                                         'stripnameinnotebook': TRUE if only filename without path should be displayed in notebook tabs.<BR>
                                                         'syntaxhighlight': TRUE if syntax highlightinh should be used.<BR>
                                                         'tabsize': Set the tab size. <BR>
                                                         'usetab': TRUE to insert real tab character (0x9), FALSE to insert spaces.*/
  FunctionList.Update( "getSetting", 1, argList, NULL, NULL, 37 );

  /*D Returns the Number of syntax schemes available */
  /*R Nothing */
  FunctionList.Update( "getSyntaxCount", 0, NULL, NULL, NULL, 154 );

  /*D Returns the name of the current syntax highlighting scheme */
  /*R Name of scheme */
  FunctionList.Update( "getSyntaxName", 0, NULL, NULL, NULL, 127 );

  /*D Returns the name of the syntax scheme at a position */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "pos", 0, 0 ); /* Position of syntax scheme (0 min, getSyntaxCount - 1 max */
  FunctionList.Update( "getSyntaxNameAtPos", 1, argList, NULL, NULL, 156 );

  /*D Get text */
  /*R Text string */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 ); /* Position where to retrieve text */
  argList = BuildArgList( argList, "size", 0, 1 ); /* Length of text */
  FunctionList.Update( "getText", 2, argList, NULL, NULL, 134 );

  /*D Returns the length of the file */
  /*R Current file length */
  FunctionList.Update( "getTextLength", 0, NULL, NULL, NULL, 112 );

  /*D Returns the current time. */
  /*R Formatted time string */
  argList = NULL;
  argList = BuildArgList( argList, "formatstring", 0, 0 ); /* Format string:<BR><BR> %a
        %A: Abbreviated weekday name<BR>
        %b: Full weekday name<BR>
        %B: Abbreviated month name<BR>
        %c: Full month name<BR>
        %d: Date and time representation appropriate for locale<BR>
        %H: Day of month as decimal number (01 - 31)<BR>
        %I: Hour in 24-hour format (00 - 23)<BR>
        %j: Hour in 12-hour format (01 - 12)<BR>
        %m: Day of year as decimal number (001 - 366)<BR>
        %M: Month as decimal number (01 - 12)<BR>
        %p: Minute as decimal number (00 - 59)<BR>
        %S: Current locale’s A.M./P.M. indicator for 12-hour clock<BR>
        %U: Second as decimal number (00 - 59)<BR>
        %w: Week of year as decimal number, with Sunday as first day of week (00 - 53)<BR>
        %W: Weekday as decimal number (0 – 6; Sunday is 0)<BR>
        %x: Week of year as decimal number, with Monday as first day of week (00 - 53)<BR>
        %X: Date representation for current locale<BR>
        %y: Time representation for current locale<BR>
        %Y: Year without century, as decimal number (00 - 99)<BR>
        %z, %Z: Year with century, as decimal number<BR>
        %%: Time-zone name or abbreviation; no characters if time zone is unknown<BR>
        %%: Percent sign<BR>
*/
  FunctionList.Update( "getTime", 1, argList, NULL, NULL, 128 );

  /*D Returns the number of opened windows (Controls) */
  /*R Number of controls */
  FunctionList.Update( "getWindowCount", 0, NULL, NULL, NULL, 116 );

  /*D Returns the filename for the specified position in the window list */
  /*R Filename */
  argList = NULL;
  argList = BuildArgList( argList, "pos", 0, 0 ); /* Position in the list */
  FunctionList.Update( "getWindowFilename", 1, argList, NULL, NULL, 117 );

  /*D Returns the ID for a specific window */
  /*R Window ID */
  argList = NULL;
  argList = BuildArgList( argList, "pos", 0, 0 ); /* Position in the window list */
  FunctionList.Update( "getWindowId", 1, argList, NULL, NULL, 118 );

  /*D Check if there exists saved positions. */
  /*R TRUE if at least one positions is saved */
  FunctionList.Update( "hasSavedPositions", 0, NULL, NULL, NULL, 165 );

  /*D Check if a block is marked. */
  /*R TRUE if a block is marked */
  FunctionList.Update( "hasValidBlockMark", 0, NULL, NULL, NULL, 86 );

  /*D Sets a highlight on the current line. */
  /*R Nothing */
  FunctionList.Update( "highlightCurrentLine", 0, NULL, NULL, NULL, 149 );

  /*D Indent a line */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 ); /* Position in text */
  argList = BuildArgList( argList, "fromColumn", 0, 1 ); /* Column from where text should be indented */
  argList = BuildArgList( argList, "toColumn", 0, 2 ); /* Column where text should be indented to */
  FunctionList.Update( "indent", 3, argList, NULL, NULL, 96 );

	/*D Indents a selected block. */
	/*R Nothing */
	// e
	// e e e e "steps" /* Spaces to indent */
	// FunctionList.Update( "indentBlock", 1, )

	/*D Inserts a line break. */
	/*R Nothing */
	// e
	// FunctionList.Update( "insertLine", 0, )

  /*D Insert a text string in the text.*/
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "string", 0, 0 ); /* String to insert */
  FunctionList.Update( "insertString", 1, argList, NULL, NULL, 108 );

  /*D Debug. Internal use only */
	/*R Nothing */
  FunctionList.Update( "integrityCheck", 0, NULL, NULL, NULL, 12 );

  /*D Check if the supplied value is an integer */
  /*R TRUE if the supplied value is an integer */
  argList = NULL;
  argList = BuildArgList( argList, "arg", 0, 0 ); /* Value to test */
  FunctionList.Update( "isInteger", 1, argList, NULL, NULL, 125 );

  /*D Check if a macro is recording. */
  /*R TRUE if a macro is recording. */
  FunctionList.Update( "isRecording", 0, NULL, NULL, NULL, 75 );

  /*D Check if the supplied value is a string */
  /*R TRUE if the supplied value is a string */
  argList = NULL;
  argList = BuildArgList( argList, "arg", 0, 0 ); /* Value to test */
  FunctionList.Update( "isString", 1, argList, NULL, NULL, 126 );

  /*D Jump to a line in the text */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "line", 0, 0 ); /* Line to jump to */
  FunctionList.Update( "jumpToLine", 1, argList, NULL, NULL, 99 );

	/*D Jumps to the line specified in the dialog. */
	/*R Nothing */
	// e
	// FunctionList.Update( "jumpToLineDialog", 0, )

  /*D Jump to next saved position */
  /*R Nothing */
  FunctionList.Update( "jumpToNextPos", 0, NULL, NULL, NULL, 141 );

  /*D Restores the current position from a slot. */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "markNum", 0, 0 ); /* Slot 0-9 */
  FunctionList.Update( "jumpToNumberedSavedPos", 1, argList, NULL, NULL, 92 );

	/*D Jumps to the position specified in the dialog. */
	/*R Nothing */
	// e
	// FunctionList.Update( "jumpToPosDialog", 0, )

  /*D Jump to an absolute character position in the text */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 ); /* Character position */
  FunctionList.Update( "jumpToPosition", 1, argList, NULL, NULL, 98 );

  /*D Jump to previous saved position */
  /*R Nothing */
  FunctionList.Update( "jumpToPrevPos", 0, NULL, NULL, NULL, 142 );

  /*D Sets if blocks should be temporary persistent. */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "keep", 0, 0 ); /* TRUE for temporary persistent blocks */
  FunctionList.Update( "keepBlock", 1, argList, NULL, NULL, 89 );

  /*D Launches a process and optionally captures the output and sends it to a control */
  /*R String */
  argList = NULL;
  argList = BuildArgList( argList, "exefile", 0, 0 ); /* EXE file name including parameters.*/
  argList = BuildArgList( argList, "arguments", 0, 1 ); /* Program arguments which may include:<BR>
                                                              %f: Name of current file */
  argList = BuildArgList( argList, "capture", 0, 2 ); /* TRUE to capture output */
  argList = BuildArgList( argList, "controlId", 0, 3 ); /* If capture, id of control to send output to */
  FunctionList.Update( "launchProcess", 4, argList, NULL, NULL, 168 );

	/*D Launches a process, captures the output to the messages window and optionally supplies a 
	    regular expression which is used for separating out filename, line and column info when
	    a line is double clicked. */
	/*R Nothing */
	// e
	// e e e e "exefile" /* EXE file name including parameters.*/
	// e e e e "arguments" /* Program arguments which may include:<BR>%f: Name of current file */
	// e e e e "positionRegExp" /* A regular expression describing the layout of the line, column and filename for a clickable row in the output. For identifying the different fields, group expressions. Example:<BR><BR>The layout of a message row is<BR><BR>FILENAME (LINE, COLUMN) : Message<BR><BR>The regular expression would be:<BR><BR>(.)+ \\((:n)+, (:n)+\\) \:*/
	// e e e e "fileAndPosGroup" /* A list of which groups should be used for file and position information. The list is defined as [# filename group, # line group, # column group]<BR>For the example above the list would be:<BR><BR>[0,1,2]<BR>*/
	// FunctionList.Update( "launchProcessToMessages", 4, )

  /*D Returns the left portion of a string */
  /*R String */
  argList = NULL;
  argList = BuildArgList( argList, "string", 0, 0 ); /* String to copy from */
  argList = BuildArgList( argList, "characters", 0, 1 ); /* Characters to copy */
  FunctionList.Update( "leftString", 2, argList, NULL, NULL, 144 );

  /*D Returns the length of an array or string */
  /*R Length of string */
  argList = NULL;
  argList = BuildArgList( argList, "array", 0, 0 ); /* Array or string */
  FunctionList.Update( "length", 1, argList, NULL, NULL, 3 );

  /*D Move one line down */
  /*R TRUE if successful */
  FunctionList.Update( "lineDown", 0, NULL, NULL, NULL, 40 );

  /*D Move one line up */
  /*R TRUE if successful */
  FunctionList.Update( "lineUp", 0, NULL, NULL, NULL, 41 );

  /*D Display the file requester. */
  /*R If a filename is selected, the filename is returned, otherwise an empty 
      string is returned */
  argList = NULL;
  argList = BuildArgList( argList, "title", 0, 0 ); /* Title of dialog, e.g. "Open", "Save." */
  argList = BuildArgList( argList, "filename", 0, 1 ); /* Initial file name */
  FunctionList.Update( "loadDialog", 2, argList, NULL, NULL, 158 );

  /*D Start loading a file */
  /*R ID of the opened buffer */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* Filename of file to load */
  argList = BuildArgList( argList, "backgroundLoad", 0, 1 ); /* TRUE if the file should be loaded asynchronously */
  argList = BuildArgList( argList, "useExisting", 0, 2 ); /* TRUE if we should focus an already exisisting control if the file is already loaded  */
  FunctionList.Update( "loadFile", 3, argList, NULL, NULL, 10 );

  /*D Load a K-module */
  /*R TRUE if successfull */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* Filename of K-file to load */
  argList = BuildArgList( argList, "preload", 0, 1 ); /* 0 = Do not preload this module, 1 = preload, 2 = reserved value */
  FunctionList.Update( "loadModule", 2, argList, NULL, NULL, 161 );

	/*D Brings up a file dialog and saves the currently recorded macro. 
	    The macro is saved as a K function. */
	/*R Nothing */
	// e
	// FunctionList.Update( "macroSaveDialog", 0, )

	/*D Marks all text in the whole document. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markAll", 0, )

	/*D Marks one char left. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markCharLeft", 0, )

	/*D Marks one char right. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markCharRight", 0, )

	/*D Marks the next word. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markNextWord", 0, )

	/*D Marks the current line. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markLine", 0, )

	/*D Marks one line down. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markLineDown", 0, )

	/*D Marks one line up. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markLineUp", 0, )

	/*D Marks one page down. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markPageDown", 0, )

	/*D Marks one page up. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markPageUp", 0, )

	/*D Marks the previous word. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markPrevWord", 0, )

	/*D Marks to document end. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markToDocumentEnd", 0, )

	/*D Marks to document start. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markToDocumentStart", 0, )

	/*D Marks to line end. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markToLineEnd", 0, )

	/*D Marks to line start. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markToLineStart", 0, )

	/*D Marks the current word. */
	/*R Nothing */
	// e
	// FunctionList.Update( "markWord", 0, )

  /*D Display a message box.*/
  /*R MBRESOK (5) <BR>
     MBRESCANCEL (6) <BR>
     MBRESYES (7) <BR>
     MBRESNO (8) <BR>
  */
  argList = NULL;
  argList = BuildArgList( argList, "title", 0, 0 ); /* Title of message box */
  argList = BuildArgList( argList, "message", 0, 1 ); /* Message */
  argList = BuildArgList( argList, "type", 1, 2 ); /* MBERROR (1), MBWARNING (2), MBMESSAGE (3), MBQUERY (4) */
  FunctionList.Update( "messageBox", 3, argList, NULL, NULL, 130 );

  /*D Displays or hides message window */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "show", 0, 0 ); /* TRUE to show */
  FunctionList.Update( "messageWindowDisplay", 1, argList, NULL, NULL, 34 );

  /*D Check if the message window is visible. */
	/*R Returns TRUE if the message window is visible */
  FunctionList.Update( "messageWindowVisible", 0, NULL, NULL, NULL, 35 );

  /*D Returns a portion of a string */
  /*R String */
  argList = NULL;
  argList = BuildArgList( argList, "string", 0, 0 ); /* String to copy from */
  argList = BuildArgList( argList, "start", 0, 1 ); /* Starting position */
  argList = BuildArgList( argList, "characters", 0, 2 ); /* Characters to copy */
  FunctionList.Update( "midString", 3, argList, NULL, NULL, 146 );

  /*D Move to next control in the opened files notebook */
  /*R Nothing */
  FunctionList.Update( "nextTab", 0, NULL, NULL, NULL, 50 );

  /*D Switch to the next view. (currently ASCIIVIEW (0) or HEXVIEW (1)). */
  /*R Nothing */
  FunctionList.Update( "nextView", 0, NULL, NULL, NULL, 80 );

	/*D Moves to the next word. */
	/*R Nothing */
	// e
	// FunctionList.Update( "nextWord", 0, )

	/*D Outdents a selected block. */
	/*R Nothing */
	// e
	// e e e e "steps" /* Spaces to outdent */
	// FunctionList.Update( "outdentBlock", 1, )

  /*D Move one page down */
  /*R TRUE if successful */
  FunctionList.Update( "pageDown", 0, NULL, NULL, NULL, 44 );

  /*D Move one page up */
  /*R TRUE if successful */
  FunctionList.Update( "pageUp", 0, NULL, NULL, NULL, 45 );

  /*D Paste text from clipboard at the current position */
  /*R TRUE if data was available */
  FunctionList.Update( "pasteBlockFromClipboard", 0, NULL, NULL, NULL, 59 );

  /*D Paste text from K-clipboard at the current position */
  /*R TRUE if data was available */
  FunctionList.Update( "pasteBlockFromKClipboard", 0, NULL, NULL, NULL, 177 );

  /*D Display a popup menu */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "windowObject", 0, 0 ); /* Passed from callback function buildPopupMenu */
  argList = BuildArgList( argList, "x", 0, 1 ); /* Passed from callback function buildPopupMenu */
  argList = BuildArgList( argList, "y", 0, 2 ); /* Passed from callback function buildPopupMenu */
  FunctionList.Update( "popupMenu", 3, argList, NULL, NULL, 28 );

  /*D Pop a position from the position stack. */
  /*R TRUE if successful */
  FunctionList.Update( "popPos", 0, NULL, NULL, NULL, 56 );

  /*D Popups a popup window with a text. Is automatically removed when the cursor is moved a certain distance. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "text", 0, 0 ); /* Text to display */
  FunctionList.Update( "popupWindow", 1, argList, NULL, NULL, 147 );

  /*D Display the preferences dialog */
  /*R Nothing */
  FunctionList.Update( "prefsDialog", 0, NULL, NULL, NULL, 39 );

  /*D Move to previous control in the opened files notebook */
  /*R Nothing */
  FunctionList.Update( "prevTab", 0, NULL, NULL, NULL, 51 );

	/*D Moves to the previous word. */
	/*R Nothing */
	// e
	// FunctionList.Update( "prevWord", 0, )

  /*D Prints a text string to the message window */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "data", 0, 0 ); /* String or value to display */
  FunctionList.Update( "print", 1, argList, NULL, NULL, 2 );

  /*D Displays the print dialog */
  /*R Nothing */
  FunctionList.Update( "printDialog", 0, NULL, NULL, NULL, 113 );

  /*D Prints the current file. */
  /*R Nothing */
  FunctionList.Update( "printFile", 0, NULL, NULL, NULL, 87 );

  /*D Push the current position onto the position stack. */
  /*R TRUE if successful */
  argList = NULL;
  argList = BuildArgList( argList, "visible", 0, 0 ); /* TRUE to show this mark in the left margin */
  FunctionList.Update( "pushPos", 1, argList, NULL, NULL, 55 );

  /*D Display a dialog with an entry field. */
  /*R Returns TRUE if the dialog was confirmed. */
  argList = NULL;
  argList = BuildArgList( argList, "title", 0, 0 ); /* Title of the dialog */
  argList = BuildArgList( argList, "query", 0, 1 ); /* Text of dialog */
  argList = BuildArgList( argList, "data", 1, 2 ); /* (By reference) Initial data and returned data */
  FunctionList.Update( "queryLineDialog", 3, argList, NULL, NULL, 36 );

  /* Returns the current view.. */
  /*R ASCIIVIEW (0) or HEXVIEW (1) */
  FunctionList.Update( "queryView", 0, NULL, NULL, NULL, 81 );

  /*D Close the current read handle. */
  /*R Nothing */
  FunctionList.Update( "readRelease", 0, NULL, NULL, NULL, 72 );

  /*D Returns a handle for read requests. Must be closed using readRelease after use! */
  /*R Reader handle */
  FunctionList.Update( "readRequest", 0, NULL, NULL, NULL, 71 );

  /*D Redo one step */
  /*R TRUE if successful */
  FunctionList.Update( "redo", 0, NULL, NULL, NULL, 54 );

  /*D Callback function. Called when the active textbuffer has redo available. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "hasRedo", 0, 0 ); /* TRUE if redo is available */
  FunctionList.Update( "redoAvailable", 1, argList, NULL, NULL, 31 );

  /*D Regenerate sections. */
  /*R Nothing */
  FunctionList.Update( "regenerateSections", 0, NULL, NULL, NULL, 174 );

  /*D Repeat last search or replace operation. */
  /*R TRUE if string was found */
  FunctionList.Update( "repeatFind", 0, NULL, NULL, NULL, 107 );

  /*D Replace a string */
  /*R TRUE if string was found */
  argList = NULL;
  argList = BuildArgList( argList, "searchString", 0, 0 ); /* String to search for */
  argList = BuildArgList( argList, "replaceString", 0, 1 ); /* String to replace with */
  argList = BuildArgList( argList, "options", 0, 2 ); /* Options string:<BR><BR>
                                                         'd': Global search. (Not set: Selected text) <BR>
                                                         'b': Backward search. (Not set: Forward search) <BR>
                                                         's': Search selected text. (Not set: Search whole file) <BR>
                                                         'u': Case insensitive search. <BR>
                                                         'w': Find whole words only. <BR>
                                                         'g': Use regular expressions. <BR>
                                                         'n': Prompt before replace.<BR>
                                                         'a': Replace all occurences. (Not set: Replace only the first found.)*/
  FunctionList.Update( "replaceString", 3, argList, NULL, NULL, 105 );

  /*D Replace strings in multiple files */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "searchString", 0, 0 );  /* String to search for */
  argList = BuildArgList( argList, "replaceString", 0, 1 ); /* String to replace with */
  argList = BuildArgList( argList, "options", 0, 2 ); /* Options string:<BR><BR>
                                                         'd': Global search. (Not set: Selected text) <BR>
                                                         'b': Backward search. (Not set: Forward search) <BR>
                                                         's': Search selected text. (Not set: Search whole file) <BR>
                                                         'u': Case insensitive search. <BR>
                                                         'w': Find whole words only. <BR>
                                                         'g': Use regular expressions. <BR>
                                                         'n': Prompt before replace.<BR>
                                                         'a': Replace all occurences. (Not set: Replace only the first found.)<BR>
                                                         'r': Search sub directories.*/
  argList = BuildArgList( argList, "path", 0, 3 ); /* Path where to start searching */
  argList = BuildArgList( argList, "files", 0, 4 ); /* Files to search, separated by ';'. Wildcards allowed. */
  FunctionList.Update( "replaceStringFiles", 5, argList, NULL, NULL, 106 );

  /*D Reset block markers. */
	/*R Nothing */
  FunctionList.Update( "resetMark", 0, NULL, NULL, NULL, 33 );

  /*D Display the search and replace dialog. The dialog will issue the replace command. */
  /*R Nothing */
  FunctionList.Update( "replaceDialog", 0, NULL, NULL, NULL, 62 );

  /*D Used to reflect changes in configuration on screen */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "keepWindowStart", 0, 0 ); /* TRUE if window start should not be reset */
  FunctionList.Update( "resetViewWin", 1, argList, NULL, NULL, 60 );

  /*D Return a character using a reader handle */
  /*R Character code */
  argList = NULL;
  argList = BuildArgList( argList, "reader", 0, 0 ); /* Reader handle */
  argList = BuildArgList( argList, "position", 0, 1 ); /* Position in the text */
  FunctionList.Update( "rGetChar", 2, argList, NULL, NULL, 101 );

  /*D Get text using reader handle */
  /*R Text string */
  argList = NULL;
  argList = BuildArgList( argList, "reader", 0, 0 ); /* Reader handle */
  argList = BuildArgList( argList, "position", 0, 1 ); /* Position where to retrieve text */
  argList = BuildArgList( argList, "size", 0, 2 ); /* Length of text */
  FunctionList.Update( "rGetText", 3, argList, NULL, NULL, 135 );

  /*D Returns the right portion of a string */
  /*R String */
  argList = NULL;
  argList = BuildArgList( argList, "string", 0, 0 ); /* String to copy from */
  argList = BuildArgList( argList, "characters", 0, 1 ); /* Characters to copy */
  FunctionList.Update( "rightString", 2, argList, NULL, NULL, 145 );

  /*D Display the file requester. */
  /*R If a filename is selected, the filename is returned, otherwise an empty 
      string is returned */
  argList = NULL;
  argList = BuildArgList( argList, "title", 0, 0 ); /* Title of dialog, e.g. "Open", "Save." */
  argList = BuildArgList( argList, "filename", 0, 1 ); /* Initial file name */
  argList = BuildArgList( argList, "encoding", 1, 2 ); /* 0 (ANSI), 1 (UTF8), 2 (UTF16). Will also return the choosen encoding. */
  FunctionList.Update( "saveDialog", 3, argList, NULL, NULL, 66 );

  /*D Save the current file */
  /*R Nothing */
  FunctionList.Update( "saveFile", 0, NULL, NULL, NULL, 67 );

  /*D Save the current file to a different file name. */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* New filename */
  argList = BuildArgList( argList, "encoding", 0, 1 ); /* 0 (ANSI), 1 (UTF8), 2 (UTF16) */
  FunctionList.Update( "saveFileAs", 2, argList, NULL, NULL, 90 );

  /*D Save the currently recorded macro */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* Filename to save to */
  FunctionList.Update( "saveMacro", 1, argList, NULL, NULL, 97 );

  /*D Saves the current position in a slot. */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "markNum", 0, 0 ); /* Slot 0-9 */
  FunctionList.Update( "saveNumberedPos", 1, argList, NULL, NULL, 91 );

  /*D Saves the current position */
  /*R Nothing */
  FunctionList.Update( "savePos", 0, NULL, NULL, NULL, 140 );

  /*D Display the search dialog. The dialog will issue the search command. */
  /*R Nothing */
  FunctionList.Update( "searchDialog", 0, NULL, NULL, NULL, 61 );

  /*D Scrolls the window one line down. */
  /*R Nothing */
  FunctionList.Update( "screenLineDown", 0, NULL, NULL, NULL, 171 );

  /*D Scrolls the window one line up. */
  /*R Nothing */
  FunctionList.Update( "screenLineUp", 0, NULL, NULL, NULL, 172 );

  /*D Sets the block end */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 ); /* Position of block end */
  FunctionList.Update( "setBlockEnd", 1, argList, NULL, NULL, 133 );

  /*D Sets the block start */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "position", 0, 0 ); /* Position of block start */
  FunctionList.Update( "setBlockStart", 1, argList, NULL, NULL, 132 );

  /*D Jump to column */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "column", 0, 0 ); /* Column to jump to */
  FunctionList.Update( "setCursorColumn", 1, argList, NULL, NULL, 100 );

  /*D Jump to column, temporarily disabling virtual space. Will be enabled next time the
      cursor moves.  */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "column", 0, 0 ); /* Column to jump to */
  FunctionList.Update( "setCursorColumnTmpDisVS", 1, argList, NULL, NULL, 173 );

  /*D Sets a list of delimitor lines which are displayed in the Ascii view. These are
     ideal when viewing files with entries in a fixed field length layout. */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "lines", 0, 0 ); /* Array of columns where delimitor lines should be placed Ex. {10, 30, 45, 60}.*/
  FunctionList.Update( "setDelimitorLines", 1, argList, NULL, NULL, 110 );

  /*D Turns on or off screen updates */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "active", 0, 0 ); /* TRUE to update screen */
  FunctionList.Update( "setScreenUpdate", 1, argList, NULL, NULL, 93 );

  /*D Sets the current syntax highlighting scheme */
  /*R Name of scheme */
  argList = NULL;
  argList = BuildArgList( argList, "name", 0, 0 ); /* Name of the syntax scheme */
  FunctionList.Update( "setSyntax", 1, argList, NULL, NULL, 155 );

  /*D Set a tool active */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* ID of the tool */
  argList = BuildArgList( argList, "active", 0, 1 ); /* TRUE or FALSE */
  FunctionList.Update( "setToolActive", 2, argList, NULL, NULL, 121 );

  /*D Set view for current file */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "view", 0, 0 ); /* ASCIVIEW (0) or HEXVIEW (1) */
  FunctionList.Update( "setView", 1, argList, NULL, NULL, 94 );

  /*D DEBUG */
  /*R Nothing */
  FunctionList.Update( "snapshot", 0, NULL, NULL, NULL, 111 );

  /*D Start recording a macro. */
  /*R Nothing */
  FunctionList.Update( "startMacroRecord", 0, NULL, NULL, NULL, 73 );

  /*D Start an undo block. Operations between this command and endUndoBlock will be
     undone/redone in a single operation. */
  /*R Nothing */
  FunctionList.Update( "startUndoBlock", 0, NULL, NULL, NULL, 76 );

  /*D Returns the character code for a character. */
  /*R Integer */
  argList = NULL;
  argList = BuildArgList( argList, "data", 0, 0 );  /* String where the first char will be converted to it's character code */
  FunctionList.Update( "stringToChar", 1, argList, NULL, NULL, 138 );

  /*D In hex view only: Switch between HEX and ASCII fields. */
  /*R Nothing */
  FunctionList.Update( "switchField", 0, NULL, NULL, NULL, 82 );

  /*D Execute the code on the control specified. Use this function with caution. If you do
	    not return to the originating control, be sure to release all readers or writer before
			switching, or a deadlock may occur. The control is not focused, for this use
			"switchToControl".*/
  /*R TRUE if success */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* Id of the control */
  FunctionList.Update( "switchExecutingControl", 1, argList, NULL, NULL, 151 );

  /*D Switch focus to a control. */
  /*R TRUE if success */
  argList = NULL;
  argList = BuildArgList( argList, "id", 0, 0 ); /* Id of the control */
  FunctionList.Update( "switchToControl", 1, argList, NULL, NULL, 8 );

  /*D Switch focus to a control containing a specific file. */
  /*R TRUE if success */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* Filename */
  FunctionList.Update( "switchToFile", 1, argList, NULL, NULL, 9 );

	/*D Inserts a tab. Inserts a tab character or spaces depending on settings. */
	/*R Nothing */
	// e
	// FunctionList.Update( "tabPressed", 0, )

  /*D Callback function. Called when the active textbuffer has been modified */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "modified", 0, 0 ); /* TRUE if modified */
  FunctionList.Update( "textModified", 1, argList, NULL, NULL, 29 );

  /*D Move to file end */
  /*R Nothing */
  FunctionList.Update( "toDocumentEnd", 0, NULL, NULL, NULL, 47 );

  /*D Move to file start */
  /*R Nothing */
  FunctionList.Update( "toDocumentStart", 0, NULL, NULL, NULL, 46 );

  /*D Toggle between column block and common block marking. */
  /*R Nothing */
  FunctionList.Update( "toggleColumnBlock", 0, NULL, NULL, NULL, 109 );

  /*D Toggles between relative position display, and absolute position display. */
  /*R Nothing */
  FunctionList.Update( "toggleRelativePos", 0, NULL, NULL, NULL, 88 );

  /*D Converts a string to an integer, if the string contains a valid numeric string. */
  /*R Integer */
  argList = NULL;
  argList = BuildArgList( argList, "data", 0, 0 ); /* String containing an integer, or an integer */
  FunctionList.Update( "toInt", 1, argList, NULL, NULL, 4 );

  /*D Move to line end */
  /*R Nothing */
  FunctionList.Update( "toLineEnd", 0, NULL, NULL, NULL, 49 );

  /*D Move to line start */
  /*R Nothing */
  FunctionList.Update( "toLineStart", 0, NULL, NULL, NULL, 48 );


  /* Converts a string or character to lowercase and returns the converted string or character. */
  argList = NULL;
  argList = BuildArgList( argList, "value", 0, 0 ); /* character or string value */
  FunctionList.Update( "toLower", 1, argList, NULL, NULL, 179 );

  /* Converts a number to a string. Returns the string. */
  argList = NULL;
  argList = BuildArgList( argList, "data", 0, 0 ); /* Integer or string value */
  FunctionList.Update( "toString", 1, argList, NULL, NULL, 5 );

  /* Converts a string or character to uppercase and returns the converted string or character. */
  argList = NULL;
  argList = BuildArgList( argList, "value", 0, 0 ); /* character or string value */
  FunctionList.Update( "toUpper", 1, argList, NULL, NULL, 180 );

  /*D Undo one step */
  /*R TRUE if successful */
  FunctionList.Update( "undo", 0, NULL, NULL, NULL, 53 );

  /*D Callback function. Called when the active textbuffer has undo available. */
	/*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "hasUndo", 0, 0 ); /* TRUE if undo is available */
  FunctionList.Update( "undoAvailable", 1, argList, NULL, NULL, 30 );

  /*D Unload a K-module */
  /*R TRUE if successfull */
  argList = NULL;
  argList = BuildArgList( argList, "filename", 0, 0 ); /* Filename of K-file to unload */
  FunctionList.Update( "unloadModule", 1, argList, NULL, NULL, 162 );

  /*D Reflect changes in the main menu to the display */
	/*R Nothing */
  FunctionList.Update( "updateMainMenu", 0, NULL, NULL, NULL, 27 );

  /*D Update a setting */
  /*R Nothing */
  argList = NULL;
  argList = BuildArgList( argList, "setting", 0, 0 ); /* One of the following strings:<BR><BR>
                                                         'autosave': TRUE if auto save should be done.<BR>
                                                         'backupfiles': TRUE if .BAK file should be created on file save.<BR>
                                                         'columnblock': Columnblock marking (TRUE/FALSE).<BR>
                                                         'crlf': TRUE if CR+LF line end sequence should be used, FALSE if LF only.<BR>
                                                         'hexcursorbg': Colorvalue(RGB) of hex cursor background.<BR>
                                                         'hexcursorfg': Colorvalue(RGB) of hex cursor.<BR>
                                                         'hilitebg': Colorvalue(RGB) of highlight background.<BR>
                                                         'hilitefg': Colorvalue(RGB) of highligt foreground.<BR>
                                                         'insertmode': TRUE if insert mode, FALSE if overwrite mode.<BR>
                                                         'leftMarginbg': Colorvalue(RGB) of left margin background.<BR>
                                                         'leftMarginfg': Colorvalue(RGB) of left margin.<BR>
                                                         'markerbg': Colorvalue(RGB) of marked position background.<BR>
                                                         'markerfg': Colorvalue(RGB) of marked position.<BR>
                                                         'markbg': Colorvalue(RGB) of mark background.<BR>
                                                         'markfg': Colorvalue(RGB) of mark foreground.<BR>
                                                         'maxUndoSize': Max number of undo steps.<BR>
                                                         'optimalfill': TRUE to fill long space runs whith tab character (0x9) when possible..<BR>
                                                         'persistentblocks': TRUE if persistent blocks should be used.<BR>
                                                         'reuseExistingBuffer : TRUE if an existing buffer and control should be used instead of reloading when a file is opened.<BR>
                                                         'rightmargin': The right margin.<BR>
                                                         'showLeftMarginInfo' : TRUE to show the left margin info.<BR>
                                                         'showLineNumber' : TRUE to show line numbers in the left margin.<BR>
                                                         'showSavedPos' : TRUE to show saved position in the left margin. 
                                                         'standarddelim': Standard delimitor string. (When no syntax highlighting is defined.)<BR>
                                                         'showrightmargin': TRUE if the right margin should be shown in the text window.<BR>
                                                         'stripnameinnotebook': TRUE if only filename without path should be displayed in notebook tabs.<BR>
                                                         'syntaxhighlight': TRUE if syntax highlightinh should be used.<BR>
                                                         'tabsize': Set the tab size. <BR>
                                                         'usetab': TRUE to insert real tab character (0x9), FALSE to insert spaces.*/
  argList = BuildArgList( argList, "update", 0, 1 ); /* New setting */
  FunctionList.Update( "updateSetting", 2, argList, NULL, NULL, 38 );

  /*D DEBUG */
  /*R Nothing */
  FunctionList.Update( "window", 0, NULL, NULL, NULL, 131 );
  
  /*D Move to line start */
  /*R Nothing */
  FunctionList.Update( "QP", 0, NULL, NULL, NULL, 181 );

  

  // MAX: 180. 6
}

TSyntax *KCommonCommands::eval( char *name, TVariableList *vars, TEval *eval, 
                                int index )
{
	switch( index )
	{
		case 1 : // bindKey
		{
			TSyntax **keySeq = vars->LookUp( "keySeq" );
			TSyntax **command = vars->LookUp( "command" );

			if( keySeq == NULL || command == NULL  )
				eval->SendError( Strings::get(47), name );


			if( (*keySeq)->Type != SString || (*command)->Type != SString )
				eval->SendError(  Strings::get(51), name );
			else
			{
				// Everything is OK with the arguments. Now add this key binding

				KeyBindings::getInstance()->addCommand( ((TString *)*keySeq)->string,
					((TString *)*command)->string );
			}
			break;
		}
		case 2 : // print
		{
			TSyntax **value = vars->LookUp( "data" );

			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
				KErrorCollector::getInstance()->message( ((TString *)*value)->string );
			else
			if( (*value)->Type == SNumeric )
			{
				char str[12];

				sprintf( str, "%d", ((TNumeric *)*value)->num );
				KErrorCollector::getInstance()->message( str );
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 3 : // length
		{
			TSyntax **value = vars->LookUp( "array" );
			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SArray )
				return new TNumeric( ((TArray *)(*value))->Size, COPY );
			else
			if( (*value)->Type == SString )
				return new TNumeric( strlen(((TString *)(*value))->string), COPY );
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 4 : // toInt
		{
			TSyntax **value = vars->LookUp( "data" );
			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
				return new TNumeric( atoi( ((TString *)*value)->string), COPY );
			else
			if( (*value)->Type == SNumeric )
				return new TNumeric( ((TNumeric *)*value)->num, COPY );
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 5 : // toString
		{
			char str[20];

			TSyntax **value = vars->LookUp( "data" );
			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
				return new TString( ((TString *)*value)->string, COPY );
			else
			if( (*value)->Type == SNumeric )
				return new TString( itoa( ((TNumeric *)*value)->num, str, 10), COPY );
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 7 : //  // charToString
		{
			char str[20];

			TSyntax **value = vars->LookUp( "data" );
			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
				return new TString( ((TString *)*value)->string, COPY );
			else
			if( (*value)->Type == SNumeric )
			{
				str[1] = 0;
				str[0] = ((TNumeric *)*value)->num;
				return new TString( str, COPY );
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 138 : // stringToChar
		{
			TSyntax **value = vars->LookUp( "data" );
			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
				return new TNumeric( ((TString *)*value)->string[0], COPY );
			else
			if( (*value)->Type == SNumeric )
			{
				return new TNumeric( ((TNumeric *)*value)->num, COPY );
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 37 : // getSetting
		{
			TSyntax **settingA = vars->LookUp( "setting" );
			if( settingA == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*settingA)->Type == SString )
			{
				char *setting = ((TString *)*settingA)->string;

				if( !strcmp( setting, "tabsize" ) )
					return new TNumeric( Settings::tabSize, COPY );
				if( !strcmp( setting, "maxUndoSize" ) )
					return new TNumeric( Settings::maxUndoSize, COPY );
				if( !strcmp( setting, "rightmargin" ) )
					return new TNumeric( Settings::rightMargin, COPY );
				if( !strcmp( setting, "markfg" ) )
					return new TNumeric( Settings::markFg, COPY );
				if( !strcmp( setting, "markbg" ) )
					return new TNumeric( Settings::markBg, COPY );
				if( !strcmp( setting, "hilitefg" ) )
					return new TNumeric( Settings::hiliteFg, COPY );
				if( !strcmp( setting, "hilitebg" ) )
					return new TNumeric( Settings::hiliteBg, COPY );
				if( !strcmp( setting, "hexcursorfg" ) )
					return new TNumeric( Settings::hexCursorFg, COPY );
				if( !strcmp( setting, "hexcursorbg" ) )
					return new TNumeric( Settings::hexCursorBg, COPY );
				if( !strcmp( setting, "markerfg" ) )
					return new TNumeric( Settings::markerFg, COPY );
				if( !strcmp( setting, "markerbg" ) )
					return new TNumeric( Settings::markerBg, COPY );
				if( !strcmp( setting, "leftMarginfg" ) )
					return new TNumeric( Settings::leftMarginFg, COPY );
				if( !strcmp( setting, "leftMarginbg" ) )
					return new TNumeric( Settings::leftMarginBg, COPY );
				if( !strcmp( setting, "standarddelim" ) )
					return new TString( Settings::standardDelim, COPY );
				if( !strcmp( setting, "columnblock" ) )
					return new TNumeric( Settings::columnBlock, COPY );
				if( !strcmp( setting, "crlf" ) )
					return new TNumeric( Settings::CRLF, COPY );
				if( !strcmp( setting, "stripnameinnotebook" ) )
					return new TNumeric( Settings::stripNameInNotebook, COPY );
				if( !strcmp( setting, "insertmode" ) )
					return new TNumeric( Settings::insertMode, COPY );
				if( !strcmp( setting, "backupfiles" ) )
					return new TNumeric( Settings::backupFiles, COPY );
				if( !strcmp( setting, "persistentblocks" ) )
					return new TNumeric( Settings::persistentBlocks, COPY );
				if( !strcmp( setting, "showrightmargin" ) )
					return new TNumeric( Settings::showRightMargin, COPY );
				if( !strcmp( setting, "syntaxhighlight" ) )
					return new TNumeric( Settings::syntaxHighlight, COPY );
				if( !strcmp( setting, "autosave" ) )
					return new TNumeric( Settings::autoSave, COPY );
				if( !strcmp( setting, "usetab" ) )
					return new TNumeric( Settings::useTab, COPY );
				if( !strcmp( setting, "optimalfill" ) )
					return new TNumeric( Settings::optimalFill, COPY );
				if( !strcmp( setting, "showLeftMarginInfo" ) )
					return new TNumeric( Settings::showLeftMarginInfo, COPY );
				if( !strcmp( setting, "showLineNumbers" ) )
					return new TNumeric( Settings::showLineNumbers, COPY );
				if( !strcmp( setting, "showSavedPos" ) )
					return new TNumeric( Settings::showSavedPos, COPY );
				if( !strcmp( setting, "reuseExistingBuffer" ) )
          return new TNumeric( Settings::reuseExistingBuffer, COPY );
			}
			else
				eval->SendError( Strings::get(51), name );
			return new TNumeric( 0, COPY );
			break;
		}
		case 38 : // updateSetting
		{
			TSyntax **settingA = vars->LookUp( "setting" );
			TSyntax **update = vars->LookUp( "update" );
			if( settingA == NULL || update == NULL)
				eval->SendError( Strings::get(47), name );
			else
			if( (*settingA)->Type == SString )
			{
				char *setting = ((TString *)*settingA)->string;
				char *str;
				int iValue;
				errorOccured = FALSE;

				if( !strcmp( setting, "tabsize" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::tabSize = iValue;

				if( !strcmp( setting, "maxUndoSize" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::maxUndoSize = iValue;

				if( !strcmp( setting, "rightmargin" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::rightMargin = iValue;

				if( !strcmp( setting, "markfg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::markFg = iValue;

				if( !strcmp( setting, "markbg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::markBg = iValue;

				if( !strcmp( setting, "hilitefg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::hiliteFg = iValue;

				if( !strcmp( setting, "hilitebg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::hiliteBg = iValue;

				if( !strcmp( setting, "hexcursorfg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::hexCursorFg = iValue;

				if( !strcmp( setting, "hexcursorbg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::hexCursorBg = iValue;

				if( !strcmp( setting, "markerfg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::markerFg = iValue;

				if( !strcmp( setting, "markerbg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::markerBg = iValue;

				if( !strcmp( setting, "leftMarginfg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::leftMarginFg = iValue;

				if( !strcmp( setting, "leftMarginbg" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::leftMarginBg = iValue;

				if( !strcmp( setting, "standarddelim" ) )
					if( syntaxToString( *update, &str ) )
						Settings::standardDelim = str;

				if( !strcmp( setting, "columnblock" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::columnBlock = iValue;

				if( !strcmp( setting, "crlf" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::CRLF = iValue;

				if( !strcmp( setting, "stripnameinnotebook" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::stripNameInNotebook = iValue;

				if( !strcmp( setting, "insertmode" ) )
					if( syntaxToInt( *update, iValue ) )
					{
						Settings::insertMode = iValue;
						MainWindowAdmin::updateAllWindows( MainWindowAdmin::INFOBAR );
					}
		
				if( !strcmp( setting, "backupfiles" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::backupFiles = iValue;

				if( !strcmp( setting, "persistentblocks" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::persistentBlocks = iValue;

				if( !strcmp( setting, "showrightmargin" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::showRightMargin = iValue;

				if( !strcmp( setting, "syntaxhighlight" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::syntaxHighlight = iValue;

				if( !strcmp( setting, "autosave" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::autoSave = iValue;

				if( !strcmp( setting, "usetab" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::useTab = iValue;

				if( !strcmp( setting, "optimalfill" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::optimalFill = iValue;

				if( !strcmp( setting, "showLeftMarginInfo" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::showLeftMarginInfo = iValue;

				if( !strcmp( setting, "showLineNumbers" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::showLineNumbers = iValue;

				if( !strcmp( setting, "showSavedPos" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::showSavedPos = iValue;

				if( !strcmp( setting, "autoCloseFrameWindow" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::autoCloseFrameWindow = iValue;

				if( !strcmp( setting, "showCommandBar" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::showCommandBar = iValue;

				if( !strcmp( setting, "virtualSpace" ) )
					if( syntaxToInt( *update, iValue ) )
						Settings::virtualSpace = iValue;

				if( !strcmp( setting, "reuseExistingBuffer" ) )
					if( syntaxToInt( *update, iValue ) )
            Settings::reuseExistingBuffer = iValue;

				if( !strcmp( setting, "sectionPulldown" ) )
					if( syntaxToInt( *update, iValue ) )
            Settings::pulldownSize = iValue;

				if( errorOccured )
					eval->SendError( Strings::get(51), name );

				return NULL;
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 11 : // debug
		{
			char str[10];
			KErrorCollector::getInstance()->message( itoa( Objects, str, 10 ) );
			break;
		}
		case 8 : // switchToControl
		{
			TSyntax **value = vars->LookUp( "id" );

			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SNumeric )
			{
				ControlInfo *control = ControlAdmin::queryControlWithId(
																((TNumeric *)*value)->num );
				if( control == NULL )
					return new TNumeric( 0, COPY );

        MainWindow *parent = control->parent;
        EditControl *editControl = control->editControl;

				parent->setFocus();  // Set focus to the MainWindow, will destroy control
				parent->selectControl( editControl );
				return new TNumeric( 1, COPY );
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 9 : // switchToFile
		{
			TSyntax **value = vars->LookUp( "filename" );

			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
			{
				ControlInfo *control = ControlAdmin::queryControlWithFilename(
																((TString *)*value)->string );
				if( control == NULL )
					return new TNumeric( 0, COPY );

        MainWindow *parent = control->parent;
        EditControl *editControl = control->editControl;

				parent->setFocus(); // Will destroy controlInfo control
				parent->selectControl( editControl );
				return new TNumeric( 1, COPY );
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 10 : // loadFile
		{
			// Load a new file
			TSyntax **fname = vars->LookUp( "filename" ),
			        **bkgr = vars->LookUp( "backgroundLoad" ),
			        **useExisting = vars->LookUp( "useExisting" );

			if( fname == NULL || bkgr == NULL || useExisting == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*fname)->Type != SString || (*bkgr)->Type != SNumeric ||
			    (*useExisting)->Type != SNumeric)
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			// Load the file in the active mainwindow

			int threadedCode, unthreadedCode;

			if( ((TNumeric *)*useExisting)->num )
			{
				threadedCode = 10;
				unthreadedCode = 9;
			}
			else
			{
				threadedCode = 1;
				unthreadedCode = 5;

			}
			return new TNumeric( Application::executeFromMainThread( ControllerWindow::getInstance(), 
														(((TNumeric *)*bkgr)->num) ? threadedCode : unthreadedCode, 
														((TString *)*fname)->string ), COPY );
			break;
		}
		case 139 : // getScratchName
		{
			char filename[MAXPATH];
			BufferAdmin::getNextScratchName( filename );
			return new TString( filename, COPY );
		}
		case 13 : // closeActiveWindow
		{
			// Add the active window in the active MainWindow to the exit list

			MainWindow *mainWnd;

			if( (mainWnd = MainWindowAdmin::getActiveWindow()) )
				ControlAdmin::addToExitList( mainWnd->getActiveControl(), FALSE );
			break;
		}
		case 14 : // closeFrameWindow
		{
			// Add the active window in the active MainWindow to the exit list
			// with flag close FrameWindow

			MainWindow *mainWnd;

			if( (mainWnd = MainWindowAdmin::getActiveWindow()) )
				ControlAdmin::addToExitList( mainWnd->getActiveControl(), TRUE );
			break;
		}
		case 15 : // createFrameWindow
		{
			Application::executeFromMainThread( ControllerWindow::getInstance(), 7, NULL );
			break;
		}
		case 104 : // findStringFiles
		{
			TSyntax **string = vars->LookUp( "searchString" );
			TSyntax **options = vars->LookUp( "options" );
			TSyntax **path = vars->LookUp( "path" );
			TSyntax **files = vars->LookUp( "files" );

			if( string == NULL || options == NULL || path == NULL || files == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*string)->Type != SString || (*options)->Type != SString ||
					(*path)->Type != SString || (*files)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			
			if( Settings::autoClearMessages ) 
			  KErrorCollector::getInstance()->clear();

			MultipleFilesFind *multipleFilesFind = new
				MultipleFilesFind( ((TString *)*string)->string, 
													 ((TString *)*options)->string );

			MultipleFiles *multipleFiles = new
				MultipleFiles( multipleFilesFind,
											 ((TString *)*path)->string,
											 ((TString *)*files)->string,
											 strchr( ((TString *)*options)->string, 'r' ) != NULL );

			delete multipleFiles;
			delete multipleFilesFind;
			break;
		}
		case 106 : // replaceStringFiles
		{
			TSyntax **findString = vars->LookUp( "searchString" );
			TSyntax **replaceString = vars->LookUp( "replaceString" );
			TSyntax **options = vars->LookUp( "options" );
			TSyntax **path = vars->LookUp( "path" );
			TSyntax **files = vars->LookUp( "files" );

			if( findString == NULL || replaceString == NULL || options == NULL || 
					path == NULL || files == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*findString)->Type != SString || (*replaceString)->Type != SString || 
					(*options)->Type != SString ||
					(*path)->Type != SString || (*files)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			if( Settings::autoClearMessages ) 
			  KErrorCollector::getInstance()->clear();

			MultipleFilesReplace *multipleFilesReplace = new
				MultipleFilesReplace( ((TString *)*findString)->string, 
															((TString *)*replaceString)->string,  
															((TString *)*options)->string );

			MultipleFiles *multipleFiles = new
				MultipleFiles( multipleFilesReplace,
											 ((TString *)*path)->string,
											 ((TString *)*files)->string,
											 strchr( ((TString *)*options)->string, 'r' ) != NULL );

			delete multipleFiles;
			delete multipleFilesReplace;
			break;
		}
		case 16 : // beginMainMenu
		{
			MainMenu::resetMenu();
			buildMenu = MAINMENU;
			break;
		}
		case 17 : // beginSubmenuUpdate
		{
			TSyntax **id = vars->LookUp( "id" );
			TSyntax **removeItems = vars->LookUp( "removeItems" );

			if( id == NULL || removeItems == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*id)->Type != SNumeric || (*removeItems)->Type != SNumeric)
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			MainMenu::startSubmenuBuild( ((TNumeric *)*id)->num, ((TNumeric *)*removeItems)->num );
			buildMenu = MAINMENU;
			break;
		}
		case 18 : // beginPopupMenu
		{
			PopupMenu::resetMenu();
			buildMenu = POPUPMENU;
			break;
		}
		case 19 : // addMenu
		{
			TSyntax **atId = vars->LookUp( "atId" );
			TSyntax **string = vars->LookUp( "string" );
			TSyntax **menuId = vars->LookUp( "menuId" );

			if( atId == NULL || string == NULL || menuId == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*atId)->Type != SNumeric || (*string)->Type != SString || 
					(*menuId)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			if( buildMenu == MAINMENU )
				MainMenu::addMenuFromId( ((TNumeric *)*atId)->num,
																 ((TString *)*string)->string,
																 ((TNumeric *)*menuId)->num );
			else
				PopupMenu::addMenuFromId( ((TNumeric *)*atId)->num,
																 ((TString *)*string)->string,
																 ((TNumeric *)*menuId)->num );
			break;
		}
		case 20 : // addMenuItem
		{
			TSyntax **atId = vars->LookUp( "atId" );
			TSyntax **string = vars->LookUp( "string" );
			TSyntax **command = vars->LookUp( "command" );
			TSyntax **menuId = vars->LookUp( "menuId" );

			if( atId == NULL || string == NULL || command == NULL || menuId == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*atId)->Type != SNumeric || (*string)->Type != SString || 
					(*command)->Type != SString || (*menuId)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			if( buildMenu == MAINMENU )
				MainMenu::addMenuCommandFromId( ((TNumeric *)*atId)->num,
																				((TString *)*string)->string,
																				((TString *)*command)->string,
																				((TNumeric *)*menuId)->num );
			else
				PopupMenu::addMenuCommandFromId( ((TNumeric *)*atId)->num,
																				((TString *)*string)->string,
																				((TString *)*command)->string,
																				((TNumeric *)*menuId)->num );
			break;
		}
		case 21 : // addMenuSeparator
		{
			TSyntax **atId = vars->LookUp( "atId" );

			if( atId == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*atId)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			if( buildMenu == MAINMENU )
				MainMenu::addSeparatorFromId( ((TNumeric *)*atId)->num );
			else
				PopupMenu::addSeparatorFromId( ((TNumeric *)*atId)->num );
			break;
		}
		case 22 : // checkMenuItem
		{
			TSyntax **id = vars->LookUp( "id" );
			TSyntax **checked = vars->LookUp( "checked" );

			if( id == NULL || checked == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*id)->Type != SNumeric || (*checked)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			MainMenu::checkItem( ((TNumeric *)*id)->num,
													 ((TNumeric *)*checked)->num );
			break;
		}
		case 23 : // enableMenuItem
		{
			TSyntax **id = vars->LookUp( "id" );
			TSyntax **enabled = vars->LookUp( "enabled" );

			if( id == NULL || enabled == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*id)->Type != SNumeric || (*enabled)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			MainMenu::enableItem( ((TNumeric *)*id)->num,
															 ((TNumeric *)*enabled)->num );
			break;
		}
		case 26 : // endMainMenu
			MainMenu::endBuild();
			break;
		case 25 : // endSubMenuUpdate
		{
			// Do nothing
			break;
		}
		case 24 : // endPopupMenu
			PopupMenu::endBuild();
			break;
		case 27 : // updateMainMenu
			Application::executeFromMainThread( MainWindowAdmin::getActiveWindow(), 
																					1, NULL );
			break;
		case 28 : // popupMenu
		{
			TSyntax **viewWindowHolder = vars->LookUp( "windowObject" );
			TSyntax **x = vars->LookUp( "x" );
			TSyntax **y = vars->LookUp( "y" );

			if( x == NULL || y == NULL || viewWindowHolder == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*x)->Type == SNumeric && (*y)->Type == SNumeric &&
					(*viewWindowHolder)->Type == SNumeric)
			{
				ViewWindowHolder *vw = (ViewWindowHolder *)(((TNumeric *)*viewWindowHolder)->num);
				Point point( ((TNumeric *)*x)->num, ((TNumeric *)*y)->num );
				vw->popupMenu( &point );
				
				return NULL;
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
		case 34 : // messageWindowDisplay
		{
			TSyntax **show = vars->LookUp( "show" );

			if( show == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*show)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			if( ((TNumeric *)*show)->num )
				KErrorCollector::getInstance()->show();
			else
				KErrorCollector::getInstance()->hide();
			break;
		}
		case 114 : // getRecentFilesCount
		{
			return new TNumeric( MainMenu::getRecentFilesCount(), COPY );
			break;
		}
		case 115 : // getRecentFile
		{
			TSyntax **pos = vars->LookUp( "pos" );

			if( pos == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*pos)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			return new TString( 
										MainMenu::getRecentFile( ((TNumeric *)*pos)->num ), COPY );
			break;
		}
		case 116 : // getWindowCount
		{
			return new TNumeric( ControlAdmin::getControls(), COPY );
			break;
		}
		case 117 : // getWindowFilename
		{
			TSyntax **pos = vars->LookUp( "pos" );

			if( pos == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*pos)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			return new TString( ControlAdmin::
													getControl(((TNumeric *)*pos)->num)->editControl->getTextBuffer()->getFilename(), 
													COPY );
			break;
		}
		case 118 : // getWindowId
		{
			TSyntax **pos = vars->LookUp( "pos" );

			if( pos == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*pos)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			return new TNumeric( ControlAdmin::
													 getControl(((TNumeric *)*pos)->num)->editControl->getId(), COPY );
			break;
		}
		case 35 : // messageWindowVisible
			return new TNumeric( KErrorCollector::getInstance()->isVisible(), COPY );
			break;
		case 39 : // prefsDialog
		{
			Application::executeFromMainThread( MainWindowAdmin::getActiveWindow(), 
																					4, NULL );
			break;
		}
		case 119 : // addToolButton
		{
			TSyntax **command = vars->LookUp( "command" );
			TSyntax **bitmap = vars->LookUp( "bitmap" );
			TSyntax **description = vars->LookUp( "description" );
			TSyntax **id = vars->LookUp( "id" );

			if( command == NULL || bitmap == NULL || description == NULL || id == NULL)
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*command)->Type != SString || (*bitmap)->Type != SString || 
					(*description)->Type != SString || (*id)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			
			// Update every mainwindows toolbar

			ToolButtonInfo *tbInfo;

			tbInfo = new ToolButtonInfo;
			
			tbInfo->command = new char[strlen(((TString *)*command)->string)+1];
			strcpy( tbInfo->command, ((TString *)*command)->string );
			tbInfo->description = new char[strlen(((TString *)*description)->string)+1];
			strcpy( tbInfo->description, ((TString *)*description)->string );
			tbInfo->bitmap = new char[strlen(((TString *)*bitmap)->string)+1];
			strcpy( tbInfo->bitmap, ((TString *)*bitmap)->string );
			tbInfo->id = ((TNumeric *)*id)->num;
			tbInfo->type = Toolbar::IMAGE;

			MainWindow::toolButtonInfo.add( tbInfo );

			// Update all window menus

	//    Application::executeFromMainThread( MainWindowAdmin::getActiveWindow(), 
	//                                        4, NULL );
			break;
		}
		case 120 : // addToolSpace
		{
			ToolButtonInfo *tbInfo;

			tbInfo = new ToolButtonInfo;
			
			tbInfo->id = -1;
			tbInfo->type = Toolbar::SPACE;

			MainWindow::toolButtonInfo.add( tbInfo );
			break;
		}
		case 121 : // setToolActive
		{
			TSyntax **id = vars->LookUp( "id" );
			TSyntax **active = vars->LookUp( "active" );

			if( id == NULL || active == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*id)->Type != SNumeric || (*active)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			MainWindow *currentMainWnd = MainWindowAdmin::getActiveWindow();

			if( currentMainWnd != NULL )
			{
				ToolButtonState tbState;

				tbState.id = ((TNumeric *)*id)->num;
				tbState.active = ((TNumeric *)*active)->num;
				Application::executeFromMainThread( currentMainWnd, 6, &tbState );
			}
			
			// Update all window menus

	//    Application::executeFromMainThread( MainWindowAdmin::getActiveWindow(), 
	//                                        4, NULL );
			break;
		}
		case 125 : // isInteger
		{
			TSyntax **arg = vars->LookUp( "arg" );

			if( arg == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			return new TNumeric( (*arg)->Type == SNumeric, TRUE );
		}
		case 126 : // isString
		{
			TSyntax **arg = vars->LookUp( "arg" );

			if( arg == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			return new TNumeric( (*arg)->Type == SString, TRUE );
		}
		case 129 : // fileExists
		{
			TSyntax **filename = vars->LookUp( "filename" );

			if( (*filename)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			if( filename == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			return new TNumeric( 
				System::fileExists( ((TString *)*filename)->string), TRUE );
		}
		case 130 : // messageBox
		{
			TSyntax **title = vars->LookUp( "title" );
			TSyntax **message = vars->LookUp( "message" );
			TSyntax **type = vars->LookUp( "type" );

			if( title == NULL || message == NULL || type == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*title)->Type != SString || (*message)->Type != SString || 
					(*type)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			MessageBoxInfo mbInfo;

			mbInfo.title = ((TString *)*title)->string;
			mbInfo.message = ((TString *)*message)->string;
			mbInfo.type = ((TNumeric *)*type)->num;

			int result = 
				Application::executeFromMainThread( MainWindowAdmin::getActiveWindow(), 
																						7, &mbInfo );
			return new TNumeric( result, TRUE );
		}
		case 136 : // copyTextToClipboard
		{
			TSyntax **text = vars->LookUp( "text" );

			if( text == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*text)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      char *str = ((TString *)(*text))->string;

      ClipBoard::setText(str, strlen( str ) );

			break;
		}
		case 128 : // getTime
		{
			TSyntax **formatstring = vars->LookUp( "formatstring" );

			if( formatstring == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*formatstring)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			char str[50];
			strcpy( str, "" );

			time_t aclock;
			struct tm *newtime;
			time( &aclock );
			newtime = localtime( &aclock );

			strftime( str, 50, ((TString *)*formatstring)->string, newtime );

			return new TString( str, COPY );
		}
		case 144 : // leftString
		{
			TSyntax **string = vars->LookUp( "string" );
			TSyntax **chars = vars->LookUp( "characters" );

			if( string == NULL || chars == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*string)->Type != SString || (*chars)->Type != SNumeric)
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      int len = ((TNumeric *)*chars)->num;

      char *str = new char[len+1];
      strncpy( str, ((TString *)*string)->string, len);
      str[len] = 0;

			return new TString( str, TRUE, COPY );
		}
		case 145 : // rightString
		{
			TSyntax **string = vars->LookUp( "string" );
			TSyntax **chars = vars->LookUp( "characters" );

			if( string == NULL || chars == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*string)->Type != SString || (*chars)->Type != SNumeric)
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      int len = ((TNumeric *)*chars)->num;

      char *str = new char[len+1];
      strcpy( str, &((TString *)*string)->string[len] );

			return new TString( str, TRUE, COPY );
		}
		case 146 : // midString
		{
			TSyntax **string = vars->LookUp( "string" );
			TSyntax **start = vars->LookUp( "start" );
			TSyntax **chars = vars->LookUp( "characters" );

			if( string == NULL || chars == NULL || start == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*string)->Type != SString || (*chars)->Type != SNumeric ||
          (*start)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      int len = ((TNumeric *)*chars)->num;

      char *str = new char[len+1];
      strncpy( str, &((TString *)*string)->string[((TNumeric *)*start)->num], len );
      str[len] = 0;

			return new TString( str, TRUE, COPY );
		}
		case 151 : // switchExecutingControl
		{
			TSyntax **id = vars->LookUp( "id" );

			if( id == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*id)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			eval->getCommandExecutor()->switchExecutor( ControlAdmin::queryControlWithId( ((TNumeric *)*id)->num )->editControl );
			return NULL;
		}
		case 152 : // browse
		{
			TSyntax **URL = vars->LookUp( "URL" );

			if( URL == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}

			if( (*URL)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			System::launchBrowser( ((TString *)*URL)->string );
			return NULL;
		}
		case 153 : // getKonDir
		{
      char *str = new char[strlen(KonSystem::baseDir)+1];
      strcpy( str, KonSystem::baseDir );
      str[strlen(str)-1] = 0;
			return new TString( str, TRUE, COPY );
		}
		case 154 : // getSyntaxCount
		{
			return new TNumeric( SyntaxContainer::getSyntaxCount(), COPY );
		}
		case 156 : // getSyntaxNameAtPos
		{
			TSyntax **pos = vars->LookUp( "pos" );

			if( pos == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*pos)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			return new TString( SyntaxContainer::getSyntaxAt(((TNumeric *)*pos)->num)->name, COPY );
		}
		case 157 : // closeControl
		{
			// Add the control to the exit list

			TSyntax **id = vars->LookUp( "id" );

			if( id == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*id)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      ControlInfo *ctrInfo = ControlAdmin::queryControlWithId( ((TNumeric *)*id)->num);

			ControlAdmin::addToExitList( ctrInfo->editControl, FALSE );
			break;
		}
    case 161 :  // loadModule
    {
			TSyntax **filename = vars->LookUp( "filename" );
			TSyntax **preload = vars->LookUp( "preload" );

			if( filename == NULL || preload == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*filename)->Type != SString || (*preload)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      BOOL success = KModules::addModule( ((TString *)*filename)->string, (ModuleLoadType)((TNumeric *)*preload)->num );
      return new TNumeric( success, COPY );
    }
    case 162 :  // unloadModule
    {
			TSyntax **filename = vars->LookUp( "filename" );

			if( filename == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*filename)->Type != SString )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      BOOL success = KModules::removeModule( ((TString *)*filename)->string );
      return new TNumeric( success, COPY );
    }
		case 163 : // focusCommandBar
		{
      MainWindowAdmin::getActiveWindow()->focusCommandBar();
			return NULL;
		}
		case 169 : // getFileId
		{
			TSyntax **value = vars->LookUp( "filename" );

			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
			{
				ControlInfo *control = ControlAdmin::queryControlWithFilename(
																((TString *)*value)->string );
				if( control == NULL )
					return new TNumeric( -1, COPY );

				return new TNumeric( control->editControl->getId(), COPY );
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
		}
    case 179 : // toLower
    case 180 : // toUpper
    {
			TSyntax **value = vars->LookUp( "value" );

			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SNumeric )
			{
			  return new TNumeric( ((index == 179) ? tolower(((TNumeric *)*value)->num) : 
			                                         toupper(((TNumeric *)*value)->num)), COPY );
			}
			else
			if( (*value)->Type == SString )
			{
			  TextElem *stri = ((TString *)*value)->string;
			  TString *strl = new TString( stri, COPY );
			  TextElem *istrl = strl->string;
			  while( *stri ) *istrl++ = (index == 179) ? tolower(*stri++) : toupper(*stri++);
			  *istrl = 0;
			  return strl;
			}
			else
				eval->SendError( Strings::get(51), name );
			break;
    }
   
		default :
			eval->SendError( Strings::get(37), name );
	}
  return NULL;
}

void CaptureThread::run()
{
  char *read;
  while( (read = pipe->readPipe()) )
  {
    char message[1000];
    char *fixedStr = KCommonCommands::getInstance()->fixString(read);
    sprintf( message, "insertString( \"%s\" );", fixedStr );
    if( fixedStr != read ) delete fixedStr;
 
    ControlAdmin::queryControlWithId(controlId)->editControl->runCommand( message );
  }
  delete pipe;
}

char *KCommonCommands::fixString( char *string )
{
  int slashes = 0, len = strlen( string ), i;
  char *newString;

  for( i = 0; i < len; i++ )
    if( string[i] == '\\' || string[i] == '\'' ) slashes++;

  if( slashes == 0 ) return string;

  newString = new char[len+slashes+1];

  int p;

  for( p = 0, i = 0; i < len; i++ )
    if( string[i] == '\\' )
    {
      newString[p++] = '\\';
      newString[p++] = '\\';
    }
    else
    if( string[i] == '\'' )
    {
      newString[p++] = '\\';
      newString[p++] = '\'';
    }
    else
      newString[p++] = string[i];

   newString[p] = 0;

  return newString;
}

BOOL KCommonCommands::syntaxToInt( TSyntax *syntax, int &integer )
{
	if( syntax->Type == SNumeric )
  {
  	integer = ((TNumeric *)syntax)->num;
    return TRUE;
  }
  else
	if( syntax->Type == SString )
  {
  	integer = atoi(((TString *)syntax)->string);
    return TRUE;
  }

	errorOccured = TRUE;

	return FALSE;
}

BOOL KCommonCommands::syntaxToString( TSyntax *syntax, char **string )
{
	if( syntax->Type == SNumeric )
  {
  	*string = new char[20];
  	sprintf( *string, "%d", ((TNumeric *)syntax)->num );
    return TRUE;
  }
  else
	if( syntax->Type == SString )
  {
  	*string = new char[strlen(((TString *)syntax)->string)+1];
    strcpy( *string, ((TString *)syntax)->string );
    return TRUE;
  }

	errorOccured = TRUE;

	return FALSE;
}
