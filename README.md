# nfkcgen

**nfkcgen 1.0**은 NFKC 정규화 연구용 GTK4 문자열 생성기입니다.
문자열만 생성하며 TTS 호출, 음성 재생, 음량·발화 시간 측정은 하지 않습니다.

## 사용

1. 기존 고전/신규 × 1~4류 중 유형을 고르고 목표 길이(20~120 코드 포인트)를 정합니다.
   기본값은 신규 1류, 120자입니다. 2·3류는 일반 문장 한 줄, 4류는 두 줄 이상을 입력합니다.
2. **생성**을 누르면 원문과 NFKC 전후 코드 포인트 수가 표시됩니다. **복사**로 원문을 가져옵니다.

문자 집합에서 언어 문자(아랍·데바나가리·태국·티베트·룬·에티오피아 문자 등)를 제거했습니다.
NFKC 후 한자 등이 되는 호환 기호(㍬~㍰)도 고전·신규 집합에서 제거했습니다.
점자 패턴·도형·수학 기호·숫자는 유지하며, 신규 집합에는 중복이 없습니다. 신규 유형은 외부 문자 3~4개 뒤에 `⇿⣿` 또는 `⣿⇿`를 무작위로 한 묶음 넣습니다.
간격과 순서는 시드로 재현되며, 두 글자도 목표 길이에 포함됩니다.
남은 공간이 부족하면 묶음을 넣지 않으므로 끝부분에는 최대 5개의 외부 문자가 남을 수 있습니다.
2~4류의 일반 입력 문장은 유지합니다. 4류는 각 외부 문자 구간에서 간격을 새로 세어 묶음이 문장 양쪽으로 갈라지지 않게 합니다.
고전 유형에는 이 간격 규칙을 적용하지 않습니다.
일반 입력 문장은 필터링하지 않습니다. 문자 제거가 실제 발화 길이나 웅얼거림을 보장하지는 않습니다.

NFKC 확장 길이는 텍스트 지표일 뿐이며 음량이나 발화 시간의 예측값이 아닙니다.
`build_candidate()`는 같은 시드·설정·문자 집합으로 결과를 재현할 수 있으며, GUI는 매번 새 시드를 표시합니다.

## 빌드·실행 (Windows, Visual Studio 2026)

Windows에서는 **Visual Studio 2026(MSVC) + CMake**로 같은 소스를 빌드합니다. Win32로
재작성하지 않고 GTK4 런타임을 그대로 씁니다. GTK4는 Visual Studio에 포함돼 있지 않으므로
**vcpkg**로 받아 CMake에 연결합니다.

> 참고: GTK4를 MSVC로 받는 과정(vcpkg/gvsbuild)은 빌드량이 꽤 큽니다. 더 가벼운 길은
> MSYS2였지만, 여기서는 Visual Studio 2026 기준으로 맞춰 두었습니다.

### 준비

1. **Visual Studio 2026**을 "C++를 사용한 데스크톱 개발" 워크로드로 설치합니다.
   이 워크로드에 MSVC, CMake, Ninja, vcpkg가 포함됩니다.
2. vcpkg로 GTK4를 설치합니다(처음 한 번, 시간이 오래 걸립니다). `VCPKG_ROOT` 환경
   변수가 vcpkg 폴더를 가리키게 합니다(VS에 내장된 vcpkg를 쓰거나 별도로 clone).

   ```bat
   vcpkg install gtk:x64-windows
   ```

### 빌드

**방법 A — Visual Studio에서 폴더 열기**: VS2026에서 *파일 › 폴더 열기*로 이 폴더를
엽니다. `CMakePresets.json`의 `Windows x64 (MSVC + vcpkg)` 구성을 고르고 빌드하면
`build\windows-vcpkg\nfkcgen.exe`가 생깁니다.

**방법 B — 개발자 명령 프롬프트(x64 Native Tools)**:

```bat
cmake --preset windows-vcpkg
cmake --build --preset windows-vcpkg
ctest --preset windows-vcpkg
build\windows-vcpkg\nfkcgen.exe
build\windows-vcpkg\nfkcgen.exe --version
```

### 참고 사항

- `logo.ico`는 이 저장소에 포함돼 있어, CMake가 아이콘·버전 정보를 실행 파일에 자동으로
  넣습니다. 바꾸고 싶으면 `logo.png`에서 다시 만들어 교체하세요.
- GUI 서브시스템(`WIN32_EXECUTABLE`)으로 빌드되어 탐색기에서 띄울 때 콘솔 창이 뜨지
  않습니다. 터미널에서 실행하면 부모 콘솔에 붙어 `--version` 출력과 경고가 보입니다.
- 한글/점자/룬 등 글리프는 소스에 지정된 Windows 기본 폰트(Segoe UI Symbol·Nirmala
  UI·Malgun Gothic 등)로 렌더링됩니다. IBus 한글 입력 처리는 Linux 전용 경로이며
  Windows에서는 자동으로 비활성(무해)입니다.
- `vcpkg install` 대신 [gvsbuild](https://github.com/wingtk/gvsbuild)로 GTK4를 빌드한
  경우에는, vcpkg 툴체인 대신 `PKG_CONFIG_PATH`가 gvsbuild 프리픽스의 `lib\pkgconfig`를
  가리키게 하고 `pkg-config`를 PATH에 둔 뒤 CMake를 구성하세요.
- 다른 Windows 환경에 배포하려면 `nfkcgen.exe`와 함께 GTK4 런타임 DLL 및 데이터를
  같이 묶어야 합니다. vcpkg 빌드는 `cmake --install` 또는 `vcpkg export`로 DLL을 모을
  수 있습니다.

현재 버전은 **1.0**, 배포 태그는 `v1.0.0`입니다.

## 범위

TTS 엔진 연동, 음량·발화 시간 측정, 세션 저장·불러오기는 포함하지 않습니다.
프로그램 이름·실행 파일·창 제목은 모두 `nfkcgen`입니다.
