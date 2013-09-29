#if !defined KEYBIND

#include <ctype.h>
#include <stdlib.h>
#include "klexer.h"

#define INBINDING (char *)1L
#define EMPTY (char *)2L

/******************************************************************************
 * The KeyBindings class holds all key sequences and their kCommands
 * This class is global, but has no static methods. Instead it is referred to
 * using the getInstance() method.
 *
 * Key bindings are added using the addCommand method.
 */

class KeyStringLexer : public TBLexer
{
public:
  KeyStringLexer( TStream *stream, KErrorCollector *errorCollector ) : 
    TBLexer( stream, errorCollector, 0 )
  {
    setupTokens();
  }
  virtual void setupTokens();
};

struct Key
{
  int modifier;
  int key;
  Key *next;
  Key( int modifier, int key )
  {
    this->modifier = modifier;
    this->key = key;
    next = NULL;
  }
  ~Key()
  {
    if( next )
      delete next;
  }
};

class KeyBindings
{
  struct Command
  {
    Key *keys;
    char *kCommand;
    void setCommand( Key *keys, char *kCommand )
    {
      this->keys = keys;
      this->kCommand = new char[strlen(kCommand)+1];
      strcpy( this->kCommand, kCommand );
    }
  } *command;

  int commands;

  int currentModifier;
  Key *currentKeys;
  static KeyBindings *instance;
  Key *parseKeys( TToken *token );
  int keyToUpper( int key )
  {
    return (key <= 255) ? toupper( key ) : key;
  }
public:
  KeyBindings() : command(NULL), commands(0), currentModifier(0),
                  currentKeys(NULL) {}

  static void createInstance()
  {
    instance = new KeyBindings();
  }

  static KeyBindings *getInstance()
  {
    return instance;
  }

  // Returns FALSE if the keyboard string is wrong
  // The kbdString is constructed as follows:
  //
  // "Modifer-Modifer..-char+Modifer-..-char+.." for example "Ctrl-k+Ctrl-c"
  //
  // Modifers are Ctrl, Alt, Shift

  BOOL addCommand( char *kbdString, char *kCommand )
  {
    TStream *stream = new TStream( kbdString, DATASTREAM );
    KeyStringLexer *lexer = new KeyStringLexer( stream, KErrorCollector::getInstance() );
    TToken *tokens = lexer->lexer();
    Key *keys = parseKeys( tokens );

/*    Key *t = keys;

    FILE *f = fopen( "key", "a" );

    while( t )
    {
      if( t->modifier & KeyDef::kALT )
        fprintf( f, "Alt-" );

      if( t->modifier & KeyDef::CTRL )
        fprintf( f, "Ctrl-" );

      if( t->modifier & KeyDef::SHIFT )
        fprintf( f, "Shift-" );

      fprintf( f, "%d", t->key );

      t = t->next;

      if( t ) fprintf( f, "+" );
    }

    fprintf( f, "\n" );

    fclose( f );*/

    delete lexer;
    delete stream;

    if( keys )
    {
      commands++;
      command = (Command *)realloc( command, sizeof( Command ) * commands );
      command[commands-1].setCommand( keys, kCommand );
      return TRUE;
    }
    return FALSE;
  }

  // If a command has a shortcut, return the key-binding for this command
  // in keyString. keyString should be long enough to hold a key sequence

  BOOL queryKeyString( char *checkCmd, char *keyString );

  // Returns a command when a complete key sequence has been built up
  // The keyboard handler just shuffles the keys into this method, which
  // will return a command string when a recognized key sequence has been
  // built up.
  // If the key and modifier is not contained within a key string
  // NULL is returned. If it is contained in a key string INBINDING is returned

  char *lookupCommand( int modifier, int key );
};

#define KEYBIND

#endif
