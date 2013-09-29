#include "SectionBrowser.h"
#include "EditCtrl.h"
#include "grep.h"

SectionBrowser::SectionBrowser( EditControl *parent, int x, int y, int width, int height ) 
  : section(NULL), sortedPos(NULL), sections(0), editControl(parent), Window( parent, x, y, width, height ),
    bufferModified(TRUE), allowSelectEvent(TRUE), selectedIndex(-1), executingSearch(FALSE), terminateSearch(FALSE)
{
  sectionCombo = new SComboBox( this, SComboBox::STATIC, 5, 5, 10, 10, 150 );
  sectionCombo->setMaxLength( 100 );
  sectionCombo->setVisible( TRUE );
  
  toolbar = new Toolbar(this, "SECTIONTOOLBAR", 15, 5, 10, 10 );
  toolbar->setVisible(TRUE);
  char str[MAXPATH];
  sprintf( str, "%sregenerate.bmp", KonSystem::bmpDir );
  toolbar->addImageButton( str, "regenerateSections();", Strings::get(250), 1000 );
  toolbar->addToolbarListener(this);
  
  parent->getTextBuffer()->addBufferObserver(this);
  addSelectionListener( parent );
  sectionCombo->addSelectionListener(this);
}

void SectionBrowser::insertion( TextBuffer *textBuffer, int reader, int position,
                        int size, int lines )
{
  if( executingSearch ) return;

  Syntax *syntax = editControl->getSyntax();
  
  if( syntax == NULL ) return;
  
  int pos = findSectionAfterPos( position );
  
  for( int i = pos; i < sections; i++ )
  {
    sortedPos[i]->sectionStartPos += size;
    sortedPos[i]->sectionEndPos += size;
  }
  
  bufferModified = TRUE;
}
void SectionBrowser::deletion( TextBuffer *textBuffer, int reader, int position,
                        int size, int lines )
{
  if( executingSearch ) return;

  Syntax *syntax = editControl->getSyntax();
  BOOL listModified = FALSE;
  
  if( syntax == NULL ) return;
  
  int pos = findSectionAfterPos( position );
  
  // Find first affected section
  if( pos > 0 && sortedPos[pos-1]->sectionEndPos > position ) pos--;
  
  // Remove deleted sections from list
  while( pos < sections && position + size > sortedPos[pos]->sectionStartPos )
  {
    for( int i = 0; i < sections; i++ ) 
    {
      if( sortedPos[pos] == section[i] )
      {
        delete section[i];
        memmove( &section[i], &section[i+1], (sections - i - 1) * sizeof( Section *) );
        listModified = TRUE;
        break;
      }
    }
    memmove( &sortedPos[pos], &sortedPos[pos+1], (sections - pos - 1) * sizeof( Section *) );
    sections--; 
  }
  
  for( int i = pos; i < sections; i++ )
  {
    sortedPos[i]->sectionStartPos -= size;
    sortedPos[i]->sectionEndPos -= size;
  }
  
  if( listModified ) fillCombo();
  
  bufferModified = TRUE;
}

void SectionBrowser::modification( TextBuffer *textBuffer, int reader, int position,
                             int size )
{
  if( executingSearch ) return;

  Syntax *syntax = editControl->getSyntax();
  
  if( syntax == NULL ) return;

  bufferModified = TRUE;
}                             

int SectionBrowser::getHeight() 
{ 
  int h1 = sectionCombo->getEditBoxHeight() + 4;
  int h2 =  Toolbar::getInset() * 2 + 16;
  return (h1 > h2) ? h1 : h2;
}

void SectionBrowser::size( int oldWidth, int oldHeight )
{
  getGraphics()->setDialogFont();

  int pixelHeight = Settings::pulldownSize * getGraphics()->getMaxFontHeight() + sectionCombo->getEditBoxHeight() + 10;

  sectionCombo->setSize(width / 2, pixelHeight);
  sectionCombo->setPos( 2, (height - sectionCombo->getEditBoxHeight()) / 2 );
  sectionCombo->setVisible(TRUE);
  toolbar->setSize( 20 + Toolbar::getInset()*2, Toolbar::getInset()*2 + 16 );
  toolbar->setPos( width / 2 + 10, (height - (Toolbar::getInset()*2 + 16)) / 2 );
  toolbar->setVisible(TRUE);
}

void SectionBrowser::paint( Rect *rect, Graphics *g )
{
  g->setBackColor( Color::BKGR );
  g->fillRect( rect );

  g->setColor( Color::WHITE );
  g->drawLine( 0, height-1, width, height-1 );

  g->setColor( Color::SHADOW );
  g->drawLine( 0, 0, width, 0 );
}

#if defined _MSC_VER
int sectionNameCmpI( const void *s1, const void *s2 )
#else
int _Optlink sectionNameCmpI( const void *s1, const void *s2 )
#endif
{
  return strcmpi( (*(Section **)s1)->name, (*(Section **)s2)->name );
}

#if defined _MSC_VER
int sectionNameCmp( const void *s1, const void *s2 )
#else
int _Optlink sectionNameCmp( const void *s1, const void *s2 )
#endif
{
  return strcmp( (*(Section **)s1)->name, (*(Section **)s2)->name );
}

#if defined _MSC_VER
int sectionNamePos( const void *s1, const void *s2 )
#else
int _Optlink sectionNamePos( const void *s1, const void *s2 )
#endif
{
  return (*(Section **)s1)->sectionStartPos - (*(Section **)s2)->sectionStartPos;
}

void SectionBrowser::scanRegion( int reader, TextBuffer *buffer, Syntax *syntax, int startPos, int endPos )
{
  for( int s = 0; s < syntax->sections; s++ )
  {
    if( terminateSearch ) break;

    int start = startPos;

    Grep grep(syntax->sectionList[s]->regexp, syntax->delimitor, !syntax->caseInsensitive, FALSE, FALSE );   

    int end = endPos, useParen = syntax->sectionList[s]->useParen - 1;
    
    while( grep.findForward( reader, buffer, NULL, start, end ) )
    {
      if( terminateSearch ) break;

      section = (Section **)realloc( section, sizeof( Section *) * (sections+1) );

      section[sections] = new Section();  
      
      section[sections]->name = new char[grep.getParenEnd(useParen) - grep.getParenStart(useParen)+1];
      
	    int p = 0;

      for( int c = grep.getParenStart(useParen); c < grep.getParenEnd(useParen); c++, p++ )
        section[sections]->name[p] = buffer->getChar( reader, c );

      section[sections]->name[p] = 0;
      section[sections]->sectionStartPos = grep.getParenStart(useParen);
      section[sections]->sectionEndPos = grep.getParenEnd(useParen);
      
      sections++;
      
      start = end;
      end = endPos;
    }
  }
}

void SectionBrowser::fillCombo()
{
  sectionCombo->clearItems();
  
  for( int s = 0; s < sections; s++ )
    sectionCombo->addItem( section[s]->name );
    
  selectedIndex = -1;    
}

void SectionBrowser::regenerate( BOOL force )
{
  int i;

  if( (!bufferModified && !force) || executingSearch ) return;

  searchSem.request();
  executingSearch = TRUE;

  bufferModified = FALSE;
  
  for( i = 0; i < sections; i++ )
    delete section[i];
  
  sections = 0;

  Syntax *syntax = editControl->getSyntax();
  
  if( syntax == NULL ) 
  {
    executingSearch = FALSE;
    searchSem.release();
    return;
  }
  
  TextBuffer *buffer = editControl->getTextBuffer();
  int reader = buffer->readRequest();
  
  CodeSection *cSection;
  int cSections = editControl->getCodeSections( &cSection );

  terminateSearch = FALSE;
  
  for( i = 0; i < cSections; i++ )
    scanRegion( reader, buffer, syntax, cSection[i].startPos, cSection[i].endPos );
    
  delete cSection;    
  
  buffer->readRelease();
  
  qsort( section, sections, sizeof( Section *), (syntax->caseInsensitive) ? sectionNameCmpI : sectionNameCmp );

  sortedPos = (Section **)realloc( sortedPos, sizeof( Section *) * (sections) );
 
  for( int s = 0; s < sections; s++ )
    sortedPos[s] = section[s];

  qsort( sortedPos, sections, sizeof( Section * ), sectionNamePos );

  if( !terminateSearch )
    fillCombo();  

  executingSearch = FALSE;
  searchSem.release();
}

void SectionBrowser::actionPerformed( ASelectionEvent *selectionEvent )
{
  if( allowSelectEvent && !executingSearch  )
  {
    selectedIndex = selectionEvent->getSelected();
    performSelectionEvent( selectedIndex );
  }    
}

void SectionBrowser::actionPerformed( ALineMoveEvent *moveEvent )
{
  if( moveEvent->getNewLine() == moveEvent->getOldLine() || executingSearch ) return;
  
  Syntax *syntax = editControl->getSyntax();
  
  if( syntax == NULL ) return;
  
  int pos = findSectionAfterPos( moveEvent->getLineStart() );
  
  if( pos >= sections )
    pos = sections-1;
  else
  {
    if( moveEvent->getLineEnd() < sortedPos[pos]->sectionStartPos && pos == 0)
      pos = -1;
    else  
    if( !(moveEvent->getLineEnd() >= sortedPos[pos]->sectionEndPos && 
          moveEvent->getLineStart() <= sortedPos[pos]->sectionStartPos) )
      pos--;
  }      

  allowSelectEvent = FALSE;
  
  int selIndex = -1;
  
  if( pos >= 0 )
  {
    for( int i = 0; i < sections; i++ )
      if( sortedPos[pos] == section[i] )
      {
        selIndex = i;
        break;
      }
  }    

  if( selIndex != selectedIndex )
  {
    selectedIndex = selIndex;
    sectionCombo->selectItem( selIndex );
  }    
    
  allowSelectEvent = TRUE;
}

void SectionBrowser::actionPerformed( AToolbarEvent *toolbarEvent )
{
 if( !executingSearch ) editControl->addCommand( toolbarEvent->getCommand() );
}

int SectionBrowser::findSectionAfterPos( int atPos )
{
  int pos = sections / 2, base = 0, Top = sections, lastPos = -1, res;

  if( sections == 0 ) return 0;

  while( 1 )
  {
    res = atPos - sortedPos[pos]->sectionStartPos;

    if( lastPos == pos ) break;

    lastPos = pos;

    if( base == Top )
      break;

    if( res < 0 )
    {
      Top = pos;
      pos = base + (pos - base) / 2;
    }
    else
    if( res > 0 )
    {
      base = pos;
      pos += (Top - pos) / 2;
    }
    else
      break;
  }

  if( res <= 0 ) 
    return pos;
  else 
    return pos + 1;
}

void SectionBrowser::terminate()
{
  terminateSearch = TRUE;
  searchSem.wait( MutexSemaphore::FOREVER );
}

SectionBrowser::~SectionBrowser()
{
  ((EditControl *)parent)->getTextBuffer()->removeBufferObserver(this);
  terminate();
  delete toolbar;
}
