#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_WINSHELLDATA
#define INCL_WINWORKPLACE

#include "bowl.h"
#include "globals.h"
#include <direct.h>
#include <ctype.h>

int DirEntry::DIR = 1;
int DirEntry::ARCHIVED = 2;
int DirEntry::HIDDEN = 4;
int DirEntry::SYSTEM = 8;

int FileError::FILENOTFOUND = 1;
int FileError::PATHNOTFOUND = 2;
int FileError::ACCESSDENIED = 3;
int FileError::SHARINGVIOLATION = 4;
int FileError::FILEOPENERROR = 5;
int FileError::FILEEOF = 6;
int FileError::READERROR = 7;
int FileError::WRITEERROR = 8;

int Thread::PRIO_IDLE = PRTYC_IDLETIME;
int Thread::PRIO_REGULAR = PRTYC_REGULAR;
int Thread::PRIO_TIMECRITICAL = PRTYC_TIMECRITICAL;

int Semaphore::FOREVER = SEM_INDEFINITE_WAIT;

char *FileError::getPlainTextMessage( int error )
{
	static str[200];
	if( error == FILENOTFOUND ) return Strings::get(152);
	if( error == PATHNOTFOUND ) return Strings::get(153);
	if( error == ACCESSDENIED ) return Strings::get(154);
	if( error == SHARINGVIOLATION ) return Strings::get(155);
	if( error == FILEOPENERROR ) return Strings::get(156);
	if( error == FILEEOF ) return Strings::get(157);
	if( error == READERROR ) return Strings::get(158);
	sprintf( (char *)str, "Unknown error: %d", error );
	return (char *)str;
}

Dir::Dir( char *pattern, int attrib, int mustHaveAttrib )
{
  HDIR          findHandle = HDIR_CREATE, dirFindHandle;
  FILEFINDBUF3  findBuffer;
  ULONG         findCount = 1;
  APIRET rc;
  int osAttrib = 0;

  if( attrib & DirEntry::DIR )
    osAttrib |= FILE_DIRECTORY;

  if( attrib & DirEntry::ARCHIVED )
    osAttrib |= FILE_ARCHIVED;

  if( attrib & DirEntry::SYSTEM )
    osAttrib |= FILE_SYSTEM;

  if( attrib & DirEntry::HIDDEN )
    osAttrib |= FILE_HIDDEN;

  if( mustHaveAttrib & DirEntry::DIR )
    osAttrib |= MUST_HAVE_DIRECTORY;

  if( mustHaveAttrib & DirEntry::ARCHIVED )
    osAttrib |= MUST_HAVE_ARCHIVED;

  if( mustHaveAttrib & DirEntry::SYSTEM )
    osAttrib |= MUST_HAVE_SYSTEM;

  if( mustHaveAttrib & DirEntry::HIDDEN )
    osAttrib |= MUST_HAVE_HIDDEN;

  rc = DosFindFirst((PSZ)pattern, &findHandle, osAttrib, (PVOID)&findBuffer,
                    sizeof(findBuffer), &findCount, FIL_STANDARD);

  while( rc == 0 )
  {
    int attrib = 0;
    if( findBuffer.attrFile & FILE_DIRECTORY )
      attrib |= DirEntry::DIR;
    if( findBuffer.attrFile & FILE_ARCHIVED )
      attrib |= DirEntry::ARCHIVED;
    if( findBuffer.attrFile & FILE_HIDDEN )
      attrib |= DirEntry::HIDDEN;
    if( findBuffer.attrFile & FILE_SYSTEM )
      attrib |= DirEntry::SYSTEM;

    dirListing.add( new DirEntry( findBuffer.achName, attrib ) );

    // Get the next entry

    rc = DosFindNext( findHandle, (PVOID)&findBuffer, sizeof( findBuffer ),
                      &findCount );
  }

  DosFindClose( findHandle );
}

#if defined __BORLANDC__
void threadStarter( void *threadObject )
#else
void _Optlink threadStarter( void *threadObject )
#endif
{
  // Set normal priority

//  DosSetPriority( PRTYS_THREAD, PRTYC_REGULAR, -1, 0 );

  // Create a message queue

	HMQ hmq = WinCreateMsgQueue( Application::hab, 0 );
  // Call the run method

  ((Thread *)threadObject)->run();

  if( ((Thread *)threadObject)->killAtExit )
    delete (Thread *)threadObject;
  Application::processMessages();
  WinDestroyMsgQueue( hmq );
}

BOOL Semaphore::muxWait( int semaphores, Semaphore *semaphore[], int timeToWait )
{
  PSEMRECORD semRecord;
  HMUX muxSem = NULLHANDLE;

  semRecord = new SEMRECORD[semaphores];

  for( int i = 0; i < semaphores; i++ )
  {
    semRecord[i].hsemCur = semaphore[i]->getHandle();
    semRecord[i].ulUser = 0;
  }

  int rc;

  if( (rc = DosCreateMuxWaitSem( NULL, &muxSem, semaphores, semRecord, DCMW_WAIT_ALL )) != 0 )
  {
/*    FILE *f = fopen( "sem", "a" );
    fprintf( f, "Muxwait create failed: %d\n", rc );
    fclose( f );*/
  }
  ULONG foo;
  rc = DosWaitMuxWaitSem( muxSem, timeToWait, &foo );
  DosCloseMuxWaitSem( muxSem );
  delete semRecord;
  return rc != ERROR_TIMEOUT;
}

BOOL MutexSemaphore::wait( int timeToWait )
{
  ULONG foo;
  Semaphore *mlist[1];

  mlist[0] = this;

  return Semaphore::muxWait( 1, mlist, timeToWait );
}

BlockFileReader::BlockFileReader( char *filename ) : error(0)
{
  strcpy( this->filename, filename );

  ULONG dummy;

  APIRET rc = DosOpen( filename,
                       &file,
                       &dummy,
                       0,
                       FILE_NORMAL,
                       FILE_OPEN,
                       OPEN_ACCESS_READONLY |
                       OPEN_SHARE_DENYWRITE,
                       NULL );
  switch( rc )
  {
    case NO_ERROR : break;
    case ERROR_FILE_NOT_FOUND : error = FileError::FILENOTFOUND; break;
    case ERROR_PATH_NOT_FOUND : error = FileError::PATHNOTFOUND; break;
    case ERROR_ACCESS_DENIED : error = FileError::ACCESSDENIED; break;
    case ERROR_SHARING_VIOLATION : error = FileError::SHARINGVIOLATION; break;
    default : error = FileError::FILEOPENERROR;
  }
}

BlockFileWriter::BlockFileWriter( char *filename ) : error(0)
{
  strcpy( this->filename, filename );

  ULONG dummy;

  APIRET rc = DosOpen( filename,
                       &file,
                       &dummy,
                       0,
                       FILE_NORMAL,
                       0x11,
                       OPEN_ACCESS_WRITEONLY |
                       OPEN_SHARE_DENYREADWRITE,
                       NULL );
  switch( rc )
  {
    case NO_ERROR : break;
    case ERROR_FILE_NOT_FOUND : error = FileError::FILENOTFOUND; break;
    case ERROR_PATH_NOT_FOUND : error = FileError::PATHNOTFOUND; break;
    case ERROR_ACCESS_DENIED : error = FileError::ACCESSDENIED; break;
    case ERROR_SHARING_VIOLATION : error = FileError::SHARINGVIOLATION; break;
    default : error = FileError::FILEOPENERROR;
  }
}

void System::getFullPath( char *file, char *fullPath, int size  )
{
  if( _fullpath( fullPath, file, size ) == NULL )
    strcpy( fullPath, file );
}

Drive *System::getDrivesInfo( char &current )
{
  ULONG driveMap = 0;
  ULONG currentDisk = 0;
  DosQueryCurrentDisk( &currentDisk, &driveMap );
  currentDisk--;  // Make 0 represent drive 'A'
  current = currentDisk + 'A';

  Drive *driveList = NULL, *cDrive = NULL;

//  FILE *f = fopen( "test", "w" );

  for( int i = 0; i < 26; i++ )
  {
    if( (driveMap<<(31-i)) >> 31 )
    {
      FSINFO fsInfo;
      char notRemovable = -1;
      char param[2] = { 0, i };
      ULONG parmLen = 2, dataLen = 1;
      char *volLabel = "unknown";

      DosDevIOCtl( -1, IOCTL_DISK, DSK_BLOCKREMOVABLE, &param, 2, &parmLen,
                   &notRemovable, 1, &dataLen );

      if( notRemovable || currentDisk == i )
      {
        DosQueryFSInfo( i+1, FSIL_VOLSER, &fsInfo, sizeof( fsInfo ) );
        volLabel = fsInfo.vol.szVolLabel;
      }

//      fprintf( f, "%d, %d, %d\n", currentDisk, notRemovable, i );

      Drive *nDrive = new Drive( i + 'A', volLabel );

      if( currentDisk == i )
        nDrive->active = TRUE;

      if( driveList == NULL )
        driveList = nDrive;

      if( cDrive != NULL )
        cDrive->next = nDrive;

      cDrive = nDrive;
    }
  }
//  fclose( f );
  return driveList;
}

BOOL System::fileExists( char *filename )
{
	FILESTATUS3 finfo;
	return DosQueryPathInfo( filename, FIL_STANDARD, &finfo, 
													 sizeof( FILESTATUS3 ) ) == 0;
}

BOOL NamedMessagePipe::create( char *name )
{
  serverMode = TRUE;
  char pname[MAXPATH+1];

  strcpy( pname, "\\PIPE\\" );
  strcat( pname, name );

  APIRET rc = DosCreateNPipe( pname, &handle, NP_ACCESS_INBOUND | 
                              NP_NOINHERIT, NP_WAIT | NP_TYPE_MESSAGE | 
                              NP_READMODE_MESSAGE|0x01, 300, 300, 10000 );
  return rc != 0;
}

BOOL NamedMessagePipe::connect( char *name )
{
  char pname[MAXPATH+1];

  strcpy( pname, "\\PIPE\\" );
  strcat( pname, name );
  
  ULONG action;

  APIRET rc = DosOpen( pname, &handle, &action, 0, FILE_NORMAL, FILE_OPEN,
                       OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE, 
                       (PEAOP2)NULL );
  return rc != 0;
}

void NamedMessagePipe::sendMessage( char *message )
{
  ULONG written;

  DosWrite( handle, message, strlen( message ), &written );
}

BOOL NamedMessagePipe::waitForMessage()
{
  APIRET rc = DosConnectNPipe( handle );

  return rc != 0;
}

BOOL NamedMessagePipe::readMessage( char *message, int &maxLength )
{
  APIRET rc = DosRead( handle, message, maxLength, (PULONG)&maxLength );
  message[maxLength] = 0;

  return rc != 0;
}

void NamedMessagePipe::closeConnection()
{
  DosDisConnectNPipe( handle );
}

NamedMessagePipe::~NamedMessagePipe()
{
  if( handle != NULL )
  {
    if( !serverMode )
      DosClose( handle );
  }
}

BOOL System::getFileTime( char *filename, BTime *fileTime )
{
	HFILE file;
	ULONG openAction = 0;
	APIRET rc;

	rc = DosOpen( filename, &file, &openAction, 0L, 0L, OPEN_ACTION_FAIL_IF_NEW |
	              OPEN_ACTION_OPEN_IF_EXISTS,
								OPEN_FLAGS_NOINHERIT | OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE,
								0L );

  if( rc != NO_ERROR ) return FALSE;

	FILESTATUS3 fi3;

  rc = DosQueryFileInfo( file, FIL_STANDARD, &fi3, sizeof( FILESTATUS3 ) );

	DosClose( file );

	if( rc != NO_ERROR ) return FALSE;

  fileTime->year = fi3.fdateLastWrite.year+1980L;
  fileTime->month = fi3.fdateLastWrite.month;
  fileTime->day = fi3.fdateLastWrite.day;
  fileTime->hour = fi3.ftimeLastWrite.hours;
  fileTime->min = fi3.ftimeLastWrite.minutes;
  fileTime->sec = fi3.ftimeLastWrite.twosecs;
  fileTime->msec = 0;

  return TRUE;
}

BOOL System::launchBrowser( char *URL )
{
  char appKey[CCHMAXPATH+1];
	unsigned long bufSize = CCHMAXPATH;
	char path[CCHMAXPATH+1];

	sprintf( path, "URL=%s", URL );

	HOBJECT URLobj = WinCreateObject( "WPUrl", "Kon help", path, "<WP_NOWHERE>", 
	                                  CO_REPLACEIFEXISTS );

  if( URLobj != NULLHANDLE )
	  WinOpenObject( URLobj, 1, TRUE );
	else
	  return FALSE;

	return TRUE;
/*	FILE *f = fopen( "test", "wb" );
	PrfQueryProfileData( HINI_USERPROFILE, "PM_Workplace:Location", "<MOZILLAEXE>",
	                     appKey, &bufSize );

  HOBJECT browser;

	browser = WinQueryObject( "<INNOTEK_FLASH_README>");
	bufSize = CCHMAXPATH;
	int rc = WinQueryObjectPath( browser, path, bufSize );
	int le = WinGetLastError( Application::hab );

	fprintf( f, "%d, %d, %d, %x: %s\n", appKey, browser, rc, le, path );
	fclose( f );

	bufSize = CCHMAXPATH;

	/// Get the handle for the object
	if( PrfQueryProfileData( HINI_USERPROFILE, "PMWP_ASSOC_FILTER", "*.HTM",
	                     appKey, &bufSize ) )
  {
	  // Find the path to the object

	}*/
}

int System::getKeyState()
{
  int modifiers = 0;

  if( WinGetKeyState( HWND_DESKTOP, VK_SHIFT ) & 0x8000 )
    modifiers |= KeyDef::SHIFT;

  if( WinGetKeyState( HWND_DESKTOP, VK_CTRL ) & 0x8000 )
    modifiers |= KeyDef::CTRL;

  if( WinGetKeyState( HWND_DESKTOP, VK_ALT ) & 0x8000 )
    modifiers |= KeyDef::kALT;
  
  if( WinGetKeyState( HWND_DESKTOP, VK_ALTGRAF ) & 0x8000 )
    modifiers |= KeyDef::kALT;
  
  return modifiers;
}

char *System::getSettingsDir()
{
  return KonSystem::baseDir;
}

Launch::Launch( char *proc, char *args, MessagePipe *stdOut) : 
  Thread(FALSE), hfSave( NULL), redirectedOutput(FALSE)
{ 
  CHAR       LoadError[CCHMAXPATH] = {0};
  CHAR       Args[1000];
  APIRET      rc               = NO_ERROR;  /* Return code */
  HFILE hfNew;

  strcpy( Args, proc );
  strcpy( &Args[strlen(Args)+1], args );

  if( stdOut != NULL )
  {
    redirectedOutput = TRUE;
    HPIPE hpR, hpW;

		hfSave = -1;
		hfNew = HF_STDOUT;

		DosDupHandle(HF_STDOUT, &hfSave);    /* Saves standard output handle      */

		DosCreatePipe(&hpR, &hpW, PIPESIZE); /* Creates pipe                      */

		DosDupHandle(hpW, &hfNew);           /* Duplicates standard output handle */

		rc = DosExecPgm(LoadError,           /* Object name buffer           */
										sizeof(LoadError),   /* Length of object name buffer */
										EXEC_ASYNC,           /* Asynchronous/Trace flags     */
										(PSZ)Args,                /* Argument string              */
										(PSZ)NULL,                /* Environment string           */
										&resc,            /* Termination codes            */
										proc);      /* Program file name            */

    DosClose(hpW);                       /* Closes write handle to ensure     */
																					/* Notification at child termination */

    DosDupHandle(hfSave, &hfNew);        /* Brings stdout back                */

    stdOut->setReadHandle( hpR );
  }
  else
  {
		rc = DosExecPgm(LoadError,           /* Object name buffer           */
										sizeof(LoadError),   /* Length of object name buffer */
										EXEC_ASYNC,           /* Asynchronous/Trace flags     */
										(PSZ)Args,                /* Argument string              */
										(PSZ)NULL,                /* Environment string           */
										&resc,            /* Termination codes            */
										proc);      /* Program file name            */
  }
}

void Launch::waitProcess()
{
  start( 4096 );
};

void Launch::run()
{
  PID pidEnded;

  // Wait until child process exits.
  DosWaitChild(DCWA_PROCESS,          /* Only the process       */
               DCWW_WAIT,             /* Waits until it is done */
               &resc,                 /* Puts the result here   */
               &pidEnded,             /* PID of ended process   */
               resc.codeTerminate);   /* Child to wait for     */

  if( redirectedOutput ) 
    DosClose( hfSave );
}

MessagePipe::create()
{
/*  SECURITY_ATTRIBUTES sa;

  CreatePipe(&pReadHandle,&pWriteHandle,&sa,0);*/
}

char *MessagePipe::readPipe()
{
  ULONG cbRead = 0;

  DosRead(pReadHandle, achBuf, PIPESIZE-1, &cbRead);

  if( !cbRead ) return NULL;

  achBuf[cbRead] = 0;

  return achBuf;
}

void MessagePipe::closeReadHandle()
{
  if( pReadHandle ) DosClose( pReadHandle );
  pReadHandle = NULL;
}

void MessagePipe::closeWriteHandle()
{
  if( pWriteHandle ) DosClose( pWriteHandle );
  pWriteHandle = NULL;
}

MessagePipe::~MessagePipe()
{
  closeReadHandle();
  closeWriteHandle();
}
