#include <stdlib.h>
#include "bowl.h"

int Notebook::VERTSPACE = 10;
int Notebook::TABINTERIORSPACING = 6;
int Notebook::PUSH = 4;
int Notebook::EXTRA = 4;
int Notebook::ALLOWCLOSE = 1;
int Notebook::CLOSEBUTTONWIDTHHEIGHT = 15;

int Toolbar::IMAGE = 1;
int Toolbar::SPACE = 2;
int Toolbar::ToolButton::UP = 0;
int Toolbar::ToolButton::DOWN = 1;
int Toolbar::vertMargin = 7;
int Toolbar::buttonMargin = 2;

void Notebook::create( ColorValue activeColor )
{
  wMove = FOLLOW;
  hMove = FOLLOW;
  activeTab = -1;
  tabXstart = 0;
  spacing = -5;
  buttonWidth= 20;
  leftButton = FALSE;
  rightButton = FALSE;
  backgroundColor = Color::BKGR;
  listener = NULL;
  addMouseListener( this );
  font = NULL;
  parent->addControl( this );
  type = NOTEBOOK;
	this->activeColor = activeColor;
	addMenuListener( this );
	closeActive = down = FALSE;
	closeListener = NULL;
}

BOOL Notebook::setSizeInit( BOOL setInit ) 
{ 
  BOOL prev = initSize; 
  initSize = setInit; 

  for( int i = 0; i < tab.getSize(); i++ )
    tab.getObjectAt(i)->window->setSizeInit( setInit );

  return prev; 
}
 
int Notebook::getWindowTab( Window *window )
{
  for( int i = 0; i < tab.getSize(); i++ )
    if( tab.getObjectAt( i )->window == window ) return i;
  return -1;
}

void Notebook::size( int prevWidth, int prevHeight)
{
  // Resize all windows
  resizeWindows();
}

void Notebook::setWindowSize( Window *window )
{
  int tabsHeight = (tab.getSize() <= 1 ) ? 0 : getTabsAreaHeight( getGraphics() );
  window->setPos( 0, 0 );
  window->setSize( width, height - tabsHeight );
}

void Notebook::resizeWindows()
{
  for( int i = 0; i < tab.getSize(); i++ )
    setWindowSize( tab.getObjectAt(i)->window );
}

void Notebook::setTabTextColor( int ctab, ColorValue color )
{
	tab.getObjectAt(ctab)->textColor = color;

	repaintTabs();

/*  int tabHeight = (tab.getSize() > 1 ) ? getTabsAreaHeight( getGraphics() ) : 0;

  Rect rect( 0, height - tabHeight, width, tabHeight );
  repaint( &rect, TRUE );*/
}

void Notebook::setTabTitle( int ctab, char *title )
{
	tab.getObjectAt(ctab)->setTitle( title );
	setTabsWidth();
	repaintTabs();
}

void Notebook::drawTab( Graphics *g, int i, int xPos, int tabsHeight )
{
  int pushDown, pushUp, fontBaseline = g->getFontBaseline();
  ColorValue bColor, textColor, shadowColor, shineColor, specialColor;

  int tabWidth = tab.getObjectAt(i)->width;

  if( i == activeTab )
  {
//    bColor = Color::createColor( 230, 240, 255 );
    bColor = activeColor;
    textColor = tab.getObjectAt(i)->textColor;
    shadowColor = Color::SHADOW;
    shineColor = Color::LIGHT;
//    specialColor = Color::createColor( 191, 200, 210 );;
    specialColor = Color::SHADOW;//Color::BKGR;
    pushDown = PUSH;
    pushUp = 0;
  }
  else
  {
//    bColor = Color::createColor( 191, 200, 210 );
    bColor = Color::BKGR;
    textColor = tab.getObjectAt(i)->textColor;
    shadowColor = Color::GRAY;
    shineColor = Color::LIGHT;
    specialColor = Color::SHADOW;
    pushDown = 0;
    pushUp = PUSH;
  }

  Point *point[4];

  point[0] = new Point( xPos, height - tabsHeight + pushUp );
  point[1] = new Point( xPos, height - pushDown - 1 - EXTRA );
  point[2] = new Point( xPos+tabWidth-TABINTERIORSPACING-2, height - pushDown - 1 - EXTRA );
  point[3] = new Point( xPos+tabWidth-2, height - tabsHeight + pushUp );

  g->setColor( bColor );
  g->fillPolygon( 4, point );

  g->setColor( shineColor );
  g->drawLine( point[0]->x, point[0]->y, point[1]->x, point[1]->y );
  g->drawLine( point[1]->x, point[1]->y, point[2]->x, point[2]->y );
  
  int l;

  for( l = 1; l <= 3; l++ )
    g->drawLine( point[0]->x+TABINTERIORSPACING-l, point[1]->y-4,
                 point[0]->x+TABINTERIORSPACING-l, point[1]->y-3);
  g->setColor( specialColor );
  g->drawLine( point[0]->x+TABINTERIORSPACING, point[1]->y-5,
               point[0]->x+TABINTERIORSPACING, point[1]->y-3);
  g->drawLine( point[0]->x+TABINTERIORSPACING-3, point[1]->y-5,
               point[0]->x+TABINTERIORSPACING, point[1]->y-5);

  g->setColor( shadowColor );
  for( l = 1; l <= 3; l++ )
    g->drawLine( point[2]->x+l, point[2]->y-l, point[3]->x+l, point[3]->y );

  if( i != activeTab )
    g->drawLine( point[3]->x, point[3]->y, point[0]->x, point[0]->y );

/*  Rect orect( point[1]->x+1+TABINTERIORSPACING, point[0]->y+1,
              point[2]->x - point[1]->x, point[1]->y - point[0]->y-2 );*/

  g->setColor( textColor );
  g->setBackColor( bColor );

  Tab *ctab = tab.getObjectAt( i );

  g->drawString( point[1]->x+1+TABINTERIORSPACING, point[0]->y + (tabsHeight - g->getFontHeight()) / 2, 
                 &ctab->title[ctab->stringStart] );
  xPos += tabWidth + spacing;
  for( int n = 0; n < 4; n++ )
    delete point[n];
}

void Notebook::drawButton( Graphics *g, int dir, int tabsHeight )
{
  int x = 0, y = height - tabsHeight;
  if( dir )
    x = width - buttonWidth - (allowClose ? CLOSEBUTTONWIDTHHEIGHT * 2 : 0);

  Point *point[4];

  if( dir )
  {
    point[0] = new Point( x, y );
    point[1] = new Point( x + TABINTERIORSPACING, y + tabsHeight - 1 );
    point[2] = new Point( x + buttonWidth-1, y + tabsHeight - 1 );
    point[3] = new Point( x + buttonWidth-1, y );
  }
  else
  {
    point[0] = new Point( x, y );
    point[1] = new Point( x, y + tabsHeight - 1 );
    point[2] = new Point( x + buttonWidth - TABINTERIORSPACING - 1, y + tabsHeight - 1 );
    point[3] = new Point( x + buttonWidth - 1, y );
  }

  g->setColor( Color::BKGR );
  g->fillPolygon( 4, point );
  g->setColor( Color::LIGHT );

  int l;
  for( l = 0; l < 2; l++ )
    g->drawLine( point[l]->x, point[l]->y, point[l+1]->x, point[l+1]->y );
  g->setColor( Color::SHADOW );
  for( l = 2; l < 4; l++ )
    g->drawLine( point[l]->x, point[l]->y, point[(l+1)%4]->x, point[(l+1)%4]->y );
    
  int n;    

  for( n = 0; n < 4; n++ )
    delete point[n];

  // Draw arrow

  int halfHeight = (tabsHeight - tabsHeight / 2) / 2;

  point[0] = new Point( buttonWidth / 3+2, tabsHeight / 2 + halfHeight );
  point[1] = new Point( buttonWidth - buttonWidth / 3+2, tabsHeight / 2 );
  point[2] = new Point( buttonWidth / 3+2, tabsHeight / 2 - halfHeight );

  for( int i = 0; i < 3; i++ )
  {
    if( !dir )
      point[i]->x = buttonWidth - point[i]->x;
    point[i]->x += x;
    point[i]->y += y;
  }

  g->setColor( Color::createColor( 150, 160, 240 ) );
  g->fillPolygon( 3, point );

  for( n = 0; n < 3; n++ )
    delete point[n];
}

void Notebook::setTab( int toTab, BOOL deactivateOld )
{
  if( toTab < 0 || toTab >= tab.getSize() ) return;

  if( deactivateOld && activeTab != -1 )
  {
    tab.getObjectAt( activeTab )->window->setVisible( FALSE );
    tab.getObjectAt( activeTab )->window->activate( FALSE );
  }
  activeTab = toTab;
  tab.getObjectAt( activeTab )->window->setVisible( TRUE );
  tab.getObjectAt( activeTab )->window->activate( TRUE );
  setActiveTabVisible();
	repaintTabs();
  ASelectionEvent event( this, activeTab );
  if( listener )
    listener->actionPerformed( &event );
}

void Notebook::repaintTabs()
{
  int tabHeight = (tab.getSize() > 1 ) ? getTabsAreaHeight( getGraphics() ) : 0;
//  Rect rect( x, y + height - tabHeight, width, tabHeight );
  Rect rect( 0, height - tabHeight, width, tabHeight );
	repaint( &rect, TRUE );
//  ((Window *)parent)->repaint( &rect, TRUE );
}

void Notebook::nextTab()
{
  int tab = getActiveTab() + 1;
  if( tab >= getTabs() ) tab = 0;
  setTab( tab );
}

void Notebook::prevTab()
{
  int tab = getActiveTab() - 1;
  if( tab < 0 ) tab = getTabs() - 1;
  setTab( tab );
}

void Notebook::add( Window *window, char *title )
{
  window->setFollowParentSize( FALSE ); // The notebook handles this by itself
  tab.add( new Tab( title, window ) );
  resizeWindows();
  setTabsWidth();
  setTab( tab.getSize() - 1 );
}

void Notebook::remove( Window *window )
{
  for( int i = 0; i < tab.getSize(); i++ )
    if( tab.getObjectAt( i )->window == window )
    {
      tab.removeAt( i );
      resizeWindows();
      setTabsWidth();
      if( activeTab == i )
        setTab( (i>0) ? i-1 : 0, FALSE );
      else
      if( tab.getSize() > 0 )
      {
        if( activeTab > i )
          activeTab--;
        setActiveTabVisible();
      }
      break;
    }
}

void Notebook::drawCloseButton( Graphics *g, BOOL up )
{
	int tabsHeight = getTabsAreaHeight( g );

	int xPos = width - CLOSEBUTTONWIDTHHEIGHT - 
					   CLOSEBUTTONWIDTHHEIGHT / 2, 
			yPos = height - (tabsHeight-CLOSEBUTTONWIDTHHEIGHT) / 2 - 
			       CLOSEBUTTONWIDTHHEIGHT;

	ColorValue fc = (up ? Color::WHITE : Color::GRAY),
	           bc = (up ? Color::GRAY : Color::WHITE);

	g->drawShadedBox( xPos, yPos, CLOSEBUTTONWIDTHHEIGHT, CLOSEBUTTONWIDTHHEIGHT,
										fc, bc );

	g->drawShadedBox( xPos+1, yPos+1, CLOSEBUTTONWIDTHHEIGHT-2, 
	                  CLOSEBUTTONWIDTHHEIGHT-2, fc, bc );

	if( !up )
	{
		xPos += 1;
		yPos -= 1;
	}

	xPos += 3;
	yPos += 3;

	g->setColor( Color::BLACK );

	g->drawLine( xPos, yPos, xPos + CLOSEBUTTONWIDTHHEIGHT - 8,
	             yPos + CLOSEBUTTONWIDTHHEIGHT - 7 );

	g->drawLine( xPos+1, yPos, xPos + CLOSEBUTTONWIDTHHEIGHT - 7,
	             yPos + CLOSEBUTTONWIDTHHEIGHT - 7 );

	g->drawLine( xPos + CLOSEBUTTONWIDTHHEIGHT - 8, yPos, xPos,
	             yPos + CLOSEBUTTONWIDTHHEIGHT - 7 );

	g->drawLine( xPos + CLOSEBUTTONWIDTHHEIGHT - 7, yPos, xPos+1,
	             yPos + CLOSEBUTTONWIDTHHEIGHT - 7 );
}

void Notebook::paint( Rect *, Graphics *g )
{
  int tabsHeight = 0,
	    notebookWidth = width - (allowClose ? CLOSEBUTTONWIDTHHEIGHT * 2 : 0);

  if( tab.getSize() > 0 )
  {
    tabsHeight = (tab.getSize() > 1 ) ? getTabsAreaHeight( g ) : 0;

    // Draw background

    Rect parentRect( 0, height - tabsHeight, width, tabsHeight );

    g->setBackColor( backgroundColor );
    g->fillRect( &parentRect );

    // Draw tabs

    if( font != NULL )
      g->selectFont( font, fontSize, 0 );
    else
      g->setSystemFont();

    int xPos = 0, activeTabPos = -1;

    if( tab.getSize() > 1 )
    {
      // Find out where to start drawing
      int startTab;

      for( startTab = 0; startTab < tab.getSize() - 1; startTab++ )
      {
        if( xPos - tabXstart >= notebookWidth )
          break;
        xPos += tab.getObjectAt(startTab)->width + spacing;
      }

      leftButton = ( tabXstart != 0 );

      int pushRight = (leftButton) ? buttonWidth : 0;

      rightButton = ( startTab < tab.getSize() - 1 ||
                      tab.getObjectAt(startTab)->width + xPos - tabXstart + 
											pushRight > 
											notebookWidth );

      for( int i = startTab; i >= 0 ; i-- )
      {
        if( i != activeTab )
          drawTab( g, i, xPos - tabXstart + pushRight, tabsHeight );
        else
          activeTabPos = xPos;

        if( i > 0 )
          xPos -= tab.getObjectAt(i-1)->width + spacing;
      }

      if( activeTabPos != -1 )
      {
        g->setFontBold();
        drawTab( g, activeTab, activeTabPos - tabXstart + pushRight, tabsHeight );
        g->setFontNormal();
      }

			// Draw close button if it is allowed

			if( allowClose )
			{
				// Clear background, we don't use clip regions

				Rect bkgr( width - CLOSEBUTTONWIDTHHEIGHT * 2, height - tabsHeight, 
									 CLOSEBUTTONWIDTHHEIGHT * 2, tabsHeight );

				g->setBackColor( backgroundColor );
				g->fillRect( &bkgr );

				drawCloseButton( g, !down );
			}
    }

    // Draw buttons

    if( leftButton )
      drawButton( g, 0, tabsHeight );
    if( rightButton )
      drawButton( g, 1, tabsHeight );

  }
}

void Notebook::repaintCloseButton()
{
	int tabHeight = getTabsAreaHeight( getGraphics() );
	Rect rect( width - CLOSEBUTTONWIDTHHEIGHT*2, height - tabHeight, 
						 CLOSEBUTTONWIDTHHEIGHT*2, tabHeight );
	repaint( &rect, TRUE );
}

int Notebook::tabAtPos( int x )
{
	int xPos = (leftButton) ? buttonWidth : 0;

	for( int i = 0; i < tab.getSize(); i++ )
	{
		if( x + tabXstart >= xPos &&
				x + tabXstart <= xPos + tab.getObjectAt(i)->width )
			return i;
		xPos += tab.getObjectAt(i)->width + spacing;
	}
	return -1;
}

struct TabMenuItem
{
  char *title;
  int id;
};

#if defined __BORLANDC__
int compare( const void *arg1, const void *arg2 )
#elif defined _MSC_VER
int compare( const void *arg1, const void *arg2 )
#else
int _Optlink compare( const void *arg1, const void *arg2 )
#endif
{
   return strcmpi( ((TabMenuItem *)arg1)->title, ((TabMenuItem *)arg2)->title );
}

BOOL Notebook::actionPerformed( AMouseEvent *mouseEvent )
{
  Point *pos = mouseEvent->getPos();

  if( mouseEvent->getButton() == AMouseEvent::RIGHT )
	{
    if( mouseEvent->getEvent() == AMouseEvent::CLICK )
    {
      // Popup a menu where the user can select a window
      Point *pos = mouseEvent->getPos();
      AMenu *menu = new AMenu( "Toolbar", 100 );

      // The window tabs should be sorted by name
      TabMenuItem *window = new TabMenuItem[tab.getSize()];

      int i;
			for( i = 0; i < tab.getSize(); i++ )
      {
        window[i].title = tab.getObjectAt(i)->title;
        window[i].id = 101 + i;
      }

      qsort( window, tab.getSize(), sizeof(TabMenuItem), compare );

			for( i = 0; i < tab.getSize(); i++ )
	      menu->addMenuItem( new AMenuItem( window[i].title, window[i].id, AMenuEntry::MENUITEM ) );

      delete window;

	    if( pos->y > height -
  	      getTabsAreaHeight( getGraphics() ) )
			{
				menuTabClicked = tabAtPos( pos->x );

				if( menuTabClicked != -1 )
				{
		      menu->addMenuItem( new AMenuItem( "", 1000, AMenuEntry::SEPARATOR ) );
		      menu->addMenuItem( new AMenuItem( Strings::get(208), 
					                                  1001, AMenuEntry::MENUITEM ) );
				}
			}
      popupMenu( menu, pos->x, pos->y );
      delete menu;
      return TRUE;
    }
	}
	else
	if( allowClose && tab.getSize() > 1 &&
			pos->x >= width - CLOSEBUTTONWIDTHHEIGHT - CLOSEBUTTONWIDTHHEIGHT / 2 &&
			pos->x < width - CLOSEBUTTONWIDTHHEIGHT / 2 &&
			pos->y >= height - CLOSEBUTTONWIDTHHEIGHT - CLOSEBUTTONWIDTHHEIGHT / 2 &&
			pos->y < height - CLOSEBUTTONWIDTHHEIGHT / 2 )
	{
		if( down && mouseEvent->getEvent() == AMouseEvent::MOVE ) return TRUE;

		if( (mouseEvent->getEvent() == AMouseEvent::MOVE && !down && closeActive) ||
		    (mouseEvent->getEvent() == AMouseEvent::DOWN && !down) )
		{
			down = TRUE;
			closeActive = TRUE;
			repaintCloseButton();
		}
		else
		if( down )
		{
			down = FALSE;
			closeActive = FALSE;

			AButtonEvent event( this, activeTab );

			if( closeListener )
				closeListener->actionPerformed( &event );

			repaintCloseButton();
		}
	}
	else
  if( allowClose && mouseEvent->getEvent() == AMouseEvent::MOVE && down )
	{
		down = FALSE;
		repaintCloseButton();
	}
	else
  if( allowClose && mouseEvent->getEvent() == AMouseEvent::RELEASE && 
	    closeActive )
	{
		closeActive = down = FALSE;
		repaintCloseButton();
	}
	else
  if( mouseEvent->getEvent() == AMouseEvent::DOWN ||
      mouseEvent->getEvent() == AMouseEvent::DOUBLECLICK )
  {
    int notebookWidth = width - (allowClose ? CLOSEBUTTONWIDTHHEIGHT * 2 : 0);

    if( pos->y > height -
        getTabsAreaHeight( getGraphics() ) )
    // Find out which tab was clicked
    {
      int selTab = -1;

      // Check if leftButton was clicked

      if( leftButton && pos->x <=
          buttonWidth )
      {
        leftJustifyTablist();
        repaint( NULL, FALSE );
        return TRUE;
      }

      // Check if rightButton was clicked

      if( rightButton && pos->x >
          notebookWidth - buttonWidth )
      {
        rightJustifyTablist();
        repaint( NULL, FALSE );
        return TRUE;
      }

			selTab = tabAtPos( pos->x );

      if( selTab != -1 )
        setTab( selTab );
    }
  }
  return TRUE;
}

void Notebook::actionPerformed( AMenuEvent *menuEvent )
{
	if( menuEvent->getId() == 1001 )
	{
		AButtonEvent event( this, menuTabClicked );

		if( closeListener )
			closeListener->actionPerformed( &event );
		repaintTabs();
	}
	else
		setTab( menuEvent->getId() - 101, TRUE );
}

void Notebook::setTabsWidth()
{
	int notebookWidth = width - (allowClose ? CLOSEBUTTONWIDTHHEIGHT * 2 : 0);

  if( font != NULL )
    getGraphics()->selectFont( font, fontSize, Font::BOLD );
  else
  {
    getGraphics()->setSystemFont();
    getGraphics()->setFontBold();
  }

  for( int i = 0; i < tab.getSize(); i++ )
  {
    Tab *ctab = tab.getObjectAt(i);

    Point *points = new Point[strlen(ctab->title) + 1];

    // Calc length of string

    getGraphics()->calcStringCharPos( 0, 0, strlen( ctab->title ), ctab->title,
                                      points );

    for( int point = 0;point < strlen(ctab->title); point++ )
    {
      if( points[strlen(ctab->title)].x - points[point].x <
          notebookWidth - TABINTERIORSPACING * 2 - TABINTERIORSPACING * 2 -
          2 * buttonWidth)
      {
        ctab->stringStart = point;
        break;
      }
    }

    ctab->width = points[strlen(ctab->title)].x - points[ctab->stringStart].x +
                  TABINTERIORSPACING * 2 + 2;
    delete points;
  }
}

int Notebook::getTabsAreaHeight( Graphics *g )
{
  if( font != NULL )
    g->selectFont( font, fontSize, Font::BOLD );
  else
    g->setSystemFont();

  g->setFontBold();
  return g->getFontHeight() + VERTSPACE + EXTRA;
}

void Notebook::leftJustifyTablist()
{
  // Move forwards until we pass tabXstart

  int lastPos = 0, xPos = 0;

  for( int i = 0; i < tab.getSize() && xPos < tabXstart; i++ )
  {
    Tab *ctab = tab.getObjectAt(i);
    lastPos = xPos;
    xPos += ctab->width + spacing;
  }
  tabXstart = lastPos;
}

void Notebook::rightJustifyTablist()
{
	int notebookWidth = width - (allowClose ? CLOSEBUTTONWIDTHHEIGHT * 2 : 0);

  // Move forwards until we pass tabXstart + width- buttonWidth *2

  int xPos = 0;

  for( int i = 0; i < tab.getSize(); i++ )
  {
    Tab *ctab = tab.getObjectAt(i);
    xPos += ctab->width + spacing;
    if( xPos > tabXstart + notebookWidth - buttonWidth * 2 )
      break;
  }
  tabXstart = xPos - notebookWidth + buttonWidth * 2;
}

void Notebook::setActiveTabVisible()
{
	int notebookWidth = width - (allowClose ? CLOSEBUTTONWIDTHHEIGHT * 2 : 0);
  int xPos = 0;

  for( int i = 0; i < activeTab; i++ )
    xPos += tab.getObjectAt(i)->width + spacing;

  if( xPos < tabXstart )
    tabXstart = xPos;
  else
  if( xPos + tab.getObjectAt(activeTab)->width > 
	    tabXstart + notebookWidth - buttonWidth * 2 )
    tabXstart = xPos - notebookWidth + tab.getObjectAt(activeTab)->width + 
		            buttonWidth * 2;
}

void Notebook::enable( BOOL enabled )
{
  ((Window *)parent)->enable( enabled );
}

void ComboBox::drawArrow( Graphics *g, int x, int y, int width, int height, int dir )
{
   Point *point[3];

   if( dir )
   {
     point[0] = new Point( x + 2, y + (height*2) / 3 );
     point[1] = new Point( x + width - 3, y + (height*2) / 3 );
     point[2] = new Point( x + width / 2, y + height / 3 );
   }
   else
   {
     point[0] = new Point( x + 2, y + height / 3 );
     point[1] = new Point( x + width / 2, y + (height*2) / 3 );
     point[2] = new Point( x + width - 3, y + height / 3 );
   }

   g->setColor( Color::BLACK );
   g->fillPolygon( 3, point );

   delete point[0];
   delete point[1];
   delete point[2];
}

BOOL ComboBox::actionPerformed( AMouseEvent *e )
{
  int repaint = FALSE;
  if( (e->getEvent() == AMouseEvent::DOWN ||
       e->getEvent() == AMouseEvent::DOUBLECLICK) && !down &&
       e->getPos()->x >= width - buttonWidth )
  {
    if( listVisible )
    {
      buttonUp = TRUE;
      listBox->setVisible( FALSE );
      listVisible = FALSE;
      setFocus();
    }
    else
    {
      placeListBox();
      buttonUp = FALSE;
      listBox->setOnTop();
      listBox->setFocus();
      listBox->setVisible( TRUE );
      listVisible = TRUE;
    }
    repaint = TRUE;
    down = TRUE;
  }

  if( e->getEvent() == AMouseEvent::RELEASE )
    down = FALSE;

  if( repaint ) repaintButton();
  return TRUE;
}

void ComboBox::actionPerformed( AFocusEvent *focusEvent )
{
  char text[255];
  int selected = ((ListBox *)focusEvent->getSource())->getSelectedItem( 255, text );

  listBox->setVisible( FALSE );
  listVisible = FALSE;
  buttonUp = TRUE;
  repaintButton();

  if( selected == -1 ) return;  // Nothing selected

	if( ((ListBox *)focusEvent->getSource())->hasFocus() ) setFocus();

  entryField->setText( text );
	//setFocus();
  performSelectionEvent( focusEvent->getFocused() ); // chain
  //performFocusEvent( focusEvent->getFocused() ); // chain
}


BOOL ComboBox::actionPerformed( AKeyEvent *keyEvent )
{
  if( listBox->getItems() == 0 ) return FALSE;

  int newPos = listBox->getSelectedIndex();

  if( keyEvent->key == KeyDef::DOWN )
  {
    if( newPos == -1 )
      newPos = 0;
    else
    if( newPos != listBox->getItems() - 1 )
      newPos++;
    else
      return TRUE;
  }
  else
  if( keyEvent->key == KeyDef::UP )
  {
    if( newPos == -1 )
      newPos = listBox->getItems() - 1;
    else
    if( newPos != 0 )
      newPos--;
    else
      return TRUE;
  }
  else
    return FALSE;

  listBox->selectItem( newPos );

  char text[300];

  listBox->getSelectedItem( 300, text );

  entryField->setText( text );
  performSelectionEvent( newPos ); // chain
  return TRUE;
}

void ComboBox::placeListBox()
{
//ShowWindow( hWndTest, SW_SHOW );
/*  int ret = ShowWindow( hWndTest, SW_SHOW );
  DWORD err = GetLastError();
  HWND t = SetFocus( hWndTest );
  err = GetLastError();*/

	// Set the listbox at it's default position

	listBox->setPos( x, y-listBox->getHeight() );

  // Check if listbox is above frame

/*  if( listBox->getY() + listBox->getHeight() > parent->getHeight() )
  {
    listBox->setPos( x, y - listBox->getHeight() );
    dir = 1;
  }*/
  // Check if listbox is below frame

  if( listBox->getY() < 0 )
  {
    listBox->setPos( x, y + height );
    dir = 0;
  }
}

int ComboBox::buttonWidth = 17;

ComboBox::ComboBox( Window *parent, int properties, int x, int y,
          int width, int height, int listHeight ) :
  Window( parent, xlatProperties( properties|FILTERDIALOGARROWS|PASSDIALOGKEYS ), x, y, width, height )
{
  buttonUp = TRUE;
  orgHeight = height;
  orgListHeight = this->listHeight = listHeight;
  listVisible = FALSE;
  down = FALSE;
  maxLength = 80;

  type = COMBOBOX;

  // Create the editcontrol

  entryField = new EntryField( this, (properties ^ ISTABSTOP) | PASSDIALOGKEYS, 2, 2,
                               width-4 - buttonWidth - 1, height-4 );
  entryField->setVisible( TRUE );
  entryField->setOnTop();
/*  if( parent->getType() == DIALOG )
    entryField->setDialogFont();*/

  addMouseListener( this  );
  entryField->addKeyListener( this  );

  // Create listbox

  listBox = new ComboListBox( this, parent, Window::NOADJUSTSIZE, x, y-listHeight,
                         width, listHeight );
  listBox->addFocusListener( this );
  listBox->addSelectionListener( this );

  dir = 1;

  placeListBox();

  parent->addControl( this );

/*  hWndTest = CreateWindow( "STATIC", "", WS_VISIBLE | WS_POPUP, 0, 0,
                                         150, 150, NULL, NULL,
                                         Application::hInstance, NULL );

  hWndTest = CreateWindow( "COMBOBOX", "", WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_CHILD | CBS_DROPDOWN, 0, 0,
                                         150, 150, hWndTest, (HMENU)12345,
                                         Application::hInstance, NULL );*/
//	oldWndProcCB = (WNDPROC)SetWindowLong(hWndEdit, GWL_WNDPROC, (LONG)WndProcCB);
//  DWORD err = GetLastError();
  //SetParent( hWndTest, hWndFrame );*/

}

void ComboBox::actionPerformed( ASelectionEvent *s )
{
	//performSelectionEvent( s->getSelected() );
}

int ComboBox::focus( BOOL newFocus )
{
  if( newFocus )
  {
    entryField->setFocus();
    return 1;
  }
  return 0;
}

void ComboBox::paint( Rect *rect, Graphics *g )
{
  g->drawShadedBox( 0, 0, width, height, Color::SHADOW, Color::SHADOW );
  g->drawShadedBox( 1, 1, width-2, height-2, Color::BLACK, Color::BKGR );

  // Draw the button

  g->drawButton( width - buttonWidth - 2, 2,
                 buttonWidth, height - 4, buttonUp );
  g->setColor( Color::BLACK );
  if( buttonUp )
    drawArrow( g, width - buttonWidth, 4,
               buttonWidth - 4, height - 8, dir );
  else
    drawArrow( g, width - buttonWidth + 1, 3,
               buttonWidth - 4, height - 8, dir );
  g->setColor( Color::SHADOW );
  g->drawLine( width - buttonWidth - 3, 2,
               width - buttonWidth - 3, height - 3 );
}

void ComboBox::size( int oldWidth, int oldHeight )
{
  entryField->setPos( 2, 2 );
  entryField->setSize( width-4 - buttonWidth - 1, height-4 );

  // Need to resize listbox, since the dialog can't handle that

  listHeight = (orgListHeight * height) / orgHeight;
  listBox->setSize( width, listHeight );
  //listBox->setPos( x, y-listHeight );
  //listBox->setOnTop();
  placeListBox();
}

void ComboBox::setText( char *text )
{
  entryField->setText( text );

	// Find the text in the listbox if it is available.

	int findIdx = listBox->findItem( text, FALSE );

	if( findIdx != -1 )
  {
		listBox->selectItem( findIdx );
    performSelectionEvent( findIdx );
	}
}

// Returns border size + buttonWidth if !horz, otherwise border size

int ComboBox::getInset( int horz )
{
  if( horz )
    return EntryField::getInset()*2+buttonWidth+4;
  else
    return 4;
}

BOOL Toolbar::ToolButton::actionPerformed( AMouseEvent *e )
{
  if( e->getEvent() == AMouseEvent::MOVE )
  {
    toolbar->lastX = (e->getPos())->x + getX();
    toolbar->lastY = (e->getPos())->y + getY();

    if( moving )
    {
      int x = (e->getPos())->x + getX();
      ((Toolbar *)getParent())->moveButton( x );
    }
    else
    if( button == AMouseEvent::DOWN )
    {
      Point *pos = e->getPos();
      if( pos->x < 0 || pos->x >= getWidth() ||
          pos->y < 0 || pos->y >= getHeight() )
      {
        if( state == ToolButton::DOWN )
        {
          state = ToolButton::UP;
          repaint( NULL );
        }
      }
      else
      {
        if( state == ToolButton::UP )
        {
          state = ToolButton::DOWN;
          repaint( NULL );
        }
      }
    }
  }
  else
  // Left button handles pushes, right button moves
  if( e->getButton() == AMouseEvent::LEFT &&
      queryToolActive() == Graphics::ACTIVE )
  {
    if( (e->getEvent() == AMouseEvent::DOWN ||
         e->getEvent() == AMouseEvent::DOUBLECLICK) &&
         !moving )
    {
      state = ToolButton::DOWN;
      repaint( NULL );
      button = AMouseEvent::DOWN;
    }
    else
    if( e->getEvent() == AMouseEvent::RELEASE )
    {
      if( state == ToolButton::DOWN )
        ((Toolbar *)getParent())->notifyObservers( command );
      state = ToolButton::UP;
      repaint( NULL );
      button = AMouseEvent::RELEASE;
    }
  }
  else  // Move this toolbarbutton
  if( e->getButton() == AMouseEvent::RIGHT )
  {
    if( e->getEvent() == AMouseEvent::DOWN && !moving)
    {
      // Pickup this button
      ((Toolbar *)getParent())->pickupButton( this,
          (e->getPos())->x );
      moving = TRUE;
    }
    else
    if( e->getEvent() == AMouseEvent::RELEASE && moving )
    {
      // Release button
      int x = (e->getPos())->x + getX();
      ((Toolbar *)getParent())->placeButton( x );
			((Toolbar *)getParent())->toolStateServer->buttonMoved();
      moving = FALSE;
    }
  }
  return TRUE;
}

void Toolbar::ToolButton::setActive( BOOL isActive ) 
{ 
	this->isActive = isActive; 
	repaint(NULL);
}

Toolbar::ImageButton::ImageButton( char *filename, BmpGraphics *g, char *command, 
                                   char *description, Toolbar *parent,
																	 int id, int x, int y, int width, int height ) :
  ToolButton( command, description, parent, id, x, y, width, height )
{
  this->g = g;
  image = g->readBitmap( filename );
  image->switchBackgroundColor( Color::BKGR );
  this->filename = new char[strlen(filename)+1];
  strcpy( this->filename, filename );
  type = Toolbar::IMAGE;
  this->image = image;
}

void Toolbar::ImageButton::paint( Rect *rect, Graphics *g )
{
  g->setBackColor(Color::BKGR);
  g->fillRect(rect);
  if( state == UP )
  {
    g->drawBitmap( image, queryToolActive(), buttonMargin-1, buttonMargin, width-Toolbar::buttonMargin*2, height-Toolbar::buttonMargin*2 );
  }
  else
  {
    g->drawBitmap( image, queryToolActive(), buttonMargin, buttonMargin-1, width-Toolbar::buttonMargin*2, height-Toolbar::buttonMargin*2 );
    g->setColor( Color::GRAY );
    g->drawLine( 0, 0, 0, height-1 );
    g->drawLine( 0, height-1, width, height-1 );
    g->setColor( Color::WHITE );
    g->drawLine( width-1, height-1, width-1, 0 );
    g->drawLine( width-1, 0, 0,0 );
  }
}

Toolbar::ToolButton *Toolbar::toolButtonAt( int x, int y )
{
  ToolButton *toolButton;

  for( int i = 0; i < buttonList.getSize(); i++ )
  {
    toolButton = (ToolButton *)buttonList.getObjectAt(i);

    if( x >= toolButton->getX() && x <= toolButton->getX() + toolButton->getWidth() &&
        y >= toolButton->getY() && y <= toolButton->getY() + toolButton->getHeight() )
      return toolButton;
  }

  return NULL;
}

void ToolbarThread::run()
{
  while( 1 )
  {
    // Sleep for a while

    System::sleep( 500 );
    if( exitThread ) break;
    toolbar->checkPopup();
  }
}

void Toolbar::checkPopup()
{
  // Check if the cursor is at the same place as before

  int cx, cy;

  getCursorPos( cx, cy );

  if( cx == lastX && lastX == lastSampleX && 
      cy == lastY && lastY == lastSampleY && 
      popupWindow == NULL)
  {
    activeBubbleButton = toolButtonAt(cx, cy);

    if( activeBubbleButton != NULL )
    {
      if( activeBubbleButton->getType() != SPACE )
      {
        Point pnt( cx, cy );
        Application::executeFromMainThread( this, 0, &pnt );
      }
    }
  }

  if( popupWindow != NULL )
  {
    // Check if the window should be closed, or a new one opened
    // on a different toolbutton

    ToolButton *currentTB = toolButtonAt(cx, cy);
    
    if( currentTB == NULL )
    {
      // Close popupWindow
      Application::executeFromMainThread( this, 1, NULL );
    }
    else
    if( activeBubbleButton != currentTB )
    {
      // Close popupWindow and open a new one with new text
      activeBubbleButton = currentTB;
      Application::executeFromMainThread( this, 1, NULL );
      if( activeBubbleButton->getType() != SPACE )
      {
        Point pnt( cx, cy );
        Application::executeFromMainThread( this, 0, &pnt );
      }
    }
  }

  lastSampleX = cx;
  lastSampleY = cy;
}

void Toolbar::notifyObservers( char *command )
{
  for( int i = 0; i < toolbarListener.getSize(); i++ )
  {
    AToolbarEvent toolEvent( this, command );
    toolbarListener.getObjectAt( i )->actionPerformed( &toolEvent );
  }
}

int Toolbar::xPosAt( int pos )
{
  int xPos = horzMargin;
  for( int i = 0; i < pos; i++ )
    xPos += ((ToolButton *)buttonList.getObjectAt(i))->getWidth() + spacing;
  return xPos;
}

void Toolbar::pushButtons( int start, int end, int dir )
{
  for( int i = start; i < end; i++ )
  {
    ToolButton *toolButton = (ToolButton *)buttonList.getObjectAt(i);
    toolButton->setPos( toolButton->getX() + dir,
                        toolButton->getY() );
  }
}

void Toolbar::pickupButton( ToolButton *button, int xPosInside )
{
  // Remove button from the buttonList
  int i = buttonList.findObject( button );
  if( i != -1 )
  {
    buttonList.removeAt(i);
    lastPickedButtonPos = i;
    pickedUpButton = button;
    // We got the button
    hotspot = xPosInside;
    repaint(NULL, TRUE);
  }
}

void Toolbar::moveButton( int xPos )
{
  // See where it fits
  ToolButton *toolButton;
  xPos -= hotspot;
  for( int i = 0; i < buttonList.getSize(); i++ )
  {
    // Check if we cover a button more than two-third
    toolButton = (ToolButton *)buttonList.getObjectAt(i);

    if( (xPos >= toolButton->getX() &&
         xPos < toolButton->getX() + toolButton->getWidth() &&
         xPos - toolButton->getX() < pickedUpButton->getWidth() / 3) ||
        (xPos < toolButton->getX() &&
         xPos + pickedUpButton->getWidth() > toolButton->getX() &&
         toolButton->getX() - xPos < (pickedUpButton->getWidth()*2) / 3) )
    {
      // Push
      if( i >= lastPickedButtonPos )
      {
        pushButtons( lastPickedButtonPos, i+1, -(pickedUpButton->getWidth() + spacing) );
        lastPickedButtonPos = i+1;
      }
      else
      {
        pushButtons( i, lastPickedButtonPos, (pickedUpButton->getWidth() + spacing) );
        lastPickedButtonPos = i;
      }
      break;
    }
  }
  if( buttonList.getSize() > 0 )
  {
    toolButton = (ToolButton *)buttonList.getObjectAt(buttonList.getSize()-1);
    if( xPos > toolButton->getX() + toolButton->getWidth() + spacing )
      xPos = toolButton->getX() + toolButton->getWidth() + spacing;
    if( xPos < 0 )
      xPos = 0;
  }
  pickedUpButton->setPos( xPos, pickedUpButton->getY() );
  pickedUpButton->setOnTop();
}

void Toolbar::recalcButtonsPos()
{
  // Place all buttons

  for( int i = 0; i < buttonList.getSize(); i++ )
  {
    ToolButton *toolButton = (ToolButton *)buttonList.getObjectAt(i);
    toolButton->setPos( xPosAt(i), toolButton->getY() );
  }
}

void Toolbar::setButtonActive( int id, BOOL active )
{
  for( int i = 0; i < buttonList.getSize(); i++ )
  {
    ToolButton *toolButton = (ToolButton *)buttonList.getObjectAt(i);
		if( toolButton->getId() == id )
			toolButton->setActive( active );
  }
}

void Toolbar::placeButton( int xPos )
{
  moveButton( xPos );
  xPos -= hotspot;

  buttonList.insertAt( lastPickedButtonPos, pickedUpButton );
  recalcButtonsPos();
}

void Toolbar::size( int, int )
{
  // Resize all buttons
  int xPos = horzMargin;

  for( int i = 0; i < buttonList.getSize(); i++ )
  {
    ToolButton *toolButton = (ToolButton *)buttonList.getObjectAt(i);
    if( toolButton->getType() == IMAGE )
    {
      toolButton->setPos( xPos, toolButton->getY() );
      toolButton->setSize( height - 2 * vertMargin + buttonMargin*2, height - 2 * vertMargin + buttonMargin*2 );
      xPos += height - 2 * vertMargin  + buttonMargin*2 + spacing;
    }
    else
    if( toolButton->getType() == SPACE )
    {
      toolButton->setPos( xPos, toolButton->getY() );
      toolButton->setSize( (height - 2 * vertMargin) / 2,
                           height - 2 * vertMargin );
      xPos += (height - 2 * vertMargin + spacing) / 2;
    }
  }
}

BOOL Toolbar::addImageButton( char *filename, char *command, char *description, int id )
{
  // Check if the command already exists in the toolbar

  for( int i = 0; i < buttonList.getSize(); i++ )
  {
    if( !strcmp( buttonList.getObjectAt(i)->getCommand(), command ) )
      return FALSE;
  }

  ImageButton *imageButton =
    new ImageButton( filename, &bmpGraphics, command, description, this,
                     id, xPosAt( buttonList.getSize() ), vertMargin-buttonMargin,
                     height - 2 * vertMargin + buttonMargin*2, height - 2 * vertMargin + buttonMargin*2 );
  imageButton->addMenuListener( this );
  imageButton->setVisible( TRUE );
  buttonList.add( imageButton );
  return TRUE;
}

BOOL Toolbar::addSpace()
{
  Space *space =
    new Space( this,
               xPosAt( buttonList.getSize() ), vertMargin,
               (height - 2 * vertMargin)/3, height - 2 * vertMargin );
  space->setVisible( TRUE );
  space->addMenuListener( this );
  buttonList.add( space );
  return TRUE;
}

BOOL Toolbar::ImageButton::actionPerformed( AMouseEvent *e )
{
  if( e->getButton() == AMouseEvent::RIGHT )
    if( e->getEvent() == AMouseEvent::CLICK )
    {
      // Popup a menu where the user can select to remove or modify the
      // button
      Point *pos = e->getPos();
      AMenuItem *remove = new AMenuItem( Strings::get(10), 101, AMenuEntry::MENUITEM );
      AMenu *menu = new AMenu( "Toolbar", 100 );
      menu->addMenuItem( remove );
      if( ((Toolbar *)parent)->toolStateServer != NULL )
      {
        AMenuItem *modify = new AMenuItem( Strings::get(11), 102, AMenuEntry::MENUITEM );
        menu->addMenuItem( modify );
      }
      popupMenu( menu, pos->x, pos->y );
      delete menu;
      return TRUE;
    }
  // Cascade mouseevent
  ToolButton::actionPerformed( e );
  return TRUE;
}

void Toolbar::actionPerformed( AMenuEvent *menuEvent )
{
  ToolButton *toolButton = (ToolButton *)menuEvent->getSource();
  switch( menuEvent->getId() )
  {
    case 101 :  // Remove button
    {
      // Remove the button
			Application::executeFromMainThreadPost(this, 2, (void *)toolButton );
      break;
    }
    case 102 :// Modify button
    {
      // Only imagebuttons can be modified
      ImageButton *imageButton = (ImageButton *)toolButton;
      if( toolbarListener.getSize() != 0 )
      {
        char *filename = imageButton->getFilename();
        char *command = imageButton->getCommand();
        char *description = imageButton->getDescription();
        if( toolStateServer->modifyButton( &filename, &command,
                                           &description ) )
          imageButton->modify( filename, command, description );
      }
      break;
    }
  }
}

BOOL Toolbar::getToolAt( int pos, int &type, char **command, char **image, char **description, int &id )
{
	ToolButton *toolButton = (ToolButton *)buttonList.getObjectAt(pos);

	type = toolButton->getType();

	*command = toolButton->getCommand();
	*description = toolButton->getDescription();
	id = toolButton->getId();

	if( toolButton->getType() == IMAGE )
		*image = ((ImageButton *)toolButton)->getFilename();
	else
		*image = NULL;

	return TRUE;
}

int Toolbar::execFromMainThread( int task, void *param )
{
  switch( task )
  {
    case 0 : // Create popupWindow
    {
      Point *pnt = (Point *)param;

      popupWindow = new PopupWindow( this, activeBubbleButton->getDescription(), 
                                     pnt->x+1, pnt->y+1, TRUE );
      popupWindow->setVisible( TRUE, FALSE );
      break;
    }
    case 1 : // close popupWindow
      delete popupWindow;
      popupWindow = NULL;
      break;
    case 2 :
	    ToolButton *toolButton = (ToolButton *)param;

	    if( buttonList.removeObject( toolButton ) )
	    {
		    delete toolButton;
		    recalcButtonsPos();
		    repaint( NULL );
		    toolStateServer->buttonDeleted( toolButton->getId() );
	    }
      break;
  }
	return TRUE;
}

BOOL Toolbar::Space::actionPerformed( AMouseEvent *e )
{
  if( e->getButton() == AMouseEvent::RIGHT )
    if( e->getEvent() == AMouseEvent::CLICK )
    {
      // Popup a menu where the user can select to remove the sapce
      Point *pos = e->getPos();
      AMenuItem *remove = new AMenuItem( Strings::get(12), 101, AMenuEntry::MENUITEM );
      AMenu *menu = new AMenu( "Toolbar", 100 );
      menu->addMenuItem( remove );
      popupMenu( menu, pos->x, pos->y );
      delete menu;
      return TRUE;
    }
  // Cascade mouseevent
  ToolButton::actionPerformed( e );
  return TRUE;
}

void Toolbar::Space::paint( Rect *rect, Graphics *g )
{
  g->setBackColor( Color::BKGR );
  g->fillRect( rect );
  g->setColor(Color::GRAY);
  g->drawLine(width/2, 0, width/2, height);
}

int ValueSet::RGBCOLOR = 1;
int ValueSet::TEXT = 1;

ValueSet::ValueSet( Window *parent, int properties, int x, int y, int width, int height, int cols, int rows, int type ) :
					Window( parent, xlatProperties( properties|FILTERDIALOGARROWS), x, y, width, height ), 
					type(type), cols(cols), rows(rows), hasFocus(FALSE)
{
	if( type == RGBCOLOR )
		grid = new ColorValue[cols*rows];
	else
		text = new char*[cols*rows];
	position = 0;
	addMouseListener(this);
	addKeyListener(this);
  parent->addControl( this );
}

void ValueSet::paint( Rect *rect, Graphics *g )
{
	Rect f(rect->x, rect->y, rect->width-1, rect->height-1 );

	g->setBackColor(Color::BKGR);
	g->fillRect(&f);

	g->drawShadedBox(0, 0, width-1, height-1, Color::SHADOW, Color::LIGHT );
	g->drawShadedBox(1, 1, width-3, height-3, Color::BLACK, Color::BKGR );
	
	int cWidth = (width-7) / cols;
	int cHeight = (height-7) / rows;

	for( int x = 0; x < cols; x++ )
		for( int y = 0; y < rows; y++ )
		{
			if( position == x+(y*cols) ) 
				g->setColor( (hasFocus) ? Color::BLACK : Color::GRAY );
			else
				g->setColor(Color::BKGR);

			Rect r(x*cWidth + 3, (rows-y-1)*cHeight + 3, cWidth - 1, cHeight - 1);
			g->drawRect(&r);

			Rect c(x*cWidth + 5, (rows-y-1)*cHeight + 5, cWidth - 4, cHeight - 4);

			if( type == RGBCOLOR )
			{
				g->setBackColor(grid[x+(y*cols)]);
				g->fillRect(&c);
			}
		}
}

void ValueSet::setCursorPos( int col, int row )
{
	int oldX, oldY;

	getCursorPos( oldX, oldY );

	position = col + (row*cols);

	int cWidth = (width-7) / cols;
	int cHeight = (height-7) / rows;

	Rect r(oldX*cWidth + 3, (rows-oldY-1) * cHeight + 3, cWidth, cHeight );
//	getGraphics()->setColor( Color::BKGR );
//	getGraphics()->drawRect(&r);
	repaint(&r);

	Rect r2(col*cWidth+3, (rows-row-1) * cHeight + 3, cWidth, cHeight );
	repaint(&r2);
/*	getGraphics()->setColor( (hasFocus) ? Color::BLACK : Color::GRAY );
	getGraphics()->drawRect(&r2);*/
}

void ValueSet::getCursorPos( int &col, int &row )
{
	row = position / cols;
	col = position - row * cols;
}

void ValueSet::setColor( int col, int row, ColorValue color )
{
	grid[col+(row*cols)] = color;
}

ColorValue ValueSet::getColor( int col, int row )
{
	return grid[col+(row*cols)];
}

BOOL ValueSet::actionPerformed( AMouseEvent *mouseEvent )
{
	static BOOL buttonDown;

	if( mouseEvent->getEvent() == AMouseEvent::DOWN ||
			(mouseEvent->getEvent() == AMouseEvent::MOVE && buttonDown) )
	{
		if( !hasFocus ) setFocus();

		buttonDown = TRUE;

		int xPos = mouseEvent->getPos()->x * cols / width;
		int yPos = rows - mouseEvent->getPos()->y * rows / height - 1;
		int oldX, oldY;

		getCursorPos( oldX, oldY );

		if( (xPos == oldX && yPos == oldY) || xPos < 0 || xPos >= cols ||
				yPos < 0 || yPos >= rows ) return TRUE;

		setCursorPos( xPos, yPos );
		performSelectionEvent(position);
	}

	if( mouseEvent->getEvent() == AMouseEvent::RELEASE )
		buttonDown = FALSE;
	return TRUE;
}

BOOL ValueSet::actionPerformed( AKeyEvent *keyEvent )
{
	int col, row;
	getCursorPos( col, row );

	if( keyEvent->key == KeyDef::RIGHT )
	{
		if( col < cols - 1 )
			col++;
	}
	else	
	if( keyEvent->key == KeyDef::LEFT )
	{
		if( col > 0 )
			col--;
	}
	else	
	if( keyEvent->key == KeyDef::UP )
	{
		if( row > 0 )
			row--;
	}
	else
	if( keyEvent->key == KeyDef::DOWN )
	{
		if( row < rows - 1)
			row++;
	}
	else
		return FALSE;

	setCursorPos( col, row );
	performSelectionEvent(position);
	return TRUE;
}

void ValueSet::focusSet( BOOL newFocus )
{
	hasFocus = newFocus;
	
	int x, y;

	// Redraw cursor

	getCursorPos( x, y );
	setCursorPos( x, y );	
}

ValueSet::~ValueSet()
{
	if( type == RGBCOLOR )
		delete grid;
	else
		delete text;
}

int HSeparator::defaultHeight = 8;
int HSeparator::buttonWidth = 30;

HSeparator::HSeparator( Window *parent, int x, int y, int width, int height ) :
					Window( parent, x, y, width, height ), minimized( FALSE )
{
  restoreY = y;
  buttonDown = FALSE;
	addMouseListener(this);
  parent->addControl( this );
  setPointer(PTR_MOVE);
}

void HSeparator::paint( Rect *rect, Graphics *g )
{
	//Rect f(rect->x, rect->y+1, rect->width, rect->height-1 );

	g->setBackColor(Color::BKGR);
	g->fillRect(rect);

	g->setColor(Color::WHITE);
  g->drawLine( 0, this->height - 1, this->width, this->height - 1 );
	g->setColor(Color::GRAY);
  g->drawLine( 0, 0, this->width, 0 );

  // Draw button

  Point *point[3];

  if( getY() == restoreY )
  {
   point[0] = new Point( getButtonXPos(), height - 3 );
   point[1] = new Point( getButtonXPos() + buttonWidth / 5, height - 3 );
   point[2] = new Point( getButtonXPos() + buttonWidth / 5 / 2, 2 );
  }
  else
  {
   point[0] = new Point( getButtonXPos(), 2 );
   point[1] = new Point( getButtonXPos() + buttonWidth / 5 / 2, height - 3 );
   point[2] = new Point( getButtonXPos() + buttonWidth / 5, 2 );
  }

  g->setColor( Color::GRAY );
  g->fillPolygon( 3, point );

  for( int i = 0; i < 3; i++ )
    point[i]->x += buttonWidth - buttonWidth / 5;

  g->fillPolygon( 3, point );

  g->drawLine( getButtonXPos() + buttonWidth / 3, defaultHeight / 3, 
               getButtonXPos() + (buttonWidth * 2) / 3, defaultHeight / 3 );
  g->drawLine( getButtonXPos() + buttonWidth / 3, (defaultHeight * 2) / 3, 
               getButtonXPos() + (buttonWidth * 2) / 3, (defaultHeight * 2) / 3 );

  g->setColor( Color::WHITE );
  g->drawLine( getButtonXPos() + buttonWidth / 3-1, defaultHeight / 3+1, 
               getButtonXPos() + (buttonWidth * 2) / 3-1, defaultHeight / 3+1 );
  g->drawLine( getButtonXPos() + buttonWidth / 3-1, (defaultHeight * 2) / 3+1, 
               getButtonXPos() + (buttonWidth * 2) / 3-1, (defaultHeight * 2) / 3+1 );

  delete point[0];
  delete point[1];
  delete point[2];
}

BOOL HSeparator::actionPerformed( AMouseEvent *mouseEvent )
{
	static BOOL buttonDown;
	static int yPos;

	if( mouseEvent->getEvent() == AMouseEvent::DOWN )
	{
		buttonDown = TRUE;
		yPos = mouseEvent->getPos()->y;
	}	
  else
	if( mouseEvent->getEvent() == AMouseEvent::RELEASE )
		buttonDown = FALSE;
  else
  if( mouseEvent->getEvent() == AMouseEvent::CLICK )
  {
    if( mouseEvent->getPos()->x >= getButtonXPos() && mouseEvent->getPos()->x < getButtonXPos() + buttonWidth )
    {
      if( minimized )
        sRestore();
      else  
        sMinimize();
    }
  }

	if( mouseEvent->getEvent() == AMouseEvent::MOVE && 
      mouseEvent->getPos()->y != yPos )
  {
    if( buttonDown )
    {
      minimized = FALSE;
      int y = mouseEvent->getPos()->y - yPos;

      y = this->y + y;

      if( y + defaultHeight > parent->getHeight() )
        y = parent->getHeight() - defaultHeight;
      
      if( y < 0 ) y = 0;

      restoreY = y;
    
      this->setPos(this->x, y );
    }
    else
    {
      int x = mouseEvent->getPos()->x;

      if( x >= getButtonXPos() && x < getButtonXPos() + buttonWidth )
        setPointer( PTR_HAND );
      else
        setPointer( PTR_MOVE );
    }
  }

	return TRUE;
}

int HSeparator::getButtonXPos()
{
  return (getWidth() - buttonWidth) / 2;
}

void HSeparator::sRestore()
{
  minimized = FALSE;
  this->setPos(this->x, restoreY );
#if defined __OS2__
  ((Window *)parent)->repaint( NULL );
#endif
}

void HSeparator::sMinimize()
{
  minimized = TRUE;
  this->setPos(this->x, 0 );
}
