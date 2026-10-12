; ARCLINE Inno Setup Installer Script
; Product: ARCLINE — "Shape Your Sound."

#define MyAppName "ARCLINE"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "ARCLINE Audio"
#define MyAppURL "https://arcline.audio"
#define MyAppExeName "ARCLINE.exe"

[Setup]
AppId={{6B5E8982-1D47-4E90-B88D-24F9185CA2A1}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=arcline-setup-{#MyAppVersion}-windows-x64
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=lowest
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\build\Arcline_artefacts\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\README.md"; DestDir: "{app}"; Flags: isreadme
Source: "..\..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Associate .arcline project files
Root: HKCU; Subkey: "Software\Classes\.arcline"; ValueType: string; ValueName: ""; ValueData: "ArclineProject"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\Classes\ArclineProject"; ValueType: string; ValueName: ""; ValueData: "ARCLINE DAW Project"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\ArclineProject\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKCU; Subkey: "Software\Classes\ArclineProject\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
