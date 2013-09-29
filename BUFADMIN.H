#if !defined BUFFERADMIN
#define BUFFERADMIN

#include "bowl.h"
#include "buffer.h"

class BufferHolder;

class BufferInfo : public AObject
{
public:
  BufferHolder *bufferHolder;
  TextBuffer *textBuffer;
  BufferInfo( BufferHolder *bufferHolder, TextBuffer *textBuffer ) :
    bufferHolder( bufferHolder ), textBuffer( textBuffer ) {}
};

class BufferAdmin
{
  static Vector<BufferInfo> bufferInfo;
	static int scratchCounter;
public:
	static void getNextScratchName( char *name );
	static BOOL isScratchName( char *name );
  static void addBufferHolder( BufferHolder *bufferHolder, 
                               TextBuffer *textBuffer );
  static TextBuffer *queryBufferWithFilename( char *filename );
  static void removeBufferHolder( BufferHolder *bufferHolder );
	static int getBuffersCount() { return bufferInfo.getSize(); };
	static TextBuffer *getBuffer( int pos ) { return bufferInfo.getObjectAt(pos)->textBuffer; }
};
#endif