; Inno Setup 설치기 소스 — nfkcgen 1.0 (Windows x64, GTK4)
;
; 이 스크립트는 CMake 설치 단계가 만든 stage 폴더(실행 파일 + GTK4 런타임 DLL과
; 데이터: 스키마·픽스버프 로더·아이콘 테마)를 그대로 설치기로 묶습니다.
;
; 준비:
;   cmake --preset windows-vcpkg
;   cmake --build --preset windows-vcpkg
;   cmake --install build\windows-vcpkg --prefix build\windows-vcpkg\stage
; 컴파일(Inno Setup 필요):
;   ISCC installer\nfkcgen.iss
; 결과: installer\Output\nfkcgen-1.0.0-setup.exe

#define AppName "nfkcgen"
#define AppVersion "1.0.0"
#define AppPublisher "xisik"
#define AppURL "https://github.com/kaede982006/nfkcgen"
#define AppExeName "nfkcgen.exe"

; stage 폴더 경로. 다른 경로면 ISCC /DStageDir=... 로 덮어쓴다.
#ifndef StageDir
  #define StageDir "..\build\windows-vcpkg\stage"
#endif

[Setup]
AppId={{B7F3B2E1-8C4A-4E7D-9A1C-0F2D3E4A5B60}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
UninstallDisplayIcon={app}\{#AppExeName}
OutputDir=Output
OutputBaseFilename=nfkcgen-{#AppVersion}-setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern

[Languages]
Name: "korean"; MessagesFile: "compiler:Languages\Korean.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
