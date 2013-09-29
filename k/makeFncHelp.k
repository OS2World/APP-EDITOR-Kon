makeFunctionHelp()
{
	functionEntry = "";
  wasPersistentBlocks = getSetting( "persistentblocks" );
  updateSetting( "persistentblocks", TRUE );

	while( findString( 'FunctionList.Update', '' ) )
	{
    pushPos( FALSE );
		functionEntry = functionEntry @ "<fun>\n";

		nextWord();

		functionNameStart = getPosition();

		findString( "\"", "" );

		functionEntry = functionEntry @ "<name>" @ 
										getText( functionNameStart, getPosition() - functionNameStart - 1 ) @
										"</name>\n";

		nextWord();
		argCountStart = getPosition();
		findString( ",", "" );

		args = getText( argCountStart, getPosition() - argCountStart - 1 );
		
		print( args );

		for( i = 0; i < args; i++ ) findString( "/*", "b" );

    pushPos( FALSE );

  	// Get return value

    findString( "/*R", "b" );
		retValStart = getPosition() + 3;
    while( getChar( retValStart ) == 32 ) retValStart++;
    findString( "*/", "" );
    retVal = getText( retValStart, 
                      getPosition() - retValStart - 2 );
	
  	// Get description

    findString( "/*D", "b" );
		descriptionStart = getPosition() + 3;
    while( getChar( descriptionStart ) == 32 ) descriptionStart++;
    findString( "*/", "" );
    description = getText( descriptionStart, 
                           getPosition() - descriptionStart - 2 );
	
		popPos();

		for( i = 0; i < args; i++ )
		{
			toLineStart();
			
			for( n = 0; n < 5; n++ ) nextWord();
			
			argNameStart = getPosition();
			
			findString( "\"", "" );

			functionEntry = functionEntry @ "<param>" @ 
											getText( argNameStart, getPosition() - argNameStart - 1 ) @
											"</param>";

			// Get description for parameter

      findString( "/*", "" );
      descriptionStart = getPosition();
      while( getChar( descriptionStart ) == 32 ) descriptionStart++;
      findString( "*/", "" );
      functionEntry = functionEntry @ "<expl>" @ getText( descriptionStart, 
                             getPosition() - descriptionStart - 2 ) @
                      "</expl>\n";

      findString( "/*", "" );
      toLineStart();
		}

		functionEntry = functionEntry @ "<descr>" @ description @ "</descr>\n";
		functionEntry = functionEntry @ "<retval>" @ retVal @ "</retval>\n";
		functionEntry = functionEntry @ "</fun>\n";
    popPos();
	}

	id = loadFile( getScratchName(), FALSE, FALSE );

	switchExecutingControl( id );
	switchToControl( id );

	insertString( functionEntry );

	toDocumentStart();

  // ********************************************
	
	// Find function boundaries in the text

	keepBlock( TRUE );

  indexList = "";
  treeIndex = "";

  resString = "";

	functions = 0;
	indexList = [];

	lastChar = " ";

	while( findString( "<fun>", "" ) )
	{
		startPos = getPosition();
		findString( "</fun>", "" );
		endPos = getPosition();

		setBlockStart( startPos );
		setBlockEnd( endPos );
		
		// Find the function name

		jumpToPosition( startPos );

		findString( "<name>", "s" );

		nameStart = getPosition();

		findString( "</name>", "s" );

		nameEnd = getPosition() - 7;

		functionName = getText( nameStart, nameEnd - nameStart );

		// Find function variables

		jumpToPosition( startPos );

		functionParam = [];

		while( findString( "<param>", "s" ) )
		{
			paramStart = getPosition();
			findString( "</param>", "s" );
			paramEnd = getPosition() - 8;
			paramName = getText( paramStart, paramEnd - paramStart );

			// An explanation must exist!
			
			findString( "<expl>", "s" );
			explStart = getPosition();
			findString( "</expl>", "s" );
			explEnd = getPosition() - 7;
			paramExpl = getText( explStart, explEnd - explStart );

			functionParam = functionParam + [[paramName, paramExpl]];
		}

		// Find function description
		
		jumpToPosition( startPos );

		findString( "<descr>", "s" );
		descrStart = getPosition();
		findString( "</descr>", "s" );
		descrEnd = getPosition() - 8;
		
		functionDescr = getText( descrStart, descrEnd - descrStart );

		findString( "<retval>", "s" );
		retvalStart = getPosition();
		findString( "</retval>", "s" );
		retvalEnd = getPosition() - 9;
		
		retval = getText( retvalStart, retvalEnd - retvalStart );

		if( functionName[0] != stringToChar(lastChar) )
		{
      if( lastChar != ' ' )
        treeIndex @= "           </span>\n";

			lastChar = charToString(functionName[0]);
      newCh = charToString( functionName[0] + (65 - 97) );
			indexList = indexList + ["<B>" @ newCh @ "</B>" ];

			print( newCh );

      treeIndex = treeIndex @
        "         <div class=\"trigger\" onClick=\"showBranch('" @ newCh @ "');swapFolder('folder" @ newCh @ "')\">\n" @
	      "         <img src=\"closed.gif\" border=\"0\" id=\"folder" @ newCh @ "\">\n" @
        "         " @ newCh @ "<br>\n" @
        "         </div>\n" @
	      "           <span class=\"branch\" id=\"" @ newCh @ "\">\n";
		}

		indexList = indexList + ["<a href='#" @ functionName @ "'>" @ functionName @
                            "</a>"];

    treeIndex @= 
		  "              <img src=\"doc.gif\"><a href=\"kfunref.html#" @ functionName @ "\" target=\"main\">" @ functionName @ "</a><br>\n";

    resString = resString @ "<HR>\n";
	  resString = resString @ "<a name='" @ functionName @ "'>\n";
	  resString = resString @ "<TABLE width='100%'>\n";
    resString = resString @ "  <TR valign=top>\n";
    resString = resString @ "    <TD width = '30%'>\n";
		resString = resString @ "<B><I>" @ functionName @ "</I></B>\n";
    resString = resString @ "    </TD>\n";
    resString = resString @ "    <TD width = '70%'>\n";
    resString = resString @ "      <TABLE width='100%'>\n";
    resString = resString @ "        <TR valign=top>\n";
    resString = resString @ "          <TD>\n";
		resString = resString @ "<B>Description:</B><BR>\n" @ functionDescr @ "<BR>";
    resString = resString @ "          </TD>\n";
    resString = resString @ "        </TR>\n";
    resString = resString @ "        <TR valign=top>\n";
    resString = resString @ "          <TD>\n";
    resString = resString @ "            <B>Parameters</B><BR>\n";
    resString = resString @ "            <TABLE width='100%' cellspacing=10>\n";

    if( length(functionParam) == 0 )
    {
      resString = resString @ "            <TR valign=top>\n";
      resString = resString @ "              <TD width = '30%'>\n";
      resString = resString @ "              <I>none</I>";
      resString = resString @ "              </TD>\n";
      resString = resString @ "            </TR>\n";
    }

		for( i = 0; i < length( functionParam ); i++ )
    {
      resString = resString @ "            <TR valign=top>\n";
      resString = resString @ "              <TD width = '20%' align=right>\n";
      resString = resString @ "<I>" @ functionParam[i][0] @ "</I>";
      resString = resString @ "              </TD>\n";
      resString = resString @ "              <TD width = '80%'>\n";
      resString = resString @ functionParam[i][1];
      resString = resString @ "              </TD>\n";
      resString = resString @ "            </TR>\n";
		}

    resString = resString @ "            </TABLE>\n";
    resString = resString @ "          </TD>\n";
    resString = resString @ "        </TR>\n";
    resString = resString @ "        <TR valign=top>\n";
    resString = resString @ "          <TD>\n";
    resString = resString @ "            <B>Return value:</B><BR>\n";
		resString = resString @ retval;
    resString = resString @ "          </TD>\n";
    resString = resString @ "        </TR>\n";
    resString = resString @ "      </TABLE>\n";
    resString = resString @ "    </TD>\n";
    resString = resString @ "  </TR>\n";
    resString = resString @ "</TABLE>\n";

		jumpToPosition( endPos );
	}

  treeIndex @= "           </span>\n";

	indexTable = "<TABLE>\n";
	indexTable @= "<html>" @
                "<head>\n" @
                "  <title>K function reference</title>\n" @
                "  <link REL=STYLESHEET TYPE=\"text/css\" HREF=\"help.css\">\n" @
                "</head>\n" @
                "<body BGCOLOR=white>\n";


	cols = 4;
	indexLen = length(indexList);

	rows = indexLen / cols;

	if( rows * cols < indexLen ) rows++;

	for( i = 0; i < rows; i++ )
	{
		indexTable = indexTable @ "  <TR>\n";
		for( n = 0; n < cols; n++ )
		{
			if( n * rows+i < indexLen )
				indexTable = indexTable @ "    <TD>" @ indexList[n*rows+i] @ "</TD>\n";
			else
				indexTable = indexTable @ "    <TD></TD>\n";
		}
	}

	indexTable = indexTable @ "</TABLE>\n";

  indexTable @= resString;

  indexTable @= 
     "</body>\n" @
     "</html>\n";

//	copyTextToClipboard( indexTable @ resString );

	copyTextToClipboard( treeIndex );

	keepBlock( FALSE );

	id = loadFile( "kfunref.html", FALSE, FALSE );

	switchExecutingControl( id );
	switchToControl( id );

	clearAll();

	insertString( indexTable );

	id = loadFile( "treeidx.html", FALSE, FALSE );

	switchExecutingControl( id );
	switchToControl( id );

	clearAll();

	insertString( treeIndex );

  updateSetting( "persistentblocks", wasPersistentBlocks );
}

initModule()
{
  bindKey( "Ctrl+l", "makeFunctionHelp();" );
}
