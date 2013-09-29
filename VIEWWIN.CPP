#include "bowl.h"
#include "viewwin.h"
#include "editctrl.h"
#include "mainwnd.h"
#include "cntrladm.h"

int ViewWindow::UNDEF = -1000;
int ViewWindow::PAGE = 2;
int ViewWindow::LINES = 3;
int ViewWindow::UP = 1;
int ViewWindow::DOWN = 2;

// Private methods
// These are supposed to be called only from the main thread
void ViewWindow::pScreenToCursor()
{
  // See if we need to scroll horizontally
  view->updateGraphicCursor();

  int column = view->getCursorColumn(), foo;
  
  // If the column is on a highlight, then we set the column to move to 
  // so that the whole highlight is visible
  if( view->gethlBegin() >= view->getLineStart() && view->gethlEnd() <= view->getLineEnd() )
  {
    view->pushPos(FALSE);
    view->jumpToPosition(view->gethlBegin());
    int startCol = view->getCursorColumn();
    view->jumpToPosition(view->gethlEnd());
    int endCol = view->getCursorColumn();
    view->popPos(TRUE);
    
    if( startCol < leftMargin + leftMarginLock ) 
      column = startCol;
    
    if( endCol + view->getCursorWidth() > width + leftMargin ) 
      column = endCol;
  }    
  
  if( column < leftMargin + leftMarginLock )
    setHorzMargin( column - leftMarginLock );
  else
  if( column + view->getCursorWidth() > width + leftMargin )
    setHorzMargin( column - width + view->getCursorWidth() );

  lockLinePos();

  if( view->getLine() >= windowStartLine &&
      view->getLine() < windowStartLine + windowLines &&
      windowLines != -1 )
  {
    releaseLinePos();
    return;
  }

  // We need to scroll. See how many lines.
  if( view->getLine() > windowStartLine )
  {
    // We need to calculate how many lines we should scroll down
    // Find out how many pixels we should scroll. If this exceeds the height
    // of the screen, we just set the screenstart to the new line
    if( windowLines == -1 )
    {
      int reader = view->textRequest();
      calcLinePos( reader, FALSE, 0 );
      view->textRelease();
    }

    if( view->getLine() < windowStartLine + windowLines )
    {
      releaseLinePos();
      return;
    }

    Graphics *g = getGraphics();

    BOOL exceeds = TRUE;
    int pixelsToScroll = 0, lineStart = linePos[windowLines].lineStart,
        linesScrolled = 0;

    if( linePos[windowLines].yPos != UNDEF )
    {
      int reader = view->textRequest();

      while( pixelsToScroll < getHeight() )
      {
        pixelsToScroll += view->getTextlineHeight( reader, lineStart, g,
                                                   View::SCREEN );
        int lines = 1;
        lineStart = view->getNewLineStartFrom( reader, lineStart, lines, foo );
        linesScrolled++;
        if( windowStartLine + windowLines + linesScrolled > view->getLine() )
        {
          exceeds = FALSE;
          break;
        }
      }

      view->textRelease();

      if( !exceeds )
      {
        // Find out how many lines should be scrolled
        int screenLine = 0, undef = FALSE;

        while( pixelsToScroll > 0 )
        {
          if( linePos[screenLine].yPos == UNDEF )
          {
            undef = TRUE;
            break;
          }
          pixelsToScroll -= linePos[screenLine].height;
          screenLine++;
        }

        if( !undef )
        {
          // Scroll the screen
          releaseLinePos();
          int reader = view->textRequest();
          // If we're in Hex-view, force screen repaint in order to 
          // remove the cursor
          if( view->getViewType() == VIEW_HEXVIEW ) windowLines = -1;
          scrollUp( reader, screenLine );
          view->textRelease();
          return;
        }
      }
    }

    // We need to find out where the new lineStart should be
    int screenLine = view->getLine();
    lineStart = view->getLineStart();
    pixelsToScroll = 0;

    int reader = view->textRequest();

    while( pixelsToScroll < getHeight() && screenLine > 1 )
    {
      pixelsToScroll += view->getTextlineHeight( reader, lineStart, g,
                                                 View::SCREEN );
      int lines = -1;
      lineStart = view->getNewLineStartFrom( reader, lineStart, lines, foo );
      screenLine--;
    }

    view->textRelease();

    setScreenStart( screenLine + 2 );
  }
  else
  {
    // If we're in Hex-view, force screen repaint in order to 
    // remove the cursor
    if( view->getViewType() == VIEW_HEXVIEW ) windowLines = -1;
    setScreenStart( view->getLine() );
	}    

  releaseLinePos();
}

// FÖRENKLA! Räkna bara ut hur många rader som behövs
// flyttas upp eller ner för en skärmsida och flytta markören så att
// den ligger så nära föregående pixelrad på skärmen som möjligt.
// Scrolla inte skärmen, utan rita om hela!

void ViewWindow::pMovePage( int dir )
{
  lockLinePos();

  int windowLine = view->getLine() - windowStartLine,
      oldWindowStart = windowStart;
  int yPos = -1;

  if( windowLine >= 0 && windowLine < 300 && windowLines != -1 &&
      windowStartLine + windowLine - 1 < view->getTextlines() )
    yPos = linePos[windowLine].yPos;

  releaseLinePos();

  if( dir == DOWN )
    screenPageDown();
  else
    screenPageUp();

  int toLine;

  if( windowStart == oldWindowStart )
  {
    // Didn't move up or down, so move the cursor to top or bottom of text
    if( dir == UP )
      toLine = 1;
    else
      toLine = view->getTextlines();
  }
  else
  {
    // Calc the new number of lines on the screen

    int reader = view->textRequest();
    calcLinePos( reader, FALSE, 0 );
    view->textRelease();

    // Calc which line the cursor should be placed on

    toLine = windowStartLine;

    if( yPos != -1 )
    {
      for( int i = 0; i < windowLines; i++ )
      {
        if( linePos[i+1].yPos < yPos )
        {
          toLine = windowStartLine + i;
          break;
        }
      }
    }
  }

  view->jumpToLine( toLine );
}

// Public Methods
int ViewWindow::nextLine( int reader, int position, int &viewLine )
{
  int lines = 1;

  return view->getNewLineStartFrom( reader, position, lines, viewLine );
}

void ViewWindow::paint( Rect *rect, Graphics *g )
{
  if( view == NULL ) return;

	// Drawing in a minimized window on OS/2 freezes Kon
	ControlInfo *ctInfo;
	int index = ControlAdmin::getControlIndex( (EditControl *)viewHolder );
	
	if( index != -1 )
	{
		ctInfo = ControlAdmin::getControl( index );

		if( ctInfo != NULL )
			if( ctInfo->parent->isMinimized() ) return;
	}

	if( !isUpdating() )
	{
		g->setBackColor( Color::BKGR );
		g->fillRect( rect );
		return;
	}

	int reader;

	// Paint should never lock the main thread. Not very nice this one, but necessary
 	reader = view->textRequest( &linePosLock, 100 );
 	
  if( reader == -1 ) 
  {
		updateRectLock.request();
    updateRect.add( new UpdateRect( rect ) );
		updateRectLock.release();
   	return;
  }
  
  view->requestScreenBuffer();

  // Hide the view's cursor
  BOOL cursorWasVisible = cursorVisible;

  hideCursor();

  int stopPos = (rect == NULL) ? 0 : rect->y,
      startPos = (rect == NULL) ? getHeight() :
                                  rect->height + rect->y,
      yPos = getHeight(), stop = 0, position = windowStart,
      line = 0, viewLine = windowStartLine, foo = 1;

  BOOL countWindowLines = (windowLines == -1 || linePosIsInvalid );
  
  leftMarginLock = 0; // Always reset at each round

  visibleText = getWidth();

  while( stop < 2 )
  {
    int height = 0, maxBaseline = 0;

    if( yPos < 0 )
    {
      if( windowLines == -1 )
        windowLines = (line > 1) ? line - 1 : 1;
      stop++;
    }

    // Break the loop if we have reached the text end
    if( viewLine > view->getTextlines() )
    {
      if( windowLines == -1 )
      {
        windowLines = line;
        linePos[windowLines].yPos = linePos[windowLines+1].yPos = UNDEF;
      }
      break;
    }

    TextFont *textFont = NULL, *firstFont = NULL;
    BOOL hasInfo = FALSE;

    if( linePos[line].yPos == UNDEF || linePosIsInvalid )
    {
      linePos[line].lineStart = position;
      firstFont = textFont =
        view->getTextlineInfo( reader, position, viewLine, height, 
				                       maxBaseline, g,
                               View::SCREEN, getWidth() + getLeftMargin() );
      yPos -= height;
      linePos[line].yPos = yPos;
      linePos[line].height = height;
      hasInfo = TRUE;
    }
    else
    {
      position = linePos[line].lineStart;
      height = linePos[line].height;
      yPos = linePos[line].yPos;
    }

    line++;

    // Check if we are outside the bounding rect, then we don't have to
    // draw the text
    if( yPos + height <= stopPos )
    {
			if( firstFont != NULL ) delete firstFont;
      // Move position forward
      if( !hasInfo )
        position = nextLine( reader, position, viewLine );
      // If we should count the lines on the screen, don't break the loop
      if( countWindowLines )
        continue;
      else
        break;
    }

    if( yPos >= startPos )
    {
      // Move position forward
			if( firstFont != NULL ) delete firstFont;
      if( !hasInfo )
        position = nextLine( reader, position, viewLine );
      continue;
    }

    // We're inside the rect, draw the text

    if( !hasInfo )
      // The position was retrieved from the position list
      firstFont = textFont =
        view->getTextlineInfo( reader, position, viewLine, height, 
				                       maxBaseline, g,
                               View::SCREEN, getWidth() + getLeftMargin() );

    BOOL lastWasItalics = FALSE, lastWasFixed = FALSE;

    int lastX = 0;

    while( textFont )
    {
      if( textFont->X == -1 )
        g->selectFont( textFont->font, textFont->points, textFont->attr );
      else
        g->selectFont( textFont->font, textFont->X, textFont->Y,
                       textFont->attr );

      TextColor *textColor = textFont->textColor;

      while( textColor )
      {
        // Fill with the old color to the new pos if we have jumped forward
        // horizontally
        if( lastX != textColor->xPos )
        {
          int xStart = lastX - leftMargin, width = textColor->xPos - lastX;

          if( lastWasFixed )
            xStart = lastX;
          else
          if( xStart < leftMarginLock )
          {
            width += xStart;
            if( width < 0 ) width = 0;
            xStart = leftMarginLock;
          }

          Rect rect( xStart, yPos,
                     width, height );
          g->fillRect( &rect );
        }

        g->setColor( textColor->fg );
        g->setBackColor( textColor->bg );

        lastX = textColor->lastX;

        int lineLength = textColor->length;

        if( textColor->lastIsTab )
        {
          lastX = textColor->xBeforeTab;
          textColor->length--;
        }

        if( textColor->locked && textColor->lastX > leftMarginLock )
          leftMarginLock = textColor->lastX;

        lastWasFixed = textColor->locked;

        if( lastX > leftMargin || textColor->locked )
        {
          // We may have to  draw the text in a colored rectangle
          // to make sure all of the background is colored
          int baseline = (maxBaseline > textFont->baseline) ? maxBaseline :
                                                              textFont->baseline;
          int baselineDelta = baseline - textFont->baseline;

          if( g->queryStringBoxRecommended() || g->getFontHeight() != g->getMaxFontHeight() ||
              textColor->xPos - leftMargin < leftMarginLock )
          {
            int xPos = textColor->locked ? textColor->xPos : textColor->xPos - leftMargin,
                tWidth = textColor->locked ? lastX : lastX - leftMargin;

            int rectX = (!textColor->locked && xPos < leftMarginLock) ? 
                        leftMarginLock : xPos;

            tWidth -= rectX;

            if( tWidth > 0 )  // LastX may be before leftMarginLock
            {
              Rect trect( rectX, yPos, tWidth, height );

              g->drawString( xPos, yPos + baseline,
                             &trect, Graphics::fontOpaque,
                             lineLength, textColor->text );
            }
          }
          else
          {
            // If the last font wasn't italics and the current is, fill the
            // slant

            if( !lastWasItalics && (textFont->attr & Font::ITALIC) )
            {
              Rect trect( textColor->xPos - leftMargin, yPos,
                          g->getAverageCharWidth(), height );
              g->fillRect( &trect );
            }

            // Draw the string

            g->drawString( textColor->xPos - leftMargin, yPos + baseline,
                           lineLength, textColor->text );

            // Fill upper part if the height is smaller than the overall line
            // height, and fill lower part if the baseline is lifted

            if( textFont->height < height )
            {
              Rect trect( textColor->xPos - leftMargin,
                          yPos+textFont->height + baselineDelta,
                          lastX - leftMargin,
                          height-textFont->height-baselineDelta );
              g->fillRect( &trect );
              if( baselineDelta != 0 )
              {
                trect.y = yPos;
                trect.height = baselineDelta;
                g->fillRect( &trect );
              }
            }
          }
        }

        textColor = textColor->next;

        // Fill to right end of screen

        if( !textColor && !textFont->next && lastX - leftMargin < width )
        {
          int xStart = lastX - leftMargin;

          if( xStart < leftMarginLock ) xStart = leftMarginLock;

          Rect rect( xStart, yPos, width - xStart,
                     height );
          g->fillRect( &rect );
        }
      }

      lastWasItalics = (textFont->attr & Font::ITALIC);

			GraphicObject *go = textFont->graphicObject;

			while( go )
			{
				g->setColor( go->fg );

        switch( go->type )
        {
          case GR_LINE :
          {
            if( go->x-leftMargin > leftMarginLock )
				      g->drawLine( go->x-leftMargin, go->y+yPos, go->x+go->width-leftMargin, go->y + go->height + yPos );
            break;
          }
          case GR_SOFTEOL :
          {
            Point fig[10];
            fig[0].x = 1;
            fig[0].y = 0;
            fig[1].x = 1;
            fig[1].y = 2;

            fig[2].x = 0;
            fig[2].y = 1;
            fig[3].x = 3;
            fig[3].y = 1;

            fig[4].x = 3;
            fig[4].y = 1;
            fig[5].x = 3;
            fig[5].y = 3;

            fig[6].x = 0;
            fig[6].y = 1;
            fig[7].x = 1;
            fig[7].y = 2;

            fig[8].x = 0;
            fig[8].y = 1;
            fig[9].x = 1;
            fig[9].y = 0;

            int i;

            for( i = 0; i < 10; i++ )
            {
              fig[i].x = (fig[i].x * height) / 4;
              fig[i].y = (fig[i].y * height) / 4;
            }

            for( i = 0; i < 10; i += 2 )
            {
              if( go->x-leftMargin > leftMarginLock )
				        g->drawLine( go->x-leftMargin + fig[i].x, go->y+yPos + fig[i].y, go->x-leftMargin+ + fig[i+1].x, go->y+yPos+ + fig[i+1].y );
            }
          }
        }

				go = go->next;
			}

      textFont = textFont->next;
    }

    int thisLineLength = lastX;
    
    // If we haven't got the whole string since it was clipped by the
    // right margin, estimate the length of the remaining text
    if( thisLineLength > longestLineInText ) longestLineInText = thisLineLength;
    
    if( firstFont != NULL )
      delete firstFont;
  }
   
  updateScrollbars();

  // Fill the rect downwards
  if( yPos > rect->y )
  {
    g->setBackColor( Color::WHITE );
    rect->height = yPos - rect->y;
    g->fillRect( rect );
  }

  linePosIsInvalid = FALSE;

  if( cursorWasVisible )
    showCursor( TRUE, reader );
  view->textRelease();
  view->releaseScreenBuffer();
  linePosLock.release();
}

void ViewWindow::lockLinePos()
{
	// Will only work if the same thread is repeatedly calling this function
  // Add check!!!
  linePosLock.request();

  linePosLocked++;
}

void ViewWindow::releaseLinePos()
{
  linePosLock.release();
  linePosLocked--;
}

void ViewWindow::popupWindow( char *text )
{
  if( popupWin != NULL )
    delete popupWin;

  int x, y;
  
  getCursorPos( x, y );

	popupX = x;
	popupY = y;

  popupWin = new PopupWindow( this, text, x+1, y+1, TRUE );
  popupWin->setVisible( TRUE, FALSE );
}

void ViewWindow::updatePosition( int reader, int start, int end )
{
  lockLinePos();
  if( windowStart > end || hideUpdates ) { releaseLinePos(); return; }

  if( windowLines == -1 )
  {
    releaseLinePos();
    repaint( NULL );
    return;
  }

  // Find the lines

  int startLine = -1, endLine = -1;

  for( int i = 0; i <= windowLines; i++ )
  {
    if( linePos[i].yPos == UNDEF && startLine == -1 )
    {
      startLine = i-1 + windowStartLine;
      break;
    }

    if( start < linePos[i].lineStart && startLine == -1 )
      startLine = i-1 + windowStartLine;

    if( end < linePos[i].lineStart && endLine == -1 )
    {
      endLine = i-1 + windowStartLine;
      break;
    }
  }

  if( startLine == -1 )
  {
    releaseLinePos();
    return;
  }

  if( endLine == -1 )
    endLine = windowStartLine + windowLines;

  releaseLinePos();

  paintLines( reader, startLine, endLine );
}

int ViewWindow::getPositionVertPixel( int position, BOOL top )
{
	for( int i = 0; i <= windowLines; i++ )
  {
  	if( linePos[i].yPos == UNDEF && 
        windowStartLine + i <= view->getTextlines() )
    {
    	// The linePos wasn't calculated, recalculate all
	    int reader = view->textRequest();
			calcLinePos( reader, (i==0) ? FALSE : TRUE, i-1 );
      view->textRelease();
    }
    
		if( linePos[i].lineStart > position || 
        windowStartLine + i > view->getTextlines())
    {
    	// Position was on the previous line
      i--;
      return (top) ? linePos[i].yPos + linePos[i].height : linePos[i].yPos;
    }
  }
  return 0;	// This shouldn't happen if the precondition that the position
  					// is visible on screen is fullfilled
}

void ViewWindow::setUpdate( BOOL shouldUpdate ) 
{
  if( shouldUpdate ) 
    hideUpdates++;
  else
    hideUpdates--;

  if( hideUpdates == 0 )
  {
    if( refreshWindowWhenAfterUpdate )
  	  repaint( NULL );
    else
      screenToCursor();
  }
  else 
  if( hideUpdates == 1 ) // First hide
    refreshWindowWhenAfterUpdate = FALSE;
}

BOOL ViewWindow::getUpdate() 
{ 
	return hideUpdates == 0;
}

void ViewWindow::resetLineDefs()
{
  lockLinePos();
  windowLines = -1;
  //leftMarginLock = 0;
  linePosIsInvalid = TRUE;  // This is really unneccesary due to the above
  releaseLinePos();
}

void ViewWindow::removeLines( int reader, int fromLine, int lines )
{
  lockLinePos();

  if( lines == 0 || fromLine + lines < windowStartLine ||
      (windowLines != -1 && fromLine > windowStartLine + windowLines) )
  {
    releaseLinePos();
    return;
  }

  BOOL repaintView = TRUE;

  if( windowLines != -1 )
  {
    if( fromLine < windowStartLine )
    {
      lines -= (windowStartLine-fromLine);
      fromLine = windowStartLine;
    }

    if( fromLine - windowStartLine + lines > windowLines )
    {
      // Reset lines from fromLine and downwards

      if( fromLine == windowStartLine ||
          (fromLine > 1 && linePos[fromLine-windowStartLine-1].yPos == UNDEF) )
      {
        resetLineDefs();
        repaint( NULL );
      }
      else
      {
        calcLinePos( reader, TRUE, fromLine-windowStartLine+1 );
        Rect rect( 0, 0, width, linePos[fromLine-windowStartLine-1].yPos );
        repaint( &rect );
      }
      releaseLinePos();
      return;
    }

    int scrollPixels = 0, top, i = 0, line;

    // Do not scroll the line where the scroll starts. This should be updated
    // below this routine to disable flickering

    top = linePos[fromLine-windowStartLine].yPos;

    if( top != UNDEF )
      for( i = 0, line = fromLine - windowStartLine; i < lines;
           i++, line++ )
      {
        if( linePos[line].yPos == UNDEF ) break;
        scrollPixels += linePos[line].height;
      }

    if( i == lines )
    {
      memmove( &linePos[line-lines+1], &linePos[line+1],
               (windowLines-line+1) * sizeof( LinePos ) );

      for( i = fromLine-windowStartLine+1; i < windowLines-lines+2; i++ )
        linePos[i].yPos += scrollPixels;

      calcLinePos( reader, TRUE, windowLines - lines + 2 );
			
			processUpdatePositions( reader );
      scrollVert( scrollPixels, 0, top );
			
			// Update the line where the scroll started

			int sLine = fromLine-windowStartLine;

			Rect rect( 0, linePos[sLine].yPos, width, 
			           ((sLine==0) ? height : linePos[sLine-1].yPos) - 
								 linePos[sLine].yPos );

			repaint( &rect );

      repaintView = FALSE;
    }
  }

  releaseLinePos();

  if( repaintView )
  {
    resetLineDefs();
    repaint( NULL );
  }
}

void ViewWindow::getClosestStartingPoint( int &start, int &toLine,
                                          BOOL useUserPosition,
                                          BOOL useWindowStart )
{
  lockLinePos();
  int delta, newLine = 1;

  delta = abs(toLine - 1);

  start = 0;

  int nextDelta = abs( view->getLine() - toLine );

  if( useUserPosition && nextDelta < delta )
  {
    delta = nextDelta;
    start = view->getLineStart();
    newLine = view->getLine();
  }

  nextDelta = abs( windowStartLine - toLine );

  if( useWindowStart && nextDelta < delta )
  {
    delta = nextDelta;
    newLine = windowStartLine;
    start = windowStart;
  }

  nextDelta = abs( view->getTextlines() - toLine );

  if( nextDelta < delta )
  {
    newLine = view->getTextlines();
    start = view->getTextSize();
  }
  toLine = newLine;
  releaseLinePos();
}

void ViewWindow::moveStart( int reader, int &startPos, int &atLine, int lines )
{
  // Check to see from which position it's best to start counting

  int start, line = atLine + lines, foo;

  getClosestStartingPoint( start, line, TRUE, TRUE );

  atLine += lines;

  if( atLine > view->getTextlines() )
    atLine = view->getTextlines();

  lines = atLine - line;

  startPos = view->getNewLineStartFrom( reader, start, lines, foo );
}

void ViewWindow::scrollUp( int reader, int lines )
{
  lockLinePos();
  if( lines >= windowLines ) // Redraw the whole screen
  {
    moveStart( reader, windowStart, windowStartLine, lines );
    resetLineDefs();
    releaseLinePos();
    repaint( NULL );
    return;
  }

  if( lines + windowStartLine - 1 >= view->getTextlines() )
    lines = windowLines - 1;

  if( lines == 0 )
  {
    releaseLinePos();
    return;
  }

  int delta = 0;

  // See if the correct line has it's y pos calculated
  if( linePos[lines-1].yPos == UNDEF || linePos[lines].yPos == UNDEF ||
      windowLines == -1 )
  {
    // Get the height of all lines
    // This should normally not happen so we can afford this
    reader = view->textRequest();
    calcLinePos( reader, FALSE, 0 );
    view->textRelease();
  }

  delta = height - linePos[lines-1].yPos;

  windowStart = linePos[lines].lineStart;

  windowStartLine += lines;

  memmove( &linePos[0], &linePos[lines],
           sizeof( LinePos ) * (windowLines-lines+2) );

  for( int i = 0; i < windowLines - lines+2; i++ )
    if( linePos[i].yPos != UNDEF )
      linePos[i].yPos += delta;

  reader = view->textRequest();
  calcLinePos( reader, TRUE, windowLines - lines + 2 ); // We have two extra lines
	processUpdatePositions( reader );
  view->textRelease();

  scrollVert( delta, 0, height );
  releaseLinePos();
}

void ViewWindow::scrollDown( int reader, int lines, int fromLine, int type )
{
  int scrollPixels = 0, topPixel = 0, foo;

  if( type == LINES )
  {
    // Check that we don't move past the window start
    if( fromLine == 0 && windowStartLine - lines < 1 )
      lines = windowStartLine - 1;

    if( lines == 0 ) return;

    lockLinePos();

    if( windowLines == -1 )
    {
      if( fromLine == 0 )
      {
        int lns = -lines;
        moveStart( reader, windowStart, windowStartLine, lns );
      }
      resetLineDefs();
      releaseLinePos();
      repaint( NULL );
      return;
    }

    if( fromLine != 0 && linePos[fromLine-1].yPos == UNDEF )
    {
      // We need to know from which pixel we should scroll, but we don't.
      // Redraw the whole screen.
      resetLineDefs();
      releaseLinePos();
      repaint( NULL );
      return;
    }

    topPixel = (fromLine == 0) ? height : linePos[fromLine-1].yPos - 1;

    // Move the lines downwards in the list
    int moveLines = windowLines-fromLine+2; // -1????

    // We shouldn't move beyond the linePos array's size.
    if( moveLines+fromLine+lines-1 >= 300 )
      moveLines = 0;

    int lineStart;

    if( fromLine == 0 )
    {
      int lns = -lines;
      moveStart( reader, windowStart, windowStartLine, lns );
      lineStart = windowStart;
    }
    else
      lineStart = linePos[fromLine-1].lineStart;

    // Push lines down in the linePos array

    if( moveLines )
      memmove( &linePos[lines+fromLine], &linePos[fromLine],
               sizeof( LinePos ) * moveLines );

    // Calculate the new linestarts for each of the lines and the total
    // height to scroll
    int stop = 0;

    int i;
    for( i = fromLine; i < lines+fromLine; i++ )
    {
      linePos[i].yPos = UNDEF;
      linePos[i].lineStart = lineStart;
      int lineHeight = view->getTextlineHeight( reader, lineStart,
                                                getGraphics(), View::SCREEN );
      scrollPixels += lineHeight;

      // Don't scroll more than necessary
      if( topPixel - scrollPixels < 0 ) stop++;

      lineStart = nextLine( reader, lineStart, foo );
      if( i >= 300 || stop > 2 )
        break;
    }

    if( moveLines && stop <= 2 )
    {
      for( i = lines+fromLine; i <= fromLine+lines+moveLines; i++ )
        linePos[i].yPos -= scrollPixels;
    }
    else
      linePosIsInvalid = TRUE;

    windowLines = -1;   
  }
  else
  {
    lockLinePos();

    // Scroll upwards until we have moved one screen height
    int lineStart = windowStart, linesMoved = 0, lineHeight = 0;

    while( 1 )
    {
      int linesMove = -1;
      int lastLineStart = lineStart, foo;

      lineStart = view->getNewLineStartFrom( reader, lineStart, linesMove, foo );

      if( lineStart == lastLineStart ) break;

      lineHeight = view->getTextlineHeight( reader, lineStart, getGraphics(),
                                                View::SCREEN );
      if( scrollPixels + lineHeight < height )
      {
        scrollPixels += lineHeight;
        linesMoved++;
      }
      else
      {
        lineStart = lastLineStart;
        break;
      }
    }

    if( linesMoved > 0 )
    {
      windowStartLine -= linesMoved;
      windowStart = lineStart;

      if( linesMoved < windowLines )
      {
        memmove( &linePos[linesMoved], &linePos[0], sizeof( LinePos ) *
                 windowLines + 2 );
        int i;
        for( i = 0; i < linesMoved; i++ )
        {
          linePos[i].yPos = UNDEF;
          linePos[i].lineStart = lineStart;
          int linesMove = 1;
          lineStart = view->getNewLineStartFrom( reader, lineStart, linesMove, foo );
        }
        for( i = linesMoved; i < windowLines + linesMoved+2; i++ )
          linePos[i].yPos -= scrollPixels;
        windowLines = -1;
      }
      else
        resetLineDefs();
      topPixel = height;
    }
    else
    {
      releaseLinePos();
      return;
    }
  }

  if( scrollPixels >= getHeight() && topPixel == getHeight() )
  {
    resetLineDefs();
    repaint(NULL);
  }
  else
  {
		processUpdatePositions( reader );
    scrollVert( -scrollPixels, 0, topPixel );
  }

  releaseLinePos();
}

void ViewWindow::screenLineDown()
{
  if( windowStartLine < view->getTextlines() )
  {
    int reader = view->textRequest();
    scrollUp( reader, 1 );
    view->textRelease();
  }
}

void ViewWindow::screenLineUp()
{
  // See that we do not try to scroll past the filestart

  if( windowStartLine > 1 )
  {
    int reader = view->textRequest();
    scrollDown( reader, 1, 0, LINES );
    view->textRelease();
  }
}

void ViewWindow::screenLinesDown( int lines )
{
  if( windowStartLine < view->getTextlines() )
  {
    int reader = view->textRequest();
    scrollUp( reader, lines );
    view->textRelease();
  }
}

void ViewWindow::screenLinesUp( int lines )
{
  // See that we do not try to scroll past the filestart

  if( windowStartLine > 1 )
  {
    int reader = view->textRequest();
    scrollDown( reader, lines, 0, LINES );
    view->textRelease();
  }
}

void ViewWindow::screenPageUp()
{
  int reader = view->textRequest();
  scrollDown( reader, 1, 0, PAGE );
  view->textRelease();
}

void ViewWindow::screenPageDown()
{
  int reader = view->textRequest();
  scrollUp( reader, windowLines );
  view->textRelease();
}

void ViewWindow::setScreenStart( int line )
{
  if( line < 1 ) line = 1;
  if( line > view->getTextlines() )
    line = view->getTextlines();

  int delta = line - windowStartLine;

  int reader = view->textRequest();

  if( delta < 0 )
    scrollDown( reader, -delta, 0, LINES );
  else
    scrollUp( reader, delta );

  view->textRelease();
}

void ViewWindow::showCursor( BOOL linePosLocked, int reader )
{
  cursorVisible = TRUE;
  if( view == NULL ) return;

  if( !linePosLocked )
    lockLinePos();

  // Query the view for the current line and horizontal pixel position

  int columnXPixel = view->getCursorColumn();

  int line = view->getLine();

  if( line < windowStartLine || line > windowStartLine + windowLines )
  {
    if( !linePosLocked )
      releaseLinePos();
    return;
  }

  if( view->getViewType() == VIEW_ASCIIVIEW && !Settings::virtualSpace )  // For virtual space
    view->updateGraphicCursor();

  setCursor( columnXPixel - leftMargin, linePos[line-windowStartLine].yPos,
             (reader == -1) ? view->getCursorWidth() : view->getCursorWidth(reader), linePos[line-windowStartLine].height );
  if( !linePosLocked )
    releaseLinePos();
}

void ViewWindow::screenToCursor()
{
	Application::executeFromMainThread(this, MOVE_SCREEN_TO_CURSOR, NULL );
}

void ViewWindow::cursorToScreen()
{
  if( view == NULL ) return;

  if( view->getLine() < windowStartLine )
  {
    view->jumpToLine( windowStartLine );
  }
  else if( view->getLine() >= windowStartLine + windowLines - 1 )
  {
    view->jumpToLine( windowStartLine + windowLines - 1 );
  }
}

BOOL ViewWindow::actionPerformed( AMouseEvent *mouseEvent )
{
  if( view == NULL ) return TRUE;

  if( mouseEvent->getEvent() == AMouseEvent::WHEEL )
  {
    if( mouseEvent->getPos()->y < 0 )
      screenLinesDown( -mouseEvent->getPos()->y );
    else
      screenLinesUp( mouseEvent->getPos()->y );
    return TRUE;
  }

  int line, position;
  BOOL setCursor = (mouseEvent->getEvent() == AMouseEvent::CLICK || 
	                  mouseEvent->getEvent() == AMouseEvent::DOUBLECLICK);
  BOOL menuButton = (mouseEvent->getButton() == AMouseEvent::RIGHT &&
                     (mouseEvent->getEvent() == AMouseEvent::RELEASE));
	BOOL leftButton = (mouseEvent->getButton() == AMouseEvent::LEFT ||
  									 (mouseButtonDown && 
                      mouseButton == AMouseEvent::LEFT));
  BOOL err = FALSE;

	if( mouseEvent->getEvent() == AMouseEvent::MOVE && popupWin != NULL )
	{
    if( abs(mouseEvent->getPos()->x - popupX) > 10 || abs(mouseEvent->getPos()->y - popupY) > 10 )
    {
      delete popupWin;
      popupWin = NULL;
    }
  }

  if( mouseEvent->getEvent() == AMouseEvent::DOWN )
  {
    mouseButtonDown = TRUE;
  	mouseButton = mouseEvent->getButton();
	}

  if( mouseEvent->getEvent() == AMouseEvent::RELEASE )
	  mouseButtonDown = FALSE;

  // Special action if outside window border (scroll)

  if( setCursor || mouseButtonDown || menuButton )
  {
		if( view->inCommand() )
    {
    	setFocus();
      return TRUE;
    }

    int reader = view->textRequest( &linePosLock );

    if( leftButton )
    {
      if( (mouseEvent->getEvent() == AMouseEvent::DOWN || setCursor) && !(System::getKeyState() & KeyDef::SHIFT) )
      {
        view->setMark();
      }
      else
      {
        // Save cursor position
        view->beginMark();
      }
 		}

    setFocus();
    viewHolder->setActive(TRUE);
    int i = 0;

    // Find out which line the cursor should be placed on

    if( mouseEvent->getPos()->y > height || mouseEvent->getPos()->y < 0 )
    {
      int average = height / windowLines;

      if( mouseEvent->getPos()->y > height )
        i = -(mouseEvent->getPos()->y - height) / average;
      else
        i = -mouseEvent->getPos()->y / average + windowLines;
    }
    else
    for( i = 0; i <= windowLines &&
                    i + windowStartLine < view->getTextlines(); i++ )
    {
      if( linePos[i].yPos == UNDEF )  // This should not happen
      {
        err = TRUE;
        break;
      }

      if( mouseEvent->getPos()->y >= linePos[i].yPos )
        break;
    }

    //releaseLinePos();

    if( !err )
    {
    	line = i + windowStartLine;

      if( line < 1 )
        line = 1;
      else
      if( line > view->getTextlines() )
        line = view->getTextlines();

			if( menuButton  )
      {
      	view->pushPos( FALSE );
        view->jumpToLine( line );
        view->setCursorColumn( mouseEvent->getPos()->x + leftMargin, TRUE, FALSE );
        position = view->getPosition();

				// Check if within blockmarks, and non-persistent block, then do not move the cursor

				BOOL moveCursor = (!Settings::persistentBlocks && 
											     (position < view->getMarkBegin() || 
											      position > view->getMarkEnd()) ) || Settings::persistentBlocks;

        view->popPos( !moveCursor );

				if( moveCursor )
				{
      		pScreenToCursor();
      		showCursor( FALSE, reader );	
				}
			}
      else
      if( leftButton )
      {
        view->jumpToLine( line );
        // Find the column
        view->setCursorColumn( mouseEvent->getPos()->x + leftMargin, TRUE, !setCursor );
      	pScreenToCursor();
      	showCursor( FALSE, reader );

        if( (mouseEvent->getEvent() == AMouseEvent::DOWN || setCursor) && !(System::getKeyState() & KeyDef::SHIFT) )
        	view->beginMark();
        else
          view->shiftStateSet( System::getKeyState() );

      	view->endMark();
		  }

      // Show position in the infobar

      char str[30];

      view->getPositionString( str );

      if( infoBar != NULL )
				infoBar->updatePosition( str );

      performPositionEvent( view->getPosition() );        
    }
    view->textRelease();
    linePosLock.release();
  }

	char cmd[100];

	// Check if this was a double click: Then call mouseDoubleClick

	if( mouseEvent->getEvent() == AMouseEvent::DOUBLECLICK )
	{
    sprintf( cmd, "mouseDoubleClick();" );
  	viewHolder->addCommand( cmd );
	}

	// Triple click

	if( mouseEvent->getEvent() == AMouseEvent::TRIPLECLICK )
	{
    sprintf( cmd, "mouseTripleClick();" );
  	viewHolder->addCommand( cmd );
	}

 	// Check if this was the right button. If so call the popup menu function

	if( menuButton && !err)
	{
    sprintf( cmd, "buildPopupMenu( %d, %d, %d, %d );", viewHolder, position, 
             mouseEvent->getPos()->x, mouseEvent->getPos()->y );
  	viewHolder->addCommand( cmd );
  }
  return TRUE;
}

void ViewWindow::movePage( int dir )
{
	Application::executeFromMainThread(this, MOVE_PAGE, (void *)dir );
}

void ViewWindow::calcLinePos( int reader, BOOL useExisting, int toLine )
{
  if( !isUpdating() ) return;

  lockLinePos();

  int line = 0, position = windowStart, yPos = getHeight(), stop = 0, foo;

  while( stop < 2 )
  {
    if( yPos < 0 )
    {
      if( stop == 0 )
        windowLines = (line > 1) ? line - 1 : 1;
      stop++;
    }

    if( line + windowStartLine > view->getTextlines() && stop == 0 )
    {
      windowLines = line;
      linePos[windowLines].yPos = linePos[windowLines+1].yPos = UNDEF;
      break;
    }

    if( useExisting && line < toLine && linePos[line].yPos != UNDEF )
    {
      yPos = linePos[line].yPos;
      // Get next linestart
      if( useExisting && line+1 < toLine && linePos[line+1].yPos != UNDEF )
        position = linePos[line+1].lineStart;
      else
        position = nextLine( reader, position, foo );
    }
    else
    {
      linePos[line].lineStart = position;
      int lineHeight = view->getTextlineHeight( reader, position,
                                                getGraphics(), View::SCREEN );
      position = nextLine( reader, position, foo );
      yPos -= lineHeight;
      linePos[line].yPos = yPos;
      linePos[line].height = lineHeight;
    }
    line++;
  }

  linePosIsInvalid = FALSE;
  releaseLinePos();
}

void ViewWindow::updateScrollbars()
{
  if( view == NULL ) return;

  oldLeftMargin = leftMargin;
  viewHolder->setHorzScrollbar( leftMargin, visibleText, longestLineInText );

  if( oldTextLines != view->getTextlines() ||
      oldWindowStartLine != windowStartLine ||
      oldWindowLines != windowLines )
  {
		if( windowLines == -1 )
		{
			int reader = view->textRequest();
			calcLinePos(reader, FALSE, 0);
			view->textRelease();
		}			

    oldTextLines = view->getTextlines();
    oldWindowStartLine = windowStartLine;
    oldWindowLines = windowLines;

    viewHolder->setVertScrollbar( windowStartLine, windowLines,
                                  view->getTextlines() );
  }
}

void ViewWindow::insertion( TextBuffer *textBuffer, int reader, int position,
                            int size, int lines )
{
  refreshWindowWhenAfterUpdate = TRUE;
  lockLinePos();
  if( windowLines == -1 && isUpdating() )
  {
    linePosIsInvalid = TRUE;
    repaint( NULL );
    updatePos.removeAll();
    releaseLinePos();
    return;
  }

  if( position < windowStart )
  {
    if( isUpdating() )
    {
      for( int l = 0; l < windowLines + 2; l++ )
        linePos[l].lineStart += size;
    }
    else
      resetLineDefs();

    windowStart += size;
    if( !view->queryUpdateAllOnModification() )
      windowStartLine += lines;
    else
      windowStartLine = view->getLineFromPosition( windowStart );
    releaseLinePos();
    processUpdatePositions( reader );
    return;
  }

  if( !isUpdating() )
  {
    resetLineDefs();
    releaseLinePos();
    processUpdatePositions( reader );
    return;
  }

  // See on which line the update started

  for( int i = 0; i < windowLines + 2; i++ )
  {
    if( linePos[i].yPos == UNDEF )
    {
      if( i >= windowLines ) break;
      // Skip the whole thing
      linePosIsInvalid  = TRUE;
      releaseLinePos();
      processUpdatePositions( reader );
      return;
    }

    if( linePos[i].lineStart > position )
    {
      if( !view->queryUpdateAllOnModification() )
      {
        // The position was on the previous line
        // Push all linepos forward size chars forward
        for( int l = i; l < windowLines + 2; l++ )
          linePos[l].lineStart += size;

        // Insert the number of lines
				if( lines > 0 && Settings::showLeftMarginInfo )
				{
					// In order to have the line numbers correctly set, we need to 
					// redraw downwards

					Rect rect;
					
					rect.x = 0;
					rect.y = 0;
					rect.width = getWidth();
					rect.height = (i < 2) ? height : linePos[i-2].yPos;

					resetLineDefs();

					repaint( &rect );
				}
				else 
        {
				  int yPos = linePos[i-1].yPos, height = linePos[i-1].height;
  
          scrollDown( reader, lines, i, LINES );

          // Update the line where the change happened

				  Rect rect( 0, yPos, width, height );

				  repaint( &rect );
        }
  
        //updatePosition( reader, position, position );
      }
      else
      {
        windowLines = -1;
        calcLinePos( reader, TRUE, i );
        updatePosition( reader, position, linePos[windowLines-1].lineStart );
      }
      break;
    }
  }

  // Lines are inserted below windowLines
  // If there's space left below the last line, text has been inserted
  // at end of text. Recalculate windowLines
  if( linePos[windowLines].yPos == UNDEF )
  {
    int lastLine = windowLines-1;
    windowLines = -1;
    calcLinePos( reader, TRUE, lastLine );
    int height = (lastLine < 2 ) ? getHeight() :
                                      linePos[lastLine-1].yPos - 1;
    Rect rect( 0, 0, getWidth(), height );

    repaint( &rect );
  }
  processUpdatePositions( reader );
  releaseLinePos();
}

void ViewWindow::deletion( TextBuffer *textBuffer, int reader, int position,
                           int size, int lines )
{
	int foo;
  refreshWindowWhenAfterUpdate = TRUE;
  lockLinePos();
  if( position + size <= windowStart )
  {
    if( isUpdating() )
    {
      for( int l = 0; l < windowLines + 2; l++ )
        linePos[l].lineStart -= size;
    }
    else
      resetLineDefs();

    windowStart -= size;
    if( !view->queryUpdateAllOnModification() )
      windowStartLine -= lines;
    else
      windowStartLine = view->getLineFromPosition( windowStart );
    releaseLinePos();

		// Redraw the whole screen if we have left margin info

		if( lines > 0 && Settings::showLeftMarginInfo )
			repaint( NULL );
		else
			processUpdatePositions( reader );
			
    return;
  }

  if( position < windowStart && position + size > windowStart )
  {
    if( !view->queryUpdateAllOnModification() )
    {
      // We need to recalc the windowLine
  
      int pos = 0;
      windowStartLine = 1;
  
      while( pos < position )
      {
        int lastPos = pos;
        pos = nextLine( reader, pos, foo );
        if( lastPos != pos )
          windowStartLine++;
        else
          break;
      }
  
      if( pos > position )
      {
        int lastPos = pos;
        int lines = -1;
        pos = view->getNewLineStartFrom( reader, pos, lines, foo );
        if( lastPos != pos ) windowStartLine--;
      }
  
      windowStart = pos;
  
      releaseLinePos();
  
      resetLineDefs();
      if( isUpdating() )
        repaint( NULL );
  //    updatePos.start = -1;
    }
    else
    {
      windowStart = position;
      windowStartLine = view->getLineFromPosition( windowStart );
      releaseLinePos();
  
      if( isUpdating() )
        repaint( NULL );
    }
    updatePos.removeAll();
    return;
  }

  if( !isUpdating() )
  {
    resetLineDefs();
    releaseLinePos();
    processUpdatePositions( reader );
    return;
  }

  // See on which line the update started

  for( int i = 0; i < windowLines + 2; i++ )
  {
    if( linePos[i].yPos == UNDEF )
    {
      if( i == windowLines ) 
      {
        // Update the current line. We were at the text end (probably)
        updatePosition( reader, position, position );
        break;
      }
      // Skip the whole thing
      linePosIsInvalid = TRUE;
      releaseLinePos();
      return;
    }

    if( linePos[i].lineStart > position )
    {
      // The position was on the previous line
      if( !view->queryUpdateAllOnModification() )
      {
        // Pull all linepos backward size chars

        for( int l = i; l < windowLines + 2; l++ )
          linePos[l].lineStart -= size;

				if( lines > 0 && Settings::showLeftMarginInfo )
				{
					// In order to have the line numbers correctly set, we need to 
					// redraw downwards

					Rect rect;
					
					rect.x = 0;
					rect.y = 0;
					rect.width = getWidth();
					rect.height = (i < 2) ? height : linePos[i-2].yPos;

					resetLineDefs();

					repaint( &rect );
				}
				else 
        if( lines > 0 ) // cut
	        removeLines( reader, i + windowStartLine-1, lines );

				// Update the line where the change happened if lines is zero
				
				if( lines == 0 )
					updatePosition( reader, position, position );

      }
      else
      {
        windowLines = -1;
        calcLinePos( reader, TRUE, i );
        updatePosition( reader, position, linePos[windowLines-1].lineStart );
      }

      break;
    }
  }

  releaseLinePos();

  processUpdatePositions( reader );
}

void ViewWindow::scrollVert( int delta, int bottom, int top )
{
	// Check for updateRects which overlap and scroll them too
	updateRectLock.request();
	int i = updateRect.getSize();

  while( i > 0 )
  {
  	Rect *rect = &updateRect.getObjectAt(0)->rect;
    int oldY = rect->y;

		//              *         *  <->
    //       |          |

    if( rect->y+rect->height >= bottom && rect->y+rect->height <= top )
    {
    	rect->height += delta;
      if( rect->y+rect->height < bottom )
      	rect->height = bottom - rect->y;
      if( rect->y+rect->height > top )
      	rect->height = top-rect->y;
    }

		//  *         *  <->
    //       |          |

    if( rect->y >= bottom && rect->y <= top )
    {
    	rect->y += delta;

      if( rect->y < bottom )
      	rect->y = bottom;
      if( rect->y > top )
      	rect->y = top;
    }

    rect->height += (oldY-rect->y);

    i--;
  }
	updateRectLock.release();
	Window::scrollVert( delta, bottom, top );
}

void ViewWindow::scrollHorz( int delta, int left, int right )
{
	// Check for updateRects which overlap and scroll them too
	updateRectLock.request();
	int i = updateRect.getSize();
	
  while( i > 0 )
  {
  	Rect *rect = &updateRect.getObjectAt(0)->rect;
    int oldX = rect->x;

		//              *         *  <->
    //       |          |

    if( rect->x+rect->width >= left && rect->x+rect->width <= right )
    {
    	rect->width += delta;
      if( rect->x+rect->width < left )
      	rect->width = left - rect->x;
      if( rect->x+rect->width > right )
      	rect->width = right-rect->x;
    }

		//  *         *  <->
    //       |          |

    if( rect->x >= left && rect->x <= right )
    {
    	rect->x += delta;

      if( rect->x < left )
      	rect->x = left;
      if( rect->x > right )
      	rect->x = right;
    }

    rect->width += (oldX-rect->x);

    i--;
  }
	updateRectLock.release();
	Window::scrollHorz( delta, left, right );
}

void ViewWindow::processUpdatePositions( int reader )
{
  for( int i = 0; i < updatePos.getSize(); i++ )
    updatePosition( reader, updatePos.getObjectAt(i)->startpos,
                    updatePos.getObjectAt(i)->endpos );
  updatePos.removeAll();
}

void ViewWindow::processUpdateRects()
{
	int i = updateRect.getSize();

  while( i > 0 )
  {
  	repaint( &updateRect.getObjectAt(0)->rect );

		updateRectLock.request();
  	updateRect.removeAt(0);
		updateRectLock.release();

    i--;
  }
}

void ViewWindow::paintLines( int reader, int startLine, int endLine )
{
  lockLinePos();
  startLine -= windowStartLine;
  endLine -= windowStartLine;

  if( endLine < 0 || startLine > windowLines ) { releaseLinePos(); return; }

  if( windowLines == -1 )
  {
    releaseLinePos();
    repaint( NULL );
    return;
  }

  if( endLine > windowLines )
    endLine = windowLines;

  if( startLine < 0 ) startLine = 0;

  calcLinePos( reader, TRUE, windowLines-1 );
  int startY = (startLine == 0) ? getHeight() : linePos[startLine-1].yPos;

  int end = linePos[endLine].yPos;

  if( end == UNDEF ) end = 0;

  Rect rect( 0, end, getWidth(), startY - end );

  releaseLinePos();
  repaint( &rect );
}

void ViewWindow::filesDropped( char **files, int count )
{
	for( int i = 0; i < count; i++ )
		ControlAdmin::getControl(ControlAdmin::getControlIndex((EditControl *)viewHolder))->parent->addFile(files[i], TRUE, FALSE);
}

/*void ViewWindow::modification( TextBuffer *buffer, int reader, int position, int size )
{
  // Reset the whole buffer

  windowStartLine = 1;
  windowStart = 0;
  linePosIsInvalid = TRUE;
  repaint( NULL );
}*/

void ViewWindow::snapshot()
{
  FILE *f = fopen( "snap", "a" );
  fprintf( f, "\n--------------\n" );
  fprintf( f, "windowLines: %d, windowStart: %d, windowStartLine: %d, cursorLine: %d, lineStart: %d, linePosIsInvalid: %d\n", windowLines, windowStart, windowStartLine, view->getLine(), view->getLineStart(), linePosIsInvalid );
  for( int i = 0; i < windowLines + 2; i++ )
  {
    fprintf( f, "[(%02d) %03d, %04d] ", i, linePos[i].yPos, linePos[i].lineStart );
  }
  fclose( f );
}

int ViewWindow::execFromMainThread( int task, void *obj )
{
  switch( task )
  {
    case MOVE_SCREEN_TO_CURSOR : 
    {
			pScreenToCursor();
			return 0;
    }
    case MOVE_PAGE : 
    {
			pMovePage( (int)obj);
			return 0;
    }
	}    
}

void ViewWindow::performPositionEvent( long position )
{
  for( int i = 0; i < positionListener.getSize(); i++ )
  {
    APositionEvent positionEvent( (Window *)this, position );
    positionListener.getObjectAt(i)->actionPerformed( &positionEvent );
  }
}