#include "bufadmin.h"

Vector<BufferInfo> BufferAdmin::bufferInfo(FALSE);
int BufferAdmin::scratchCounter = 0;

void BufferAdmin::addBufferHolder( BufferHolder *bufferHolder, 
                                  TextBuffer *textBuffer )
{
  BufferInfo *newB = new BufferInfo( bufferHolder, textBuffer );
  bufferInfo.insertAt( 0, newB );
}

void BufferAdmin::getNextScratchName( char *name )
{
	sprintf( name, "*scratch%d*", scratchCounter++ );
}

BOOL BufferAdmin::isScratchName( char *name )
{
	return *name == '*';
}

TextBuffer *BufferAdmin::queryBufferWithFilename( char *filename )
{
  char fullPath[MAXPATH+1];

  if( isScratchName( filename ) )
    strcpy( fullPath, filename );
  else
    System::getFullPath( filename, fullPath, MAXPATH );

  for( int i = 0; i < bufferInfo.getSize(); i++ )
    if( !stricmp( bufferInfo.getObjectAt(i)->textBuffer->getFilename(),
                 fullPath ) )
      return bufferInfo.getObjectAt(i)->textBuffer;
  return NULL;
}

void BufferAdmin::removeBufferHolder( BufferHolder *bufferHolder )
{
  int i = 0;
  TextBuffer *textBuffer = NULL;
  while( i < bufferInfo.getSize() )
  {
    if( bufferInfo.getObjectAt(i)->bufferHolder == bufferHolder )
    {
      textBuffer = bufferInfo.getObjectAt(i)->textBuffer;
      bufferInfo.removeAt( i);
    }
    else
      i++;
  }

  // Check if this textBuffer exists at more places in the list

  for( i = 0; i < bufferInfo.getSize(); i++ )
    if( bufferInfo.getObjectAt(i)->textBuffer == textBuffer ) break;

  if( i == bufferInfo.getSize() )
    delete textBuffer; // No more bufferHolders exists with this textBuffer
}
