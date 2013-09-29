#include <direct.h>
#include <string.h>
#include "bowl.h"
#include "printdlg.h"
#include "cntrladm.h"
#include "editctrl.h"

BOOL PrintDialog::printColor = FALSE;
BOOL PrintDialog::printSyntaxHL = TRUE;
BOOL PrintDialog::twoUp = FALSE;
BOOL PrintDialog::evenPages = TRUE;
BOOL PrintDialog::oddPages = TRUE;
BOOL PrintDialog::fixedWidthOnly = TRUE;
BOOL PrintDialog::initialized = FALSE;
int PrintDialog::selection = DALL;
int PrintDialog::from = 1;
int PrintDialog::to = 1;
int PrintDialog::leftMargin = 1;
int PrintDialog::rightMargin = 1;
int PrintDialog::topMargin = 1;
int PrintDialog::bottomMargin = 1;
char PrintDialog::header[MAXHEADLEN];
char PrintDialog::footer[MAXHEADLEN];
char PrintDialog::printer[MAXPRINTERNAMELEN];

PrintDialog::PrintDialog( Window *frame, Window *parent ) :
  Dialog( frame, parent, Strings::get( 147 ),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 515, 330 ), closing(FALSE)
{
  char *namestr = "printDialog";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*4 - 10,
      interSpace = fontHeight + fontHeight / 2,
      labelHeight = fontHeight + fontHeight / 3;

	int top = yStart + fontHeight * 3;
  int pageTop = top, yStartTop = yStart;

  Notebook *notebook =
    new Notebook( this, 0, 0, getClientWidth(), getClientHeight(), 
		              Color::BKGR, 0 );

  sourceDlg = new Dialog( notebook, notebook, "", NOTITLE,0, 0,
                          getClientWidth(),
                          getClientHeight()-Notebook::getTabInset() );

  pageDlg = new Dialog( notebook, notebook, "", NOTITLE, 0, 0,
												getClientWidth(),
												getClientHeight()-Notebook::getTabInset() );

  headerDlg = new Dialog( notebook, notebook, "", NOTITLE, 0, 0,
                          getClientWidth(),
                          getClientHeight()-Notebook::getTabInset() );

  notebook->add( sourceDlg, Strings::get(163) );
  notebook->add( headerDlg, Strings::get(164) );
  notebook->add( pageDlg, Strings::get(165) );

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2;

  entireDocButton = new RadioButton( sourceDlg, Strings::get(120),
                                     NEWGROUP | ISTABSTOP, 20, yStart, 100,
                                     fontHeight*2 );

  yStart -= interSpace;

  selectedButton = new RadioButton( sourceDlg, Strings::get(121),
                                    INGROUP, 20, yStart, 100,
                                    fontHeight*2 );

  yStart -= interSpace;

  linesButton = new RadioButton( sourceDlg, Strings::get(122),
                                 INGROUP, 20, yStart, 100,
                                 fontHeight*2 );

	fromLineEntry = new EntryField( sourceDlg, BORDER | ISTABSTOP, 125, yStart,
                               	  50, editBoxHeight );

  Label *lbl = new Label( sourceDlg, Label::RIGHT, Strings::get(123), 175,
                          yStart, 30, labelHeight );

	toLineEntry = new EntryField( sourceDlg, BORDER | ISTABSTOP, 210, yStart,
                               	  50, editBoxHeight );

  yStart -= fontHeight / 2;

  GroupBox *gBox = new GroupBox( sourceDlg, Strings::get(119), 0, 10, yStart,
                                 260, top - yStart );

  yStart -= fontHeight / 2;

  int top2 = yStart;

  // ***

	yStart = top - fontHeight * 3;

  printColorCheck = new CheckBox( sourceDlg, Strings::get(125),
                            		  INGROUP | ISTABSTOP, 290, yStart, 200, 
                                  fontHeight*2 );

  yStart -= interSpace;

  printSyntaxHLCheck  = new CheckBox( sourceDlg, Strings::get(126),
                                      INGROUP, 290, yStart, 200,
                                      fontHeight*2 );

/*  yStart -= interSpace;

  printItalicCheck = new CheckBox( sourceDlg, Strings::get(127),
                                   INGROUP, 290, yStart, 200,
                                   fontHeight*2 );*/

  yStart -= fontHeight / 2;

  gBox = new GroupBox( sourceDlg, Strings::get(124), 0, 280, yStart,
                       220, top - yStart );

	// ***

  yStart -= fontHeight / 2;

  top = yStart;

  yStart = top2;	//******************* <--

  yStart -= fontHeight * 2 + fontHeight / 2;

  int top3 = yStart;

	lbl = new Label( sourceDlg, Label::LEFT, Strings::get(129), 20,
                   yStart, 50, labelHeight );

	textFontCombo = new SComboBox( sourceDlg, ISTABSTOP|SComboBox::STATIC, 20, yStart,
                            	  200, editBoxHeight, 100 );

  int comboHeight = textFontCombo->getEditBoxHeight();
  yStart -= comboHeight + comboHeight / 2;
  textFontCombo->setPos( textFontCombo->getX(), yStart);

	yStart = top3;

	lbl = new Label( sourceDlg, Label::LEFT, Strings::get(130), 230,
                   yStart, 50, labelHeight );

  yStart -= comboHeight + comboHeight / 2;

	textFontSizeCombo = new SComboBox( sourceDlg, ISTABSTOP|NUMERIC, 230, yStart,
                            	      80, editBoxHeight, 100 );

  yStart -= fontHeight*2;

  fixedWidthOnlyCheck  = new CheckBox( sourceDlg, Strings::get(131),
                                       ISTABSTOP, 20, yStart, 200,
                                       fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( sourceDlg, Strings::get(128), 0, 10, yStart,
                       320, top2 - yStart );

  yStart = 10;

  OK1Button = new Button( sourceDlg, Strings::get(147), AWindow::ISDEFAULT|ISTABSTOP,
                             10, yStart, 70, 30 );
  OK1Button->setYMove( FOLLOW );
  cancel1Button = new Button( sourceDlg, Strings::get(9), ISTABSTOP, 90, yStart, 70, 30 );
  cancel1Button->setYMove( FOLLOW );

  // ***** PAGE SETTINGS

  top = pageTop;
  yStart = yStartTop+fontHeight / 2;

	lbl = new Label( pageDlg, Label::LEFT, Strings::get(133), 20,
                   yStart, 50, labelHeight );

  yStart -= comboHeight + comboHeight / 2;

	printersCombo = new SComboBox( pageDlg, ISTABSTOP|SComboBox::STATIC, 20, yStart,
                            	  475, editBoxHeight, 100 );
  printersCombo->setWMove( FOLLOW );
  printersCombo->setMaxLength(255);

  yStart -= fontHeight;

  gBox = new GroupBox( pageDlg, Strings::get(132), 0, 10, yStart,
                       495, top - yStart );
  gBox->setWMove( FOLLOW );


	top2 = yStart - fontHeight / 2;
  top = pageTop;
	yStart = top2 - fontHeight * 3;

  twoUpCheck = new CheckBox( pageDlg, Strings::get(135),
                             ISTABSTOP, 250, yStart, 120,
                             fontHeight*2 );

  yStart -= interSpace;

  evenPagesCheck = new CheckBox( pageDlg, Strings::get(136),
                                 0, 250, yStart, 120,
                                 fontHeight*2 );

  yStart -= interSpace;

  oddPagesCheck = new CheckBox( pageDlg, Strings::get(137),
                                0, 250, yStart, 120,
                                fontHeight*2 );

  yStart -= fontHeight / 2;

  gBox = new GroupBox( pageDlg, Strings::get(134), 0, 
  										 240, yStart,
                       140, top2 - yStart );

	yStart = top2 - fontHeight / 2 - fontHeight * 2;

	lbl = new Label( pageDlg, Label::RIGHT, Strings::get(142), 20,
                   yStart, 70, labelHeight );

	leftMarginEntry = new EntryField( pageDlg, ISTABSTOP|NEWGROUP|BORDER, 100, yStart,
                               	    50, editBoxHeight );
	
  yStart -= editBoxHeight;

	lbl = new Label( pageDlg, Label::RIGHT, Strings::get(143), 20,
                   yStart, 70, labelHeight );

	rightMarginEntry = new EntryField( pageDlg, ISTABSTOP|BORDER, 100, yStart,
                               	    50, editBoxHeight );
	
  yStart -= editBoxHeight;

	lbl = new Label( pageDlg, Label::RIGHT, Strings::get(144), 20,
                   yStart, 70, labelHeight );

	topMarginEntry = new EntryField( pageDlg, ISTABSTOP|BORDER, 100, yStart,
                             	     50, editBoxHeight );
	
  yStart -= editBoxHeight;

	lbl = new Label( pageDlg, Label::RIGHT, Strings::get(145), 20,
                   yStart, 70, labelHeight );

	bottomMarginEntry = new EntryField( pageDlg, ISTABSTOP|BORDER, 100, yStart,
                               	      50, editBoxHeight );
	
  yStart -= fontHeight / 2;

  gBox = new GroupBox( pageDlg, Strings::get(141), 0, 
  										 10, yStart,
                       190, top2 - yStart );

  yStart = 10;

  OK2Button = new Button( pageDlg, Strings::get(147), AWindow::ISDEFAULT|ISTABSTOP,
                             10, yStart, 70, 30 );
  OK2Button->setYMove( FOLLOW );
  cancel2Button = new Button( pageDlg, Strings::get(9), ISTABSTOP, 90, yStart, 70, 30 );
  cancel2Button->setYMove( FOLLOW );
	setupButton = new Button( pageDlg, Strings::get(146), ISTABSTOP, pageDlg->getWidth() - 120, yStart, 110, 30 );
  setupButton->setYMove( FOLLOW );
  setupButton->setXMove( FOLLOW );

  // *** HEADER/FOOTER PAGE

  top = pageTop;
  yStart = yStartTop+fontHeight / 2;

	lbl = new Label( headerDlg, Label::LEFT, Strings::get(139), 20,
                   yStart, 200, labelHeight );

  yStart -= editBoxHeight;

	headerEntry = new EntryField( headerDlg, BORDER|ISTABSTOP, 20, yStart,
                               	300, editBoxHeight );
  headerEntry->setWMove( FOLLOW );
	
  yStart -= interSpace;

	lbl = new Label( headerDlg, Label::LEFT, Strings::get(140), 20,
                   yStart, 200, labelHeight );

  yStart -= editBoxHeight;

	footerEntry = new EntryField( headerDlg, BORDER|ISTABSTOP, 20, yStart,
                               	300, editBoxHeight );
  footerEntry->setWMove( FOLLOW );
	
  yStart -= fontHeight / 2;

  gBox = new GroupBox( headerDlg, Strings::get(138), 0, 
  										 10, yStart,
                       320, top - yStart );
  gBox->setWMove( FOLLOW );

  yStart -= fontHeight / 2;

  top = yStart;

  yStart -= fontHeight * 2 + fontHeight / 2;

	lbl = new Label( headerDlg, Label::LEFT, Strings::get(148), 20,
                   yStart, 200, labelHeight );

  yStart -= comboHeight + comboHeight / 2;

	headerFontCombo = new SComboBox( headerDlg, ISTABSTOP|SComboBox::STATIC, 20, yStart,
                            	  200, editBoxHeight, 100 );
  headerFontCombo->setWMove( FOLLOW );

	yStart = top;
	
  yStart -= fontHeight * 2 + fontHeight / 2;

	lbl = new Label( headerDlg, Label::LEFT, Strings::get(130), 230,
                   yStart, 80, labelHeight );
  lbl->setXMove( FOLLOW );

  yStart -= comboHeight + comboHeight / 2;

	headerFontSizeCombo = new SComboBox( headerDlg, ISTABSTOP|NUMERIC, 230, yStart,
                              	      80, editBoxHeight, 100 );
  headerFontSizeCombo->setXMove( FOLLOW );

  yStart -= fontHeight;

  gBox = new GroupBox( headerDlg, Strings::get(148), 0, 
  										 10, yStart,
                       320, top - yStart );
  gBox->setWMove( FOLLOW );

  yStart = 10;

  OK3Button = new Button( headerDlg, Strings::get(147), AWindow::ISDEFAULT|ISTABSTOP,
                             10, yStart, 70, 30 );
  OK3Button->setYMove( FOLLOW );
  cancel3Button = new Button( headerDlg, Strings::get(9), ISTABSTOP, 90, yStart, 70, 30 );
  cancel3Button->setYMove( FOLLOW );

	// Set default values
  entireDocButton->addButtonListener( this );
  linesButton->addButtonListener( this );
  selectedButton->addButtonListener( this );

	if( !initialized )
	{
		strcpy( header, "&d" );
		strcpy( footer, "&p" );
		strcpy( printer, "" );
		selection = DALL;
		initialized = TRUE;
	}

	if( !Print::initialized )
	{
		strcpy( Print::printerFont, "Courier" );
		strcpy( Print::headerFont, "Courier" );
		Print::initialized = TRUE;
	}

	// Fill in fonts

	char numstr[30];
	int i;

	FontList *fonts = getGraphics()->getFonts();

	for( i = 0; i < fonts->fonts; i++ )
		if( (fixedWidthOnly && fonts->font[i].monoSpaced) || 
				!fixedWidthOnly )
			textFontCombo->addItem( fonts->font[i].name );

	textFontCombo->addSelectionListener(this);
	textFontCombo->setText( Print::printerFont );
	sprintf( numstr, "%d", Print::printerFontSize );
	textFontSizeCombo->setText( numstr );


	for( i = 0; i < fonts->fonts; i++ )
		headerFontCombo->addItem( fonts->font[i].name );
	headerFontCombo->addSelectionListener(this);
	headerFontCombo->setText( Print::headerFont );

	sprintf( numstr, "%d", Print::headerFontSize );
	headerFontSizeCombo->setText( numstr );

	// Fill in printers

  char **printers;
  int count;
  PrinterGraphics::getPrinters( &printers, count );

	for( i = 0; i < count; i++ )
		printersCombo->addItem( printers[i] );

	if( strcmp( printer, "" ) )
		printersCombo->setText( printer );
	else
		printersCombo->selectItem( 0 );

	printColorCheck->check( printColor );
	printSyntaxHLCheck->check( printSyntaxHL );
	twoUpCheck->check( twoUp );
	evenPagesCheck->check( evenPages );
	oddPagesCheck->check( oddPages );
	fixedWidthOnlyCheck->check( fixedWidthOnly );

	fixedWidthOnlyCheck->addButtonListener( this);

	if( selection == DALL )
		entireDocButton->check( TRUE );
	else
	if( selection == DSELECTED )
		selectedButton->check( TRUE );
	else
		linesButton->check( TRUE );

	sprintf( numstr, "%d", from );
	fromLineEntry->setText( numstr );

	sprintf( numstr, "%d", to );
	toLineEntry->setText( numstr );

	sprintf( numstr, "%d", leftMargin );
	leftMarginEntry->setText( numstr );

	sprintf( numstr, "%d", rightMargin );
	rightMarginEntry->setText( numstr );

	sprintf( numstr, "%d", topMargin );
	topMarginEntry->setText( numstr );

	sprintf( numstr, "%d", bottomMargin );
	bottomMarginEntry->setText( numstr );

	headerEntry->setText( header );
	footerEntry->setText( footer );

  OK1Button->addButtonListener( this );
  cancel1Button->addButtonListener( this );
  OK2Button->addButtonListener( this );
  cancel2Button->addButtonListener( this );
  OK3Button->addButtonListener( this );
  cancel3Button->addButtonListener( this );
  setupButton->addButtonListener( this );

  sourceDlg->setControlsVisible();
  pageDlg->setControlsVisible();
  headerDlg->setControlsVisible();
	setControlsVisible();
  notebook->setTab( 0 );
	printersCombo->setFocus();
	headerEntry->setFocus();
	entireDocButton->setFocus();

	sourceDlg->updateControlPos( NULL );

  adjustForFontSize();

  // Save/restore window position

  initSize();

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, x, y, width, height );

  setSize( width, height );
  center( x, y );
  setPos( x, y );
}

void PrintDialog::actionPerformed( AButtonEvent *buttonEvent )
{
	if( buttonEvent->getSource() == entireDocButton ||
      buttonEvent->getSource() == selectedButton ||
      buttonEvent->getSource() == linesButton )
  {
    if( closing ) return; // Can't say why I need this!!
  	fromLineEntry->enable( linesButton->queryCheck() );
  	toLineEntry->enable( linesButton->queryCheck() );
  }
  else
  if( buttonEvent->getSource() == fixedWidthOnlyCheck )
  {
		FontList *fonts = getGraphics()->getFonts();

		textFontCombo->clearItems();

		for( int i = 0; i < fonts->fonts; i++ )
		{
			if( (fixedWidthOnlyCheck->queryCheck() && fonts->font[i].monoSpaced) || 
					!fixedWidthOnlyCheck->queryCheck() )
				textFontCombo->addItem( fonts->font[i].name );
		}
		textFontCombo->selectItem( 0 );
    return;
  }
	else
	if( buttonEvent->getSource() == setupButton )
	{
    PrinterGraphics::setupPrinter( printersCombo->getEditText() );
		return;
	}
	else
	if( buttonEvent->getSource() == OK1Button ||
			buttonEvent->getSource() == OK2Button ||
			buttonEvent->getSource() == OK3Button )
	{
		// Read values and set

		printColor = printColorCheck->queryCheck();
		printSyntaxHL = printSyntaxHLCheck->queryCheck();
		twoUp = twoUpCheck->queryCheck();
		evenPages = evenPagesCheck->queryCheck();
		oddPages = oddPagesCheck->queryCheck();
		fixedWidthOnly = fixedWidthOnlyCheck->queryCheck();

		if( entireDocButton->queryCheck() )
			selection = DALL;
		else
		if( selectedButton->queryCheck() )
			selection = DSELECTED;
		else
			selection = DFROMTO;

		from = atoi(fromLineEntry->getText());
		to = atoi(toLineEntry->getText());
	
		leftMargin = atoi(leftMarginEntry->getText());
		rightMargin = atoi(rightMarginEntry->getText());
		topMargin = atoi(topMarginEntry->getText());
		bottomMargin = atoi(bottomMarginEntry->getText());

		strcpy( header, headerEntry->getText() );
		strcpy( footer, footerEntry->getText() );
		strcpy( printer, printersCombo->getEditText() );

		strcpy( Print::printerFont, textFontCombo->getEditText() );
		Print::printerFontSize = atoi( textFontSizeCombo->getEditText() );
		strcpy( Print::headerFont, headerFontCombo->getEditText() );
    char fontSize[10];
    headerFontSizeCombo->getSelectedItem( 5, fontSize );
		Print::headerFontSize = atoi( fontSize );

    closing = TRUE;
		closeDialog( TRUE );
	}
	else
	if( buttonEvent->getSource() == cancel1Button ||
			buttonEvent->getSource() == cancel2Button ||
			buttonEvent->getSource() == cancel3Button )
	{
    closing = TRUE;
		closeDialog( FALSE );
	}
}

void PrintDialog::actionPerformed( ASelectionEvent *selectionEvent )
{
  if( selectionEvent->getSource() == textFontCombo ||
			selectionEvent->getSource() == headerFontCombo )
  {
    char *font = ((SComboBox *)selectionEvent->getSource())->getEditText();

		// Find the font in the fontlist

		FontList *fonts = getGraphics()->getFonts();
	
		for( int i = 0; i < fonts->fonts; i++ )
		{
			if( !strcmp( fonts->font[i].name, font ) )
			{
				SComboBox *sizesCombo;

				if( selectionEvent->getSource() == textFontCombo )
					sizesCombo = textFontSizeCombo;
				else
					sizesCombo = headerFontSizeCombo;

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
			}
		}
	}
}

void PrintDialog::saveSettings( FILE * f )
{
	long pos;
	if( !ClassProperties::savePropertyName( f, "PRINTDLG001", &pos ) ) return;
	fwrite( &printColor, sizeof( BOOL ), 1, f );
	fwrite( &printSyntaxHL, sizeof( BOOL ), 1, f );
	fwrite( &twoUp, sizeof( BOOL ), 1, f );
	fwrite( &evenPages, sizeof( BOOL ), 1, f );
	fwrite( &oddPages, sizeof( BOOL ), 1, f );
	fwrite( &fixedWidthOnly, sizeof( BOOL ), 1, f );
	fwrite( &initialized, sizeof( BOOL ), 1, f );
	fwrite( header, MAXHEADLEN, 1, f );
	fwrite( footer, MAXHEADLEN, 1, f );
	fwrite( printer, MAXPRINTERNAMELEN, 1, f );
	fwrite( &selection, sizeof( int ), 1, f );
	fwrite( &from, sizeof( int ), 1, f );
	fwrite( &to, sizeof( int ), 1, f );
	fwrite( &leftMargin, sizeof( int ), 1, f );
	fwrite( &rightMargin, sizeof( int ), 1, f );
	fwrite( &topMargin, sizeof( int ), 1, f );
	fwrite( &bottomMargin, sizeof( int ), 1, f );
	ClassProperties::savePropertySize( f, pos );
}

void PrintDialog::restoreSettings( FILE * f )
{
	if( !ClassProperties::checkPropertyExists( f, "PRINTDLG001" ) ) return;
	fread( &printColor, sizeof( BOOL ), 1, f );
	fread( &printSyntaxHL, sizeof( BOOL ), 1, f );
	fread( &twoUp, sizeof( BOOL ), 1, f );
	fread( &evenPages, sizeof( BOOL ), 1, f );
	fread( &oddPages, sizeof( BOOL ), 1, f );
	fread( &fixedWidthOnly, sizeof( BOOL ), 1, f );
	fread( &initialized, sizeof( BOOL ), 1, f );
	fread( header, MAXHEADLEN, 1, f );
	fread( footer, MAXHEADLEN, 1, f );
	fread( printer, MAXPRINTERNAMELEN, 1, f );
	fread( &selection, sizeof( int ), 1, f );
	fread( &from, sizeof( int ), 1, f );
	fread( &to, sizeof( int ), 1, f );
	fread( &leftMargin, sizeof( int ), 1, f );
	fread( &rightMargin, sizeof( int ), 1, f );
	fread( &topMargin, sizeof( int ), 1, f );
	fread( &bottomMargin, sizeof( int ), 1, f );
}

PrintDialog::~PrintDialog()
{
  WinProperties::updateProperty( name, getX(), getY(),
                                 getWidth(), getHeight() );
	delete name;
	delete sourceDlg;
	delete pageDlg;
	delete headerDlg;
}

AbortPrintDialog::AbortPrintDialog( Window *frame, Window *parent, Print *print ) :
  Dialog( frame, parent, Strings::get( 149 ),
          AWindow::CENTER | AWindow::ADJUSTSIZE,
          0, 0, 300, 200 ), print(print)
{
  char *namestr = "abortPrint";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*4 - 10,
      interSpace = fontHeight + fontHeight / 2;
  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2,
      labelHeight = fontHeight + fontHeight / 3;

	int top = yStart + fontHeight * 3;
  int pageTop = top, yStartTop = yStart;
	print->setAbortPrintDialog( this );

	Label *label = new Label( this, Label::RIGHT, Strings::get( 151 ), 10, yStart, 100, labelHeight );
	pageLabel = new Label( this, Label::LEFT, "", 110, yStart, 100, labelHeight );

  abortButton = new Button( this, Strings::get(150), AWindow::ISDEFAULT,
                             (width-100) / 2, 10, 100, 30 );
	
	setControlsVisible();

  adjustForFontSize();

  // Save/restore window position

  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, x, y, width, height );

  setSize( width, height );
  center( x, y );
  setPos( x, y );
}

void AbortPrintDialog::setPage( int page )
{
	char str[20];
	pageLabel->setText( itoa(page, str, 10 ) );
}

void AbortPrintDialog::actionPerformed( AButtonEvent *buttonEvent )
{
	if( buttonEvent->getSource() == abortButton )
	{
		print->abortPrint();
		closeDialog( 0 );
	}
}