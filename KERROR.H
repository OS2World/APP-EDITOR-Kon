#if !defined KERR

class TErrorCollector
{
protected:
  int errors;
public:
  int GetErrorCount() { return errors; };
  TErrorCollector()  : errors(0) {};
  virtual void clearErrorCount()
  {
    errors = 0;
  }
  virtual void Error( int column, int line, char *str )
  {
    printf( "(%d:%d) %s\n", column, line, str ); errors++;
  };
  virtual void Error( char *file, int column, int line, char *str )
  {
    printf( "%s (%d:%d) %s\n", file, column, line, str ); errors++;
  };
  virtual void notice( char *note )
  {
    printf( "%s\n", note );
  }
};

#define KERR

#endif
