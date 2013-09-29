#include <stdlib.h>
#include <string.h>
#include <process.h>

#define MAXPATH 300
#define BLOCKSIZE 5164

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
  // If attrib is 0, all files are included, otherwise only the ones specified
  Dir( char *pattern, int attrib, int mustHave );
  // Gets the number of entries found
  int getEntries() { return dirListing.getSize(); }
  // Gets one entry
  DirEntry *getEntryAt( int i ) { return dirListing.getObjectAt( i ); }
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

/******************************************************************************
 * The thread class defines a thread
 * When the thread is finished runnigng (i.e. run() exits), the thread may
 * kill the thread object by itself.
 */

#if defined __BORLANDC__
void threadStarter( void * );
#elif defined _MSC_VER
void threadStarter( void * );
#else
void _Optlink threadStarter( void * );
#endif

class Thread
{
  int id;
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
    #if defined __BORLANDC_
    id = (int)_beginthread( threadStarter, stacksize, this );
	#elif defined _MSC_VER
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
    SetPriorityClass( (HANDLE)id, delta );
  }
  virtual ~Thread() {}
};

void semDebug( char *semType, char *caller, char *error, DWORD errCode );

class Semaphore
{
public:
	static int FOREVER;
  static void muxWait( int semaphores, Semaphore *semaphore[], char *caller = "")
  {
  	muxWait( semaphores, semaphore, FOREVER, caller );
  }

  // Returns FALSE if timeout
  static int muxWait( int semaphores, Semaphore *semaphore[], int timeToWait, char *caller = "");
  virtual HANDLE getHandle()
  {
    return NULL;
  }
};

class MutexSemaphore : public Semaphore
{
  HANDLE hmtx;
public:
  MutexSemaphore()
  {
    // Create an unnamed private mutual exclution semaphore
    // Assume creation went normal.

    hmtx = CreateMutex( NULL, FALSE, NULL );
  }
  void request(char *caller = "")
  {
    semDebug( "Enter request", caller, "", 0 );
    int rc = WaitForSingleObject( hmtx, INFINITE );
    if( rc == WAIT_FAILED )
      semDebug( "request", caller, "", GetLastError() );
    else
      semDebug( "Leave request", caller, "", 0 );
  }
  int release(char *caller = "")
  {
    semDebug( "Enter relase", caller, "", 0 );
    BOOL rc = ReleaseMutex( hmtx );
    if( rc == 0 )
      semDebug( "release", caller, "", GetLastError() );
    else
      semDebug( "Leave release", caller, "", 0 );

    return rc == 0;
  }
  BOOL wait( int timeToWait );
  virtual HANDLE getHandle()
  {
    return (HANDLE)hmtx;
  }
  ~MutexSemaphore()
  {
    CloseHandle( hmtx );
  }
};

class EventSemaphore : public Semaphore
{
  HANDLE hev;
//  ULONG count;
public:
  EventSemaphore();
  void wait( char *caller = "" );
  int check( char *caller = "" );
  void post()
  {
    SetEvent( hev );
//    count++;
  }
  void reset()
  {
    ResetEvent( hev );
//    count = 0;
  }
  virtual HANDLE getHandle()
  {
    return hev;
  }
  ~EventSemaphore()
  {
    CloseHandle( hev );
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
  char filename[MAX_PATH+1];
  int error;
  HANDLE file;
public:
  BlockFileReader( char *filename );
  int getStatus() { return error; }
  int getFileLength()
  {
    DWORD size = GetFileSize( file, NULL );
    return (size==0xffffffff) ? 0 : size;
  }
  int readBlock( char *text, int size )
  {
    int read;
    if( !ReadFile( file, text, size, (LPDWORD)&read, NULL) )
      error = FileError::READERROR;
    return read;
  }
  ~BlockFileReader()
  {
    CloseHandle( file );
  }
};

class BlockFileWriter
{
  char filename[MAX_PATH+1];
  int error;
  FILE *file;
public:
  BlockFileWriter( char *filename );
  int getStatus() { return error; }
  void setFileSize( int size )
  {
//    DosSetFileSize( file, size );
  }
  int writeBlock( char *text, int size, EncodingTypes encType = ANSI );
  ~BlockFileWriter()
  {
		if( file != NULL )
			fclose( file );
  }
};

class System
{
  static char settingsDir[MAX_PATH+1];
public:
  static BOOL copyFile( char *source, char *target )
  {
    return CopyFile( (PSZ)source, (PSZ)target, FALSE );
  }

	static BOOL deleteFile( char *file )
	{
		return DeleteFile( file );
	}

	static BOOL fileExists( char *filename );

  // fullPath must be allocated with size bytes

  static void getFullPath( char *file, char *fullPath, int size );

  // Delete the returned drive list after use

  static Drive *getDrivesInfo( char &current );

  static BOOL getFileTime( char *filename, BTime *fileTime );

	static void sleep( DWORD msec )
	{
		Sleep( msec );
	}

  static BOOL launchBrowser( char *URL );
  static int getKeyState();
  static char *getSettingsDir();
  static int toUpper( int c ) { return toupper( c ); }
  static int toLower( int c ) { return tolower( c ); }
};

class NamedMessagePipe
{
  HANDLE pHandle;
  BOOL serverMode;
public:
	static int supportsPipes()
	{
		OSVERSIONINFO os;
		os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx( &os );
		return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
	}
  NamedMessagePipe() : pHandle(NULL) {}
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
  HANDLE pReadHandle, pWriteHandle;
  CHAR lpBuffer[256];
public:
  MessagePipe() : pReadHandle( NULL ), pWriteHandle( NULL ) {}
  void create();
  char *readPipe();
  HANDLE getReadHandle() { return pReadHandle; }
  HANDLE getWriteHandle() { return pReadHandle; }
  void setReadHandle( HANDLE pHandle ) { closeReadHandle(); pReadHandle = pHandle; }
  void setWriteHandle( HANDLE pHandle ) { closeWriteHandle(); pWriteHandle = pHandle;; }
  void closeReadHandle();
  void closeWriteHandle();
  ~MessagePipe();
};

class Launch : public Thread
{
  PROCESS_INFORMATION pi;
  BOOL redirectedOutput;
  HANDLE hStdIn;
public:
  Launch( char *proc, char *args, MessagePipe *stdOut = NULL);
  void waitProcess();
  virtual void run();
};

class UnicodeHelper
{
  static char *imctext;
  static long imctextSize;
public:
  static void fixFirstBytes( unsigned char *firstBytes, int &len );
  static EncodingTypes checkEncoding( unsigned char *firstBytes, int len);
  static void unicodeToAnsi( char *inBuf, char *outBuf, int &bytesLen, EncodingTypes encoding  );
  static void ansiToUnicode( char *inBuf, char *outBuf, int &bytesLen, EncodingTypes encoding  );
};