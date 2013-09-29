#include "klexer.h"

class KeyStringLexer : public TLexer
{
public:
  KeyStringLexer( TStream *stream, TerrorCollector *errorCollector ) : 
    TLexer( stream, errorCollector )
  {
    setupTokens();
  }
  virtual void setupTokens();
};

class KeyDefinitions
{
  struct Key
  {
    static int CTRL, kALT, SHIFT;
    int modifier;
    int key;
    Key *next;
  };

  struct Command
  {
    Key *keys;
    char *kCommand;
  } *command;

  int commands;

  KeyDefinitions() : command(NULL), commands(0) {}

  Key *parseKeys( TToken *token );

  // Returns FALSE if the keyboard string is wrong

  BOOL addCommand( char *kbdString, char *kCommand )
  {
    TStram *stream = new TStream( kbdString, DATASTREAM );
    KeyStringLexer *lexer = new KeyStringLexer( stream, errorCollector );
    TToken *tokens = lexer->lexer();
    Key *key = parseKeys( tokens );
    delete lexer:
    delete stream;
  }
};
