class ClipBoard
{
public:
  static void setText( char *text, int length );

  // Returns the text length. The text-elem is allocated with the needed size

  static int getText( char **text );
};