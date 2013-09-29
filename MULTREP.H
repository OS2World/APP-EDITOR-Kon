/*****************************************************************************
 * Replace ocurrences of a string in a file. Display progress in the
 * ErrorCollector
 */

#if !defined MULTREPLACE
#define MULTREPLACE

#include "multop.h"

class MultipleFilesReplace : public MultipleFilesOperation
{
	char *searchString, *replaceString, *options;
public:
	MultipleFilesReplace( char *searchString, char *replaceString, char *options );
  virtual BOOL operateFile( char *filename, View *view, BOOL isVisible );
  ~MultipleFilesReplace() { delete options; }
};

#endif