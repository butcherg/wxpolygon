

#define MyAppName "wxpolygon"
#define MyAppVersion "1.0"
#define MyAppPublisher "Glenn Butcher"
#define MyAppExeName "wxpolygon.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{0f63828e-b0fa-4955-a187-058a76307a01}
AppName={#MyAppName}                              
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppContact={#MyAppPublisher}
DefaultDirName={commonpf}\{#MyAppName}
SetupIconFile=..\wxpolygon.ico
DisableProgramGroupPage=yes
DisableWelcomePage=no
PrivilegesRequired=admin
UsedUserAreasWarning=no
OutputBaseFilename={#MyAppName}-{#MyAppVersion}-w64
Compression=lzma
SolidCompression=yes
OutputDir=.
VersionInfoDescription=A simple wxpolygon program.
VersionInfoCopyright=Copyright (C) 2021 Glenn Butcher.
VersionInfoCompany={#MyAppPublisher}
ChangesEnvironment=true
ArchitecturesInstallIn64BitMode=x64
;InfoBeforeFile=readme.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Dirs]
Name: "{userappdata}\{#MyAppName}"

[Files]
Source: "{#MyAppName}.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MyAppName}.conf"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent


