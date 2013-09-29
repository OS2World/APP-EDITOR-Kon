#if !defined BOWL

#if defined __OS2__
#define INCL_PM
#define INCL_BASE
#define INCL_GPI
#define INCL_DOSMODULEMGR
#define INCL_DOSRESOURCES
#define INCL_ERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_WINCLIPBOARD
#define INCL_DOSNMPIPES
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_SPLERRORS

#include "os2.h"
#else
#include <windows.h>
#include <ddeml.h>
#endif

#include "basedef.h"
#include "windef.h"
#include "graph.h"
#include "event.h"
#include "window.h"
#include "winprop.h"
#include "app.h"

// All files beginning with os2* are OS/2 specific files
//     files beginning with msw* are Windows specific files

#if defined __OS2__
#include <process.h>
#include "os2rc.h"
#include "os2app.h"
#include "os2graph.h"
#include "os2win.h"
#include "os2sys.h"
#include "os2dlg.h"
#include "os2clip.h"
#include "os2print.h"

#define GUI_FONT_NAME "Helv"
#define GUI_FONT_SIZE 10

#else
#include "mswrc.h"
#include "mswapp.h"
#include "mswgraph.h"
#include "mswwin.h"
#include "mswsys.h"
#include "mswdlg.h"
#include "mswclip.h"
#include "mswprint.h"

#define GUI_FONT_NAME "Arial"
#define GUI_FONT_SIZE 10

#endif

#include "controls.h"
#define BOWL
#endif

