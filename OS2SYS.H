#include <stdlib.h>
#include <string.h>

#define MAXPATH	CCHMAXPATH
#define BLOCKSIZE 5164

#define HF_STDOUT 1      /* Standard output handle */
#define HF_STDIN 0      /* Standard output handle */
#define PIPESIZE 256

/******************************************************************************
 * DirEntry holds one directory entry, such as a file or directory together
 * with it's attributes.
 */

class DirEntry : public AObject
{
  char *name;
  int attrib;
public:
  static int DIR, ARCHIVED, SYSTEM, HIDDEN;
  DirEntry( char *name, int attrib )
  {
    this->name = new char[strlen( name )+1];
    strcpy( this->name, name );
    this->attrib = attrib;
  }
  char *getName() { return name; }
  int getAttrib() { return attrib; }

  ~DirEntry()
  {
    delete name;
  }
};

/******************************************************************************
 * Dir.
 * Retrieves a directory. The direcotry is retrieved in the constructor from
 * the searchpattern and the file attributes pattern.
 */

class Dir : public AObject
{
  Vector<DirEntry> dirListing;
public:
  // attrib : include files with these flags
  // mustHaveAttrib: include only files with these flags
  Dir( char *pattern, int attrib, int mustHaveAttrib );
  // Gets the number of entries found
  int getEntries() { return dirListing.getSize(); }
  // Gets one entry
  DirEntry *getEntryAt( int i ) { return dirListing.getObjectAt( i ); }
};

/******************************************************************************
 * The thread class defines a thread
 * When the thread is finished runnigng (i.e. run() exits), the thread may
 * kill the thread object by itself.
 */

#if defined __BORLANDC__
void threadStarter( void * );
#else
void _Optlink threadStarter( void * );
#endif

class Thread
{
  TID id;
public:
  static int PRIO_IDLE, PRIO_REGULAR, PRIO_TIMECRITICAL;
  BOOL killAtExit;
  Thread( BOOL killAtExit )
  {
    this->killAtExit = killAtExit;
  }
  // Start starts the thread. This function should not be subclassed
  void start( int stacksize )
  {
    #if defined __BORLANDC__
    id = (int)_beginthread( threadStarter, stacksize, this );
    #else
    id = (int)_beginthread( threadStarter, NULL, stacksize, this );
    #endif
  }
  // This function is the thread function. When the thread is started this
  // method is called
  virtual void run() {}
  void setPriority( int prioClass, int delta )
  {
    // prioClasses has been defined to the correct PRTYC_* values
    DosSetPriority( PRTYS_THREAD, prioClass, delta, (TID)id );
  }
  virtual ~Thread() {}
};


class Semaphore
{
public:
	static int FOREVER;
  static void muxWait( int semaphores, Semaphore *semaphore[] )
  {
  	muxWait( semaphores, semaphore, FOREVER );
  }

  // Returns FALSE if timeout
  static BOOL muxWait( int semaphores, Semaphore *semaphore[], int timeToWait );
  virtual HSEM getHandle()
  {
    return NULLHANDLE;
  }
};

class MutexSemaphore : public Semaphore
{
  HMTX hmtx;
public:
  MutexSemaphore()
  {
    // Create an unnamed private mutual exclution semaphore
    // Assume creation went normal.

    DosCreateMutexSem( NULL, &hmtx, 0, FALSE );
  }
  int request()
  {
    int rc = DosRequestMutexSem( hmtx, (LONG)SEM_INDEFINITE_WAIT );
    if( rc != 0 )
    {
/*      FILE *f = fopen( "sem", "a" );
      fprintf( f, "DosRequestMutexSem error: %d\n", rc );
      fclose( f );*/
    }
    return rc;
  }
  int release()
  {
    int rc = DosReleaseMutexSem( hmtx );
    if( rc != 0 )
    {
/*      FILE *f = fopen( "sem", "a" );
      fprintf( f, "DosReleaseMutexSem error: %d\n", rc );
      fclose( f );*/
    }
    return rc;
  }
  virtual HSEM getHandle()
  {
    return (HSEM)hmtx;
  }
  BOOL wait( int timeToWait );
  ~MutexSemaphore()
  {
    DosCloseMutexSem( hmtx );
  }
};

class EventSemaphore : public Semaphore
{
  HEV hev;
public:
  EventSemaphore()
  {
    // Create an unnamed private event semaphore
    // Assume creation went normal.

    DosCreateEventSem( NULL, &hev, 0, FALSE );
  }
  void wait()
  {
    DosWaitEventSem( hev, (ULONG)SEM_INDEFINITE_WAIT );
  }
  int check()
  {
    ULONG count;
    DosQueryEventSem( hev, &count );
    return count;
  }
  void post()
  {
    DosPostEventSem( hev );
  }
  void reset()
  {
    ULONG posted;
    DosResetEventSem( hev, &posted );
  }
  virtual HSEM getHandle()
  {
    return (HSEM)hev;
  }
  ~EventSemaphore()
  {
    DosCloseEventSem( hev );
  }
};

/******************************************************************************
 *
 */

struct FileError
{
  static int FILENOTFOUND, PATHNOTFOUND, ACCESSDENIED, SHARINGVIOLATION,
             FILEOPENERROR, READERROR, FILEEOF, WRITEERROR;
	static char *getPlainTextMessage( int error );
};

class BlockFileReader
{
  char filename[CCHMAXPATH+1];
  int error;
  HFILE file;
public:
  BlockFileReader( char *filename );
  int getStatus() { return error; }
  int getFileLength()
  {
    FILESTATUS FileInfoBuf;

    APIRET rc = DosQueryFileInfo( file, 1, &FileInfoBuf, sizeof( FILESTATUS ) );

    if( rc != 0 )
      return 0;
    else
      return (LONG)FileInfoBuf.cbFile;
  }
  int readBlock( char *text, int size )
  {
    ULONG read;
    if( DosRead( file, text, size, &read) )
      error = FileError::READERROR;
    return (int)read;
  }
  ~BlockFileReader()
  {
    DosClose( file );
  }
};

class BlockFileWriter
{
  char filename[CCHMAXPATH+1];
  int error;
  HFILE file;
public:
  BlockFileWriter( char *filename );
  int getStatus() { return error; }
  void setFileSize( int size )
  {
    DosSetFileSize( file, size );
  }
  int writeBlock( char *text, int size, EncodingTypes encType = ANSI  )
  {
    ULONG written;
    if( DosWrite( file, text, size, &written) )
      error = FileError::WRITEERROR;
    return (int)written;
  }
  ~BlockFileWriter()
  {
    DosClose( file );
  }
};

struct Drive
{
  char letter;
  char *name;
  Drive *next;
  BOOL active;
  Drive( char letter, char *name )
  {
    this->letter = letter;
    this->name = new char[strlen(name)+1];
    strcpy( this->name, name );
    next = NULL;
    active = FALSE;
  }
  ~Drive()
  {
    delete name;
    if( next )
      delete next;
  }
};

class System
{
public:
  static BOOL launchBrowser( char *URL );
  static BOOL copyFile( char *source, char *target )
  {
    return DosCopy( (PSZ)source, (PSZ)target, DCPY_EXISTING );
  }

	static BOOL deleteFile( char *file )
	{
		return DosDelete( file );
	}

  // fullPath must be allocated with size bytes

  static void getFullPath( char *file, char *fullPath, int size );

  // Delete the returned drive list after use

  static Drive *getDrivesInfo( char &current );

	static BOOL fileExists( char *filename );

  static BOOL getFileTime( char *filename, BTime *fileTime );

	static void sleep( LONG msec )
	{
		DosSleep( msec );
	}
  static int getKeyState();
  static char *getSettingsDir();
};

class NamedMessagePipe
{
  HPIPE handle;
  BOOL serverMode;
public:
	static supportsPipes() { return TRUE; }
  NamedMessagePipe() : handle(NULL) {}
  // Create a new pipe
  BOOL create( char *name );
  // Connect to an existing pipe
  BOOL connect( char *name );
  void sendMessage( char *message );
  BOOL waitForMessage();
  BOOL readMessage( char *message, int &maxLength );
  void closeConnection();
  ~NamedMessagePipe();
};

class MessagePipe
{
  HPIPE pReadHandle, pWriteHandle;
  CHAR achBuf[PIPESIZE];
public:
  MessagePipe() : pReadHandle( NULL ), pWriteHandle( NULL ) {}
  create();
  char *readPipe();
  HPIPE getReadHandle() { return pReadHandle; }
  HPIPE getWriteHandle() { return pReadHandle; }
  void setReadHandle( HPIPE pHandle ) { pReadHandle = pHandle; }
  void setWriteHandle( HPIPE pHandle ) { pWriteHandle = pHandle;; }
  void closeReadHandle();
  void closeWriteHandle();
  ~MessagePipe();
};

class Launch : public Thread
{
  BOOL redirectedOutput;
  HFILE hfSave;
  RESULTCODES resc;
public:
  Launch( char *proc, char *args, MessagePipe *stdOut = NULL);
  void waitProcess();
  virtual void run();
};

class DDE
{
public:
	DDE() {}
	BOOL startServer( char *, char * ) { return FALSE; }
	BOOL connect( char *, char * ) { return FALSE; }
	virtual void incoming( char * ) {}
	BOOL send( char *, int ) { return FALSE; }
	~DDE() {}
};

class UnicodeHelper
{
  static char *imctext;
  static long imctextSize;
public:
  static void fixFirstBytes( unsigned char *firstBytes, int &len ) {};
  static EncodingTypes checkEncoding( unsigned char *firstBytes, int len) { return ANSI; };
  static void unicodeToAnsi( char *inBuf, char *outBuf, int &bytesLen, EncodingTypes encoding ) {};
  static void ansiToUnicode( char *inBuf, char *outBuf, int &bytesLen, EncodingTypes encoding ) {};
};
