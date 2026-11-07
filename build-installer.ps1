# Release 빌드 후 Inno Setup으로 dist\nfkcgen-<버전>-setup.exe를 만든다.
# 사용법: powershell -ExecutionPolicy Bypass -File build-installer.ps1
# 필요: Visual Studio 2026(C++ 데스크톱 개발), Inno Setup 6 (winget install JRSoftware.InnoSetup)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -property installationPath
if (-not $vs) { throw "Visual Studio를 찾지 못했습니다." }

& "$vs\MSBuild\Current\Bin\MSBuild.exe" nfkcgen.vcxproj /p:Configuration=Release /p:Platform=x64 /m /nologo /v:minimal
if ($LASTEXITCODE -ne 0) { throw "빌드 실패" }

New-Item -ItemType Directory -Force dist\schemas | Out-Null
& vcpkg_installed\x64-windows\tools\glib\glib-compile-schemas.exe --targetdir=dist\schemas vcpkg_installed\x64-windows\share\glib-2.0\schemas
if ($LASTEXITCODE -ne 0) { throw "스키마 컴파일 실패" }

$crt = Get-ChildItem "$vs\VC\Redist\MSVC\*\x64\Microsoft.VC*.CRT" -Directory | Sort-Object FullName | Select-Object -Last 1
if (-not $crt) { throw "VC++ 런타임(Microsoft.VC*.CRT)을 찾지 못했습니다." }

$iscc = @("$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe", "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe", "$env:ProgramFiles\Inno Setup 6\ISCC.exe") |
    Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) { throw "Inno Setup 6을 찾지 못했습니다. winget install JRSoftware.InnoSetup 으로 설치하세요." }

& $iscc /Q "/DCrtDir=$($crt.FullName)" nfkcgen.iss
if ($LASTEXITCODE -ne 0) { throw "인스톨러 생성 실패" }
Get-ChildItem dist\*.exe | ForEach-Object { "완료: $($_.FullName)" }
