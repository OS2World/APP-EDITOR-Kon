/******************************************************************************
 * The Search and GrepSearch classes finds text in a textBuffer
 * They do a plain character by character search and does not care about
 * any internal formatting.
 *
 * A textBuffer reader must already have been opened and should be supplied at
 * creation.
 */

#include "buffer.h"

class Search
{
  TextBuffer *textBuffer;
  TextElem *text, *delimitors;
  int textLen;
  BOOL caseSensitive, wholeWords, wrapAround;
  int lastChar[256], firstChar[256];
  int reader;
public:
  Search( TextBuffer *textBuffer, int reader, TextElem *text, int textLen,
          TextElem *delimitors, BOOL caseSensitive, BOOL wholeWords, BOOL wrapAround );
  BOOL findForward( int &position, int &len, int start, int end );
  BOOL findBackward( int &position, int &len, int start, int end );
};

