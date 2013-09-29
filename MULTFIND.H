/*****************************************************************************
 * Find ocurrences of a string in a file and display the result of the find
 * in the ErrorCollector
 */

#if !defined MULTFIND
#define MULTFIND

#include "multop.h"

class MultipleFilesFind : public MultipleFilesOperation
{
	char *searchString, *options;
public:
	MultipleFilesFind( char *searchString, char *options );
  virtual BOOL operateFile( char *filename, View *view, BOOL isVisible );
  ~MultipleFilesFind() { delete options; }
};

#endif