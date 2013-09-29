#include <direct.h>
#include <string.h>
#include "bowl.h"
#include "prefsdlg.h"
#include "globals.h"
#include "mainwadm.h"

PrefsDialog::PrefsDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, Strings::get( 168 ),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 500, 365 )
{
  char *namestr = "PrefsDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*2 - 20,
      interSpace = fontHeight + fontHeight / 2;

  prevUseBigFont = AGraphics::useBigFont;

  Notebook *notebook =
	  new Notebook( this, 0, 50, getClientWidth(), getClientHeight() - 50, 
		              Color::BKGR, 0 );

	asciiDialog = new AsciiDialog( notebook, notebook );
	ascii2Dialog = new Ascii2Dialog( notebook, notebook );
	hexDialog = new HexDialog( notebook, notebook );
	editorDialog = new EditorDialog( notebook, notebook );
	displayDialog = new DisplayDialog( notebook, notebook );
	miscDialog = new MiscDialog( notebook, notebook );

	notebook->add( editorDialog, Strings::get(188) );
	notebook->add( displayDialog, Strings::get(190) );
	notebook->add( asciiDialog, Strings::get(166) );
	notebook->add( ascii2Dialog, Strings::get(209) );
	notebook->add( hexDialog, Strings::get(179) );
	notebook->add( miscDialog, Strings::get(196) );

  OKButton = new Button( this, Strings::get(8), AWindow::ISDEFAULT|ISTABSTOP,
                             10, 10, 70, 30 );
  OKButton->setYMove( FOLLOW );

  cancelButton = new Button( this, Strings::get(9), ISTABSTOP, 90, 10, 70, 30 );
  cancelButton->setYMove( FOLLOW );

	setControlsVisible();
  notebook->setTab( 0 );

  OKButton->addButtonListener( this );
  cancelButton->addButtonListener( this );

  adjustForFontSize();

  // Save/restore window position

  initSize();

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, x, y, width, height );

  setSize( width, height );
  center( x, y );
  setPos( x, y );
}

void PrefsDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == OKButton )
	{
		editorDialog->saveSettings();
		asciiDialog->saveSettings();
		ascii2Dialog->saveSettings();
		hexDialog->saveSettings();
		displayDialog->saveSettings();
		miscDialog->saveSettings();
		closeDialog( TRUE );
		return;
	}
  closeDialog( FALSE );
}

PrefsDialog::~PrefsDialog()
{
  if( prevUseBigFont != AGraphics::useBigFont )
  {
    // Clear all window sizes

    WinProperties::clearProperties();
  }
  else
  {
    WinProperties::updateProperty( name, getX(), getY(),
                                   getWidth(), getHeight() );
  }
	delete name;
	delete asciiDialog;
	delete hexDialog;
	delete editorDialog;
	delete displayDialog;
	delete miscDialog;
}

FontPreviewWindow::FontPreviewWindow( Window *parent, int X, int Y, int height, int width ) :
	Window( parent, X, Y, height, width )
{
	font = NULL;
	fg = Color::BLACK;
	bg = Color::WHITE;
  parent->addControl( this );
}

void FontPreviewWindow::selectFont( Font *font, int X, int Y, int points )
{
	this->font = font;
	this->X = X;
	this->Y = Y;
	this->points = points;
}

void FontPreviewWindow::setColor( ColorValue fg, ColorValue bg )
{
	this->fg = fg;
	this->bg = bg;
}

void FontPreviewWindow::paint( Rect *, Graphics *g )
{
	if( font == NULL ) return;

	if( X == -1 )
		g->selectFont( font, points, 0 );
	else
		g->selectFont( font, X, Y, 0 );

	g->setBackColor( bg );
	Rect rect(0, 0, width, height);
	g->fillRect(&rect);
	g->setColor( fg );

	char *testStr = font->name;

	g->drawString( width / 20, (height - g->getFontHeight()) / 2 + g->getFontBaseline(), 
	               strlen( testStr ), testStr );
	g->setColor( Color::BLACK );
	rect.height--;
	rect.width--;
	g->drawRect( &rect );
}

ColorValue AsciiDialog::colors[20];

AsciiDialog::AsciiDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, "",
          NOTITLE,
          0, 0, frame->getClientWidth(), frame->getClientHeight() - Notebook::getTabInset() )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight,
      interSpace = fontHeight + fontHeight / 2,
      labelHeight = fontHeight + fontHeight / 3;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;
  int editBoxInterSpace = editBoxHeight + fontHeight / 3;

	int top = yStart;

  yStart -= fontHeight * 2 + fontHeight / 2;

	int top2 = yStart;

	Label *lbl = new Label( this, Label::LEFT, Strings::get(129), 20,
                    yStart, 50, labelHeight );

	fontCombo = new SComboBox( this, ISTABSTOP|SComboBox::STATIC, 20, yStart,
                            240, editBoxHeight, 100 );

  int comboHeight = fontCombo->getEditBoxHeight();
  yStart -= comboHeight + comboHeight / 2;
  fontCombo->setPos( fontCombo->getX(), yStart);
  //lbl->setSize( lbl->getWidth(), comboHeight);
  //lbl->setPos( lbl->getX(), yStart);

	yStart = top2;

	lbl = new Label( this, Label::LEFT, Strings::get(130), 270,
                    yStart, 50, labelHeight );

  yStart -= comboHeight + comboHeight / 2;

	sizeCombo = new SComboBox( this, ISTABSTOP|NUMERIC, 270, yStart,
                            	      80, editBoxHeight, 100 );
  sizeCombo->setMaxLength( 5 );
  yStart -= fontHeight*2;

  fixedWidthOnlyCheck  = new CheckBox( this, Strings::get(131),
                                       ISTABSTOP, 20, yStart, 150,
                                       fontHeight*2 );

  bitmapFontsCheck  = new CheckBox( this, Strings::get(167),
                                    ISTABSTOP, 190, yStart, 150,
                                    fontHeight*2 );

  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( this, Strings::get(128), 0, 10, yStart,
                                 360, top - yStart );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 2 + 15;

	lbl = new Label( this, Label::RIGHT, Strings::get(171), 20,
                    yStart, 90, labelHeight );

	textItemCombo = new SComboBox( this, ISTABSTOP|SComboBox::STATIC, 120, yStart,
                            	      width - 380, editBoxHeight, 100 );

  yStart -= interSpace;

  fgButton = new RadioButton( this, Strings::get(169),
                              NEWGROUP | ISTABSTOP, 20, yStart, 100,
                              interSpace );

  bgButton = new RadioButton( this, Strings::get(170),
                              INGROUP, 130, yStart, 100,
                              interSpace );


  yStart -= fontHeight/4 + 60;

	colorSet = new ValueSet( this, ISTABSTOP, 20, yStart, width - 280, fontHeight * 3.5, 10, 2, ValueSet::RGBCOLOR );

	yStart -= fontHeight;

  gBox = new GroupBox( this, Strings::get(128), 0, 10, yStart, width-260, top - yStart );

	colors[0] = Color::BLACK;
	colors[1] = Color::GRAY;
	colors[2] = Color::DARKRED;
	colors[3] = Color::BROWN;
	colors[4] = Color::DARKGREEN;
	colors[5] = Color::DARKCYAN;
	colors[6] = Color::DARKBLUE;
	colors[7] = Color::DARKPINK;
	colors[8] = Color::LIGHT;
	colors[9] = Color::SHADOW;
	colors[10] = Color::WHITE;
	colors[11] = Color::LIGHTGRAY;
	colors[12] = Color::RED;
	colors[13] = Color::YELLOW;
	colors[14] = Color::GREEN;
	colors[15] = Color::CYAN;
	colors[16] = Color::BLUE;
	colors[17] = Color::PINK;
	colors[18] = Color::BKGR;
	colors[19] = Color::DEFAULT;

	for( int yy = 0; yy < 2; yy++ )
		for( int xx = 0; xx < 10; xx++ )
			colorSet->setColor( xx, yy, colors[yy*10+xx] );

	colorSet->setVisible(TRUE);

	fontPreviewWindow = new FontPreviewWindow( this, width - 220, (top-10-70) / 2, 200, 70 );
  fontPreviewWindow->setWMove( FOLLOW );
  fontPreviewWindow->setHMove( FOLLOW );

	if( Settings::asciiFont == NULL )
	{
		Settings::asciiFont = new char[strlen(Graphics::getEditorFontName())+1];
		strcpy( Settings::asciiFont, Graphics::getEditorFontName());
		Settings::asciiFontSizeP = Graphics::getEditorFontSize();
	}

	fontPreviewWindow->selectFont( getGraphics()->getFontFromName( Settings::asciiFont, 0 ), 
	                               -1, -1, Settings::asciiFontSizeP );

	// Fill in fonts

	char numstr[30];
	int i;

	FontList *fonts = getGraphics()->getFonts();

	for( i = 0; i < fonts->fonts; i++ )
		if( (Settings::asciiFixedWidthOnly && fonts->font[i].monoSpaced) || 
				!Settings::asciiFixedWidthOnly  )
			fontCombo->addItem( fonts->font[i].name );

	fontCombo->addSelectionListener(this);

  if( Settings::asciiFontSizeP != -1 )
		sprintf( numstr, "%d", Settings::asciiFontSizeP );
	else
		sprintf( numstr, "%dx%d", Settings::asciiFontSizeX, 
             Settings::asciiFontSizeY );

	sizeCombo->addSelectionListener(this);

	if( Settings::asciiFixedWidthOnly )
		fixedWidthOnlyCheck->check(TRUE);	

	textItemCombo->addItem( Strings::get( 172 ) );
	textItemCombo->addItem( Strings::get( 173 ) );
	textItemCombo->addItem( Strings::get( 174 ) );
  textItemCombo->addItem( Strings::get( 214 ) );
  textItemCombo->addItem( Strings::get( 215 ) );
  textItemCombo->setText( Strings::get( 172 ) );

	textItemCombo->addSelectionListener( this );

	fontCombo->setText( Settings::asciiFont  );

	textFg = Settings::textFg;
	textBg = Settings::textBg;
	markFg = Settings::markFg;
	markBg = Settings::markBg;
	hiliteFg = Settings::hiliteFg;
	hiliteBg = Settings::hiliteBg;
	markerFg = Settings::markerFg;
	markerBg = Settings::markerBg;
	leftMarginFg = Settings::leftMarginFg;
	leftMarginBg = Settings::leftMarginBg;

  currentColorSet = -1;
  currentCp = 0;

  setColor( textFg );

	fgButton->check(TRUE);

	sizeCombo->setText( numstr );

  fontCombo->setFocus();

	fixedWidthOnlyCheck->addButtonListener(this);
	bitmapFontsCheck->addButtonListener(this);
	fgButton->addButtonListener(this);
	bgButton->addButtonListener(this);
	colorSet->addSelectionListener( this );
	
	updateControlPos( NULL );

	setControlsVisible();		
}

void AsciiDialog::setColor( ColorValue c )
{
	for( int i = 0; i < 20; i++ )
  	if( c == colors[i] )
    {
    	int col, row;

      row = i / 10;
      col = i - row * 10;
    	colorSet->setCursorPos( col, row );
    }
}

void AsciiDialog::setColorFromSelected()
{
  int col, row;
  colorSet->getCursorPos( col, row );
  ColorValue color = colorSet->getColor( col, row );

  switch( currentColorSet )
  {
  	case 0 : 
    	if( currentCp == 0 )
      	textFg = color;
      else
      	textBg = color;
      break;
  	case 1 : 
    	if( currentCp == 0 )
      	markFg = color;
      else
      	markBg = color;
      break;
  	case 2 : 
    	if( currentCp == 0 )
      	hiliteFg = color;
      else
      	hiliteBg = color;
      break;
  	case 3: 
    	if( currentCp == 0 )
      	leftMarginFg = color;
      else
      	leftMarginBg = color;
      break;
  	case 4: 
    	if( currentCp == 0 )
      	markerFg = color;
      else
      	markerBg = color;
  }

  if( fgButton->queryCheck() ) 
  	currentCp = 0;
  else
  	currentCp = 1;

  if( !strcmp( textItemCombo->getEditText(), 
               Strings::get( 172 ) ) )
	{
  	fontPreviewWindow->setColor( textFg, textBg );
  	setColor( (currentCp == 0) ? textFg : textBg );
    currentColorSet = 0;
	}
  else
 	if( !strcmp( textItemCombo->getEditText(), 
                Strings::get( 173 ) ) )
	{
  	fontPreviewWindow->setColor( markFg, markBg );
  	setColor( (currentCp == 0) ? markFg : markBg );
    currentColorSet = 1;
  }
  else
  if( !strcmp( textItemCombo->getEditText(), 
               Strings::get( 174 ) ) )
  {
  	fontPreviewWindow->setColor( hiliteFg, hiliteBg );
    setColor( (currentCp == 0) ? hiliteFg : hiliteBg );
    currentColorSet = 2;
  }
  else
  if( !strcmp( textItemCombo->getEditText(), 
               Strings::get( 214 ) ) )
  {
  	fontPreviewWindow->setColor( leftMarginFg, leftMarginBg );
    setColor( (currentCp == 0) ? leftMarginFg : leftMarginBg );
    currentColorSet = 3;
  }
  else
  if( !strcmp( textItemCombo->getEditText(), 
               Strings::get( 215 ) ) )
  {
  	fontPreviewWindow->setColor( markerFg, markerBg );
    setColor( (currentCp == 0) ? markerFg : markerBg );
    currentColorSet = 4;
  }
	fontPreviewWindow->repaint(NULL);
}

void AsciiDialog::saveSettings()
{
	delete Settings::asciiFont;
  Settings::asciiFont = new char[fontCombo->getTextLength()+1];
	strcpy( Settings::asciiFont, fontCombo->getEditText() );
	int size, X = -1, Y = -1;

  if( !strchr( sizeCombo->getEditText(), 'x' ) )
		size = atoi(sizeCombo->getEditText());
  else
	{
		// Bitmapped font
		sscanf( sizeCombo->getEditText(), "%dx%d", &X, &Y );
		size = -1;
	}
	Settings::asciiFontSizeP = size;
	Settings::asciiFontSizeX = X;
	Settings::asciiFontSizeY = Y;
	Settings::asciiBitmappedOnly = bitmapFontsCheck->queryCheck();
	Settings::asciiFixedWidthOnly = fixedWidthOnlyCheck->queryCheck();
	Settings::textBg = textBg;
	Settings::textFg = textFg;
	Settings::markBg = markBg;
	Settings::markFg = markFg;
	Settings::markerFg = markerFg;
	Settings::markerBg = markerBg;
	Settings::leftMarginFg = leftMarginFg;
	Settings::leftMarginBg = leftMarginBg;
}

void AsciiDialog::actionPerformed( ASelectionEvent *selectionEvent )
{
	BOOL updatePreview = FALSE;
  if( selectionEvent->getSource() == fontCombo )
  {  
    char *font = ((SComboBox *)selectionEvent->getSource())->getEditText();

		// Find the font in the fontlist

		FontList *fonts = getGraphics()->getFonts();
	
		for( int i = 0; i < fonts->fonts; i++ )
		{
			if( !strcmp( fonts->font[i].name, font ) )
			{
				sizeCombo->clearItems();

				char sizeText[30];
		
				if( fonts->font[i].sizes > 0 )
				{
					int xRes = fonts->font[i].size[0].devX, 
					      yRes = fonts->font[i].size[0].devY;
					for( int s = 0; s < fonts->font[i].sizes; s++ )
					{
						if( xRes != fonts->font[i].size[s].devX ||
						    yRes != fonts->font[i].size[s].devY ) break;
						sprintf( sizeText, "%dx%d", fonts->font[i].size[s].X, fonts->font[i].size[s].Y );
						sizeCombo->addItem( sizeText );
					}
				}
				else
				{
				  int s;
					for( s = 8; s <= 12; s++ )
					{
						sprintf( sizeText, "%d", s );
						sizeCombo->addItem( sizeText );
					}
					for( s = 14; s <= 24; s += 2 )
					{
						sprintf( sizeText, "%d", s );
						sizeCombo->addItem( sizeText );
					}
				}
				sizeCombo->getItem( 0, 30, sizeText );
				sizeCombo->setText( sizeText );
				updatePreview = TRUE;
			}
		}
	}
	else
	if( selectionEvent->getSource() == sizeCombo )
		updatePreview = TRUE;
	else
  if( selectionEvent->getSource() == textItemCombo )
  	setColorFromSelected();
	else
  if( selectionEvent->getSource() == colorSet )
  	setColorFromSelected();

	if( updatePreview )
	{
		int size = -1, X = -1, Y = -1;
    
    char fontSize[10];
    sizeCombo->getSelectedItem( 5, fontSize );

    if( !strchr( fontSize, 'x' ) )
			size = atoi(fontSize);
    else
			// Bitmapped font
			sscanf( fontSize, "%dx%d", &X, &Y );

		fontPreviewWindow->selectFont(getGraphics()->getFontFromName(fontCombo->getEditText(), 0 ),
		                               X, Y, size );	
		fontPreviewWindow->repaint(NULL);
	}
}

void AsciiDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == fixedWidthOnlyCheck )
  {
		FontList *fonts = getGraphics()->getFonts();

		fontCombo->clearItems();

		for( int i = 0; i < fonts->fonts; i++ )
		{
			if( (fixedWidthOnlyCheck->queryCheck() && fonts->font[i].monoSpaced) || 
					!fixedWidthOnlyCheck->queryCheck() )
				fontCombo->addItem( fonts->font[i].name );
		}
		fontCombo->selectItem( 0 );
    return;
  }
	else
  if( buttonEvent->getSource() == fgButton ||
  		buttonEvent->getSource() == bgButton )
  	setColorFromSelected();
}

Ascii2Dialog::Ascii2Dialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, "",
          NOTITLE,
          0, 0, frame->getClientWidth(), frame->getClientHeight() - Notebook::getTabInset() )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight,
      interSpace = fontHeight + fontHeight / 2,
      labelHeight = fontHeight + fontHeight / 3;

	int top = yStart;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;

	yStart -= fontHeight * 3;

  lineNumbersCheck  = new CheckBox( this, Strings::get(210),
                                         NEWGROUP|ISTABSTOP, 20, yStart, 150,
                                         fontHeight*2 );

  savedPositionsCheck  = new CheckBox( this, Strings::get(211),
                                         INGROUP, 180, yStart, 150,
                                         fontHeight*2 );

  yStart -= interSpace;

	Label *lbl = new Label( this, Label::RIGHT, Strings::get(213), 20,
                   yStart, 150, labelHeight );

	leftMarginCharsEntry = new EntryField( this, ISTABSTOP|BORDER, 180, yStart,
                                   	    130, editBoxHeight );
	
  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( this, Strings::get(212), 0, 10, yStart,
                                 360, top - yStart );

	if( Settings::showLineNumbers )
		lineNumbersCheck->check( TRUE );

	if( Settings::showSavedPos )
		savedPositionsCheck->check( TRUE );

	char str[20];

	sprintf( str, "%d", Settings::leftMarginChars );
	leftMarginCharsEntry->setText( str );

	lineNumbersCheck->setFocus();

	setControlsVisible();		
}

void Ascii2Dialog::saveSettings()
{
	Settings::showLineNumbers = lineNumbersCheck->queryCheck();
	Settings::showSavedPos = savedPositionsCheck->queryCheck();
	Settings::showLeftMarginInfo = (Settings::showLineNumbers || Settings::showSavedPos);
	Settings::leftMarginChars = atoi(leftMarginCharsEntry->getText());
}

HexDialog::HexDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, "",
          NOTITLE,
          0, 0, frame->getClientWidth(), frame->getClientHeight() - Notebook::getTabInset() )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight,
      interSpace = fontHeight + fontHeight / 2;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;
  int editBoxInterSpace = editBoxHeight + fontHeight / 3,
      labelHeight = fontHeight + fontHeight / 3;

	int top = yStart;

  yStart -= fontHeight * 2 + fontHeight / 2;

	int top2 = yStart;

	Label *lbl = new Label( this, Label::LEFT, Strings::get(129), 20,
                    yStart, 50, labelHeight );

	fontCombo = new SComboBox( this, ISTABSTOP|SComboBox::STATIC, 20, yStart,
                            240, editBoxHeight, 100 );

  int comboHeight = fontCombo->getEditBoxHeight();
  yStart -= comboHeight + comboHeight / 2;
  fontCombo->setPos( fontCombo->getX(), yStart);

	yStart = top2;

	lbl = new Label( this, Label::LEFT, Strings::get(130), 270,
                    yStart, 50, labelHeight );

  yStart -= comboHeight + comboHeight / 2;

	sizeCombo = new SComboBox( this, ISTABSTOP|NUMERIC, 270, yStart,
                            	      80, editBoxHeight, 100 );
  sizeCombo->setMaxLength( 5 );

  yStart -= fontHeight*2;

  bitmapFontsCheck = new CheckBox( this, Strings::get(167),
                                   ISTABSTOP, 20, yStart, 150,
                                   fontHeight*2 );

  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( this, Strings::get(128), 0, 10, yStart,
                                 360, top - yStart );

  yStart -= fontHeight / 2;

  top = yStart;

	fontPreviewWindow = new FontPreviewWindow( this, width - 220, (top-10-70) / 2, 200, 70 );
  fontPreviewWindow->setWMove( FOLLOW );
  fontPreviewWindow->setHMove( FOLLOW );

	if( Settings::hexFont == NULL )
	{
		Settings::hexFont = new char[strlen(Graphics::getHexEditorFontName())+1];
		strcpy( Settings::hexFont, Graphics::getHexEditorFontName());
		Settings::hexFontSizeP = Graphics::getHexEditorFontSize();
	}

	fontPreviewWindow->selectFont( getGraphics()->getFontFromName( Settings::hexFont, 0 ), 
	                               -1, -1, Settings::hexFontSizeP );


  yStart -= fontHeight * 3;

  numericInfoCheck = new CheckBox( this, Strings::get(231), ISTABSTOP, 20, yStart, 150, fontHeight * 2 );

  yStart -= interSpace;

  linesHexCheck = new CheckBox( this, Strings::get(246), ISTABSTOP, 20, yStart, width-280, fontHeight * 2 );

  yStart -= interSpace;

  hideAscii32Check = new CheckBox( this, Strings::get(254), ISTABSTOP, 20, yStart, width-280, fontHeight * 2 );

	yStart -= fontHeight / 2;

  gBox = new GroupBox( this, Strings::get(232), 0, 10, yStart, width-260, top - yStart );

	// Fill in fonts

	char numstr[30];
	int i;

	FontList *fonts = getGraphics()->getFonts();

	for( i = 0; i < fonts->fonts; i++ )
		if( fonts->font[i].monoSpaced )
			fontCombo->addItem( fonts->font[i].name );

	fontCombo->addSelectionListener(this);

  if( Settings::hexFontSizeP != -1 )
		sprintf( numstr, "%d", Settings::hexFontSizeP );
	else
		sprintf( numstr, "%dx%d", Settings::hexFontSizeX, 
             Settings::hexFontSizeY );

	sizeCombo->addSelectionListener(this);

	fontCombo->setText( Settings::hexFont  );

	sizeCombo->setText( numstr );

  fontCombo->setFocus();

  numericInfoCheck->check( Settings::showBytesInfo );
  linesHexCheck->check( Settings::linesInHex );
  hideAscii32Check->check( Settings::hideAscii32 );

	bitmapFontsCheck->addButtonListener(this);

	updateControlPos( NULL );

	setControlsVisible();		
}

void HexDialog::saveSettings()
{
	delete Settings::hexFont;
  Settings::hexFont = new char[fontCombo->getTextLength()+1];
	strcpy( Settings::hexFont, fontCombo->getEditText() );
	int size, X = -1, Y = -1;

  if( !strchr( sizeCombo->getEditText(), 'x' ) )
		size = atoi(sizeCombo->getEditText());
  else
	{
		// Bitmapped font
		sscanf( sizeCombo->getEditText(), "%dx%d", &X, &Y );
		size = -1;
	}
	Settings::hexFontSizeP = size;
	Settings::hexFontSizeX = X;
	Settings::hexFontSizeY = Y;
	Settings::hexBitmappedOnly = bitmapFontsCheck->queryCheck();
  Settings::showBytesInfo = numericInfoCheck->queryCheck();
  Settings::linesInHex = linesHexCheck->queryCheck();
  Settings::hideAscii32 = hideAscii32Check->queryCheck();
}

void HexDialog::actionPerformed( ASelectionEvent *selectionEvent )
{
	BOOL updatePreview = FALSE;
  if( selectionEvent->getSource() == fontCombo )
  {
    char *font = ((SComboBox *)selectionEvent->getSource())->getEditText();

		// Find the font in the fontlist

		FontList *fonts = getGraphics()->getFonts();
	
		for( int i = 0; i < fonts->fonts; i++ )
		{
			if( !strcmp( fonts->font[i].name, font ) )
			{
				SComboBox *sizesCombo;

				if( selectionEvent->getSource() == fontCombo )
					sizesCombo = sizeCombo;

				sizesCombo->clearItems();

				char sizeText[30];
		
				if( fonts->font[i].sizes > 0 )
				{
					int xRes = fonts->font[i].size[0].devX, 
					      yRes = fonts->font[i].size[0].devY;
					for( int s = 0; s < fonts->font[i].sizes; s++ )
					{
						if( xRes != fonts->font[i].size[s].devX ||
						    yRes != fonts->font[i].size[s].devY ) break;
						sprintf( sizeText, "%dx%d", fonts->font[i].size[s].X, fonts->font[i].size[s].Y );
						sizesCombo->addItem( sizeText );
					}
				}
				else
				{
				  int s;
					for( s = 8; s <= 12; s++ )
					{
						sprintf( sizeText, "%d", s );
						sizesCombo->addItem( sizeText );
					}
					for( s = 14; s <= 24; s += 2 )
					{
						sprintf( sizeText, "%d", s );
						sizesCombo->addItem( sizeText );
					}
				}
				sizesCombo->getItem( 0, 30, sizeText );
				sizesCombo->setText( sizeText );
				updatePreview = TRUE;
			}
		}
	}
	else
	if( selectionEvent->getSource() == sizeCombo )
		updatePreview = TRUE;

	if( updatePreview )
	{
		int size = -1, X = -1, Y = -1;
    
    char fontSize[10];
    sizeCombo->getSelectedItem( 5, fontSize );

    if( !strchr( fontSize, 'x' ) )
			size = atoi(fontSize);
    else
			// Bitmapped font
			sscanf( fontSize, "%dx%d", &X, &Y );

		fontPreviewWindow->selectFont(getGraphics()->getFontFromName(fontCombo->getEditText(), 0 ),
		                               X, Y, size );	
		fontPreviewWindow->repaint(NULL);
	}
}

void HexDialog::actionPerformed( AButtonEvent *buttonEvent )
{
/*  if( buttonEvent->getSource() == fixedWidthOnlyCheck )
  {
		FontList *fonts = getGraphics()->getFonts();

		fontCombo->clearItems();

		for( int i = 0; i < fonts->fonts; i++ )
		{
			if( (fixedWidthOnlyCheck->queryCheck() && fonts->font[i].monoSpaced) || 
					!fixedWidthOnlyCheck->queryCheck() )
				fontCombo->addItem( fonts->font[i].name );
		}
		fontCombo->selectItem( 0 );
    return;
  }*/
}

EditorDialog::EditorDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, "",
          NOTITLE,
          0, 0, frame->getClientWidth(), frame->getClientHeight() - Notebook::getTabInset() )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight,
      interSpace = fontHeight + fontHeight / 2;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;
  int editBoxInterSpace = editBoxHeight + fontHeight / 3,
      labelHeight = fontHeight + fontHeight / 3;

	int top = yStart;

	yStart -= fontHeight * 3;

  persistentBlocksCheck  = new CheckBox( this, Strings::get(180),
                                         NEWGROUP|ISTABSTOP, 20, yStart, 150,
                                         fontHeight*2 );

  useTabCheck  = new CheckBox( this, Strings::get(202),
                                         INGROUP, 180, yStart, 100,
                                         fontHeight*2 );

	yStart -= interSpace;

  optimalFillCheck  = new CheckBox( this, Strings::get(203),
                                         INGROUP, 20, yStart, 150,
                                         fontHeight*2 );

  virtualSpaceCheck  = new CheckBox( this, Strings::get(230),
                                         INGROUP, 180, yStart, 150,
                                         fontHeight*2 );

  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( this, Strings::get(186), 0, 10, yStart,
                                 360, top - yStart );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 3;

	Label *lbl = new Label( this, Label::RIGHT, Strings::get(181), 20,
                   yStart, 150, labelHeight );

	tabSizeEntry = new EntryField( this, ISTABSTOP|NEWGROUP|BORDER|NUMERIC, 180, yStart,
                               	    50, editBoxHeight );
	
  yStart -= editBoxInterSpace;

	lbl = new Label( this, Label::RIGHT, Strings::get(183), 20,
                   yStart, 150, labelHeight );

	delimEntry = new EntryField( this, ISTABSTOP|BORDER, 180, yStart,
                               	    130, editBoxHeight );
	
  yStart -= editBoxInterSpace;

	lbl = new Label( this, Label::RIGHT, Strings::get(182), 20,
                   yStart, 150, labelHeight );

	maxUndoSizeEntry = new EntryField( this, ISTABSTOP|BORDER|NUMERIC, 180, yStart,
                               	    50, editBoxHeight );
	
  yStart -= fontHeight / 2;

  gBox = new GroupBox( this, Strings::get(187), 0, 
  										 10, yStart,
                       320, top - yStart );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 3;

  CRLFButton = new RadioButton( this, Strings::get(184),
                                            NEWGROUP|ISTABSTOP, 20, yStart, 70,
                                            fontHeight*2 );

  LFButton = new RadioButton( this, Strings::get(185),
                                            0, 100, yStart, 70,
                                            fontHeight*2 );
  yStart -= fontHeight / 2;

  gBox = new GroupBox( this, Strings::get(189), 0, 
  										 10, yStart,
                       180, top - yStart );

	char str[100];

	if( Settings::persistentBlocks )
		persistentBlocksCheck->check( TRUE );

	if( Settings::useTab )
		useTabCheck->check( TRUE );

	if( Settings::optimalFill )
		optimalFillCheck->check( TRUE );

	if( Settings::virtualSpace )
		virtualSpaceCheck->check( TRUE );

	sprintf( str, "%d", Settings::tabSize );
	tabSizeEntry->setText( str );

	char *delim = new char[strlen(Settings::standardDelim) * 2 + 1];

	int len = 0;

	for( int i = 0; i < strlen( Settings::standardDelim ); i++ )
	{
		if( Settings::standardDelim[i] == 0x9 )
		{
			delim[len++] = '\\';
			delim[len++] = 't';
		}
		else
		if( Settings::standardDelim[i] == '\\' )
		{
			delim[len++] = '\\';
			delim[len++] = '\\';
		}
		else
			delim[len++] = Settings::standardDelim[i];
	}

	delim[len] = 0;

	delimEntry->setText( delim );

	delete delim;

	sprintf( str, "%d", Settings::maxUndoSize / 1024 );
	maxUndoSizeEntry->setText( str );

	if( Settings::CRLF )
		CRLFButton->check(TRUE);
	else
		LFButton->check(TRUE);

	persistentBlocksCheck->setFocus();

	setControlsVisible();		
}

void EditorDialog::saveSettings()
{
	Settings::persistentBlocks = persistentBlocksCheck->queryCheck();
	Settings::useTab = useTabCheck->queryCheck();
	Settings::optimalFill = optimalFillCheck->queryCheck();
	Settings::virtualSpace = virtualSpaceCheck->queryCheck();
	Settings::tabSize = atoi( tabSizeEntry->getText() );

	if( Settings::tabSize < 2 ) Settings::tabSize = 2;

	if( Settings::delimAllocated )
		delete Settings::standardDelim;

	Settings::standardDelim = new char[delimEntry->getTextLength() + 1];
	
	int len = 0;
	char *eText = delimEntry->getText();

	for( int i = 0; i < delimEntry->getTextLength(); i++ )
	{
		if( eText[i] == '\\' )
		{
			i++;
			if( eText[i] == 't' )
				Settings::standardDelim[len++] = 0x9;
			else
				Settings::standardDelim[len++] = eText[i];
		}
		else
			Settings::standardDelim[len++] = eText[i];
	}

	Settings::standardDelim[len++] = 0;

	Settings::maxUndoSize = atol( maxUndoSizeEntry->getText() ) * 1024;

	Settings::CRLF = CRLFButton->queryCheck();
}

DisplayDialog::DisplayDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, "",
          NOTITLE,
          0, 0, frame->getClientWidth(), frame->getClientHeight() - Notebook::getTabInset() )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight,
      interSpace = fontHeight + fontHeight / 2,
      labelHeight = fontHeight + fontHeight / 3;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;

	int top = yStart;

	yStart -= fontHeight * 3;

  syntaxHighlightCheck  = new CheckBox( this, Strings::get(192),
                                         ISTABSTOP, 20, yStart, 150,
                                         fontHeight*2 );

  showSectionsCheck  = new CheckBox( this, Strings::get(248),
                                     ISTABSTOP, 180, yStart, 150,
                                     fontHeight*2 );

  yStart -= interSpace;

  autogenSectionsCheck  = new CheckBox( this, Strings::get(249),
                                         ISTABSTOP, 20, yStart, 250,
                                         fontHeight*2 );

  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( this, Strings::get(191), 0, 10, yStart,
                                 360, top - yStart );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 3;

  showRightMarginCheck  = new CheckBox( this, Strings::get(194),
                                         ISTABSTOP, 20, yStart, 150,
                                         fontHeight*2 );

  yStart -= interSpace;

	Label *lbl = new Label( this, Label::RIGHT, Strings::get(195), 20,
                   yStart, 150, labelHeight );

	rightMarginEntry = new EntryField( this, ISTABSTOP|BORDER, 180, yStart,
                               	    130, editBoxHeight );
	
  yStart -= fontHeight / 2;

  gBox = new GroupBox( this, Strings::get(187), 0, 
  										 10, yStart,
                       320, top - yStart );

	char str[100];

	if( Settings::syntaxHighlight )
		syntaxHighlightCheck->check( TRUE );

	if( Settings::showRightMargin )
		showRightMarginCheck->check( TRUE );

	if( Settings::showSectionBrowser )
		showSectionsCheck->check( TRUE );

	if( Settings::autogenerateSections )
		autogenSectionsCheck->check( TRUE );

	sprintf( str, "%d", Settings::rightMargin );
	rightMarginEntry->setText( str );

	syntaxHighlightCheck->setFocus();

	setControlsVisible();		
}

void DisplayDialog::saveSettings()
{
	Settings::syntaxHighlight = syntaxHighlightCheck->queryCheck();
	Settings::showRightMargin = showRightMarginCheck->queryCheck();
	Settings::showSectionBrowser = showSectionsCheck->queryCheck();
	Settings::autogenerateSections = autogenSectionsCheck->queryCheck();
	Settings::rightMargin = atoi( rightMarginEntry->getText() );
}

MiscDialog::MiscDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, "",
          NOTITLE,
          0, 0, frame->getClientWidth(), frame->getClientHeight() - Notebook::getTabInset() )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight,
      interSpace = fontHeight + fontHeight / 2,
      labelHeight = fontHeight + fontHeight / 3;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;

	int top = yStart;

	yStart -= fontHeight * 3;

  backupFilesCheck  = new CheckBox( this, Strings::get(198),
                                         NEWGROUP|ISTABSTOP, 20, yStart, 150,
                                         fontHeight*2 );

  useSystemFileDialogCheck  = new CheckBox( this, Strings::get(204),
                                         INGROUP, 180, yStart, 150,
                                         fontHeight*2 );

  yStart -= interSpace;

  useBigFontCheck  = new CheckBox( this, Strings::get(207),
                                         INGROUP, 20, yStart, 150,
                                         fontHeight*2 );

  autoCloseFrameWindowCheck  = new CheckBox( this, Strings::get(216),
                                         INGROUP, 180, yStart, 170,
                                         fontHeight*2 );

  yStart -= interSpace;

  showCommandBarCheck  = new CheckBox( this, Strings::get(220),
                                         INGROUP, 20, yStart, 150,
                                         fontHeight*2 );

  reuseExistingBufferCheck = new CheckBox( this, Strings::get(233),
                                         INGROUP, 180, yStart, 170,
                                         fontHeight*2 );

  yStart -= interSpace;

  showToolbarCheck = new CheckBox( this, Strings::get(253),
                                         INGROUP, 20, yStart, 170,
                                         fontHeight*2 );

  yStart -= interSpace;

  queryUnicodeCheck  = new CheckBox( this, Strings::get(240),
                                         INGROUP, 20, yStart, 250,
                                         fontHeight*2 );
  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( this, Strings::get(186), 0, 10, yStart,
                                 360, top - yStart );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 3;

  autoSaveCheck  = new CheckBox( this, Strings::get(200),
                                         ISTABSTOP, 20, yStart, 150,
                                         fontHeight*2 );

  yStart -= interSpace;

	Label *lbl = new Label( this, Label::RIGHT, Strings::get(201), 20,
                   yStart, 150, labelHeight );

	autosaveDelayEntry = new EntryField( this, ISTABSTOP|BORDER, 180, yStart,
                               	    130, editBoxHeight );
	
  yStart -= fontHeight / 2;

  gBox = new GroupBox( this, Strings::get(199), 0, 
  										 10, yStart,
                       320, top - yStart );

	char str[100];

	if( Settings::backupFiles )
		backupFilesCheck->check( TRUE );

	if( Settings::autoSave )
		autoSaveCheck->check( TRUE );

	if( Settings::useSystemFileDialog )
		useSystemFileDialogCheck->check( TRUE );

  if( Settings::autoCloseFrameWindow )
		autoCloseFrameWindowCheck->check( TRUE );

  if( Settings::reuseExistingBuffer )
		reuseExistingBufferCheck->check( TRUE );

  if( Settings::showCommandBar )
    showCommandBarCheck->check( TRUE );
    
  if( Settings::showToolbar )
		showToolbarCheck->check( TRUE );

 	queryUnicodeCheck->check( Settings::queryIfUnicode );

	if( AGraphics::useBigFont )
		useBigFontCheck->check( TRUE );

	sprintf( str, "%d", Settings::autosaveDelay );
	autosaveDelayEntry->setText( str );

	backupFilesCheck->setFocus();

	setControlsVisible();
}

void MiscDialog::saveSettings()
{
	Settings::backupFiles = backupFilesCheck->queryCheck();
	Settings::autoSave = autoSaveCheck->queryCheck();
	Settings::autosaveDelay = atoi( autosaveDelayEntry->getText() );
	Settings::useSystemFileDialog = useSystemFileDialogCheck->queryCheck();
  Settings::autoCloseFrameWindow = autoCloseFrameWindowCheck->queryCheck();
  Settings::showCommandBar = showCommandBarCheck->queryCheck();
  Settings::reuseExistingBuffer = reuseExistingBufferCheck->queryCheck();
  Settings::queryIfUnicode = queryUnicodeCheck->queryCheck();
  Settings::showToolbar = showToolbarCheck->queryCheck();
  AGraphics::useBigFont = useBigFontCheck->queryCheck();
	if( Settings::autosaveDelay < 30 ) Settings::autosaveDelay = 30;
}

