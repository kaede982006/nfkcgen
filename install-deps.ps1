# vcpkg.json에 적힌 의존성(GTK 4와 그 의존 라이브러리)을 vcpkg_installed\에 받는다.
# 사용법: powershell -ExecutionPolicy Bypass -File install-deps.ps1
$ErrorActionPreference = 'Stop'

# vcpkg 찾기: nfkcgen.vcxproj와 같은 순서 (VCPKG_ROOT -> Visual Studio에 포함된 vcpkg)
$vcpkg = $null
if ($env:VCPKG_ROOT -and (Test-Path "$env:VCPKG_ROOT\vcpkg.exe")) {
    $vcpkg = "$env:VCPKG_ROOT\vcpkg.exe"
}
if (-not $vcpkg) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vs = & $vswhere -latest -prerelease -products * -property installationPath
        if ($vs -and (Test-Path "$vs\VC\vcpkg\vcpkg.exe")) { $vcpkg = "$vs\VC\vcpkg\vcpkg.exe" }
    }
}
if (-not $vcpkg) {
    throw "vcpkg를 찾지 못했습니다. Visual Studio에 'C++를 사용한 데스크톱 개발' 워크로드를 설치하거나 VCPKG_ROOT를 지정하세요."
}

Write-Host "vcpkg: $vcpkg"
& $vcpkg install --triplet x64-windows "--x-manifest-root=$PSScriptRoot" "--x-install-root=$PSScriptRoot\vcpkg_installed"
if ($LASTEXITCODE -ne 0) { throw "vcpkg install 실패 (종료 코드 $LASTEXITCODE)" }
Write-Host "완료: $PSScriptRoot\vcpkg_installed\x64-windows"
