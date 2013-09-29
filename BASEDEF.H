#include <stdlib.h>
#include <string.h>

// Some basic defines

#define SHORT   short           /* s   */
#define LONG    long            /* l   */
#define FALSE 0
#define TRUE  1

//typedef unsigned long BOOL;

#define STRING char *

enum EncodingTypes { ANSI = 0, UTF8 = 1, UTF16 = 2 };

/******************************************************************************
 * Base object class
 */

class AObject
{
public:
  AObject() {};
  virtual ~AObject() {};
  virtual void deleteInternals() {};
  virtual int equals( AObject *object ) { return FALSE; }
};

struct BTime
{
  int year, month, day;
  int hour, min, sec, msec;
};

/******************************************************************************
 * Vector
 * The vector class is used extensively in this windows framework
 * The implementation is very basic, and there is probably a good standard
 * class available for this, but since I'm not sure that the C++ class libraries
 * are compatible with each other I have created this one.
 */

template<class T>
class Vector : public AObject
{
  int size;
  void resize( int toSize );
  int deleteContents;
  T **content;
public:
  Vector() : size(0), deleteContents(TRUE), content(NULL) {};
  Vector( const int deleteContents ) : 
		size(0), deleteContents(deleteContents), content(NULL) {};
  void deleteContent( int deleteContents )
    { this->deleteContents = deleteContents; }
  void add( T *object );
  BOOL insertAt( int index, T *object );
  BOOL removeAt( int index );
  int findObject( T *object );
  T *findObjectOnEq( T *object );
  BOOL removeObject( T *object );
  int getSize() { return size; }
  T *getObjectAt( int index );
  void removeAll();
  virtual ~Vector()
  {
    if( deleteContents )
      removeAll();
		else
		if( content != NULL )
			free( content );
  }
};

template<class T>
void Vector<T>::resize( int toSize )
{
/*  if( size == 0 )
    content = (T **)malloc( sizeof( T * ) * toSize);
  else*/
  content = (T **)realloc( (void *)content, sizeof( T * ) * toSize );
};

template<class T>
void Vector<T>::add( T *object )
{
  resize( size + 1 );
  content[size++] = object;
};

template<class T>
BOOL Vector<T>::insertAt( int index, T *object )
{
  if( index > size ) return FALSE;

  resize( size + 1);

  memmove( &content[index+1], &content[index],
          sizeof( T *) * (size - index) );

  content[index] = object;

  size++;

  return TRUE;
};

template<class T>
BOOL Vector<T>::removeAt( int index )
{
  if( index >= size ) return FALSE;

  if( deleteContents )
    delete content[index];

  memmove( &content[index], &content[index+1],
          sizeof( T *) * (size - index - 1) );

  size--;

  return TRUE;
};

template<class T>
T *Vector<T>::getObjectAt( int index )
{
  if( index >= size ) return NULL;

  return content[index];
};

template<class T>
int Vector<T>::findObject( T *object )
{
  for( int i = 0; i < size; i++ )
    if( content[i] == object )
      return i;
  return -1;
};

template<class T>
T *Vector<T>::findObjectOnEq( T *object )
{
  for( int i = 0; i < size; i++ )
    if( content[i]->equals( (AObject *)object ) )
      return content[i];
  return NULL;
};

template<class T>
BOOL Vector<T>::removeObject( T *object )
{
  int i;
  if( (i = findObject( object )) != -1 )
    return removeAt(i);
  return FALSE;
};

template<class T>
void Vector<T>::removeAll()
{
  for( int i = size-1; i >= 0; i-- )
  {
    size--;
    delete content[i];
  }

  if( content != NULL )
	{
    free( content );
		content = NULL;
	}

  size = 0;
};
