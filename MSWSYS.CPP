#include "bowl.h"
#include "direct.h"
#include "globals.h"
#include <shlobj.h>
#include <time.h>

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

int Thread::PRIO_IDLE = IDLE_PRIORITY_CLASS;
int Thread::PRIO_REGULAR = NORMAL_PRIORITY_CLASS;
int Thread::PRIO_TIMECRITICAL = REALTIME_PRIORITY_CLASS;

int Semaphore::FOREVER = INFINITE;

char System::settingsDir[MAX_PATH+1];

char *FileError::getPlainTextMessage( int error )
{
	static char str[200];
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
  HANDLE dir;

  WIN32_FIND_DATA wfdata;

  dir = FindFirstFile( pattern, &wfdata );

  BOOL rc = TRUE;

  while( rc && dir != INVALID_HANDLE_VALUE )
  {
    if( !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE && !(attrib & DirEntry::ARCHIVED)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN && !(attrib & DirEntry::HIDDEN)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM && !(attrib & DirEntry::SYSTEM)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY && !(attrib & DirEntry::DIR)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) != FILE_ATTRIBUTE_ARCHIVE && (mustHaveAttrib & DirEntry::ARCHIVED)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != FILE_ATTRIBUTE_HIDDEN && (mustHaveAttrib & DirEntry::HIDDEN)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != FILE_ATTRIBUTE_HIDDEN && (mustHaveAttrib & DirEntry::SYSTEM)) &&
        !( (wfdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY && (mustHaveAttrib & DirEntry::DIR)))
      dirListing.add( new DirEntry( wfdata.cFileName, attrib ) );
    // Get the next entry

    rc = FindNextFile( dir, &wfdata );
  }
  FindClose( dir );
}

#if defined __BORLANDC__
void threadStarter( void *threadObject )
#elif defined _MSC_VER
void threadStarter( void *threadObject )
#else
void _Optlink threadStarter( void *threadObject )
#endif
{
  // Call the run method

  ((Thread *)threadObject)->run();

  if( ((Thread *)threadObject)->killAtExit )
    delete (Thread *)threadObject;
  Application::processMessages();
}

//#define SEMDEBUG

void semDebug( char *semType, char *caller, char *error, DWORD errCode )
{
#if defined SEMDEBUG
  char filename[255];
  sprintf( filename, "%s%s", KonSystem::baseDir, "sem" );
  time_t ltime;
  time( &ltime );
  char timeStr[100];

  strcpy( timeStr, ctime( &ltime ) );
  timeStr[ strlen( timeStr )-1] = 0;

  FILE *f = fopen( filename, "a" );
  if( !strcmp( error, "" ) && errCode == 0 )
    fprintf( f, "%s: %s (%s)\n", caller, semType, timeStr );
  else
    fprintf( f, "**** %s: %s <%s:%d> (%s)\n", caller, semType, error, errCode, timeStr );
  fclose( f );
#endif
}

BOOL Semaphore::muxWait( int semaphores, Semaphore *semaphore[], int timeToWait, char *caller )
{
  HANDLE *handles;
  handles = new HANDLE[semaphores];

  for( int i = 0; i < semaphores; i++ )
    handles[i] = semaphore[i]->getHandle();

  semDebug( "Enter muxwait", caller, "", 0 );
  int rc = WaitForMultipleObjects( semaphores, handles, TRUE, timeToWait );

  if( rc == WAIT_FAILED ) 
    semDebug( "muxWait", caller, "", GetLastError() );
  else
    semDebug( "Leave muxWait", caller, "", 0 );

	delete[] handles;
	
  return (rc==WAIT_TIMEOUT) ? FALSE : TRUE;
}

BOOL MutexSemaphore::wait( int timeToWait )
{
  int rc = WaitForSingleObject( hmtx, timeToWait );

  return (rc==WAIT_TIMEOUT) ? FALSE : TRUE;
}

EventSemaphore::EventSemaphore()// : count(0)
{
  // Create an unnamed private event semaphore
  // Assume creation went normal.

  hev = CreateEvent( NULL, TRUE, FALSE, NULL );
}

void EventSemaphore::wait( char *caller )
{
//    count--;
  semDebug( "Enter wait", caller, "", 0 );
  int rc = WaitForSingleObject( hev, INFINITE );

  if( rc == WAIT_FAILED ) 
    semDebug( "wait", caller, "", GetLastError() );
  else
    semDebug( "Leave wait", caller, "", 0 );
//  ResetEvent( hev );
}

int EventSemaphore::check( char *caller )
{
  semDebug( "Enter check", caller, "", 0 );
  int count = WaitForSingleObject( hev, 0 );
  if( count == WAIT_FAILED ) 
    semDebug( "wait", caller, "", GetLastError() );
  else
    semDebug( "Leave wait", caller, "", 0 );
//  if( count == WAIT_OBJECT_0 )
//    SetEvent( hev );
  return (count == WAIT_OBJECT_0) ? 1 : 0;
}

BlockFileReader::BlockFileReader( char *filename ) : error(0)
{
  strcpy( this->filename, filename );

  file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

  if( file == INVALID_HANDLE_VALUE ) error = FileError::FILEOPENERROR;
}

BlockFileWriter::BlockFileWriter( char *filename ) : error(0)
{
  strcpy( this->filename, filename );

  file = fopen( filename, "wb" );

  if( file == NULL ) error = FileError::FILEOPENERROR;
}

int BlockFileWriter::writeBlock( char *text, int size, EncodingTypes encType )
{
  if( encType != ANSI )
  {
    char *utext = new char[size*2];
    UnicodeHelper::ansiToUnicode(text, utext, size, encType );
    fwrite( utext, size, 1, file );
    delete utext;
  }
  else
    fwrite( text, size, 1, file );
  return size;
}

BOOL System::fileExists( char *filename )
{
  WIN32_FIND_DATA FindFileData;

	return FindFirstFile(filename, &FindFileData) != INVALID_HANDLE_VALUE;
}

void System::getFullPath( char *file, char *fullPath, int size  )
{
  if( _fullpath( fullPath, file, size ) == NULL )
    strcpy( fullPath, file );
}

Drive *System::getDrivesInfo( char &current )
{
  char drives[110], curDir[MAXPATH+1], *drv;

  GetLogicalDriveStrings( 109, drives );

  drv = drives;

  GetCurrentDirectory( MAXPATH, curDir );

  Drive *driveList = NULL, *cDrive = NULL;

  while( *drv )
  {
    int type = GetDriveType( drv );

    char volLabel[15] = "unknown";

    if( type != DRIVE_REMOVABLE || *curDir == *drv )
    {
      DWORD foo;
      GetVolumeInformation( drv, volLabel, 14, NULL, &foo, &foo, NULL, 0 );
    }

    Drive *nDrive = new Drive( *drv, volLabel );

    if( toupper(*curDir) == toupper(*drv) )
      nDrive->active = TRUE;

    if( driveList == NULL )
      driveList = nDrive;

    if( cDrive != NULL )
      cDrive->next = nDrive;

    cDrive = nDrive;

    drv = drv + strlen( drv ) + 1;
  }
  return driveList;
}

BOOL NamedMessagePipe::create( char *name )
{
  serverMode = TRUE;
  char pname[MAXPATH+1];

  strcpy( pname, "\\\\.\\pipe\\" );
  strcat( pname, name );

  pHandle = CreateNamedPipe( pname, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE |
                            PIPE_READMODE_MESSAGE, 1,
                            300, 300, 1000, NULL );

  return pHandle == INVALID_HANDLE_VALUE;
}

BOOL NamedMessagePipe::connect( char *name )
{
  char pname[MAXPATH+1];

  strcpy( pname, "\\\\.\\pipe\\" );
  strcat( pname, name );

  pHandle = CreateFile( pname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
  DWORD dwMode = PIPE_READMODE_MESSAGE;
  SetNamedPipeHandleState( pHandle, &dwMode, NULL, NULL );

  return pHandle == INVALID_HANDLE_VALUE;
}

void NamedMessagePipe::sendMessage( char *message )
{
  ULONG written;

  WriteFile( pHandle, message, strlen( message ), &written, NULL );
}

BOOL NamedMessagePipe::waitForMessage()
{
  BOOL rc = ConnectNamedPipe( pHandle, NULL );

  return rc == FALSE;
}

BOOL NamedMessagePipe::readMessage( char *message, int &maxLength )
{
  BOOL rc = ReadFile( pHandle, message, maxLength, (PULONG)&maxLength, NULL );
  message[maxLength] = 0;

  return rc == FALSE;
}

void NamedMessagePipe::closeConnection()
{
  DisconnectNamedPipe( pHandle );
}

NamedMessagePipe::~NamedMessagePipe()
{
  if( pHandle != NULL )
  {
  	CloseHandle( pHandle );
	}
}

BOOL System::getFileTime( char *filename, BTime *fileTime )
{
  HANDLE file = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

  if( file == INVALID_HANDLE_VALUE ) return FALSE;

  FILETIME ftime;
  SYSTEMTIME stime;

  if( !GetFileTime( file, NULL, NULL, &ftime ) ) 
  {
    CloseHandle( file );
  
    return FALSE;
  }

  CloseHandle( file );

  FileTimeToSystemTime( &ftime, &stime );

  fileTime->year = stime.wYear;
  fileTime->month = stime.wMonth;
  fileTime->day = stime.wDay;
  fileTime->hour = stime.wHour;
  fileTime->min = stime.wMinute;
  fileTime->sec = stime.wSecond;
  fileTime->msec = stime.wMilliseconds;

  return TRUE;
}

Launch::Launch( char *proc, char *args, MessagePipe *stdOut) :
  Thread(TRUE), hStdIn( NULL), redirectedOutput(FALSE)
{ 
  HANDLE hOutputReadTmp,hOutputRead,hOutputWrite;
  HANDLE hInputWriteTmp,hInputRead,hInputWrite;
  HANDLE hErrorWrite;

  STARTUPINFO si;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

  char fullCommand[MAXPATH*3];
  strcpy( fullCommand, proc );
  strcat( fullCommand, " " );
  strcat( fullCommand, args );

  if( stdOut != NULL )
  {
    redirectedOutput = TRUE;

    SECURITY_ATTRIBUTES sa;

    // Set up the security attributes struct.
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Create the child output pipe.
    CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0);


    // Create a duplicate of the output write handle for the std error
    // write handle. This is necessary in case the child application
    // closes one of its std output handles.
    DuplicateHandle(GetCurrentProcess(),hOutputWrite,
                    GetCurrentProcess(),&hErrorWrite,0,
                    TRUE,DUPLICATE_SAME_ACCESS);

    // Create the child input pipe.
    CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0);

    // Create new output read handle and the input write handles. Set
    // the Properties to FALSE. Otherwise, the child inherits the
    // properties and, as a result, non-closeable handles to the pipes
    // are created.
    DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,
                    GetCurrentProcess(),
                    &hOutputRead, // Address of new handle.
                    0,FALSE, // Make it uninheritable.
                    DUPLICATE_SAME_ACCESS);

    DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,
                    GetCurrentProcess(),
                    &hInputWrite, // Address of new handle.
                    0,FALSE, // Make it uninheritable.
                    DUPLICATE_SAME_ACCESS);

    // Close inheritable copies of the handles you do not want to be
    // inherited.
    CloseHandle(hOutputReadTmp);
    CloseHandle(hInputWriteTmp);

    // Start process
    STARTUPINFO si;

    // Set up the start up info struct.
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hOutputWrite;
    si.hStdInput  = hInputRead;
    si.hStdError  = hErrorWrite;
    // Use this if you want to hide the child:
    si.wShowWindow = SW_HIDE;

    // Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
    // use the wShowWindow flags.


    // Launch the process that you want to redirect (in this case,
    // Child.exe). Make sure Child.exe is in the same directory as
    // redirect.c launch redirect from a command line to prevent location
    // confusion.

    CreateProcess(NULL,fullCommand,NULL,NULL,TRUE, 0,NULL,NULL,&si,&pi);

    CloseHandle(hOutputWrite);
    CloseHandle(hInputRead );
    CloseHandle(hErrorWrite);

    stdOut->setReadHandle( hOutputRead );
    stdOut->setWriteHandle( hInputWrite );

    // Get std input handle so you can close it and force the ReadFile to
    // fail when you want the input thread to exit.
    hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  }
  else
    CreateProcess( NULL, fullCommand, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );

  //memcpy( &this->pi, pi, sizeof( PROCESS_INFORMATION ) );
}

void Launch::waitProcess()
{
  start( 1024 );
};

void Launch::run()
{
  // Wait until child process exits.
  WaitForSingleObject( pi.hProcess, INFINITE );

  // Close process and thread handles. 
  CloseHandle( pi.hProcess );
  CloseHandle( pi.hThread );

  if( redirectedOutput ) CloseHandle(hStdIn);
}

BOOL System::launchBrowser( char *URL )
{
  // Find the browser
  HKEY hKey;
  TCHAR htmlEntry[255];
  DWORD dwBufLen = 255;
  LONG lRet;
  int URLpos = 7;
  
  if( strncmp( URL, "file:/", 6 ) == 0 )
  {
    URLpos = 8;
    for( int i = 6; i < strlen(URL); i++ )
    {
      switch( URL[i] )
      {
        //case '\\' : URL[i] = '/'; break;
        //case ':' : URL[i] = '|'; break;
      }
    }
  }

  if(RegOpenKeyEx(HKEY_CLASSES_ROOT,
      TEXT(".htm"), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) return FALSE;

  lRet = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)htmlEntry, &dwBufLen);

  RegCloseKey(hKey);

  if(lRet != ERROR_SUCCESS) return FALSE;

  char shellPath[100], command[255];

  strcpy( shellPath, htmlEntry );
  strcat( shellPath, "\\shell\\open\\command" );

  if(RegOpenKeyEx(HKEY_CLASSES_ROOT,
      TEXT(shellPath), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) return FALSE;

  dwBufLen = 255;

  lRet = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE)command, &dwBufLen);

  RegCloseKey(hKey);

  if(lRet != ERROR_SUCCESS) return FALSE;

  char launchCmd[1000];

  char *percent = strstr( command, "%1" );

  if( percent != NULL ) 
    *(percent+1) = 's';
  else
    strcat( command, " \"%s\"" );   

  sprintf( launchCmd, command, URL+URLpos );

  Launch *launch = new Launch( launchCmd, "" );

  launch->waitProcess();
  
  return TRUE;
}

int System::getKeyState()
{
  int modifiers = 0;

  if( GetKeyState( VK_SHIFT ) & 0x80 )
    modifiers |= KeyDef::SHIFT;

  if( GetKeyState( VK_CONTROL ) & 0x80 )
    modifiers |= KeyDef::CTRL;

  if( GetKeyState( VK_MENU ) & 0x80 )
    modifiers |= KeyDef::kALT;
  
  return modifiers;
}

char *System::getSettingsDir()
{
  if( SHGetSpecialFolderPath( NULL, settingsDir, CSIDL_APPDATA, TRUE ) )
  {
    strcat( settingsDir, "\\Kon\\" );
    CreateDirectory( settingsDir, NULL );
    return settingsDir;
  }
  else 
    return KonSystem::baseDir;
}

char *UnicodeHelper::imctext = NULL;
long UnicodeHelper::imctextSize = 0;

void UnicodeHelper::fixFirstBytes( unsigned char *firstBytes, int &len)
{
  // Check BOM
  int strip = 0;

  EncodingTypes encoding = checkEncoding( firstBytes, len );

  if( encoding == UTF8 )
    strip = 3;
  else if( encoding == UTF16 )
    strip = 2;

  if( encoding != ANSI )
  {
    memmove( firstBytes, firstBytes + strip, len - strip );
    len -= strip;
  }
}

void MessagePipe::create()
{
  SECURITY_ATTRIBUTES sa;

  CreatePipe(&pReadHandle,&pWriteHandle,&sa,0);
}

char *MessagePipe::readPipe()
{
  DWORD nBytesRead;

  if(!ReadFile(pReadHandle,lpBuffer,sizeof(lpBuffer)-1, &nBytesRead,NULL) || !nBytesRead)
    return NULL;
  else
    lpBuffer[nBytesRead] = 0;

  return lpBuffer;
}

void MessagePipe::closeReadHandle()
{
  if( pReadHandle ) CloseHandle( pReadHandle );
  pReadHandle = NULL;
}

void MessagePipe::closeWriteHandle()
{
  if( pWriteHandle ) CloseHandle( pWriteHandle );
  pWriteHandle = NULL;
}

MessagePipe::~MessagePipe()
{
  closeReadHandle();
  closeWriteHandle();
}

EncodingTypes UnicodeHelper::checkEncoding( unsigned char *firstBytes, int len)
{
  // Check BOM
  if( len >= 3 )
  {
    if((UCHAR)firstBytes[0] == 0xEF && (UCHAR)firstBytes[1] == 0xBB && (UCHAR)firstBytes[2] == 0xBF)
      return UTF8;
  }

  if( len >= 2 )
  {
    if(((UCHAR)firstBytes[0] == 0xFE && (UCHAR)firstBytes[1] == 0xFF) || 
               ((UCHAR)firstBytes[0] == 0xFF && (UCHAR)firstBytes[1] == 0xFE))
      return UTF16;
  }
  return ANSI;
}

void UnicodeHelper::unicodeToAnsi( char *inBuf, char *outBuf, int &bytesLen, EncodingTypes encoding )
{
  if( encoding == UTF16 )
    bytesLen = WideCharToMultiByte( CP_ACP, WC_DEFAULTCHAR|WC_COMPOSITECHECK, (LPCWSTR)inBuf, bytesLen/2, outBuf, bytesLen, NULL, NULL );
  else
  {
    if( imctextSize == 0 )
      imctext = (char *)malloc( bytesLen * 2 );
    else if( imctextSize < bytesLen * 2 )
    {
      imctextSize = bytesLen * 2;
      imctext = (char *)realloc( (void *)imctext, imctextSize );
    }

    int converted = MultiByteToWideChar( CP_UTF8, 0, inBuf, bytesLen, (LPWSTR)imctext, bytesLen );
    bytesLen = WideCharToMultiByte( CP_ACP, WC_DEFAULTCHAR|WC_COMPOSITECHECK, (LPWSTR)imctext, converted, outBuf, bytesLen*2, NULL, NULL );
  }
}

void UnicodeHelper::ansiToUnicode( char *inBuf, char *outBuf, int &bytesLen, EncodingTypes encoding  )
{
  int written = MultiByteToWideChar( CP_ACP, 0, inBuf, bytesLen, (LPWSTR)outBuf, bytesLen);
  if( encoding == UTF8 )
  {
    char *u16text = new char[written*2];
    memcpy( u16text, outBuf, written*2);
    bytesLen = WideCharToMultiByte( CP_UTF8, 0, (LPCWSTR)u16text, written, outBuf, written*2, NULL, NULL );
    delete u16text;
  }
  else
    bytesLen = written * 2;
}
