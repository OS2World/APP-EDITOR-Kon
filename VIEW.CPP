#include "bowl.h"
#include "view.h"
#include "search.h"
#include "grep.h"
#include "globals.h"
#include "srchdlg.h"
#include "repldlg.h"
#include "cntrladm.h"
#include "mainwnd.h"

SearchOp PreviousSearchOperation::lastSearchOp = eUndef;
char PreviousSearchOperation::op[15];
char *PreviousSearchOperation::searchString = NULL;
char *PreviousSearchOperation::replaceString = NULL;

void PreviousSearchOperation::setNewOperation( SearchOp searchOp, char *options, 
                                               char *searchString,
                                               char *replaceString )
{
  lastSearchOp = searchOp;

  strcpy( op, options );

  if( PreviousSearchOperation::searchString != NULL )
    delete PreviousSearchOperation::searchString;

  PreviousSearchOperation::searchString = new char[strlen( searchString ) + 1];

  strcpy( PreviousSearchOperation::searchString, searchString );

  if( searchOp == eReplace )
  {
    if( PreviousSearchOperation::replaceString != NULL )
      delete PreviousSearchOperation::replaceString;
  
    PreviousSearchOperation::replaceString =
       new char[strlen( replaceString ) + 1];
  
    strcpy( PreviousSearchOperation::replaceString, replaceString );
  }
}

int View::PRINTER = 1;
int View::SCREEN = 2;

PreviousSearchOperation View::previousSearchOperation;

View::View( ViewWindowHolder *vwHolder, TextBuffer *textBuffer, InfoBar *infoBar ) :
  bufferLines(-1), defaultFont(NULL), currentLine(1), currentColumn(1),
  markBegin(-1), markEnd(-1), hlBegin(-1), hlEnd(-1), mlLineStart(-1),
  _keepBlock(0), relativePos(FALSE), positionViewChanged( FALSE),
	defaultFontPoints(-1), defaultFontX(-1), defaultFontY(-1),
	savedPosSorted(NULL), savedPositions(0)
{
	defaultFontName = new char[1];
	*defaultFontName = 0;
  this->vwHolder = vwHolder;
  this->textBuffer = textBuffer;
  this->infoBar = infoBar;
  userPosition = 0;
  textBuffer->addBufferObserver( this );
  textBuffer->getUndoBuffer()->addUndoObserver( this );
  for( int i = 0; i < 10; i++ )
  	savedPos[i] = -1;
}

void View::setHighlight( int begin, int end )
{
  if( vwHolder == NULL ) return;

  int oldBegin = hlBegin, oldEnd = hlEnd, oldMBegin = markBegin, oldMEnd = markEnd;

  hlBegin = begin;
  hlEnd = end;
  
  if( !Settings::persistentBlocks )
  {
    markBegin = begin;
    markEnd = end;
  }

  int reader = textBuffer->readRequest();

  if( oldBegin != -1 && vwHolder != NULL )
    vwHolder->updatePosition( this, reader, oldBegin, oldEnd );

  if( (markBegin != oldMBegin || markEnd != oldMEnd) && 
      oldBegin != oldMBegin && oldMEnd != oldEnd && vwHolder != NULL )
    vwHolder->updatePosition( this, reader, oldMBegin, oldMEnd );

  vwHolder->updatePosition( this, reader, hlBegin, hlEnd );

  textBuffer->readRelease();
}

void View::resetHighlight()
{
  int oldBegin = hlBegin, oldEnd = hlEnd;

  hlBegin = hlEnd = -1;

  if( oldBegin == -1 ) return;

  int reader = textBuffer->readRequest();

  vwHolder->updatePosition( this, reader, oldBegin, oldEnd );

  textBuffer->readRelease();
}

void View::setLineMark( int lineStart, int cursorPos )
{
  if( vwHolder == NULL ) return;

  int oldMl = mlLineStart;

  mlLineStart = lineStart;
  mlCursorPos = cursorPos;

  int reader = textBuffer->readRequest();

  if( oldMl != -1 && vwHolder != NULL )
    vwHolder->updatePosition( this, reader, oldMl, oldMl );

  vwHolder->updatePosition( this, reader, mlLineStart, mlLineStart );

  textBuffer->readRelease();
}

void View::resetLineMark()
{
  int oldMl = mlLineStart;

  mlLineStart = -1;

  if( oldMl == -1 ) return;

  int reader = textBuffer->readRequest();
  vwHolder->updatePosition( this, reader, oldMl, oldMl );
  textBuffer->readRelease();
}

void View::jumpToNumberedSavedPos( int mark )
{
	if( savedPos[mark] != -1 )
		jumpToPosition( savedPos[mark] );

  setLineMark( getLineStart(), getPosition() );
}

void View::saveNumberedPos( int mark )
{
	int reader, oldMark = savedPos[mark];

	savedPos[mark] = userPosition;

	if( Settings::showLeftMarginInfo )
	{
		reader = textBuffer->readRequest();

		if( oldMark != -1 )
			vwHolder->updatePosition( this, reader, oldMark, oldMark );

		vwHolder->updatePosition( this, reader, userPosition, userPosition );

		textBuffer->readRelease();
	}
}

void View::savePos()
{
	BOOL removed = FALSE;

  int i;
	for( i = 0; i < savedPositions; i++ )
		if( savedPosSorted[i] >= userPosition ) break;

	if( i < savedPositions )	// Don't trust optimizer..
		if( savedPosSorted[i] == userPosition ) 
		{
			// Remove the position
			memmove( &savedPosSorted[i], &savedPosSorted[i+1], 
			         sizeof(int) * (savedPositions - i - 1) );
			savedPositions--;
			removed = TRUE;
		}

	if( !removed )
	{
		savedPositions++;
		
		savedPosSorted = (int *)realloc( (void *)savedPosSorted, 
																		 sizeof( int ) * savedPositions );
		memmove( &savedPosSorted[i+1], &savedPosSorted[i], 
						 sizeof(int) * (savedPositions - i - 1) );

		savedPosSorted[i] = userPosition;
	}

	if( Settings::showLeftMarginInfo )
	{
		int reader = textBuffer->readRequest();

		vwHolder->updatePosition( this, reader, userPosition, userPosition );

		textBuffer->readRelease();
	}
}

void View::jumpToNextPos()
{
  if( savedPositions == 0 ) return;

  int i;
	for( i = 0; i < savedPositions; i++ )
		if( savedPosSorted[i] > userPosition ) break;
		
	if( i < savedPositions )
		jumpToPosition( savedPosSorted[i] );
	else
		jumpToPosition( savedPosSorted[0] );

  setLineMark( getLineStart(), getPosition() );
}

void View::jumpToPrevPos()
{
  if( savedPositions == 0 ) return;

  int i;
	for( i = savedPositions - 1; i >= 0; i-- )
		if( savedPosSorted[i] < userPosition ) break;
		
	if( i >= 0 )
		jumpToPosition( savedPosSorted[i] );
	else
		jumpToPosition( savedPosSorted[savedPositions-1] );

  setLineMark( getLineStart(), getPosition() );
}

int View::hasSavedPositions()
{
  return savedPositions;
}

void View::clearSavedPositions()
{
  savedPositions = 0;
}

void View::markModified()
{
	if( vwHolder == NULL ) return;

  char text[100];

  sprintf( text, "blockMarked( %d );", validMark() );

  vwHolder->addCommand( text );
}

void View::resetMark()
{
  int oldMarkBegin = markBegin, oldMarkEnd = markEnd;
	
	markBegin = markEnd = -1;

  if( oldMarkBegin == -1 ) return;

  int reader = textBuffer->readRequest();

  vwHolder->updatePosition( this, reader, oldMarkBegin, oldMarkEnd );
  textBuffer->readRelease();
}

BOOL View::validMark()
{
	return markBegin < markEnd && markBegin != -1 && markBegin != markEnd;
}

void View::undoAvailable( BOOL hasUndo, BOOL atUnmodified )
{
	if( vwHolder == NULL ) return;
  if( vwHolder->viewActive() && vwHolder->getView() == this )
  {
    // Notify that undo/redo state has changed

    if( hasUndo )
      vwHolder->addCommand( "undoAvailable( 1 );" );
    else
      vwHolder->addCommand( "undoAvailable( 0 );" );
		if( atUnmodified )
			textBuffer->setUnmodified( FALSE );
  }
}

void View::redoAvailable( BOOL hasRedo, BOOL atUnmodified  )
{
	if( vwHolder == NULL ) return;
  if( vwHolder->viewActive() && vwHolder->getView() == this )
  {
    // Notify that undo/redo state has changed

    if( hasRedo )
      vwHolder->addCommand( "redoAvailable( 1 );" );
    else
      vwHolder->addCommand( "redoAvailable( 0 );" );

		if( atUnmodified )
			textBuffer->setUnmodified( FALSE );
  }
}

char *View::translateHexCodes( char *hexCodes, int &len )
{
  char *xlat = new char[strlen(hexCodes)];
  len = 0;
  
  // No errorchecking!
  while( *hexCodes )
  {
    while( *hexCodes == 0x20 ) hexCodes++;
    
    int digit1 = -1, digit2;
    
    if( *hexCodes ) 
      digit1 = *hexCodes++;
    
    if( *hexCodes && *hexCodes != ' ')
    {
      digit2 = *hexCodes;
      hexCodes++;
    }
    else
    {
      digit2 = digit1;  // No mote characters or only one digit
      digit1 = '0';
    }      
    
    if( digit2 != -1 )
    {
      if( digit1 >= '0' && digit1 <= '9' )
        digit1 = digit1 - '0';
      else
        digit1 = toupper( digit1 )- 'A' + 10;  

      if( digit2 >= '0' && digit2 <= '9' )
        digit2 = digit2 - '0';
      else
        digit2 = toupper( digit2 )- 'A' + 10;  
        
      unsigned char character = digit2 | (digit1 << 4);
      *(xlat + len++) = character;
    }
  }
  
  *(xlat + len) = 0;  // Not really necessary
  
  return xlat;
}

BOOL View::findString( char *text, char *op, BOOL repeat, char **group[], int *groups )
{
  int reader = textBuffer->readRequest();
  int start, end, position;

	if( vwHolder != NULL )
		vwHolder->userAsynchClearMessage();

  if( strchr( op, 's' ) )
  {
    start = markBegin;
    end = markEnd;
  }
  else
  {
    start = 0;
    end = textBuffer->getSize();
  }

  if( strchr( op, 'd' ) && !repeat)
  {
    if( !strchr( op, 'b' ) )
      position = start;
    else
      position = end;
  }
  else
    position = userPosition;

  char *searchText = text;
  int result, len = strlen( searchText );

  if( strchr( op, 'h' ) )
     searchText = translateHexCodes( text, len );
  
  if( strchr( op, 'g' ) )
  {
    Grep *grep = new Grep( searchText, Settings::standardDelim, (BOOL)!strchr( op, 'u' ),
                           (BOOL)strchr( op, 'w' ), (BOOL)strchr( op, 'x' ) );

    if( grep->parseSuccess() )
    {
      start = position;
      result = grep->findForward( reader, textBuffer, NULL, start, end );
      if( group != NULL )
      {
        if( result )
        {
          *group = (char **)malloc( sizeof( char* ) * grep->getParens());
          *groups = grep->getParens();
          for( int i = 0; i < *groups; i++ )
          {
            (*group)[i] = new char[grep->getParenEnd(i) - grep->getParenStart(i) + 1];
            int s;
            for( s = 0; s < grep->getParenEnd(i) - grep->getParenStart(i); s++ )
              (*group)[i][s] = textBuffer->getChar( reader, s + grep->getParenStart(i) );
            (*group)[i][s] = 0;  
          }
        }
        else
        {
          *group = NULL;
          *groups = 0;
        }
      }
      position = end;
      len = end-start;
    }
    else
      result = FALSE;
    delete grep;
  }
  else
  {
    Search search( textBuffer, reader, searchText, len,
                   Settings::standardDelim, (BOOL)!strchr( op, 'u' ),
                   (BOOL)strchr( op, 'w' ), (BOOL)strchr( op, 'x' ) );

    if( !strchr( op, 'b' ) )
      result = search.findForward( position, len, start, end);
    else
      result = search.findBackward( position, len, start, end);
  }

  // Show the highlight
  if( result )
  {
    jumpToPosition( reader, position );

    start = (!strchr( op, 'b' )) ? position-len : position;
    end = (!strchr( op, 'b' )) ? position : position+len;

    setHighlight( start, end );
  }
	else
	if( vwHolder != NULL )
		vwHolder->userAsynchMessage(Strings::get(162));

  textBuffer->readRelease();

  if( strchr( op, 'h' ) ) delete searchText;

  return result;
}

BOOL View::replaceString( char *sText, char *rText, char *op, int &replacements,
                         BOOL multiFile, int &queryResult )
{
  int writer = textBuffer->writeRequest();
  int start, end, position;
  BOOL firstLoop = TRUE;

  replacements = 0;
  queryResult = QueryReplaceDialog::REPLACE;

  int len = strlen( sText ), rLen = strlen( rText );
  BOOL grepSearch = strchr( op, 'g' ) != NULL, result = TRUE;
  Search *search;
  Grep *grep;

  if( strchr( op, 'h' ) )
  {
     sText = translateHexCodes( sText, len );
     rText = translateHexCodes( rText, rLen );
  }     

	if( grepSearch )
  {
  	grep = new Grep( sText, Settings::standardDelim, (BOOL)!strchr( op, 'u' ),
      	             (BOOL)strchr( op, 'w' ), (BOOL)strchr( op, 'x' ) );

		if( !grep->parseSuccess() )
    	result = FALSE;
  }
  else
  {
    search = new Search( textBuffer, writer, sText, len,
                         Settings::standardDelim, (BOOL)!strchr( op, 'u' ),
                         (BOOL)strchr( op, 'w' ), (BOOL)strchr( op, 'x' ) );
  }

  BOOL replaceAll = (BOOL)strchr( op, 'a' ), 
       backwards = (BOOL)strchr( op, 'b' ),
       query = (BOOL)(strchr( op, 'n' ) == NULL);
  int lastPos = -1;

  textBuffer->startUndoRepeat();

  if( strchr( op, 's' ) )
  {
    start = markBegin;
    end = markEnd;
  }
  else
  {
    start = 0;
    end = textBuffer->getSize();
  }

  if( strchr( op, 'd' ) )
  {
    if( strchr( op, 'b' ) )
      position = end;
    else
      position = start;
  }
  else
    position = userPosition;
    
  int initialPos = position;

  while( result )
  {
    TextElem *foundText = sText;

	  if( grepSearch )
    {
      start = position;
      int stopPos = end;
      result = grep->findForward( writer, textBuffer, NULL, start, stopPos );
      position = stopPos;
      len = stopPos-start;

      foundText = new char[len+1];

      int i;
      for( i = 0; i < len; i++ )
        foundText[i] = textBuffer->getChar( writer, start + i );

      foundText[i] = 0;

      backwards = FALSE;
    }
    else
    {
			int lastPos = position;
			
      if( !strchr( op, 'b' ) )
        result = search->findForward( position, len, start, end);
      else
        result = search->findBackward( position, len, start, end);
        
			if( position < lastPos ) // Wrapped around. Allowed once
			{
			  if( !firstLoop ) result = 0;
				firstLoop = FALSE;
			}
			
			if( position - len >= initialPos && !firstLoop ) result = 0; // After wrap around, never search past original pos	
        
			if( !result )
			{
				result = 0;
			}
    }
        
    // Replace the text
  
    if( result )
    {
      int pos;

      if( backwards )
        pos = position;
      else
        pos = position - len;

      TextElem *replaceText = rText;
      int replaceLen = rLen;

      if( grepSearch )
      {
        // Replace paren references with found string portions
        int i;
        for( i = 0; i < grep->getParens(); i++ )
        {
          int inc = grep->getParenEnd(i) - grep->getParenStart(i);
          
          if( inc > 0 )
            replaceLen += inc;
        }          

        replaceText = new char[replaceLen+1];

        replaceLen = 0;

        for( i = 0; i < rLen; i++ )
        {
          if( rText[i] == '\\' )
          {
            if( rText[i+1] >= '1' && rText[i+1] <= '9' )
            {
              int p = rText[i+1] - '1';
              i++;

              // Don't care about non-existing parens...

              if( p < grep->getParens() )
              {
                for( int s = grep->getParenStart(p); 
                     s < grep->getParenEnd(p); s++ )
                  replaceText[replaceLen++] = 
                    textBuffer->getChar( writer, s );
              }
            }
            else if (rText[i+1] == 'x' )
            {
					    TextElem character;
						  if( rText[i+3] <= '9' )
							  character = rText[i+3]-'0';
						  else
							  character = toupper(rText[i+3])-'A'+10;

						  if( rText[i+2] <= '9' )
							  character += (rText[i+2]-'0') * 16;
						  else
							  character += (toupper(rText[i+2])-'A'+10)*16;
              replaceText[replaceLen++] = character;
              i += 3;
            }
            else
              replaceText[replaceLen++] = rText[i++];
          }
          else
            replaceText[replaceLen++] = rText[i];
        }
      }

    	// Query the user

      BOOL doReplace = TRUE;
        
			if( query )
      {
        replaceAll = TRUE;  // Always loop forever if query, until the user terminates
      	replaceText[replaceLen] = 0; // NULL terminate for the dialog
        if( vwHolder == NULL )
        {
          // We have no viewWindow attached. Return the offset of the found
          // text
          queryResult = -pos - 1;
          doReplace = FALSE;
          replaceAll = FALSE;
        }
        else
        {
          textBuffer->writeRelease();

          // Set the found text visible
  
          setHighlight( pos, pos+len );
  
          jumpToPosition( pos );
  
          vwHolder->setCursorVisible();
  
          int res = vwHolder->queryReplaceDialog( foundText, replaceText, pos, 
                                                  len, multiFile );
  
          if( res == QueryReplaceDialog::DONTREPLACE )
            doReplace = FALSE;
          else
          if( res == QueryReplaceDialog::REPLACEALL )
          {
            replaceAll = TRUE;
            query = FALSE;
          }
          else
          if( res == QueryReplaceDialog::CANCEL )
          {
            queryResult = res;
            doReplace = FALSE;
            replaceAll = FALSE; // Will break the loop
          }
          else
          if( res == QueryReplaceDialog::REPLACEINALLFILES )
          {
            queryResult = res;
            doReplace = TRUE;
            replaceAll = TRUE; // Will break the loop
            query = FALSE;
          }
          else
          if( res == QueryReplaceDialog::SKIPREST )
          {
            queryResult = res;
            doReplace = FALSE;
            replaceAll = FALSE; // Will break the loop
          }
          writer = textBuffer->writeRequest();
        }
      }

      if( doReplace )
      {
        replacements++;
        textBuffer->deleteText( writer, pos, len, TRUE, TRUE );
        textBuffer->insertText( writer, pos, replaceText, replaceLen, TRUE, TRUE );

				if( pos <= initialPos ) initialPos += replaceLen - len;
				end += replaceLen - len;

				if( replacements % 100 == 0 && vwHolder != NULL )
				{
					char str[100];
					sprintf( str, Strings::get(160), replacements );
					vwHolder->userAsynchMessage( str );
	      }
			}
      if( grepSearch && grep->getParens() != 0 )
        delete replaceText;

      if( foundText != sText )
        delete foundText;

      if( !backwards )
        position += (replaceLen-len);

      lastPos = position;
    }

    if( !replaceAll ) break; // Do not replace all
  }

  textBuffer->endUndoRepeat();

  if( !grepSearch )
    delete search;
  else
    delete grep;

  textBuffer->writeRelease();

  if( strchr( op, 'h' ) ) 
  {
    delete sText;
    delete rText;
  }

  // Show the highlight

  if( lastPos != -1 )
    jumpToPosition( lastPos );

	if( vwHolder != NULL )
	{
		char str[100];
		sprintf( str, Strings::get(161), replacements );
		vwHolder->userAsynchMessage( str );
	}

	return result;
}

TSyntax *View::eval( char *name, TVariableList *vars, TEval *eval, int index )
{
	switch( index )
	{
		case 40 : // lineDown
			return new TNumeric( lineDown(), COPY );
		case 41 : // lineUp
			return new TNumeric( lineUp(), COPY );
		case 42 : // charRight
			return new TNumeric( charRight(), COPY );
		case 43 : // charLeft
			return new TNumeric( charLeft(), COPY );
		case 46 : // toDocumentStart
			return new TNumeric( jumpToPosition(0), COPY );
		case 47 : // toDocumentEnd
			return new TNumeric( jumpToPosition(textBuffer->getSize()), COPY );
		case 48 : // toLineStart
		{
			toLineStart();
			return NULL;
		}
		case 49 : // toLineEnd
		{
			toLineEnd();
			return NULL;
		}
    // Flip screenLineDown/up. They have different meaning to the user interface
		case 78 : // beginMark
		{
      shiftStateSet(0);
			beginMark();
			return NULL;
		}
		case 79 : // endMark
		{
			endMark();
			return NULL;
		}
		case 53 : // undo
			return new TNumeric( undo(), COPY );
		case 54 : // redo
			return new TNumeric( redo(), COPY );
		case 55 : // pushPos
		{
			TSyntax **visible = vars->LookUp( "visible" );
			if( visible == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*visible)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			pushPos( ((TNumeric *)*visible)->num );
			return NULL;
		}
		case 56 : // popPos
		{
			popPos( TRUE );
			return NULL;
		}
		case 76 : // startUndoBlock
		{
			textBuffer->startUndoRepeat();
			return NULL;
		}
		case 77 : // endUndoBlock
		{
			textBuffer->endUndoRepeat();
			return NULL;
		}
		case 58 : // cutBlockToClipboard
		{
			cutBlockToClipboard();
			return NULL;
		}
		case 57 : // copyBlockToClipboard
		{
			copyBlockToClipboard();
			return NULL;
		}
		case 176 : // copyBlockToKClipboard
		{
      if( !validMark() ) return NULL;

      KonSystem::kClipboard = (char *)realloc( KonSystem::kClipboard, markEnd-markBegin );

      int reader = textBuffer->readRequest();

      int t = 0;
      int i;

      for( i = markBegin, t = 0; i < markEnd; i++, t++ )
        KonSystem::kClipboard[t] = textBuffer->getChar( reader, i );

      KonSystem::kClipboard[t] = 0;

      textBuffer->readRelease();
			
			return NULL;
		}
		case 33 : // resetMark
		{
			resetMark();
			return NULL;
		}
		case 59 : // pasteBlockFromClipboard
			return new TNumeric( pasteBlockFromClipboard(), COPY );
    case 177 : // pasteBlockFromKClipboard
			return new TNumeric( pasteBlockFromKClipboard(), COPY );
		case 52 : // deleteCharacter
		{
			deleteCharacter();
			return NULL;
		}
		case 95 : // deleteText
		{
			TSyntax **position = vars->LookUp( "position" ),
							**size = vars->LookUp( "size" );
			if( position == NULL || size == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric || (*size)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			int writer = textBuffer->writeRequest();

			textBuffer->deleteText( writer, ((TNumeric *)*position)->num, 
															((TNumeric *)*size)->num, TRUE, TRUE );
		
			textBuffer->writeRelease();
			return NULL;
		}
		case 98 : // jumpToPosition
		{
			TSyntax **position = vars->LookUp( "position" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			return new TNumeric( jumpToPosition( ((TNumeric *)*position)->num ), COPY );
		}
		case 99 : // jumpToLine
		{
			TSyntax **line = vars->LookUp( "line" );
			if( line == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*line)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			return new TNumeric( jumpToLine( ((TNumeric *)*line)->num ), COPY );
		}
		case 89 : // keepBlock
		{
			TSyntax **__keepBlock = vars->LookUp( "keep" );
			if( __keepBlock == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*__keepBlock)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			discardBlock( !((TNumeric *)*__keepBlock)->num );
			return NULL;
		}
		case 91 : // setNumberedMark
		{
			TSyntax **position = vars->LookUp( "markNum" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			saveNumberedPos( ((TNumeric *)*position)->num );
			return NULL;
		}
		case 92 : // jumpToNumberedMark
		{
			TSyntax **position = vars->LookUp( "markNum" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			jumpToNumberedSavedPos( ((TNumeric *)*position)->num );
			return NULL;
		}
		case 93 : // setScreenUpdate
		{
			TSyntax **position = vars->LookUp( "active" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			vwHolder->setUpdate( ((TNumeric *)*position)->num );

			return NULL;
		}
    case 164 :  // getScreenUpdate
    {
      return new TNumeric( vwHolder->getUpdate(), COPY );
    }
		case 100 : // setCursorColumn
		{
			TSyntax **position = vars->LookUp( "column" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			cursorToColumn( ((TNumeric *)*position)->num );
			return NULL;
		}
		case 173 : // setCursorColumnTmpDisVS
		{
			TSyntax **position = vars->LookUp( "column" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			cursorToColumn( ((TNumeric *)*position)->num, TRUE );
			return NULL;
		}
		case 83 : // getCursorColumn
			return new TNumeric( getColumn(), COPY );
		case 84 : // getBlockStart
			return new TNumeric( getMarkBegin(), COPY );
		case 85 : // getBlockEnd
			return new TNumeric( getMarkEnd(), COPY );
		case 86 : // hasValidBlockMark
			return new TNumeric( validMark(), COPY );
		case 102 : // getChar
		{
			TSyntax **position = vars->LookUp( "position" );
			if( position == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			return new TNumeric( getChar( ((TNumeric *)*position)->num ), COPY );
		}
		case 101 : // rGetChar
		{
			TSyntax **reader = vars->LookUp( "reader" );
			TSyntax **position = vars->LookUp( "position" );
			if( position == NULL || reader == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric || (*reader)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}
			return new TNumeric( getCharAt( ((TNumeric *)*reader)->num,
																			((TNumeric *)*position)->num ), COPY );
		}
		case 71 : // readRequest
			return new TNumeric( textRequest(), COPY );
		case 88 : // toggleRelativePos
		{
			relativePos = !relativePos;
			relativeStart = userPosition;
			positionViewChanged = TRUE;
			return NULL;
		}
		case 72 : // readRelease
		{
			textRelease();
			return NULL;
		}
		case 70 : // getLineStart
			return new TNumeric( getLineStart(), COPY );
		case 150 : // getLineEnd
			return new TNumeric( getLineEnd(), COPY );
		case 68 : // getPosition
			return new TNumeric( getPosition(), COPY );
		case 103 : // findString
		case 105 : // replaceString
		case 170 : // findStringGrep
		case 107 : // Repeat find
		{
      BOOL repeat = FALSE;
			if( index == 103 || index == 170 || 
			   (index == 107 && PreviousSearchOperation::lastSearchOp == eSearch) )
			{
				TextElem *text;
				char *op;
  			TSyntax **substrings;

				if( index == 103 || index == 170)
				{
					TSyntax **string = vars->LookUp( "string" );
					TSyntax **options = vars->LookUp( "options" );

					if( string == NULL || options == NULL )
					{
						eval->SendError( Strings::get(47), name );
						return NULL;
					}
					else
					if( (*string)->Type != SString || (*options)->Type != SString )
					{
						eval->SendError( Strings::get(51), name );
						return NULL;
					}
					text = ((TString *)*string)->string;

          if( index == 170 )
          {
					  substrings = vars->LookUp( "substrings" );
					  if( substrings == NULL )
					  {
						  eval->SendError( Strings::get(47), name );
						  return NULL;
					  }
					  else
					  if( (*substrings)->Type != SArray)
					  {
						  eval->SendError( Strings::get(51), name );
						  return NULL;
					  }

            ((TString *)*options)->Concat("g");
          }

					op = ((TString *)*options)->string;
					PreviousSearchOperation::setNewOperation( eSearch, op, text, NULL );
				}
				else
				{
          repeat = TRUE;
					text = PreviousSearchOperation::searchString;
					op = PreviousSearchOperation::op;
				}

        int result;

        if( index == 170 )
        {
          char **substring;
          int groups;
  				result = findString( text, op, repeat, &substring, &groups );

          for( int i = 0; i < groups; i++ )
          {
            ((TArray *)*substrings)->InsertElement( new TString( substring[i], ORIGINAL ), i );
            delete substring[i];
          }
          if( substring != NULL )
            free( substring );
        }
        else
  				result = findString( text, op, repeat );

				return new TNumeric( result, COPY );
			}
			else
			if( index == 105 || 
			   (index == 107 && PreviousSearchOperation::lastSearchOp == eReplace) )
			{
				TextElem *sText, *rText;
				char *op;

				if( index == 105 )
				{
					TSyntax **sString = vars->LookUp( "searchString" ),
									**rString = vars->LookUp( "replaceString" );
					TSyntax **options = vars->LookUp( "options" );

					if( sString == NULL || rString == NULL || options == NULL )
					{
						eval->SendError( Strings::get(47), name );
						return NULL;
					}
					else
					if( (*sString)->Type != SString || 
							(*rString)->Type != SString || (*options)->Type != SString )
					{
						eval->SendError( Strings::get(51), name );
						return NULL;
					}
					sText = ((TString *)*sString)->string;
					rText = ((TString *)*rString)->string;
					op = ((TString *)*options)->string;
					PreviousSearchOperation::setNewOperation( eReplace, op, sText, rText );
				}
				else
				{
					sText = PreviousSearchOperation::searchString;
					rText = PreviousSearchOperation::replaceString;
					op = PreviousSearchOperation::op;
				}

				int foo, replacements;
				
				int result = replaceString( sText, rText, op, replacements, FALSE, foo );

				return new TNumeric( result, COPY );
			}
			return NULL;
		}
		case 108 : // insertString
		{
			TSyntax **value = vars->LookUp( "string" );

			if( value == NULL )
				eval->SendError( Strings::get(47), name );
			else
			if( (*value)->Type == SString )
			{
				insertString( ((TString *)*value)->string );
				return NULL;
			}
			else
			if( (*value)->Type == SNumeric )
			{
				char str[11];
				sprintf( str, "%d", ((TNumeric *)*value)->num );
				insertString( str );
				return NULL;
			}
			else
				eval->SendError( Strings::get(51), name );
			return NULL;
		}
		case 112 : // getTextLength
			return new TNumeric( textBuffer->getSize(), COPY );
		case 32 : // blockMarked" ) ) 
			return NULL;
		case 30 : // undoAvailable 
			return NULL;
		case 31 : // redoAvailable" ) ) // Dummy
			return NULL;
		case 29 : // textModified" ) ) // Dummy
			return NULL;
		case 69 : // getLine
			return new TNumeric( getLine(), COPY );
		case 122 : // getFilename
			return new TString( textBuffer->getFilename(), COPY );
		case 134 : // getText
		{
			TSyntax **position = vars->LookUp( "position" ),
							**size = vars->LookUp( "size" );
			if( position == NULL || size == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric || (*size)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			int reader = textBuffer->readRequest();

      int startPos = ((TNumeric *)*position)->num;

      char *str = new char[((TNumeric *)*size)->num + 1];

      int i;
      for( i = 0; i < ((TNumeric *)*size)->num; i++ )
			  str[i] = textBuffer->getChar(reader, i + startPos );
      str[i] = 0;

			textBuffer->readRelease();
			TString *retStr = new TString( str, COPY );
      delete str;
      return retStr;
		}
		case 135 : // rGetText
		{
			TSyntax **reader = vars->LookUp( "reader" ),
              **position = vars->LookUp( "position" ),
							**size = vars->LookUp( "size" );
			if( position == NULL || size == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*position)->Type != SNumeric || (*size)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			int creader = ((TNumeric *)*reader)->num;

      int startPos = ((TNumeric *)*position)->num;

      char *str = new char[((TNumeric *)*size)->num + 1];

      int i;
      for( i = 0; i < ((TNumeric *)*size)->num; i++ )
			  str[i] = textBuffer->getChar(creader, i + startPos );
      str[i] = 0;

			TString *retStr = new TString( str, COPY );
      delete str;
      return retStr;
		}
		case 140 : // savePos
			savePos();
			return NULL;
		case 141 : // jumpToNextPos
			jumpToNextPos();
			return NULL;
		case 142 : // jumpToPrevPos
			jumpToPrevPos();
			return NULL;
    case 143 : // clearSavedPositions
      clearSavedPositions();
      return NULL;
		case 165 : // hasSavedPositions
			return new TNumeric( hasSavedPositions(), COPY );
			return NULL;
    case 148 : // deleteBlock
      deleteBlock();
      return NULL;
    case 159: // getEncoding
      return new TNumeric( textBuffer->getEncoding(), COPY );
    case 160 :
    {
			TSyntax **encodingv = vars->LookUp( "encoding" );
			if( encodingv == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			if( (*encodingv)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

			EncodingTypes encoding = (EncodingTypes)((TNumeric *)*encodingv)->num;

      clearSavedPositions();
      resetMark();
      jumpToPosition(0);
      textBuffer->writeRequest(FALSE);
      textBuffer->convertFromUnicode(encoding);
      textBuffer->writeRelease(FALSE);
      vwHolder->repaintView(NULL);
      return NULL;
    }
    case 168 :  // launchProcess
    {
			TSyntax **exefile = vars->LookUp( "exefile" ),
              **arguments = vars->LookUp( "arguments" ), 
              **capture = vars->LookUp("capture"),
              **controlId = vars->LookUp("controlId");

			if( exefile == NULL || capture == NULL || controlId == NULL || 
          arguments == NULL )
			{
				eval->SendError( Strings::get(47), name );
				return NULL;
			}
			else
			if( (*exefile)->Type != SString || (*arguments)->Type != SString || 
          (*capture)->Type != SNumeric || (*controlId)->Type != SNumeric )
			{
				eval->SendError( Strings::get(51), name );
				return NULL;
			}

      BOOL captureOutput = ((TNumeric *)*capture)->num;
      MessagePipe *pipe = NULL;
      int cid = ((TNumeric *)*controlId)->num;

      if( captureOutput )
      {
        if( ControlAdmin::queryControlWithId(cid) == NULL )
        {
				  eval->SendError( Strings::get(245), name );
          return NULL;
        }
        pipe = new MessagePipe();
      }

      char args[1000];
      strcpy( args, ((TString *)*arguments)->string );

      // Replace parameters
      char *found;
      while( (found = strstr( args, "%f")) != NULL )
      {
        memmove( found + strlen( textBuffer->getFilename() ), found + 2, strlen(found) );
        memcpy( found, textBuffer->getFilename(), strlen( textBuffer->getFilename() ) );
      }

      Launch *pt = new Launch( ((TString *)*exefile)->string, args, pipe );

      if( captureOutput )
      {
        CaptureThread *cap = new CaptureThread( pipe, cid );
        cap->start(16389);
      }

      pt->run();

      delete pt;

      return NULL;
    }
    case 149 :  // highlightCurrentLine
      setLineMark( getLineStart(), getPosition() );
      return NULL;
	 	default:
			return KCommonCommands::getInstance()->eval( name, vars, eval, index );
	}
}

BOOL View::undoredo( BOOL isUndo )
{
  Undo *undo;

  undo = textBuffer->getUndoBuffer();

  int writer = textBuffer->writeRequest();

  UndoPos *undoElem;

  if( isUndo )
    undoElem = undo->undo();
  else
    undoElem = undo->redo();

  BOOL retval = TRUE, oneUndo = FALSE;

  if( undoElem )
  {
    BOOL repeat = (undoElem == STARTREPEAT) || (undoElem == ENDREPEAT);
    UndoPos *repeatType = undoElem;
    int nesting = 1;

    do
    {
      if( undoElem != STARTREPEAT && undoElem != ENDREPEAT )
      {
        oneUndo = TRUE;
        if( (undoElem->type == DELETION && isUndo) ||
            (undoElem->type == INSERTION && !isUndo) )
        {
          textBuffer->insertTextNoUndo( writer, undoElem->position,
                                        undoElem->text, undoElem->length, TRUE, TRUE );
          jumpToPosition( writer, undoElem->position + undoElem->length );
        }
        else
        {
          textBuffer->deleteTextNoUndo( writer, undoElem->position,
                                        undoElem->length, TRUE, TRUE );
          jumpToPosition( writer, undoElem->position );
        }
      }

      if( repeat )
      {
        if( isUndo )
          undoElem = undo->undo();
        else
          undoElem = undo->redo();

        if( undoElem == ENDREPEAT && repeatType == STARTREPEAT ||
            undoElem == STARTREPEAT && repeatType == ENDREPEAT )
          nesting--;
        else
        if( undoElem == repeatType )
          nesting++;

        if( undoElem == NULL ) break;

        if( nesting == 0 )
        {
          if( !oneUndo )  // We skip this repetion and continue with the next
          {
            oneUndo = FALSE;

            if( isUndo )
              undoElem = undo->undo();
            else
              undoElem = undo->redo();            

            if( !undoElem ) break;

            repeat = (undoElem == STARTREPEAT) || (undoElem == ENDREPEAT);
            repeatType = undoElem;
            nesting = 1;
          }
          else
            break;
        }
      }
      else
        break;

    } while( TRUE );
  }
  else
    retval = FALSE;

  textBuffer->writeRelease();

	undo->reportUndoRedo();

  return retval;
}

void View::modified( BOOL isModified )
{
	int idx =	ControlAdmin::getControlIndex( (EditControl *)vwHolder );

  if( idx != -1 )
  {
  	ControlInfo *ctrlInfo = ControlAdmin::getControl( idx );
    if( ctrlInfo->parent )
  	  ctrlInfo->parent->showBufferModified( ctrlInfo->editControl, isModified );
  }
}

void View::filenameUpdated()
{
	int idx =	ControlAdmin::getControlIndex( (EditControl *)vwHolder );

  if( idx != -1 )
  {
  	ControlInfo *ctrlInfo = ControlAdmin::getControl( idx );
    if( ctrlInfo->parent )
    	ctrlInfo->parent->filenameUpdated( ctrlInfo->editControl );
  }
}

int View::saveFile()
{
  // Copy to bak-file

  if( Settings::backupFiles )
  {
  	char bakname[MAXPATH+1];

    strcpy( bakname, textBuffer->getFilename() );

    char *dot = strrchr( bakname, '.' );

    if( dot ) *dot = 0;

  	strcat( bakname, ".BAK" );
	
  	System::copyFile( textBuffer->getFilename(), bakname );
  }

  // Open the file

  BlockFileWriter *writer = new BlockFileWriter( textBuffer->getFilename() );

	if( writer->getStatus() != 0 ) return writer->getStatus();

  textBuffer->readRequest( NULL );
  textBuffer->beginEnum();

  writer->setFileSize( textBuffer->getSize() );

  int blockSize;

  TextElem *text = textBuffer->enumTextBlock( blockSize );

  if( textBuffer->getEncoding() != ANSI )
    writer->writeBlock( textBuffer->getLeadBytes(), textBuffer->getLeadBytesCount(), ANSI );

  while( text )
  {
    writer->writeBlock( (char *)text, blockSize, textBuffer->getEncoding() );
    text = textBuffer->enumTextBlock( blockSize );
  }

  textBuffer->readRelease();

  textBuffer->setUnmodified( TRUE );

  delete writer;

  BTime timeWritten;

  System::getFileTime( textBuffer->getFilename(), &timeWritten );
  textBuffer->setTimeWritten(&timeWritten );

  // Loop all controls with this buffer and notify the filename may have
  // been changed and a new syntax definition may be needed
  for( int i = 0; i < ControlAdmin::getControls(); i++ )
    if( ControlAdmin::getControl(i)->editControl->getTextBuffer() == textBuffer )
      ControlAdmin::getControl(i)->editControl->fileRenamed();
  return 0;
}

void View::getText( TextElem *str, int position, int len, int reader )
{
	int useReader = reader;
	
	if( reader == -1 ) useReader = textBuffer->readRequest( NULL );

	for( int i = 0; i < len; i++ )
		str[i] = textBuffer->getChar( useReader, position + i );

	if( reader == -1 ) textBuffer->readRelease();
}

BOOL View::pasteBlockFromKClipboard()
{
  if( KonSystem::kClipboard == NULL ) return 0;

  int writer = textBuffer->writeRequest();

  int len = strlen( KonSystem::kClipboard );

  if( len )
  {
    BOOL blockUndo = FALSE;

    if( !Settings::persistentBlocks && validMark() && !keepBlock() )
    {
  	  textBuffer->startUndoRepeat();
  	  blockUndo = TRUE;
  	  textBuffer->deleteText( writer, markBegin, markEnd-markBegin, TRUE, TRUE );
    }

    int oldMarkBegin = markBegin, oldMarkEnd = markEnd;
     textBuffer->insertText( writer, userPosition, KonSystem::kClipboard, len, TRUE, TRUE );

		if( Settings::persistentBlocks )
		{
	    markBegin = userPosition;
		  markEnd = userPosition + len;
		}
		else
			markBegin = markEnd = userPosition;

    vwHolder->updatePosition( this, writer, oldMarkBegin, oldMarkEnd );
    vwHolder->updatePosition( this, writer, userPosition, userPosition + len );

    jumpToPosition( writer, userPosition + len );

    markModified();

    if( blockUndo )
  	  textBuffer->endUndoRepeat();
  }

  textBuffer->writeRelease();

  return len != 0;
}
