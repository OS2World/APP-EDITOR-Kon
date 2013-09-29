#if !defined KMODULES

#include "k.h"

enum ModuleLoadType { noPrelopad = 0, preload = 1, startupLoad = 2 };

// Holds loadable K-modules
class KModules
{
  static TModule **modules;
  static int modulesCount;
  static BOOL newModules;
public:
  static BOOL addModule( char *filename, ModuleLoadType loadType );
  static BOOL removeModule( char *filename );
  static TModule **getModules();
  static int getModulesCount() { return modulesCount;}
  static void saveModulesList();
};
#define KMODULES
#endif