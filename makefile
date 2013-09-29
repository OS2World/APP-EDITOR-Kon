# makefile
#
# The actions included in this make file are:
#  Compile::Resource Compiler
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj .rc .res 

{.\}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\os2\%|fF.RES

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ti- /Ss /J- /Si /Gm+ /C /qtune=pentium /O+ /Oc- /Sp8 /Su4 /qlibansi /qignerrno /Gx+ /Gl+ /Oi- /Fo.\os2\ %s 
#    icc.exe /Ss /J- /Si /Ti+ /Gm+ /C /qtune=pentium /Oc- /Sp8 /Su4 /qlibansi /qignerrno /Gx+ /Gl+ /Oi- /Fo.\os2\ %s

{.\}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Ti- /Ss /J- /Si /Gm+ /C /qtune=pentium /O+ /Oc- /Sp8 /Su4 /qlibansi /qignerrno /Gx+ /Gl+ /Oi- /Fo.\os2\ %s
#    icc.exe /Ss /J- /Si /Ti+ /Gm+ /C /qtune=pentium /Oc- /Sp8 /Su4 /qlibansi /qignerrno /Gx+ /Gl+ /Oi- /Fo.\os2\ %s

all: \
    .\os2\konos2.exe

.\os2\konos2.exe: \
    .\os2\window.obj \
    .\os2\winprop.obj \
    .\os2\asciview.obj \
    .\os2\bufadmin.obj \
    .\os2\buffer.obj \
    .\os2\bytesinfo.obj \
    .\os2\cmdq.obj \
    .\os2\cntrladm.obj \
    .\os2\controls.obj \
    .\os2\ctrlwin.obj \
    .\os2\editctrl.obj \
    .\os2\filedlg.obj \
    .\os2\globals.obj \
    .\os2\graph.obj \
    .\os2\grep.obj \
    .\os2\hexview.obj \
    .\os2\infobar.obj \
    .\os2\k.obj \
    .\os2\kcommon.obj \
    .\os2\kerrcoll.obj \
    .\os2\keval.obj \
    .\os2\keybind.obj \
    .\os2\klexer.obj \
    .\os2\kparse.obj \
    .\os2\main.obj \
    .\os2\mainmenu.obj \
    .\os2\mainwadm.obj \
    .\os2\mainwnd.obj \
    .\os2\multfile.obj \
    .\os2\multfind.obj \
    .\os2\multrep.obj \
    .\os2\os2app.obj \
    .\os2\os2clip.obj \
    .\os2\os2dlg.obj \
    .\os2\os2graph.obj \
    .\os2\os2main.obj \
    .\os2\os2sys.obj \
    .\os2\os2win.obj \
    .\os2\repldlg.obj \
    .\os2\search.obj \
    .\os2\sectionbrowser.obj \
    .\os2\srchdlg.obj \
    .\os2\printdlg.obj \
    .\os2\syntax.obj \
    .\os2\undo.obj \
    .\os2\view.obj \
    .\os2\viewwin.obj \
    .\os2\print.obj \
    .\os2\os2print.obj \
    .\os2\autosave.obj \
    .\os2\comdlg.obj \
    .\os2\popupwin.obj \
    .\os2\prefsdlg.obj \
    .\os2\kmodules.obj \
    .\os2\lic.obj \
    .\os2\kon.res
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /Tdp 
     /B" /de /exepack:2 /pmtype:pm /packd /OPTFUNC /STACK:0xfffff"
     /Fekonos2.exe 
     .\os2\window.obj
     .\os2\winprop.obj
     .\os2\asciview.obj
     .\os2\bufadmin.obj
     .\os2\buffer.obj
     .\os2\bytesinfo.obj
     .\os2\cmdq.obj
     .\os2\cntrladm.obj
     .\os2\controls.obj
     .\os2\ctrlwin.obj
     .\os2\editctrl.obj
     .\os2\filedlg.obj
     .\os2\globals.obj
     .\os2\graph.obj
     .\os2\grep.obj
     .\os2\hexview.obj
     .\os2\infobar.obj
     .\os2\k.obj
     .\os2\kcommon.obj
     .\os2\kerrcoll.obj
     .\os2\keval.obj
     .\os2\keybind.obj
     .\os2\klexer.obj
     .\os2\kparse.obj
     .\os2\print.obj
     .\os2\main.obj
     .\os2\mainmenu.obj
     .\os2\mainwadm.obj
     .\os2\mainwnd.obj
     .\os2\multfile.obj
     .\os2\multfind.obj
     .\os2\multrep.obj
     .\os2\os2app.obj
     .\os2\os2clip.obj
     .\os2\os2dlg.obj
     .\os2\os2graph.obj
     .\os2\os2main.obj
     .\os2\os2sys.obj
     .\os2\os2win.obj
     .\os2\repldlg.obj
     .\os2\search.obj
     .\os2\srchdlg.obj
     .\os2\printdlg.obj
     .\os2\sectionbrowser.obj
     .\os2\syntax.obj
     .\os2\undo.obj
     .\os2\view.obj
     .\os2\viewwin.obj
     .\os2\os2print.obj
     .\os2\autosave.obj
     .\os2\comdlg.obj
     .\os2\popupwin.obj
     .\os2\prefsdlg.obj
     .\os2\kmodules.obj
     .\os2\lic.obj
<<
    rc.exe .\os2\kon.res konos2.exe

.\os2\kon.res: \
    .\kon.rc \
    .\zed.ico \
    {.\;$(INCLUDE)}os2rc.H

.\os2\asciview.obj: \
  .\asciview.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}print.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\autosave.obj: \
  .\autosave.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\bufadmin.obj: \
  .\bufadmin.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\buffer.obj: \
  .\buffer.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\bytesinfo.obj: \
  .\bytesinfo.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}bytesinfo.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\cmdq.obj: \
  .\cmdq.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\cntrladm.obj: \
  .\cntrladm.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\comdlg.obj: \
  .\comdlg.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}comdlg.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\controls.obj: \
  .\controls.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\ctrlwin.obj: \
  .\ctrlwin.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\editctrl.obj: \
  .\editctrl.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}bytesinfo.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}comdlg.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}filedlg.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}multfile.h \
	{.\;$(INCLUDE);}multop.h \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}print.h \
	{.\;$(INCLUDE);}printdlg.h \
	{.\;$(INCLUDE);}repldlg.h \
	{.\;$(INCLUDE);}srchdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\filedlg.obj: \
  .\filedlg.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}filedlg.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\globals.obj: \
  .\globals.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\graph.obj: \
  .\graph.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\grep.obj: \
  .\grep.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}grep.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\hexview.obj: \
  .\hexview.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}print.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\infobar.obj: \
  .\infobar.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\sectionbrowser.obj: \
  .\sectionbrowser.cpp \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\k.obj: \
  .\k.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\kcommon.obj: \
  .\kcommon.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}multfile.h \
	{.\;$(INCLUDE);}multfind.h \
	{.\;$(INCLUDE);}multop.h \
	{.\;$(INCLUDE);}multrep.h \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\kerrcoll.obj: \
  .\kerrcoll.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\keval.obj: \
  .\keval.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\keybind.obj: \
  .\keybind.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\klexer.obj: \
  .\klexer.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\kmodules.obj: \
  .\kmodules.cpp \
	{.\;$(INCLUDE);}kmodules.h

.\os2\kparse.obj: \
  .\kparse.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\main.obj: \
  .\main.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}comdlg.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}kstartup.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}lic.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\mainmenu.obj: \
  .\mainmenu.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\mainwadm.obj: \
  .\mainwadm.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\mainwnd.obj: \
  .\mainwnd.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}filedlg.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}prefsdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2APP.obj: \
  .\OS2APP.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2CLIP.obj: \
  .\OS2CLIP.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2DLG.obj: \
  .\OS2DLG.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2GRAPH.obj: \
  .\OS2GRAPH.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2MAIN.obj: \
  .\OS2MAIN.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2print.obj: \
  .\OS2print.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2SYS.obj: \
  .\OS2SYS.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\OS2WIN.obj: \
  .\OS2WIN.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\lic.obj: \
  .\lic.cpp \
	{.\;$(INCLUDE);}lic.h

.\os2\multfile.obj: \
  .\multfile.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}multfile.h \
	{.\;$(INCLUDE);}multop.h \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\multfind.obj: \
  .\multfind.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}multfind.h \
	{.\;$(INCLUDE);}multop.h \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\multrep.obj: \
  .\multrep.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}multop.h \
	{.\;$(INCLUDE);}multrep.h \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}repldlg.h \
	{.\;$(INCLUDE);}srchdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\popupwin.obj: \
  .\popupwin.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\prefsdlg.obj: \
  .\prefsdlg.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}prefsdlg.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\print.obj: \
  .\print.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}print.h \
	{.\;$(INCLUDE);}printdlg.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\printdlg.obj: \
  .\printdlg.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}print.h \
	{.\;$(INCLUDE);}printdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\repldlg.obj: \
  .\repldlg.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}repldlg.h \
	{.\;$(INCLUDE);}srchdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\search.obj: \
  .\search.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\srchdlg.obj: \
  .\srchdlg.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}srchdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\syntax.obj: \
  .\syntax.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\undo.obj: \
  .\undo.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\view.obj: \
  .\view.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}grep.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}repldlg.h \
	{.\;$(INCLUDE);}srchdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\viewwin.obj: \
  .\viewwin.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\window.obj: \
  .\window.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h

.\os2\winprop.obj: \
  .\winprop.cpp \
	{.\;$(INCLUDE);}app.h \
	{.\;$(INCLUDE);}asciview.h \
	{.\;$(INCLUDE);}autosave.h \
	{.\;$(INCLUDE);}basedef.h \
	{.\;$(INCLUDE);}bowl.h \
	{.\;$(INCLUDE);}bufadmin.h \
	{.\;$(INCLUDE);}bufhldr.h \
	{.\;$(INCLUDE);}cmdq.h \
	{.\;$(INCLUDE);}cntrladm.h \
	{.\;$(INCLUDE);}controls.h \
	{.\;$(INCLUDE);}ctrlwin.h \
	{.\;$(INCLUDE);}editctrl.h \
	{.\;$(INCLUDE);}event.h \
	{.\;$(INCLUDE);}globals.h \
	{.\;$(INCLUDE);}graph.h \
	{.\;$(INCLUDE);}hexview.h \
	{.\;$(INCLUDE);}infobar.h \
	{.\;$(INCLUDE);}k.h \
	{.\;$(INCLUDE);}kcmdexec.h \
	{.\;$(INCLUDE);}kcommon.h \
	{.\;$(INCLUDE);}kerr.h \
	{.\;$(INCLUDE);}kerrcoll.h \
	{.\;$(INCLUDE);}kerror.h \
	{.\;$(INCLUDE);}keval.h \
	{.\;$(INCLUDE);}keybind.h \
	{.\;$(INCLUDE);}klexer.h \
	{.\;$(INCLUDE);}kparse.h \
	{.\;$(INCLUDE);}mainmenu.h \
	{.\;$(INCLUDE);}mainwadm.h \
	{.\;$(INCLUDE);}mainwnd.h \
	{.\;$(INCLUDE);}OS2APP.H \
	{.\;$(INCLUDE);}OS2CLIP.H \
	{.\;$(INCLUDE);}OS2DLG.H \
	{.\;$(INCLUDE);}OS2GRAPH.H \
	{.\;$(INCLUDE);}OS2print.h \
	{.\;$(INCLUDE);}OS2SYS.H \
	{.\;$(INCLUDE);}OS2WIN.H \
	{.\;$(INCLUDE);}popupwin.h \
	{.\;$(INCLUDE);}print.h \
	{.\;$(INCLUDE);}printdlg.h \
	{.\;$(INCLUDE);}repldlg.h \
	{.\;$(INCLUDE);}srchdlg.h \
	{.\;$(INCLUDE);}syntax.h \
	{.\;$(INCLUDE);}undo.h \
	{.\;$(INCLUDE);}view.h \
	{.\;$(INCLUDE);}viewwin.h \
	{.\;$(INCLUDE);}vwholder.h \
	{.\;$(INCLUDE);}window.h \
	{.\;$(INCLUDE);}winprop.h
