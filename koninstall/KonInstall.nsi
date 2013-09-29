; KonInstall.nsi
;
; This script is based on example1.nsi, but adds uninstall support
; and (optionally) start menu shortcuts.
;
; It will install notepad.exe into a directory that the user selects,
;

; The name of the installer
Name "Kon 2.0"

; The file to write
OutFile "KonInstall.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Kon
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\NSIS_KON "Install_Dir"

; The text to prompt the user to enter a directory
ComponentText "This will install the less simple example2 on your computer. Select which optional things you want installed."
; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"

; The stuff to install
Section "Kon (required)"
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  ; Put file there
  File "..\vskon.exe"
  File "..\english.prp"
  File "..\konabout.bmp"
  File "..\kon.syn"
  SetOutPath $INSTDIR\k
  File "..\k\*.k"
  SetOutPath $INSTDIR\bmp
  File "..\bmp\*.bmp"
  SetOutPath $INSTDIR\asv
  SetOutPath $INSTDIR\help
  File "..\help\*"
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_Kon "Install_Dir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kon" "DisplayName" "Kon (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kon" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd

; optional section
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Kon"
  CreateShortCut "$SMPROGRAMS\Kon\Kon.lnk" "$INSTDIR\vskon.exe" "" "$INSTDIR\vskon.exe" 0
SectionEnd

; uninstall stuff

UninstallText "This will uninstall example2. Hit next to continue."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Kon"
  DeleteRegKey HKLM SOFTWARE\NSIS_Kon
  ; remove files
  Delete $INSTDIR\*.*
  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\Kon\*.*"
  ; remove directories used.
  RMDir /r $INSTDIR\k
  RMDir /r $INSTDIR\bmp
  RMDir /r $INSTDIR\asv
  RMDir /r $INSTDIR\help
  RMDir $INSTDIR
SectionEnd

; eof
