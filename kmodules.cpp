#include "kerrcoll.h"
#include "kcommon.h"
#include "kmodules.h"
#include "globals.h"

TModule **KModules::modules = NULL;
int KModules::modulesCount = 0;
BOOL KModules::newModules = FALSE;

BOOL KModules::addModule( char *filename, ModuleLoadType loadType )
{
  // Remove the old one if it exists
  removeModule( filename );
  modules = (TModule **)realloc( modules, (modulesCount+1) * sizeof( TModule * ));
  modules[modulesCount] = new TModule( filename, FILESTREAM, 
                                       KErrorCollector::getInstance(), (BOOL)loadType  );
  if( modules[modulesCount]->success() )
  {
    if( loadType == preload ) newModules = TRUE;
    modulesCount++;
    // Check if there is an init function
    int idx = FunctionList.FindIndex("initModule");

    if( idx != -1 )
    {
      // Check if the function is in this module
      if( FunctionList.getFunction(idx)->Module == modules[modulesCount-1] )
      {
        // Execute it
		    KCommonCommands::getInstance()->runCommand( "initModule();" );
      }
    }
    return TRUE;
  }
  else
  {
    KErrorCollector::getInstance()->notice( Strings::get(50) );
    delete modules[modulesCount];
  }
  return FALSE;
};

BOOL KModules::removeModule( char *filename )
{
  for( int i = 0; i < modulesCount; i++ )
  {
    if( !strcmp(modules[i]->getFilename(), filename ) )
    {
      memmove( modules[i], modules[i+1], modulesCount - i -1 );
      modulesCount--;
      return TRUE;
    }
  }
  return FALSE;
}

TModule **KModules::getModules()
{
  return modules;
}

void KModules::saveModulesList()
{
  if( !newModules ) return;
  char filename[MAXPATH+1];
  sprintf( filename, "%s%s", KonSystem::kDir, "modules.k" );
  FILE *f = fopen( filename, "w" );
  fprintf( f, "// Warning! This file is overwritten when Kon exits if new modules\n// has been added from Kon!\n\n" );
  fprintf( f, "loadModules()\n{\n" );

  for( int i = 0; i < modulesCount; i++ )
  {
    if( modules[i]->getPreload() )
    {
      char *fixedFilename = KCommonCommands::getInstance()->fixString( modules[i]->getFilename() );

      fprintf( f, "  loadModule( \"%s\", 2 );\n", fixedFilename );

      if( fixedFilename != modules[i]->getFilename() ) delete fixedFilename;
    }
  }

  fprintf( f, "}\n" );
  fclose( f );
}
