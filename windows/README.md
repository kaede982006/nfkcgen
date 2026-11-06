# nfkcgen — Windows 버전 (Visual Studio 2026)

이 폴더만으로 Visual Studio 2026에서 빌드됩니다. 소스(`main.c`, `generator.c`, `generator.h`)는 `../linux/`의 것과 같은 내용의 사본이므로, 한쪽을 고치면 다른 쪽에도 똑같이 반영해야 합니다.

## 라이브러리

Windows에는 GTK 4와 GLib이 기본으로 없으므로 **vcpkg**로 받아 씁니다. `vcpkg.json`에 `gtk`(GTK 4) 하나만 적혀 있고, glib·pango·cairo·harfbuzz·gdk-pixbuf·graphene 등 의존 라이브러리는 vcpkg가 함께 설치합니다. 빌드하면 필요한 DLL이 실행 파일 옆(`x64\Debug` 또는 `x64\Release`)으로 자동 복사됩니다.

vcpkg의 GTK 포트는 arm64 Windows를 지원하지 않으므로 **x64만** 제공합니다.

## 준비 (한 번만)

1. Visual Studio 2026 설치 시 **C++를 사용한 데스크톱 개발** 워크로드를 선택합니다. vcpkg가 함께 설치됩니다.
2. **개발자 PowerShell**(도구 → 명령줄)에서 MSBuild 통합을 켭니다.

   ```powershell
   vcpkg integrate install
   ```

## 빌드 및 실행

1. `nfkcgen.slnx`를 엽니다.
2. 구성을 `Debug | x64` 또는 `Release | x64`로 고르고 빌드합니다.
   - 첫 빌드 때 vcpkg가 GTK 4와 의존 라이브러리를 소스에서 빌드하므로 **수십 분** 걸릴 수 있습니다. 결과는 `vcpkg_installed\`에 남아 다음부터는 빠릅니다.
3. 실행합니다.
   - **Debug:** 콘솔 창이 함께 떠서 GTK 경고를 볼 수 있습니다.
   - **Release:** 콘솔 없이 창만 뜹니다.

## Linux 버전과 다른 점

- 한국어 입력은 Windows IME가 처리하므로 fcitx 설정(`GTK_IM_MODULE`)을 하지 않습니다.
- 소스가 UTF-8이므로 컴파일러에 `/utf-8`을 넘깁니다. 이 옵션이 없으면 한국어 문자열이 깨집니다.
- `make selftest`에 해당하는 프로젝트는 없습니다. 분류 로직 검증은 Linux에서 합니다.
