	// Find function boundaries in the text

	keepBlock( TRUE );

  indexList = "";

  resString = "";

	functions = 0;
	indexList = ["<B>a</B>"];

	lastChar = "a";

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

//		print( "name: " @ nameStart @ " " @ nameEnd );

		functionName = getText( nameStart, nameEnd - nameStart );

		// Find function variables

		jumpToPosition( startPos );

		functionParam = [];

		while( findString( "<param>", "s" ) )
		{
			paramStart = getPosition();
			findString( "</param>", "s" );
			paramEnd = getPosition() - 8;
//			print( "param: " @ paramStart @ " " @ paramEnd );
			paramName = getText( paramStart, paramEnd - paramStart );

			// An explanation must exist!
			
			findString( "<expl>", "s" );
			explStart = getPosition();
			findString( "</expl>", "s" );
			explEnd = getPosition() - 7;
//			print( "expl: " @ explStart @ " " @ explEnd );
			paramExpl = getText( explStart, explEnd - explStart );

			functionParam = functionParam + [[paramName, paramExpl]];
		}

		// Find function description
		
		jumpToPosition( startPos );

		findString( "<descr>", "s" );
		descrStart = getPosition();
		findString( "</descr>", "s" );
		descrEnd = getPosition() - 8;
		
//		print( "descr: " @ descrStart @ " " @ descrEnd );

		functionDescr = getText( descrStart, descrEnd - descrStart );

		findString( "<retval>", "s" );
		retvalStart = getPosition();
		findString( "</retval>", "s" );
		retvalEnd = getPosition() - 9;
		
//		print( "descr: " @ descrStart @ " " @ descrEnd );

		retval = getText( retvalStart, retvalEnd - retvalStart );

		if( toChar(functionName[0]) != lastChar )
		{
			lastChar = toChar(functionName[0]);
			indexList = indexList + ["<B>" @ lastChar @ "</B>" ];
		}

		indexList = indexList + ["<a href='#" @ functionName @ "'>" @ functionName @
                            "</a>"];

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

		print( "Function: '" @ functionName @ "'  '" @ functionDescr @ "' '" @ retval @ "'");
		
		for( i = 0; i < length( functionParam ); i++ )
			print( "param: '" @ functionParam[i][0] @ "', '" @ functionParam[i][1] @ "'" );

		jumpToPosition( endPos );
	}

	indexTable = "<TABLE>\n";

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

	copyTextToClipboard( indexTable @ resString );

	keepBlock( FALSE );
