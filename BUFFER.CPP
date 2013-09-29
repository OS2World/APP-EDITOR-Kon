#include <stdlib.h>
#include "bowl.h"
#include "buffer.h"

int TextBuffer::TextBlock::maxSize = 10000;
int TextBuffer::TextBlock::startSize = 5000;

Vector<BufferObserver> hej;
Vector<LoadObserver> hej2;

void TextBuffer::convertFromUnicode( EncodingTypes encoding )
{
  if( encoding != UTF8 && encoding != UTF16 ) return;

  // Iterate through all buffers and convert to unicode
  beginEnum();  // Use enumBlock
  TextElem *outBuf = new TextElem[10000];
  currentBlock = enumBlock;

  while( enumBlock )
  {
    int orgSize = enumBlock->textSize;
    UnicodeHelper::unicodeToAnsi( enumBlock->text, outBuf, enumBlock->textSize, encoding );
    memcpy( enumBlock->text, outBuf, enumBlock->textSize );
    textSize -= (orgSize - enumBlock->textSize);
    enumBlock = enumBlock->next;
  }

  blockStart = 0;
  stackSize = 1;
  stackPos = 0;

  setEncoding(encoding);

  delete outBuf;
}

void TextBuffer::setEncoding( EncodingTypes encType ) 
{ 
  encodingType = encType;
  
  if( encType == ANSI )
  {
    convertedUNICODE = FALSE; 
    leadBytesCount = 0;
  }
  else
  if( encType == UTF8 )
  {
    convertedUNICODE = TRUE; 
    leadBytesCount = 3;
    leadBytes[0] = 0xEF;
    leadBytes[1] = 0xBB;
    leadBytes[2] = 0xBF;
  }
  else 
  if( encType == UTF16 )
  {
    convertedUNICODE = TRUE; 
    leadBytes[0] = 0xFF;
    leadBytes[1] = 0xFE;
    leadBytesCount = 2;
  }
}

int TextBuffer::writeRequest( BOOL setBlockStarts )
{
  // Only allow write if there are no readers and writers
  while( 1 )
  {
    // Wait for readers to finish
    noreadersFlag.wait();

    Semaphore *semList[2];
    semList[0] = &bufferPointer;
    semList[1] = &writeLock;

    Semaphore::muxWait( 2, semList );

    // The noreadersFlag may have been reset, so we have to check again
    if( noreadersFlag.check() == 0 )
    {
      // Someone slipped in between, we have to retry
      writeLock.release();
      bufferPointer.release();
    }
    else
      break;  // We may continue
  }

  writers++;
  
  if( setBlockStarts )
  {
    blockStartStack[stackPos] = blockStart;
    currentBlockStack[stackPos] = currentBlock;
    stackPos++;
  }

  bufferPointer.release();
  return stackPos-1;
}

int TextBuffer::readRequest( MutexSemaphore *additionalSemaphore, int timeout )
{
  // MuxWait for pointer and writer and possibly an additional semaphore
  Semaphore *semList[3];
  int semaphores = 2;
  semList[0] = &bufferPointer;
  semList[1] = &writeLock;

  if( additionalSemaphore )
  {
    semList[2] = additionalSemaphore;
    semaphores = 3;
  }

  if( !Semaphore::muxWait( semaphores, semList, timeout ) ) return -1;	// Timeout

  // Lock the buffer for writers
  if( readers == 0 )
    noreadersFlag.reset();

  writeLock.release();  // Was set by the muxwait

  int pos;

  readers++;

  if( stackPos >= stackSize )
  {
    stackSize++;
    blockStartStack = (int *)realloc( blockStartStack,
                                       sizeof(int) * (stackSize) );
    currentBlockStack = (TextBlock **)realloc( currentBlockStack,
                                         sizeof(TextBlock *) * (stackSize) );
  }

  blockStartStack[stackPos] = blockStart;
  currentBlockStack[stackPos] = currentBlock;
  pos = stackPos;
  stackPos++;

  bufferPointer.release();

  return pos;
}

void TextBuffer::writeRelease( BOOL setBlockStarts )
{
  bufferPointer.request();
  writers--;
  if( setBlockStarts )
  {
    stackPos--;
    blockStart = blockStartStack[stackPos];
    currentBlock = currentBlockStack[stackPos];
  }

  // Allow new writer or readers
  writeLock.release();
  bufferPointer.release();
  
	displayObserverLock.request();
  for( int i = 0; i < displayObservers.getSize(); i++ )
    displayObservers.getObjectAt(i)->noWriter();
	displayObserverLock.release();
}

void TextBuffer::readRelease()
{
  bufferPointer.request();

	// Since we don't supply the handle of the reader, we cannot remove
	// any buffer for any handle allocated. We do that when the last
	// reader is released.

  readers--;

	// A guess that the handles were released in order.
	// If they are not, it doesn't matter since
	// no modification has been done to the blocks
	// This is just an optimization issue
  blockStart = blockStartStack[readers];
  currentBlock = currentBlockStack[readers];

  // Release the buffer for writers
  if( readers == 0 )
  {
		stackPos = 0;
    noreadersFlag.post();
  }

  bufferPointer.release();
}

TextElem TextBuffer::getChar( int reader, int position )
{
  // Move to the correct block
  if( position >= blockStartStack[reader] )
  {
    // Move forward until we find the correct block
    while( position >=
           blockStartStack[reader] + currentBlockStack[reader]->textSize )
    {
      if( currentBlockStack[reader]->next == NULL ) return 0;  // Out of range
      blockStartStack[reader] += currentBlockStack[reader]->textSize;
      currentBlockStack[reader] = currentBlockStack[reader]->next;
    }
  }
  else
  {
    // Move backwards until position is found
    if( position < 0 ) return 0;

    while( position < blockStartStack[reader] )
    {
      currentBlockStack[reader] = currentBlockStack[reader]->prev;
      blockStartStack[reader] -= currentBlockStack[reader]->textSize;
    }
  }
  return currentBlockStack[reader]->text[position-blockStartStack[reader]];
}

void TextBuffer::TextBlock::resize(  int newSize )
{
  // Check if we need to resize the block
  int delta = newSize - allocatedSize;

  if( delta > 0 )
  {
    allocatedSize = (allocatedSize + delta) / 4 + allocatedSize + delta;

    if( allocatedSize > maxSize )
      allocatedSize = maxSize;

    text = (TextElem *)realloc( (void *)text, allocatedSize );
  }
  else
  {
    if( delta < 1000 )
    {
      allocatedSize = newSize + 1000L;
      text = (TextElem *)realloc( (void *)text, allocatedSize );
    }
  }
  textSize = newSize;
}

void TextBuffer::insertText( int writer, int position, TextElem *text, int size,
                             BOOL reportDisplay, BOOL reportBuffer, BOOL addUndo )
{
  int lines = 0, orgPos = position, longestLine = 0, thisLine = 0;
  TextElem *lfCount = text;

  setModified( TRUE, FALSE );

  if( addUndo )
    undo.addInsertion( position, text, size );

  // Count the number of lines in the text block
  int i;
  
  for( i = 0; i < size; i++ )
  {
    thisLine++;
    if( *(lfCount++) == 0xa )
    {
      if( thisLine > longestLine ) longestLine = thisLine;
      thisLine = 0;
      lines++;
    }
  }

  if( thisLine > longestLine ) longestLine = thisLine;

  if( longestLine > 5000 )
   isBinaryFile = TRUE;

  LFCount += lines;

  // Use getChar to find the correct block where insertion should take place
  getChar( writer, position );

  // If we stand at the start of a block and there's space at the end of
  // the previous block, chain backwards one step
  int insertPos = position - blockStartStack[writer];

  if( insertPos == 0 && currentBlockStack[writer]->prev != NULL )
  {
    if( currentBlockStack[writer]->prev != NULL &&
        currentBlockStack[writer]->prev->textSize < TextBlock::maxSize )
    {
      currentBlockStack[writer] = currentBlockStack[writer]->prev;
      blockStartStack[writer] -= currentBlockStack[writer]->textSize;
    }
  }

  int blockPos = 0;

  while( blockPos < size )
  {
    insertPos = position - blockStartStack[writer];

    // Check if the current block is full
    if( currentBlockStack[writer]->textSize == TextBlock::maxSize )
    {
      if( insertPos == currentBlockStack[writer]->textSize )
      {
        // At end of current block, chain to the next one if it exists,
        // otherwise create a new one after this block

        blockStartStack[writer] += currentBlockStack[writer]->textSize;

        if( currentBlockStack[writer]->next )
          currentBlockStack[writer] = currentBlockStack[writer]->next;
        else
        {
          // Create a new block

          TextBlock *lastBlock = new TextBlock();
          lastBlock->next = NULL;
          lastBlock->prev = currentBlockStack[writer];
          currentBlockStack[writer]->next = lastBlock;
          currentBlockStack[writer] = lastBlock;
          lastBlock->dirty = TRUE;
        }
      }
      else
      if( insertPos == 0 )
      {
        // At the start of a full block, create a new one in front
        TextBlock *insBlock = new TextBlock();
        insBlock->next = currentBlockStack[writer];
        insBlock->prev = currentBlockStack[writer]->prev;
        if( currentBlockStack[writer]->prev != NULL )
          currentBlockStack[writer]->prev->next = insBlock;
        currentBlockStack[writer]->prev = insBlock;
        currentBlockStack[writer] = insBlock;
        insBlock->dirty = TRUE;
      }
      else
      {
        // In the middle of a full block, split it

        TextBlock *sndBlock =
          new TextBlock( &currentBlockStack[writer]->text[insertPos],
                         currentBlockStack[writer]->textSize - insertPos );
        sndBlock->next = currentBlockStack[writer]->next;
        sndBlock->prev = currentBlockStack[writer];
        if( currentBlockStack[writer]->next )
          currentBlockStack[writer]->next->prev = sndBlock;
        currentBlockStack[writer]->next = sndBlock;
        currentBlockStack[writer]->resize( insertPos );
        currentBlockStack[writer]->dirty = TRUE;
        sndBlock->dirty = TRUE;
      }
      continue;
    }

    // Find out how much to insert in the current block

    int insertSize = (size - blockPos <=
                      TextBlock::maxSize-currentBlockStack[writer]->textSize) ?
                      size - blockPos :
                      TextBlock::maxSize-currentBlockStack[writer]->textSize;

    currentBlockStack[writer]->resize( currentBlockStack[writer]->textSize +
                                       insertSize );
    // Insert the text

    memmove( &currentBlockStack[writer]->text[insertPos + insertSize],
             &currentBlockStack[writer]->text[insertPos],
             currentBlockStack[writer]->textSize - insertPos - insertSize );    
		memcpy( &currentBlockStack[writer]->text[insertPos], &text[blockPos],
            insertSize );
    currentBlockStack[writer]->dirty = TRUE;

    position += insertSize;
    blockPos += insertSize;
  }

  textSize += size;

  // Report insertion to observers

  if( reportBuffer )
    for( i = 0; i < bufferObservers.getSize(); i++ )
      bufferObservers.getObjectAt(i)->insertion( this, writer, orgPos, size,
                                                lines );
  if( reportDisplay )
    for( i = 0; i < displayObservers.getSize(); i++ )
      displayObservers.getObjectAt(i)->insertion( this, writer, orgPos, size,
                                                  lines );
}

void TextBuffer::deleteText( int writer, int position, int size, BOOL reportDisplay, BOOL reportBuffer,
                             BOOL addUndo )
{
  if( position + size > textSize )
    size = textSize - position;

  if( position < 0 || position >= textSize || size <= 0 ) return;

  setModified( TRUE, FALSE );

  TextElem *text = new TextElem[size];

  int lines = 0;

  // Use getChar to find the correct block where deletion should start
  getChar( writer, position );
  int blockPos = position - blockStartStack[writer], left = size;
  TextBlock *delBlock = currentBlockStack[writer];

  while( left > 0 )
  {
    if( left <= delBlock->textSize - blockPos )
    {
      // delete text in the current block
      TextElem *lfCount = &delBlock->text[blockPos];
      for( int i = 0; i < left; i++ )
        if( *(lfCount++) == 0xa )
          lines++;

      memcpy( &text[size-left], &delBlock->text[blockPos], left );

      if( delBlock->textSize - left == 0 )
      {
        // Remove the whole block
        if( delBlock->prev )
          delBlock->prev->next = delBlock->next;

        if( delBlock->next )
          delBlock->next->prev = delBlock->prev;

        if( currentBlockStack[writer] == delBlock )
        {
          if( delBlock->next )
            currentBlockStack[writer] = delBlock->next;
          else
          if( delBlock->prev )
          {
            currentBlockStack[writer] = delBlock->prev;
            blockStartStack[writer] -= delBlock->prev->textSize;
          }
          else
            currentBlockStack[writer] = new TextBlock();
        }
        delete delBlock;
      }
      else
      {
        memmove( &delBlock->text[blockPos],
                 &delBlock->text[blockPos + left],
                 delBlock->textSize - blockPos - left );
        delBlock->resize( delBlock->textSize - left );
        delBlock->dirty = TRUE;
      }
      textSize -= left;
      break;
    }
    else
    {
      if( blockPos != 0 )
      {
        int remove = delBlock->textSize - blockPos;

        // Remove parts (at end) of this block
        TextElem *lfCount = &delBlock->text[blockPos];
        for( int i = 0; i < remove; i++ )
          if( *(lfCount++) == 0xa )
            lines++;

        memcpy( &text[size-left], &delBlock->text[blockPos], remove );

        textSize -= remove;
        left -= remove;

        delBlock->resize( blockPos );
        delBlock->dirty = TRUE;
        delBlock = delBlock->next;
      }
      else
      {
        // Delete the whole block
        TextElem *lfCount = delBlock->text;
        for( int i = 0; i < delBlock->textSize; i++ )
          if( *(lfCount++) == 0xa )
            lines++;

        memcpy( &text[size-left], &delBlock->text[blockPos],
                delBlock->textSize );

        if( delBlock->prev )
          delBlock->prev->next = delBlock->next;

        if( delBlock->next )
          delBlock->next->prev = delBlock->prev;

        if( currentBlockStack[writer] == delBlock )
        {
          if( delBlock->next )
            currentBlockStack[writer] = delBlock->next;
          else
          if( delBlock->prev )
          {
            currentBlockStack[writer] = delBlock->prev;
            blockStartStack[writer] -= delBlock->prev->textSize;
          }
          else
            currentBlockStack[writer] = new TextBlock();
        }

        textSize -= delBlock->textSize;
        left -= delBlock->textSize;
        TextBlock *tempBlock = delBlock->next;
        delete delBlock;
        delBlock = tempBlock;
      }
    }
    blockPos = 0;
  }

  if( addUndo )
    undo.addDeletion( position, text, size );

  delete[] text;

  // Report deletion to observers
  if( reportBuffer )
    for( int i = 0; i < bufferObservers.getSize(); i++ )
      bufferObservers.getObjectAt(i)->deletion( this, writer, position, size,
                                                lines );
  if( reportDisplay )
    for( int i = 0; i < displayObservers.getSize(); i++ )
      displayObservers.getObjectAt(i)->deletion( this, writer, position, size,
                                                 lines );
}

void TextBuffer::removeDisplayObserver( BufferObserver *displayObserver )
{
	displayObserverLock.request();
  displayObservers.removeObject( displayObserver );
	displayObserverLock.release();
}

void TextBuffer::notifyObservers( int reader )
{
  int i; 
  for( i = 0; i < bufferObservers.getSize(); i++ )
    bufferObservers.getObjectAt(i)->modification( this, reader, 0, textSize );
  for( i = 0; i < displayObservers.getSize(); i++ )
    displayObservers.getObjectAt(i)->modification( this, reader, 0, textSize );
}

void TextBuffer::textModified( int reader, int start, int end )
{
  int i;
  for( i = 0; i < bufferObservers.getSize(); i++ )
    bufferObservers.getObjectAt(i)->modification( this, reader, start, end-start );
  for( i = 0; i < displayObservers.getSize(); i++ )
    displayObservers.getObjectAt(i)->modification( this, reader, start, end-start );
}

void TextBuffer::setModified( BOOL isModified, BOOL saved )
{
	if( this->isModified != isModified )
  {
		if( saved )
			undo.setSavedMark();
  	this->isModified = isModified;
    for( int i = 0; i < bufferObservers.getSize(); i++ )
      bufferObservers.getObjectAt(i)->modified( isModified );
  }
}

void TextBuffer::filenameUpdated()
{
  for( int i = 0; i < bufferObservers.getSize(); i++ )
    bufferObservers.getObjectAt(i)->filenameUpdated();
}

void TextBuffer::finishedLoading( BOOL isBinaryFile, BOOL reloaded )
{
  // Subsequent calls to addLoadObserver will notify the observer
  // directly.
  isModified = FALSE;
  doneLoading = TRUE;
  this->isBinaryFile = isBinaryFile;
  for( int i = 0; i < loadObservers.getSize(); i++ )
    loadObservers.getObjectAt(i)->textLoaded( isBinaryFile, reloaded );
}

// Delete the buffer
void TextBuffer::deleteBuffers()
{
  int writer = writeRequest();

  // Traverse backwards until buffer start
  while( currentBlockStack[writer]->prev != NULL ) currentBlockStack[writer] = currentBlockStack[writer]->prev;
  // Delete all buffers
  while( currentBlockStack[writer]->next )
  {
    currentBlockStack[writer] = currentBlockStack[writer]->next;
    delete currentBlockStack[writer]->prev;
  }
  delete currentBlockStack[writer];

  writeRelease();
}

int TextBuffer::countLF( int reader, int start, int end )
{
  // Find the correcy buffer

  getChar( reader, start );

  int textPos = blockStartStack[reader];
  int startPos = start - textPos;
  TextBlock *block = currentBlockStack[reader];
  int lines = 0;

	while( start < end )
	{
    // See if the stop pos is within the current text block

		int stopPos = end - textPos;
    int bufStopPos;

    int bufPos = startPos;

		if( stopPos >= block->textSize )
			bufStopPos = block->textSize;
		else
			bufStopPos = stopPos;

		// Search current buffer to end

		while( bufPos < bufStopPos )
		{
			if( block->text[bufPos] == 0xa )
				lines++;
			bufPos++;
		}

    start += (bufPos - startPos);

		textPos = start;

    // Move to the next block

    startPos = 0;
    block = block->next;
	}
  return lines;
}

void TextBuffer::integrityCheck( char *file )
{
  TextBlock *block = currentBlockStack[0], *prevBlock = NULL;

  while( block->prev ) block = block->prev;

  int blCount = 0;

  char file2[100];
  strcpy( file2, file );
  strcat( file2, "2" );

  FILE *f = fopen( file, "w" );
  FILE *f2 = fopen( file2, "wb" );

  while( block )
  {
    fprintf( f2, "**************************************************\n" );
    fprintf( f2, "* %d\n", blCount );
    fprintf( f2, "**************************************************\n" );
    fwrite( block->text, 1, block->textSize, f2 );
    fprintf( f2, "\n" );

    fprintf( f, "Count: %d, size: %d, %p\n", blCount, block->textSize, block );

    if( blCount )
      if( prevBlock->next != block )
        fprintf( f, "prevBlock->next mismatch: p %p, b %p\n", blCount, prevBlock->next, block );

    if( !block ) break;

    if( block->prev != prevBlock )
      fprintf( f, "block->prev mismatch: b %p, p %p\n", blCount, block->prev, prevBlock );

    prevBlock = block;
    block = block->next;

    blCount++;
  }

  fclose( f );
  fclose( f2 );
}

void TextBuffer::beginEnum()
{
  enumBlock = currentBlock;

  while( enumBlock->prev ) enumBlock = enumBlock->prev;
}

TextElem *TextBuffer::enumTextBlock( int &size )
{
  if( enumBlock )
  {
    size = enumBlock->textSize;
    TextElem *text = enumBlock->text;
    enumBlock = enumBlock->next;
    return text;
  }
  size = 0;
  return NULL;
}

void TextBuffer::reset()
{
  deleteBuffers();
  stackPos = blockStart = textSize = 0;
  LFCount = 0;
  isModified = FALSE;
  currentBlock = new TextBlock();
}

TextElem TextBuffer::getCharNoReader( int pos )
{
  int reader;

  if( hasWriters() )
    reader = stackPos - 1;
  else
    reader = readRequest();

  TextElem ch = getChar( reader, pos );

  if( !hasWriters() ) readRelease();

  return ch;
}
