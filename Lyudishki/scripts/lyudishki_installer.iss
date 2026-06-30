; --- Lyudishki Windows Installer (Inno Setup) ---
;
; Prerequisites:
;   1. Build the project on Windows:
;      mkdir build && cd build
;      cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64 -DCMAKE_BUILD_TYPE=Release
;      cmake --build . --config Release
;
;   2. Run windeployqt:
;      C:\Qt\6.x.x\msvc2019_64\bin\windeployqt.exe build\Release\Lyudishki.exe
;
;   3. Compile this .iss with Inno Setup Compiler (iscc.exe)
;
; Adjust paths below to match your build output directory.

#define MyAppName "Людишки"
#define MyAppNameEn "Lyudishki"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Lyudishki"
#define MyAppExeName "Lyudishki.exe"

; >>> ADJUST THIS to your actual build output <<<
#define BuildOutputDir "..\build\Release"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppNameEn}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=..\build
OutputBaseFilename=Lyudishki-Setup-{#MyAppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
SetupIconFile=..\resources\icons\Lyudishki.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "startupicon"; Description: "Запускать при старте Windows"; GroupDescription: "Автозапуск:"

[Files]
; Main executable and all Qt DLLs/plugins deployed by windeployqt
Source: "{#BuildOutputDir}\Lyudishki.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildOutputDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs
Source: "{#BuildOutputDir}\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "{#BuildOutputDir}\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "{#BuildOutputDir}\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs
Source: "{#BuildOutputDir}\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "{#BuildOutputDir}\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "{#BuildOutputDir}\tls\*"; DestDir: "{app}\tls"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: DirExists(ExpandConstant('{#BuildOutputDir}\tls'))

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Удалить {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Autostart
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppNameEn}"; ValueData: """{app}\{#MyAppExeName}"" --minimized"; Flags: uninsdeletevalue; Tasks: startupicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{userappdata}\{#MyAppNameEn}"
