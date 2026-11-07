; Inno Setup 스크립트. 직접 돌리지 말고 build-installer.ps1로 만든다 (CrtDir를 넘겨 준다).
#define AppName "NFKC 연구용 문자열 생성기"
#define AppVersion "1.0.0"

[Setup]
AppId={{D1F7FF99-F960-4FD2-813C-2C88EBFDB494}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=xisik
DefaultDirName={autopf}\nfkcgen
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
; 관리자 권한 없이 사용자 폴더(%LOCALAPPDATA%\Programs)에 설치한다
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\nfkcgen.exe
OutputDir=dist
OutputBaseFilename=nfkcgen-{#AppVersion}-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "x64\Release\nfkcgen.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "x64\Release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
; VC++ 런타임을 앱 폴더에 같이 넣어 재배포 패키지 설치 없이 실행되게 한다
Source: "{#CrtDir}\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#CrtDir}\vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
; GTK 입력란의 이모지 선택기 등이 GSettings 스키마를 찾는다. 없으면 프로그램이 종료된다.
Source: "dist\schemas\gschemas.compiled"; DestDir: "{app}\share\glib-2.0\schemas"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\nfkcgen.exe"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\nfkcgen.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\nfkcgen.exe"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
