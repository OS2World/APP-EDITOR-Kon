#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bowl.h"
#include "keybind.h"

KeyBindings *KeyBindings::instance = NULL;

void KeyStringLexer::setupTokens()
{
  char *orgSymbols = "+-";
  symbols = new char[strlen(orgSymbols)+1];
  strcpy( symbols, orgSymbols );
  char *orgSpecials = "";
  specials = new char[strlen(orgSpecials )+1];
  strcpy( specials, orgSpecials );
  char *orgWords[31] = { "Ctrl", "Alt", "Shift", "Left", "Right", "Up", "Down",
                        "PageUp", "PageDown", "Home", "End", "Insert", "Delete",
                        "Escape", "Enter", "Backspace", "F1", "F2", "F3", "F4",
                        "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
                        "Return", "Tab", "Space" };

  wordsCount = 31;
  words = (char **)malloc( sizeof( char * ) * wordsCount );
  for( int i = 0; i < wordsCount; i++ )
  {
    words[i] = new char[strlen( orgWords[i] ) + 1];
    strcpy( words[i], orgWords[i] );
  }
}

Key *KeyBindings::parseKeys( TToken *token )
{
  int modifiers = 0;
  int key = 0;
  Key *keyList = NULL, *lastKey = NULL;
  BOOL inKey = FALSE;

  while( token )
  {
    if( token->type == lexWord )
    {
      int lastModifers = modifiers;

      if( !strcmp( token->string, "Ctrl" ) )
        modifiers |= KeyDef::CTRL;
      else
      if( !strcmp( token->string, "Alt" ) )
        modifiers |= KeyDef::kALT;
      else
      if( !strcmp( token->string, "Shift" ) )
        modifiers |= KeyDef::SHIFT;

      if( lastModifers != modifiers )
      {
        token = token->next;
        if( token->type == lexSymbol && *token->string == '+' )
          token = token->next;
        else
          return NULL;
        inKey = TRUE;
      }
      else
      {
        if( !strcmp( token->string, "Left" ))
          key = KeyDef::LEFT;
        else
        if( !strcmp( token->string, "Right" ) )
          key = KeyDef::RIGHT;
        else
        if( !strcmp( token->string, "Up" ) )
          key = KeyDef::UP;
        else
        if( !strcmp( token->string, "Down" ) )
          key = KeyDef::DOWN;
        else
        if( !strcmp( token->string, "PageUp" ) )
          key = KeyDef::PAGEUP;
        else
        if( !strcmp( token->string, "PageDown" ) )
          key = KeyDef::PAGEDOWN;
        else
        if( !strcmp( token->string, "Home" ) )
          key = KeyDef::HOME;
        else
        if( !strcmp( token->string, "End" ) )
          key = KeyDef::END;
        else
        if( !strcmp( token->string, "Insert" ) )
          key = KeyDef::INSERT;
        else
        if( !strcmp( token->string, "Delete" ) )
          key = KeyDef::kDELETE;
        else
        if( !strcmp( token->string, "Escape" ) )
          key = KeyDef::ESCAPE;
        else
        if( !strcmp( token->string, "Enter" ) )
          key = KeyDef::ENTER;
        else
        if( !strcmp( token->string, "Return" ) )
          key = KeyDef::RETURN;
        else
        if( !strcmp( token->string, "Space" ) )
          key = KeyDef::SPACE;
        else
        if( !strcmp( token->string, "Backspace" ) )
          key = KeyDef::BACKSPACE;
        else
        if( !strcmp( token->string, "Tab" ) )
          key = KeyDef::TAB;
        else
        if( !strcmp( token->string, "F1" ) )
          key = KeyDef::F1;
        else
        if( !strcmp( token->string, "F2" ) )
          key = KeyDef::F2;
        else
        if( !strcmp( token->string, "F3" ) )
          key = KeyDef::F3;
        else
        if( !strcmp( token->string, "F4" ) )
          key = KeyDef::F4;
        else
        if( !strcmp( token->string, "F5" ) )
          key = KeyDef::F5;
        else
        if( !strcmp( token->string, "F6" ) )
          key = KeyDef::F6;
        else
        if( !strcmp( token->string, "F7" ) )
          key = KeyDef::F7;
        else
        if( !strcmp( token->string, "F8" ) )
          key = KeyDef::F8;
        else
        if( !strcmp( token->string, "F9" ) )
          key = KeyDef::F9;
        else
        if( !strcmp( token->string, "F10" ) )
          key = KeyDef::F10;
        else
        if( !strcmp( token->string, "F11" ) )
          key = KeyDef::F11;
        else
        if( !strcmp( token->string, "F11" ) )
          key = KeyDef::F12;

        inKey = FALSE;
        Key *newKey = new Key( modifiers, key );
        if( lastKey != NULL )
        {
          lastKey->next = newKey;
          lastKey = newKey;
        }
        else
          lastKey = keyList = newKey;
        key = 0;
        modifiers = 0;
        token = token->next;
      }
    }
    else
    if( token->type == lexId ||
        token->type == lexNum )
    {
      key = *token->string;
      token = token->next;
      inKey = FALSE;
      Key *newKey = new Key( modifiers, key );
      if( lastKey != NULL )
      {
        lastKey->next = newKey;
        lastKey = newKey;
      }
      else
        lastKey = keyList = newKey;
      key = 0;
      modifiers = 0;
    }
    else
    if( token->type == lexSymbol && *token->string == '-' && !inKey &&
        modifiers == 0 )
      token = token->next;
    else
      return NULL;
  }
  return keyList;
}

char *KeyBindings::lookupCommand( int modifier, int key )
{
  currentModifier |= modifier;

  if( key )
  {
/*    FILE *f = fopen( "key", "a" );
    fprintf( f, "%s-%s-%s %d\n", (modifier & KeyDef::CTRL) ? "CTRL" : "",
                                 (modifier & KeyDef::kALT) ? "ALT" : "",
                                 (modifier & KeyDef::SHIFT) ? "SHIFT" : "",
                                 key );
    fclose( f );*/
    Key *newKey = new Key( modifier, key );

    // Add key to current key sequence

    Key *lastKey = currentKeys;

    while( lastKey )
    {
      if( lastKey->next )
        lastKey = lastKey->next;
      else
        break;
    }

    if( lastKey )
      lastKey->next = newKey;
    else
      currentKeys = newKey;

    // Loop through all key definitions and see if anyone conforms to this

    BOOL somePassed = FALSE;

    for( int i = 0; i < commands; i++ )
    {
      Key *checkKey = command[i].keys;
      Key *userKey = currentKeys;
      BOOL firstLoop = TRUE;  // Don't check CTRL on second loop

      while( checkKey && userKey )
      {
        if( (checkKey->modifier == userKey->modifier ||
             (!firstLoop && (checkKey->modifier & ~KeyDef::CTRL) ==
                             userKey->modifier)) &&
            keyToUpper(checkKey->key) == keyToUpper(userKey->key) )
        {
          // If this is the last in the users key-sequence and
          // everything checks OK to this point, note that we may
          // accept further entries in this key sequence

          if( checkKey->next && userKey->next == NULL )
            somePassed = TRUE;
        }
        else
          break;

        checkKey = checkKey->next;
        userKey = userKey->next;
        firstLoop = FALSE;
      }

      if( !checkKey && !userKey )
      {
        // Command found
        delete currentKeys;
        currentKeys = NULL;
        return command[i].kCommand;
      }
    }
    if( !somePassed )
    {
      // None of the keys fitted
      delete currentKeys;
      currentKeys = NULL;
    }
    else
      return INBINDING;
  }

  // Do not send virtual keys as chars

  if( key > 255 ) return EMPTY;

  return NULL;
}

BOOL KeyBindings::queryKeyString( char *checkCmd, char *keyString )
{
  for( int i = 0; i < commands; i++ )
  {
    if( !strcmp( checkCmd, command[i].kCommand ) )
    {
      Key *key = command[i].keys;

      *keyString = 0;

      while( key )
      {
        if( key->modifier & KeyDef::CTRL )
          strcat( keyString, "Ctrl+" );

        if( key->modifier & KeyDef::SHIFT )
          strcat( keyString, "Shift+" );

        if( key->modifier & KeyDef::kALT )
          strcat( keyString, "Alt+" );

        if( key->key <= 255 )
        {
          char str[2] = {(char)key->key, 0};
          strcat( keyString, str );
        }
        else
        {
          if( key->key == KeyDef::UP )
            strcat( keyString, "Up" );
          else
          if( key->key == KeyDef::DOWN )
            strcat( keyString, "Down" );
          else
          if( key->key == KeyDef::PAGEUP )
            strcat( keyString, "PageUp" );
          else
          if( key->key == KeyDef::PAGEDOWN )
            strcat( keyString, "PageDown" );
          else
          if( key->key == KeyDef::INSERT )
            strcat( keyString, "Insert" );
          else
          if( key->key == KeyDef::kDELETE )
            strcat( keyString, "Delete" );
          else
          if( key->key == KeyDef::HOME )
            strcat( keyString, "Home" );
          else
          if( key->key == KeyDef::END )
            strcat( keyString, "End" );
          else
          if( key->key == KeyDef::ESCAPE )
            strcat( keyString, "ESC" );
          else
          if( key->key == KeyDef::F1 )
            strcat( keyString, "F1" );
          else
          if( key->key == KeyDef::F2 )
            strcat( keyString, "F2" );
          else
          if( key->key == KeyDef::F3 )
            strcat( keyString, "F3" );
          else
          if( key->key == KeyDef::F4 )
            strcat( keyString, "F4" );
          else
          if( key->key == KeyDef::F5 )
            strcat( keyString, "F5" );
          else
          if( key->key == KeyDef::F6 )
            strcat( keyString, "F6" );
          else
          if( key->key == KeyDef::F7 )
            strcat( keyString, "F7" );
          else
          if( key->key == KeyDef::F8 )
            strcat( keyString, "F8" );
          else
          if( key->key == KeyDef::F9 )
            strcat( keyString, "F9" );
          else
          if( key->key == KeyDef::F10 )
            strcat( keyString, "F10" );
          else
          if( key->key == KeyDef::F11 )
            strcat( keyString, "F11" );
          else
          if( key->key == KeyDef::F12 )
            strcat( keyString, "F12" );
          else
          if( key->key == KeyDef::ENTER )
            strcat( keyString, "Enter" );
          else
          if( key->key == KeyDef::BACKSPACE )
            strcat( keyString, "Backspace" );
          else
          if( key->key == KeyDef::LEFT )
            strcat( keyString, "Left" );
          else
          if( key->key == KeyDef::RIGHT )
            strcat( keyString, "Right" );
          else
          if( key->key == KeyDef::TAB )
            strcat( keyString, "Tab" );
          else
          if( key->key == KeyDef::RETURN )
            strcat( keyString, "Return" );
          else
          if( key->key == KeyDef::SPACE )
            strcat( keyString, "Space" );
        }

        key = key->next;

        if( key )
          strcat( keyString, "-" );
      }
      return TRUE;
    }
  }
  return FALSE;
}
