#if !defined BUFFER

#include <stdlib.h>
#include "autosave.h"

#define BUFFER
/******************************************************************************
 * TextBuffer holds the text for one file.
 *
 */

// TextElem can be modified in case we need to use double-byte character-sets
typedef char TextElem;

#include "undo.h"

/******************************************************************************
 * BufferObserver listens to modifications of the buffer
 * Note that lines refer to the number of LF's. The actual number of lines
 * may be different depending on the view.
 */

class TextBuffer;

class BufferObserver : public AObject
{
public:
  virtual void insertion( TextBuffer *textBuffer, int reader, int position,
                          int size, int lines ) {}
  virtual void deletion( TextBuffer *textBuffer, int reader, int position,
                         int size, int lines ) {}
  virtual void modification( TextBuffer *textBuffer, int reader, int position,
                             int size ) {}
	virtual void modified( BOOL isModified ) {}
	virtual void filenameUpdated() {}
  // When a writer releases the buffer, this method is called
  // Note that new writers may slip in while this is executing!
  virtual void noWriter() {}
};

/******************************************************************************
 * LoadObserver is notified when the buffer has finished loading its text
 * Note that the buffer itself is not loading the text. The text is loaded
 * by an outside class which calls the finishedLoading method in buffer.
 */

class LoadObserver : public AObject
{
public:
  virtual void textLoaded( BOOL isBinaryFile, BOOL reloaded ) {}
};

class TextBuffer
{
  BOOL doneLoading; // Set to true when the buffer is initialized with text
  BOOL convertedUNICODE;
  EncodingTypes encodingType;
  /* TextBlock holds one piece of text
   */
  struct TextBlock
  {
    static int maxSize, startSize;
    struct TextBlock *next, *prev;  // prev and next are NULL at ends
    TextElem *text;
    int textSize;
    int allocatedSize;
    int positionInFile;
    BOOL dirty, swappedOut;
    void resize( int addSize );
    TextBlock()
    {
      text = NULL;
      textSize = allocatedSize = 0;
      dirty = FALSE;
      next = prev = NULL;
      positionInFile = 0;
      swappedOut = FALSE;
    }
    TextBlock( TextElem *insertText, int size )
    {
      text = NULL;
      textSize = allocatedSize = 0;
      dirty = FALSE;
      next = prev = NULL;
      resize( size );
      if( size != 0 )
        memcpy( text, insertText, size );
      positionInFile = 0;
      swappedOut = FALSE;
    }
    ~TextBlock()
    {
      if( text != NULL )
        free( text );
    }
  };

 TextBlock **currentBlockStack, *currentBlock, *enumBlock;
  int  textSize,    // The size of the full text
       blockStart,
       *blockStartStack,  // The absolute position in the text where
                          // the current block starts
       stackPos, stackSize,
       readers,           // The current number of readers
       writers;           // -"-         number of writers (always o or 1)

  // Displayobservers are called last
  Vector<BufferObserver> bufferObservers, displayObservers;
  Vector<LoadObserver> loadObservers;
  // Buffer pointers semaphores, for locking modification of blockStartStack
  // and currentBlockStack
  MutexSemaphore bufferPointer, writeLock, readLock, displayObserverLock;
  EventSemaphore noreadersFlag;
  char filename[MAXPATH+1];
  BOOL bufferNamed, isModified, isBinaryFile;
  Undo undo;
  int LFCount;
  void setModified( BOOL isModified, BOOL saved );
  BTime timeWritten;
  char leadBytes[5];
  int leadBytesCount;
public:
  // Allocate an empty textblock
  TextBuffer( char *filename ) : textSize(0), blockStart(0),
                 currentBlock( new TextBlock() ),
                 bufferObservers(FALSE), displayObservers(FALSE),
                 loadObservers(FALSE), doneLoading(FALSE),
                 stackSize(10), stackPos(0), readers(0), writers(0),
                 LFCount(0), isModified(FALSE), isBinaryFile(FALSE),
                 convertedUNICODE(FALSE) 
  {
    currentBlockStack = (TextBlock **)malloc( sizeof( TextBlock * ) * stackSize );
    blockStartStack = (int *)malloc( sizeof( int ) * stackSize );

    bufferNamed = (filename != NULL);

    if( bufferNamed )
      strcpy( this->filename, filename );
    noreadersFlag.post();
    System::getFileTime(filename,&timeWritten );
  }

  char *getLeadBytes() { return leadBytes; }
  int getLeadBytesCount() { return leadBytesCount;}
  BOOL isBinary() { return isBinaryFile; }
  BOOL isConvertedUNICODE() { return convertedUNICODE; }
  void setEncoding( EncodingTypes encType );
  EncodingTypes getEncoding() { return encodingType; }
  void convertFromUnicode( EncodingTypes encoding );

  void setTimeWritten( BTime *written )
  {
    memcpy( &timeWritten, written, sizeof( BTime ) );
  }

  void getTimeWritten( BTime *written )
  {
    memcpy( written, &timeWritten, sizeof( BTime ) );
  }

  int getLFCountInserted() { return LFCount; }

  Undo *getUndoBuffer() { return &undo; }

  char *getFilename() { return filename; }
  void setFilename( char *filename )
  {
  	strcpy( this->filename, filename );
  }

  // Everyone reading from the buffer should claim access for doing this
  // by calling readRequest and retrieve a handle, and when finished call
  // readRelease. Requests and releases may be nested.
  //
  // Several readers are allowed at once, but only one writer.
  // New readers claiming access are blocked until writer finishes
  // New writers claiming access are blocked until all readers are done.
  //
  // The standard solution to readers/writers problem is not possible
  // since OS/2 only allows the owner of a mutex semaphore to release it.
  //
  // Note that requests cannot be nested within the same thread

  int readRequest()
  {
    return readRequest(NULL);
  }

  int readRequest( MutexSemaphore *additionalSemaphore, int timeout );
  int readRequest( MutexSemaphore *additionalSemaphore )
  {
  	return readRequest( additionalSemaphore, Semaphore::FOREVER );
  }
  void readRelease();
  int writeRequest( BOOL setBlockStarts = TRUE );
  void writeRelease( BOOL setBlockStarts = TRUE );
  BOOL hasWriters() { return writers > 0; }

  TextElem getChar( int reader, int position );
  void insertText( int writer, int position, TextElem *text, int size,
                   BOOL reportDisplay, BOOL reportBuffer, BOOL addUndo );
  void insertText( int writer, int position, TextElem *text, int size,
                   BOOL reportDisplay, BOOL reportBuffer )
  {
    insertText( writer, position, text, size, reportDisplay, reportBuffer, TRUE );
  }
  void insertTextNoUndo( int writer, int position, TextElem *text, int size,
                         BOOL reportDisplay, BOOL reportBuffer )
  {
    insertText( writer, position, text, size, reportDisplay, reportBuffer, FALSE );
  }

  void deleteText( int writer, int position, int size, BOOL reportDisplay, BOOL reportBuffer,
                   BOOL addUndo );

  void deleteText( int writer, int position, int size, BOOL reportDisplay, BOOL reportBuffer )
  {
    deleteText( writer, position, size, reportDisplay, reportBuffer, TRUE );
  }

  void deleteTextNoUndo( int writer, int position, int size, BOOL reportDisplay, BOOL reportBuffer )
  {
    deleteText( writer, position, size, reportDisplay, reportBuffer, FALSE );
  }

  void startUndoRepeat()
  {
    undo.setStartRepeat();
  }

  void endUndoRepeat()
  {
    undo.setEndRepeat();
  }

  void addBufferObserver( BufferObserver *bufferObserver )
  {
    bufferObservers.add( bufferObserver );
  }
  void addDisplayObserver( BufferObserver *displayObserver )
  {
    displayObservers.add( displayObserver );
  }
  void addLoadObserver( LoadObserver *loadObserver )
  {
    loadObservers.add( loadObserver );
    if(doneLoading )
      loadObserver->textLoaded( isBinaryFile, FALSE );
  }
  void removeBufferObserver( BufferObserver *bufferObserver )
  {
    bufferObservers.removeObject( bufferObserver );
  }
  void removeDisplayObserver( BufferObserver *displayObserver );
  void removeLoadObserver( LoadObserver *loadObserver )
  {
    loadObservers.removeObject( loadObserver );
  }
  void notifyObservers( int reader );
  void textModified( int reader, int start, int end );
  void finishedLoading( BOOL isBinaryFile, BOOL reloaded );
  BOOL textModified() { return isModified; }
  void setUnmodified( BOOL saved ) { setModified( FALSE, saved ); }
  int getSize() { return textSize; }
  // Helper method for counting LF's between two positions. Runs much faster
  // than using getChar from the outside.
  int countLF( int reader, int start, int end );
  // Delete the buffer
  void deleteBuffers();

  void integrityCheck( char *file );

  // Enumerate text buffers
  // First call readRequest(), then beginEnum() to start enumeration
  void beginEnum();

  // enumTextBlock will enumerate all text blocks until the end is reached.
  // NULL or size == 0 returned means end of text
  TextElem *enumTextBlock( int &size );

	// The bufferholder should decide when the update should be shown
  void filenameUpdated();
  void reset();
  TextElem getCharNoReader( int pos );

  ~TextBuffer()
  {
    deleteBuffers();
    free( currentBlockStack );
    free( blockStartStack );

		// Remove myself from autosave directory
		AutosaveThread::getInstance()->deleteASVFile( filename );
  }
};

#endif
