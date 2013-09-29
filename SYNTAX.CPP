#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "syntax.h"

ColorValue SyntaxContainer::currentFg = Color::DEFAULT;
ColorValue SyntaxContainer::currentBg = Color::DEFAULT;
int SyntaxContainer::currentFontAttr = 0;
BOOL SyntaxContainer::delimEnd = FALSE;

Vector<Syntax> SyntaxContainer::syntaxes;
Vector<ColorObject> SyntaxContainer::colors;

// Sort helper functions

#if defined __BORLANDC__
int wordCmp( const void *word1, const void *word2 )
#elif defined _MSC_VER
int wordCmp( const void *word1, const void *word2 )
#else
int _Optlink wordCmp( const void *word1, const void *word2 )
#endif
{
  return strcmp( ((AWord *)word1)->string, ((AWord *)word2)->string );
}

#if defined __BORLANDC__
int wordCmpi( const void *word1, const void *word2 )
#elif defined _MSC_VER
int wordCmpi( const void *word1, const void *word2 )
#else
int _Optlink wordCmpi( const void *word1, const void *word2 )
#endif
{
  return strcmpi( ((AWord *)word1)->string, ((AWord *)word2)->string );
}

#if defined __BORLANDC__
int pairCmp( const void *pair1, const void *pair2 )
#elif defined _MSC_VER
int pairCmp( const void *pair1, const void *pair2 )
#else
int _Optlink pairCmp( const void *pair1, const void *pair2 )
#endif
{
  return strcmp( ((Pair *)pair1)->openp, ((Pair *)pair2)->openp );
}

BOOL Syntax::wrapUp()
{
  if( name == NULL ) return FALSE;

  // Sort all lists
  if( keyWord )
    qsort( keyWord, keyWords, sizeof( AWord ), (caseInsensitive) ? wordCmpi :  wordCmp );
  if( eolWord )
    qsort( eolWord, eolWords, sizeof( AWord ), (caseInsensitive) ? wordCmpi :  wordCmp );
  if( previousWord )
    qsort( previousWord, previousWords, sizeof( AWord ), (caseInsensitive) ? wordCmpi :  wordCmp );
  if( eolComment )
    qsort( eolComment, eolComments, sizeof( AWord ), wordCmp );
  if( pair )
    qsort( pair, pairs, sizeof( Pair ), pairCmp );
  if( multilinePair )
    qsort( multilinePair, multilinePairs, sizeof( Pair ), pairCmp );
  if( multilineComment )
    qsort( multilineComment, multilineComments, sizeof( Pair ), pairCmp );
  if( noDelim )
    qsort( noDelim, noDelims, sizeof( AWord ), (caseInsensitive) ? wordCmpi :  wordCmp );
  if( grep )
    qsort( grep, greps, sizeof( AWord ), (caseInsensitive) ? wordCmpi :  wordCmp );

  int i;
  for( i = 0; i < eolWords; i++ )
    if( strlen( eolWord[i].string ) > longestEolComment )
      longestEolComment = strlen( eolWord[i].string );

  for( i = 0; i < multilineComments; i++ )
  {
    if( strlen( multilineComment[i].openp ) > longestMultiline )
      longestMultiline = strlen( multilineComment[i].openp );
    if( strlen( multilineComment[i].closep ) > longestMultiline )
      longestMultiline = strlen( multilineComment[i].closep );
  }

  if( delimitors == 0 )
  {
    usesStandardDelim = TRUE;
    delimitor = Settings::standardDelim;
    delimitors = strlen( Settings::standardDelim );
  }

  return TRUE;
}

Syntax::~Syntax()
{
  if( delimitor && !usesStandardDelim )
    free( delimitor );
	
  int i;
	for( i = 0; i < keyWords; i++ )
		delete keyWord[i].string;

  if( keyWord )
    free( keyWord );

	for( i = 0; i < previousWords; i++ )
		delete previousWord[i].string;

  if( previousWord )
    free( previousWord );

	for( i = 0; i < eolWords; i++ )
		delete eolWord[i].string;

  if( eolWord )
    free( eolWord );

	for( i = 0; i < eolComments; i++ )
		delete eolComment[i].string;

  if( eolComment )
    free( eolComment );

	for( i = 0; i < noDelims; i++ )
		delete noDelim[i].string;

	if( noDelim )
		free( noDelim );

	for( i = 0; i < greps; i++ )
		delete grep[i].string;

	if( grep )
		free( grep );

//	for( i = 0; i < pairs; i++ )
//		delete pair[i];

  if( pair )
    delete pair;

  if( multilinePair )
    delete multilinePair;

  if( multilineComment )
    delete multilineComment;
  for( i = 0; i < extensions; i++ )
    delete extensionList[i];
  if( extensionList )
    free( extensionList );

  for( i = 0; i < sections; i++ )
    delete sectionList[i];
  if( sectionList )
    free( sectionList );

  if( name )
    delete name;
}

class SyntaxLexer : public TBLexer
{
public:
  SyntaxLexer( TStream *stream, TErrorCollector *errorCollector ) : 
    TBLexer( stream, errorCollector, 0 )
  {
    setupTokens();
  }
  virtual void setupTokens();
};

void SyntaxLexer::setupTokens()
{
  char *orgSymbols = "=";
  symbols = new char[strlen(orgSymbols)+1];
  strcpy( symbols, orgSymbols );
  char *orgSpecials = ",:";
  specials = new char[strlen(orgSpecials )+1];
  strcpy( specials, orgSpecials );
  char *orgWords[23] = { "extensions", "define", "color", "caseinsensitive",
                         "set", "foreground", "background", "fontattribute",
                         "keyword", "eolcomment", "previousword", "pair",
                         "multilinepair", "multilinecomment", "delimitors",
                         "literal", "eolword", "name", "requiresDelimFront",
												 "requiresDelimEnd", "grep", "requiresDelim",
                         "sections" };


  wordsCount = 23;
  words = (char **)malloc( sizeof( char * ) * wordsCount );
  for( int i = 0; i < wordsCount; i++ )
  {
    words[i] = new char[strlen( orgWords[i] ) + 1];
    strcpy( words[i], orgWords[i] );
  }
}

BOOL SyntaxContainer::retrieveList( TToken **token, int &items,
                                    char ***itemList )
{
  while( *token )
  {
    // Add to list
    items++;
    *itemList = (char **)realloc( *itemList, sizeof( char * ) * items );
    (*itemList)[items-1] = new char[strlen((*token)->string)+1];
    strcpy( (*itemList)[items-1], (*token)->string );
    *token = (*token)->next;
    if( *token == NULL || strcmp( (*token)->string, "," ) )
      break;
    *token = (*token)->next;
  }
  return FALSE;
}

BOOL SyntaxContainer::getWordList( TToken **token, int &words, AWord **wordList )
{
  while( *token )
  {
    // Add to list
    words++;
    *wordList = (AWord *)realloc( *wordList, sizeof( AWord ) * words );
    (*wordList)[words-1].string = new char[strlen((*token)->string)+1];
    strcpy( (*wordList)[words-1].string, (*token)->string );
    (*wordList)[words-1].fg = currentFg;
    (*wordList)[words-1].bg = currentBg;
    (*wordList)[words-1].fontAttr = currentFontAttr;
    (*wordList)[words-1].len = strlen((*token)->string);
    (*wordList)[words-1].delimEnd = delimEnd;
    *token = (*token)->next;
    if( *token == NULL || strcmp( (*token)->string, "," ) )
      break;
    *token = (*token)->next;
  }
  return FALSE;
}

BOOL SyntaxContainer::getPairList( TToken **token, int &pairs, Pair **pairList )
{
  while( *token )
  {
    // Add to list
    pairs++;
    TToken *prevToken = *token;
    *pairList = (Pair *)realloc( *pairList, sizeof( Pair ) * pairs );
    (*pairList)[pairs-1].openp = new char[strlen((*token)->string)+1];
    strcpy( (*pairList)[pairs-1].openp, (*token)->string );
    (*pairList)[pairs-1].fg = currentFg;
    (*pairList)[pairs-1].bg = currentBg;
    (*pairList)[pairs-1].fontAttr = currentFontAttr;

    *token = (*token)->next;

    if( *token == NULL || strcmp( (*token)->string, ":" ) )
    {
      displayError( Strings::get(29), prevToken->line );
      return TRUE;
    }

    *token = (*token)->next;

    if( *token == NULL )
    {
      displayError( Strings::get(30), prevToken->line );
      return TRUE;
    }

    (*pairList)[pairs-1].closep = new char[strlen((*token)->string)+1];
    strcpy( (*pairList)[pairs-1].closep, (*token)->string );

    *token = (*token)->next;
    if( *token == NULL || strcmp( (*token)->string, "," ) )
      break;
    *token = (*token)->next;
  }
  return FALSE;
}

BOOL SyntaxContainer::getRGBvalue( TToken **token, ColorValue &colorValue )
{
  int values[3];
  for( int i = 0; i < 3; i++ )
  {
    if( *token && (*token)->type == lexNum )
    {
      values[i] = atoi( (*token)->string );
      *token = (*token)->next;
      if( i < 2 && (!*token || strcmp( (*token)->string, "," )) )
      {
        displayError( Strings::get(19), (*token)->line );
        return FALSE;
      }
      else
      if( *token && i < 2 )
        *token = (*token)->next;
    }
    else
    {
      if( *token )
        displayError( Strings::get(18), (*token)->string, (*token)->line );
      else
        displayError( Strings::get(25), 0 );
      return FALSE;
    }
  }
  colorValue = Color::createColor( values[0], values[1], values[2] );
  return TRUE;
}

void SyntaxContainer::insertSyntax( Syntax *syntax )
{
  int i = 0;
  while( i < syntaxes.getSize() )
  {
    if( strcmp( syntaxes.getObjectAt(i)->name, syntax->name ) >= 0 ) break;
    i++;
  }
  syntaxes.insertAt( i, syntax );
}

void SyntaxContainer::loadSyntaxes( char *filename )
{
  // Insert the background value

  colors.add( new ColorObject( "default", Color::DEFAULT ) );

  TStream *stream = new TStream( filename, FILESTREAM );
  SyntaxLexer *lexer = new SyntaxLexer( stream, KErrorCollector::getInstance() );

  TToken *token = lexer->lexer();

  Syntax *currentSyntax = NULL;

  // Parse the tokens

  BOOL error = FALSE;

  while( token && !error )
  {
    switch( token->type )
    {
      case lexWord :
      {
        if( !strcmp( token->string, "name" ) )
        {
          // Start a new syntax definition
          if( currentSyntax != NULL )
          {
            if( currentSyntax->wrapUp() )
              insertSyntax( currentSyntax );
            else
            {
              displayError( Strings::get(60), "", 0 );
              delete currentSyntax;
              error = TRUE;
              break;
            }
          }

          currentSyntax = new Syntax();

          // Next token should be an '='

          TToken *prevToken = token;
          token = token->next;
          if( !strcmp( token->string, "=" ) )
          {
            token = token->next;
            // Retrieve name
            if( token->type == lexString )
            {
              currentSyntax->name = new char[strlen(token->string)+1];
              strcpy( currentSyntax->name, token->string );
              token = token->next;
            }
            else
            {
              displayError( Strings::get(59), "name", prevToken->line );
              error = TRUE;
            }
          }
          else
          {
            displayError( Strings::get(14), "name", prevToken->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "extensions" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          // Next token should be an '='

          TToken *prevToken = token;
          token = token->next;
          if( !strcmp( token->string, "=" ) )
          {
						delimEnd = FALSE;
            token = token->next;
            // Retrieve all extensions

            error = retrieveList( &token, currentSyntax->extensions,
                                  &currentSyntax->extensionList );
          }
          else
          {
            displayError( Strings::get(14), "extensions", prevToken->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "sections" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          // Next token should be an '='

          TToken *prevToken = token;
          token = token->next;
          if( !strcmp( token->string, "=" ) )
          {
						delimEnd = FALSE;
            token = token->next;

            // Retrieve all extensions
            while( token )
            {
              currentSyntax->sectionList = (SectionDef **)realloc(currentSyntax->sectionList, 
                                                               (currentSyntax->sections+1) * sizeof( SectionDef *) );
              char *regexp = token->string;

              prevToken = token;
              token = token->next;
              if( token == NULL || strcmp( token->string, ","))
              {
                displayError( Strings::get(19), "sections", prevToken->line );
                error = TRUE;
                break;
              }

              prevToken = token;
              token = token->next;
              if( token == NULL )
              {
                displayError( Strings::get(251), "sections", prevToken->line );
                error = TRUE;
                break;
              }
              
              int paren = atoi(token->string);
              currentSyntax->sectionList[currentSyntax->sections] = new SectionDef( regexp, paren );
              currentSyntax->sections++;

              token = token->next;
              if( token == NULL || strcmp( token->string, ",")) break;
              prevToken = token;
              token = token->next;
            }
          }
          else
          {
            displayError( Strings::get(14), "sections", prevToken->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "define" ) )
        {
          token = token->next;
          if( token && !strcmp( token->string, "color" ) )
          {
            token = token->next;

            if( token && token->type == lexId )
            {
              char *color = token->string;
              token = token->next;

              if( token && !strcmp( token->string, "=" ) )
              {
                // Get RGB values
                token = token->next;
                ColorValue colorValue;
                if( getRGBvalue( &token, colorValue ) )
                {
                  // Create a new color
                  colors.add( new ColorObject( color, colorValue ) );
                }
                else
                {
                  error = TRUE;
                  break;
                }
              }
              else
              {
                if( token )
                  displayError( Strings::get(14), "define", token->line );
                else
                  displayError( Strings::get(25), 0 );
                error = TRUE;
              }
            }
            else
            {
              if( token )
                displayError( Strings::get(17), token->string, token->line );
              else
                displayError( Strings::get(25), 0 );
              error = TRUE;
            }
          }
          else
          {
            if( token )
              displayError( Strings::get(16), token->string, token->line );
            else
              displayError( Strings::get(25), 0 );
            error = TRUE;
          }
        }
        else
        if( currentSyntax == NULL )
        {
          displayError( Strings::get(20), token->line );
          error = TRUE;
        }
        else
        if( !strcmp( token->string, "set" ) )
        {
          token = token->next;
          if( token && !strcmp( token->string, "caseinsensitive" ) )
          {
            if( token->next && !strcmp( token->next->string, "=" ) )
            {
              token = token->next->next;
              if( token )
              {
                if( !strcmp( token->string, "yes" ) )
                  currentSyntax->caseInsensitive = TRUE;
                else
                if( !strcmp( token->string, "no" ) )
                  currentSyntax->caseInsensitive = FALSE;
                else
                {
                  displayError( Strings::get(21), token->line );
                  error = TRUE;
                }
                token = token->next;
              }
              else
              {
                displayError( Strings::get(25), 0 );
                error = TRUE;
              }
            }
            else
            {
              displayError( Strings::get(14), "set", token->line );
              error = TRUE;
            }
          }
					else
          if( token && !strcmp( token->string, "requiresDelimFront" ) )
          {
            if( token->next && !strcmp( token->next->string, "=" ) )
            {
              token = token->next->next;
              if( token )
              {
                if( !strcmp( token->string, "yes" ) )
                  currentSyntax->requiresDelim = TRUE;
                else
                if( !strcmp( token->string, "no" ) )
                  currentSyntax->requiresDelim = FALSE;
                else
                {
                  displayError( Strings::get(21), token->line );
                  error = TRUE;
                }
                token = token->next;
              }
              else
              {
                displayError( Strings::get(25), 0 );
                error = TRUE;
              }
            }
					}
					else
          if( token && (!strcmp( token->string, "requiresDelimEnd" ) ||
                        !strcmp( token->string, "requiresDelim" )) )
          {
            if( token->next && !strcmp( token->next->string, "=" ) )
            {
              token = token->next->next;
              if( token )
              {
                if( !strcmp( token->string, "yes" ) )
                  delimEnd = TRUE;
                else
                if( !strcmp( token->string, "no" ) )
									delimEnd = FALSE;
                else
                {
                  displayError( Strings::get(21), token->line );
                  error = TRUE;
                }
                token = token->next;
              }
              else
              {
                displayError( Strings::get(25), 0 );
                error = TRUE;
              }
            }
            else
            {
              displayError( Strings::get(14), "set", token->line );
              error = TRUE;
            }
          }
          else
          if( token && (!strcmp( token->string, "foreground" ) ||
                        !strcmp( token->string, "background" )) )
          {
            BOOL foreground = !strcmp( token->string, "foreground" );
            ColorValue colorValue;

            token = token->next;
            if( token && !strcmp( token->string, "=" ) )
            {
              token = token->next;

              if( token->type == lexNum )
              {
                // RGB value
                if( !getRGBvalue( &token, colorValue ) )
                {
                  error = TRUE;
                  break;
                }
              }
              else
              {
                // Find the color in the color list
                int i;
                for( i = 0; i < colors.getSize(); i++ )
                  if( !strcmp( colors.getObjectAt(i)->name, token->string ) )
                    break;

                if( i < colors.getSize() )
                  colorValue = colors.getObjectAt(i)->color;
                else
                {
                  displayError( Strings::get(24), token->string, token->line );
                  error = TRUE;
                  break;
                }
                token = token->next;
              }

              if( foreground )
                currentFg = colorValue;
              else
                currentBg = colorValue;
            }
            else
            {
              displayError( Strings::get(14), "set", token->line );
              error = TRUE;
            }
          }
          else
          if( token && !strcmp( token->string, "fontattribute" ) )
          {
            if( token->next && !strcmp( token->next->string, "=" ) )
            {
              char **fontAttribute = NULL;
              int fontAttributes = 0;

              TToken *prevToken = token = token->next->next;
              error = retrieveList( &token, fontAttributes, &fontAttribute );
              currentFontAttr = 0;
              if( !error )
                for( int i = 0; i < fontAttributes; i++ )
                {
                  if( !strcmp( fontAttribute[i], "bold" ) )
                    currentFontAttr |= Font::BOLD;
                  else
                  if( !strcmp( fontAttribute[i], "italic" ) )
                    currentFontAttr |= Font::ITALIC;
                  else
                  if( !strcmp( fontAttribute[i], "underscore" ) )
                    currentFontAttr |= Font::UNDERSCORE;
                  else
                  if( !strcmp( fontAttribute[i], "normal" ) )
                    currentFontAttr = 0;
                  else
                  {
                    displayError( Strings::get(26), fontAttribute[i],
                                  prevToken->line );
                    error = TRUE;
                    break;
                  }
                }
              for( int i =  0; i < fontAttributes; i++ )
                delete fontAttribute[i];
              free( fontAttribute );
            }
            else
            {
              displayError( Strings::get(14), "set", token->line );
              error = TRUE;
            }
          }
          else
          {
            if( token )
              displayError( Strings::get(22), token->line );
            else
              displayError( Strings::get(25), 0 );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "literal" ) )
        {
          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            if( currentSyntax == NULL )
            {
              displayError( Strings::get(61), "", 0 );
              error = TRUE;
              break;
            }

            token = token->next->next;
            if( token && strlen( token->string ) == 1 )
            {
              currentSyntax->literalChar = token->string[0];
              token = token->next;
            }
            else
            {
              if( token )
                displayError( Strings::get(27), token->line );
              else
                displayError( Strings::get(25), 0 );
              error = TRUE;
            }
          }
          else
          {
            displayError( Strings::get(14), "literal", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "delimitors" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            if( token && token->type == lexString )
            {
              if( currentSyntax->delimitor )
                delete currentSyntax->delimitor;
              currentSyntax->delimitor = new char[strlen( token->string ) + 3];
              strcpy( currentSyntax->delimitor, token->string );
              strcat( currentSyntax->delimitor, " \t" );
              currentSyntax->delimitors = strlen( currentSyntax->delimitor );
              token = token->next;
            }
            else
            {
              if( token )
                displayError( Strings::get(28), token->line );
              else
                displayError( Strings::get(25), 0 );
              error = TRUE;
            }
          }
          else
          {
            displayError( Strings::get(14), "delimitors", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "keyword" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
						if( currentSyntax->requiresDelim )
							error = getWordList( &token, currentSyntax->keyWords,
								                   &currentSyntax->keyWord );
						else
							error = getWordList( &token, currentSyntax->noDelims,
								                   &currentSyntax->noDelim );
          }
          else
          {
            displayError( Strings::get(14), "keyword", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "grep" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
						error = getWordList( &token, currentSyntax->greps,
								                  &currentSyntax->grep );
          }
          else
          {
            displayError( Strings::get(14), "grep", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "eolword" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            error = getWordList( &token, currentSyntax->eolWords,
                                 &currentSyntax->eolWord );

          }
          else
          {
            displayError( Strings::get(14), "eolword", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "eolcomment" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            error = getWordList( &token, currentSyntax->eolComments,
                                 &currentSyntax->eolComment );

          }
          else
          {
            displayError( Strings::get(14), "eolcomment", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "previousword" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            error = getWordList( &token, currentSyntax->previousWords,
                                 &currentSyntax->previousWord);

          }
          else
          {
            displayError( Strings::get(14), "previousword", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "pair" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            error = getPairList( &token, currentSyntax->pairs,
                                 &currentSyntax->pair );

          }
          else
          {
            displayError( Strings::get(14), "pair", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "multilinepair" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            error = getPairList( &token, currentSyntax->multilinePairs,
                                 &currentSyntax->multilinePair );

          }
          else
          {
            displayError( Strings::get(14), "multilinepair", token->line );
            error = TRUE;
          }
        }
        else
        if( !strcmp( token->string, "multilinecomment" ) )
        {
          if( currentSyntax == NULL )
          {
            displayError( Strings::get(61), "", 0 );
            error = TRUE;
            break;
          }

          if( token->next && !strcmp( token->next->string, "=" ) )
          {
            token = token->next->next;
            error = getPairList( &token, currentSyntax->multilineComments,
                                 &currentSyntax->multilineComment );

          }
          else
          {
            displayError( Strings::get(14), "multilinecomment", token->line );
            error = TRUE;
          }
        }
        else
        {
          displayError( Strings::get(15), token->string, token->line );
          error = TRUE;
        }
        break;
      }
      default :
      {
        displayError( Strings::get(15), token->string, token->line );
        error = TRUE;
        break;
      }
    }
    if( error && currentSyntax )
      delete currentSyntax;
  }

  if( currentSyntax && !error )
  {
    if( currentSyntax->wrapUp() )
    {
      insertSyntax( currentSyntax );
    }
    else
    {
      displayError( Strings::get(60), "", 0 );
      delete currentSyntax;
    }
  }

  delete stream;
  delete lexer;
}

Syntax *SyntaxContainer::getSyntax( char *name )
{
  for( int s = 0; s < syntaxes.getSize(); s++ )
    if( !strcmp(syntaxes.getObjectAt(s)->name, name) ) return syntaxes.getObjectAt(s);
  return NULL;
}

Syntax *SyntaxContainer::retrieveSyntax( char *filename )
{
  char *ext = strrchr( filename, '.' );
  if( ext == NULL ) return NULL;

  for( int s = 0; s < syntaxes.getSize(); s++ )
  {
    for( int e = 0; e < syntaxes.getObjectAt(s)->extensions; e++ )
      if( !stricmp( ext, syntaxes.getObjectAt(s)->extensionList[e] ) )
        return syntaxes.getObjectAt(s);
  }
  return NULL;
}

