/*****************************************************************************
 * Search a directory (recursively) and does an operation on each file it
 * encounters.
 *****************************************************************************/

#if !defined MULTFILE
#define MULTFILE

#include "multop.h"
#include "bufadmin.h"
#include "bufhldr.h"

class MultipleFiles : public BufferHolder
{
	char *path, *files;
  int searchFile( char *file );	// Returns number of matches
  void searchDir( char *dir, char *files );
  BOOL recursive;
  static BOOL terminate;
  MultipleFilesOperation *operation;
public:
	MultipleFiles( MultipleFilesOperation *operation, char *path, char *files, 
                 BOOL recursive );
	static void terminateFileOperation() { terminate = TRUE; }
};

#endif