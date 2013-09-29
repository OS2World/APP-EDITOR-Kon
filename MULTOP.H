/*****************************************************************************
 * Classes that should operate on multiple files should inherit from this
 * class. operateFile is called from MultipleFiles
 */

#if !defined MULTOP
#define MULTOP

#include "view.h"

class MultipleFilesOperation
{
public:
  // isVisible is TRUE if the view is in a notebook
  // Return FALSE to terminate
  virtual BOOL operateFile( char *filename, View *view, BOOL isVisible ) 
    { return TRUE; }
};

#endif