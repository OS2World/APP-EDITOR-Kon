// Utility functions

// swap, sort3 and median3 are used by the quicksort function (qsort)

swap( &a, &b )
{
  m = a;
  a = b;
  b = m;
}

sort3( &array, Start, End )
{
  Middle = Start + (End-Start)/2;

  if( array[Start] > array[Middle] )
    swap( array[Start], array[Middle] );

  if( array[Start] > array[End-1] )
    swap( array[Start], array[End-1] );

  if( array[Middle] > array[End-1] )
    swap( array[Middle], array[End-1] );

  return Middle;
}

median3( &array, Start, End )
{
  Middle = sort3( array, Start, End );

  Pivot = array[Middle];

  swap( array[Middle], array[End-1] );

  return Pivot;
}

// **************************************************************
// qsort does a quicksort on an array.
// Start: The index of the first element to sort
// End:   The index of the last element + 1

qsort( &array, Start, End )
{
  if( End-Start <= 3 )
  {
    if( End-Start == 2 )
    {
      if( array[Start] > array[End-1] )
        swap( array[Start], array[End-1] );
    }
    else
    if( End-Start == 3 )
      sort3( array, Start, End );
    return;
  }

  pivot = median3( array, Start, End );

  i = Start;
  j = End - 1;

  while( 1 )
  {
    while( array[++i] < pivot );

    while( array[--j] > pivot );

    if( j <= i ) break;

    swap( array[i], array[j] );
  }

  swap( array[i], array[End-1] );

  qsort( array, Start, i );
  qsort( array, i+1, End );
}

// Place the escape characters ('\') in front of characters that otherwise
// would be specially interpreted.

escapeChars( s )
{
  s2 = "";

  for( i = 0; i < length(s); i++ )
  {
    sChar = charToString(s[i]);
    if( sChar == "\\" || sChar == "\"" )
    {
      s2 = s2 @ "\\" @ sChar;
    }
    else
      s2 = s2 @ sChar;
  }

  return s2;
}