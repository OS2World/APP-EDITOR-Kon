#if !defined BUFFERHOLDER
#define BUFFERHOLDER

#include "bufadmin.h"
#include "buffer.h"

class BufferHolder
{
public:
  ~BufferHolder() { BufferAdmin::removeBufferHolder( this ); }
};
#endif