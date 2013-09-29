#include <string.h>
#include <time.h>
#include "bowl.h"
#include "comdlg.h"
#include "globals.h"
#include "lic.h"

char QueryLineDialog::text[1000];

QueryLineDialog::QueryLineDialog( Window *frame, Window *parent, char *title, char *query,
																	char *initText ) :
  Dialog( frame, parent, title,
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 510, 130 )
{
  char *namestr = "title";
  name = new char[strlen(namestr)+1];
  strcpy( name, namestr );

  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*2 - 10,
      interSpace = fontHeight + fontHeight / 2;
  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2,
      labelHeight = fontHeight + fontHeight / 3;

	int top = yStart + fontHeight * 3;
  int pageTop = top, yStartTop = yStart;

	Label *label = new Label( this, Label::LEFT, query, 10, yStart, 100, labelHeight );

	yStart -= interSpace;

	textEntry = new EntryField( this, BORDER, 10, yStart, 490, editBoxHeight );
  textEntry->setWMove( FOLLOW );

  OKButton = new Button( this, Strings::get(8), AWindow::ISDEFAULT,
                         (width-100) / 2, 10, 100, 30 );
  OKButton->setXMove( PROPORTIONAL );	

	textEntry->setText( initText );
	textEntry->selectText();
	setControlsVisible();
	textEntry->setFocus();
	OKButton->addButtonListener( this );

  adjustForFontSize();

  // Save/restore window position

  initSize();
  int x = getX(), y = getY(), width = getWidth(), height = getHeight();
  WinProperties::retrieveProperty( name, x, y, width, height );

  setSize( width, height );
  center( x, y );
  setPos( x, y );
}

void QueryLineDialog::actionPerformed( AButtonEvent *buttonEvent )
{
	if( buttonEvent->getSource() == OKButton )
	{
		strcpy( text, textEntry->getText() );
		closeDialog( TRUE );
	}
	else
	if( buttonEvent->getSource() == cancelButton )
		closeDialog( FALSE );
}

AboutDialog::AboutDialog( Window *parent ) :
  Dialog( Desktop::getInstance(), parent, Strings::get(206),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 360, 300 )
{
	BmpGraphics bmpg;
	char bmpFile[MAXPATH+1];

	sprintf( bmpFile, "%s%s", KonSystem::baseDir , "konabout.bmp" );

	bmp = bmpg.readBitmap( bmpFile );

	AboutWindow *aboutWnd = new AboutWindow( this, 0, 50, getClientWidth(), 
	                                         getClientHeight()-50, bmp );
  aboutWnd->setWMove( FOLLOW );
  aboutWnd->setHMove( FOLLOW );
	addControl( aboutWnd );

  int buttonWidth = 100;
  int xPos = (width-buttonWidth) / 2;

  if( !KonSystem::registered )
    xPos = (width - (buttonWidth * 2 + 20)) / 2;

  OKButton = new Button( this, Strings::get(8), 
                         KonSystem::registered ? AWindow::ISDEFAULT : 0,
                         xPos, 10, buttonWidth, 30 );

  OKButton->addButtonListener(this);
  OKButton->setYMove( FOLLOW );
  OKButton->setXMove( PROPORTIONAL );

  if( !KonSystem::registered )
  {
    xPos += buttonWidth + 20;

    registerButton = new Button( this, Strings::get(223), 
                                 !KonSystem::registered ? AWindow::ISDEFAULT : 0,
                                 xPos, 10, buttonWidth, 30 );

    registerButton->addButtonListener(this);
  }

	setControlsVisible();

	OKButton->setFocus();
  initSize();
}

void AboutDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == registerButton )
  {
    RegisterDialog *registerDialog =
      new RegisterDialog( this );

    registerDialog->setVisible( TRUE );

    delete registerDialog;
  }
  closeDialog(1);
}

void AboutWindow::paint( Rect *rect, Graphics *g )
{
  int bmpX = 0;
  int bmpY = 35;

  g->setColor( Color::LIGHTGRAY );
  g->setBackColor( Color::BLACK );
  g->fillRect( rect );

	g->drawBitmap(bmp, 0, bmpX, bmpY, bmp->getWidth(), bmp->getHeight() );
  g->selectFont( "Courier", 8, 0 );
  int fontHeight = g->getFontHeight();
  char verStr[100];

  sprintf( verStr, "Version %d.%02d", KonSystem::major, KonSystem::minor );
  g->drawString( 10, 5+fontHeight*8, verStr );

  g->setColor( Color::RED );
  // g->drawString( 10, 5+fontHeight*4, "***ALPHA VERSION***" );

  if( !KonSystem::registered )
  {
    g->setColor( Color::RED );
    g->drawString( 10, 5+fontHeight*6, Strings::get(224) );
    g->setColor( Color::WHITE );
    g->drawString( 10, 5+fontHeight*5, Strings::get(225) );
    g->drawString( 10, 5+fontHeight*4, Strings::get(226) );
  }
  else
  {
    g->setColor( Color::WHITE );
    g->selectFont( "Courier", 8, Font::BOLD );
    g->drawString( 10, 5+fontHeight*6, "Registered to:" );
    g->selectFont( "Courier", 8, 0 );
    g->drawString( 10, 5+fontHeight*5, KonSystem::regName );
    //g->drawString( 10, 5+fontHeight*1, KonSystem::regKey );
  }
  
  g->setColor( Color::GREEN );
  g->selectFont( "Courier", 8, Font::BOLD );
  g->drawString( 10, 5+fontHeight*3, Strings::get(243) );
  g->selectFont( "Courier", 8, 0 );
  g->drawString( 150, 5+fontHeight*3, "http://www.koneditor.com" );
  g->selectFont( "Courier", 8, Font::BOLD );
  g->drawString( 10, 5+fontHeight*2, Strings::get(244) );
  g->selectFont( "Courier", 8, 0 );
  g->drawString( 150, 5+fontHeight*2, "support@koneditor.com" );

  time_t timeNow;
  time( &timeNow );

  long timeRunning = (long)difftime( timeNow, KonSystem::startupTime );
  int days = timeRunning / (60*60*24);
  int hours = timeRunning / (60*60) - days * 24;
  int minutes = timeRunning / 60 - (days * 24 + hours) * 60;
  char msg[100];
  
  sprintf( msg, "Time running: %d days %d h %d m", days, hours, minutes );

  g->setColor( Color::GRAY );
  g->drawString( 10, 5, msg );
}

RegisterDialog::RegisterDialog( Window *parent ) :
  Dialog( Desktop::getInstance(), parent, Strings::get(206),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 300, 180 )
{
  getGraphics()->setDialogFont();

  int margin = 10, fontHeight = Graphics::dialogDesignFontHeight,
      yStart = getClientHeight() - fontHeight*2,
      interSpace = fontHeight + fontHeight / 2;

  int editBoxHeight = EntryField::getInset() * 2 + fontHeight + 2,
      labelHeight = fontHeight + fontHeight / 3;

  Label *lbl = new Label( this, Label::LEFT, Strings::get(228), 10,
                          yStart, 80, labelHeight );

  yStart -= editBoxHeight;

	nameEntry = new EntryField( this, BORDER|ISTABSTOP, 10, yStart, getClientWidth() - 20, editBoxHeight );
  nameEntry->setWMove( FOLLOW );

  yStart -= interSpace;

  lbl = new Label( this, Label::LEFT, Strings::get(229), 10,
                          yStart, 80, labelHeight );

  yStart -= editBoxHeight;

	keyEntry = new EntryField( this, BORDER|ISTABSTOP, 10, yStart, getClientWidth() - 20, editBoxHeight );
  keyEntry->setWMove( FOLLOW );

  int buttonWidth = 100;
  //int xPos = (width - (buttonWidth * 2 + 20)) / 2;

  OKButton = new Button( this, Strings::get(8), ISDEFAULT|ISTABSTOP,
                         10, 10, buttonWidth, 30 );

  OKButton->addButtonListener(this);
  OKButton->setYMove( FOLLOW );

  cancelButton = new Button( this, Strings::get(9), 0,
                               20 + buttonWidth, 10, buttonWidth, 30 );
  cancelButton->setYMove( FOLLOW );

  cancelButton->addButtonListener(this);

	setControlsVisible();

	nameEntry->setFocus();

  adjustForFontSize();
  initSize();
}

void RegisterDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  if( buttonEvent->getSource() == OKButton )
  {
    // Check key
    char result[MAXREGCODELEN*2];
    result[0] = 0;
    int ok = scramble( nameEntry->getText(), result );

    if( !ok || strcmp( result, keyEntry->getText() ) )
    {
  	  PopupMessage *message = 
    	  new PopupMessage( this,
                          Strings::get(221), Strings::get(227),
                          PopupMessage::error );
		  message->getResult();
    }
    else
    {
      strcpy( KonSystem::regName, nameEntry->getText() );
      strcpy( KonSystem::regKey, keyEntry->getText() );
      KonSystem::registered = TRUE;

      // Store the data
      char regFile[MAXPATH];
      sprintf( regFile, "%s%s", KonSystem::baseDir, "kon.reg" );

      FILE *f = fopen( regFile, "wb");

      if( f != NULL )
      {
        // Scramble
        char scrambledStr[100];

        strcpy( scrambledStr, KonSystem::regName );
        
        int i;
        for( i = 0; i < strlen(KonSystem::regName); i++ )
          scrambledStr[i] ^= 0xef;

        fwrite( scrambledStr, sizeof(KonSystem::regName), 1, f );

        strcpy( scrambledStr, KonSystem::regKey );
        for( i = 0; i < strlen(KonSystem::regKey); i++ )
          scrambledStr[i] ^= 0xef;

        fwrite( scrambledStr, sizeof(KonSystem::regKey), 1, f );

        fclose( f );
      }
      closeDialog(1);
    }
  }
  else
    closeDialog(1);
}

UnregisteredDialog::UnregisteredDialog( Window *parent ) :
  Dialog( Desktop::getInstance(), parent, Strings::get(206),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 300, 250 )
{
	BmpGraphics bmpg;
	char bmpFile[MAXPATH+1];

	sprintf( bmpFile, "%s%s", KonSystem::baseDir , "konUnregistered.bmp" );

	bmp = bmpg.readBitmap( bmpFile );

	UnregisteredWindow *UnregisteredWnd = new UnregisteredWindow( this, 0, 50, getClientWidth(), 
	                                         getClientHeight()-50, bmp );
	addControl( UnregisteredWnd );

  int buttonWidth = 100;
  int xPos = (width-buttonWidth) / 2;

  OKButton = new Button( this, Strings::get(8), AWindow::ISDEFAULT,
                         xPos, 10, buttonWidth, 30 );

  OKButton->addButtonListener(this);

	setControlsVisible();

	OKButton->setFocus();
}

void UnregisteredDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  closeDialog(1);
}

void UnregisteredWindow::paint( Rect *rect, Graphics *g )
{
	g->drawBitmap(bmp, 0, 0, 0, getClientWidth(), getClientHeight() );
  g->selectFont( "Courier", 8, 0 );
  g->setColor( Color::LIGHTGRAY );
  g->setBackColor( Color::BLACK );
  int fontHeight = g->getFontHeight();
  g->drawString( 10, 10+fontHeight*2, "Version 2.0" );
  g->setColor( Color::RED );
  g->drawString( 10, 10+fontHeight, "***ALPHA VERSION***" );
  g->setColor( Color::LIGHTGRAY );
//  g->drawString( 10, 10, "Licensed to Wolfram Schmid" );
}

DirDialog::DirDialog( Window *parent ) :
  Dialog( Desktop::getInstance(), parent, Strings::get(206),
          AWindow::CENTER | AWindow::ISMODAL | AWindow::ADJUSTSIZE,
          0, 0, 360, 250 )
{
  int buttonWidth = 100;
  int xPos = (width-buttonWidth) / 2;

  xPos = (width - (buttonWidth * 2 + 20)) / 2;

  OKButton = new Button( this, Strings::get(8), 
                         KonSystem::registered ? AWindow::ISDEFAULT : 0,
                         xPos, 10, buttonWidth, 30 );

  OKButton->addButtonListener(this);
  OKButton->setYMove( FOLLOW );
  OKButton->setXMove( PROPORTIONAL );

	setControlsVisible();

	OKButton->setFocus();
  initSize();
}

void DirDialog::actionPerformed( AButtonEvent *buttonEvent )
{
  closeDialog(1);
}

