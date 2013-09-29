	functionEntry = "";

	while( findString( 'FunctionList.Update', '' ) )
	{
    pushPos();
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

    pushPos();

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

	copyTextToClipboard( functionEntry );

