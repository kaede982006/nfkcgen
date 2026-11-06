# NFKC 연구용 문자열 생성기 (GTK 4 / C)

GTK 4 기반으로 고전·신규 외부 문자 집합에 대해 1~4류 실험용 문자열을 생성하는 Linux 데스크톱 프로그램입니다.

## 빌드 및 실행

Debian 13 기준 의존성:

```sh
sudo apt update
sudo apt install build-essential pkg-config libgtk-4-dev
```

한국어 등 다국어 입력(fcitx5)을 쓰려면 GTK4용 입력기 모듈도 설치하고 fcitx5를 다시 시작합니다.

```sh
sudo apt install fcitx5-frontend-gtk4
```

프로그램은 시작 시 `GTK_IM_MODULE`이 비어 있으면 `fcitx`로 설정합니다. 데스크톱에서 이미 다른 입력기(ibus 등)를 지정했다면 그 값을 그대로 존중합니다.

일부 GTK4 테마(예: KDE Breeze)는 스크롤바 슬라이더에 음수 최소 크기를 주어 `GtkGizmo (slider) reported min ... but sizes must be >= 0` 경고를 반복 출력합니다. 앱 코드로는 못 고치는 테마 버그이며 무해하므로, 프로그램은 이 메시지만 좁게 걸러내고 다른 로그는 그대로 출력합니다(`drop_gizmo_size_warnings`).

빌드 및 실행 (이 `linux/` 폴더에서):

```sh
make
./nfkcgen
```

분류 로직만 검증(UI 없이):

```sh
make selftest
```

## 패키지 (.deb / .rpm)

미리 만든 패키지가 `dist/`에 있습니다.

| 파일 | 대상 | 설치 |
|---|---|---|
| `dist/nfkcgen_1.0.0_amd64.deb` | Debian·Ubuntu 계열 | `sudo apt install ./dist/nfkcgen_1.0.0_amd64.deb` |
| `dist/nfkcgen-1.0.0-1.x86_64.rpm` | Fedora·RHEL 계열 | `sudo dnf install ./dist/nfkcgen-1.0.0-1.x86_64.rpm` |

두 패키지 모두 `/usr/bin/nfkcgen`과 데스크톱 메뉴 항목(`/usr/share/applications/org.xisik.nfkcgen.desktop`)을 설치합니다. 다시 만들려면:

```sh
sudo apt install rpm      # .rpm을 만들 때만 필요
make packages             # 또는 make deb / make rpm
```

`.rpm`은 Debian 13(glibc 2.41)에서 빌드하므로 glibc 2.34 이상 배포판이 필요합니다.

## 외부 문자 집합

- **고전(classic):** 원본 관찰 문자 `⣿ ㍰ █`. `main.c`의 `classic_alphabet` 상수를 고치면 넓힐 수 있습니다.
- **신규(modern):** 현재 요청 표본 기반 집합(`modern_alphabet`).

## 문자열 유형

고전·신규 각각에 대해 네 가지 유형이 있습니다(드롭다운 8종).

- **1류 — 완전한 NFKC 문자열:** 외부 문자만 생성합니다. 일반 문자열 입력란은 비활성화됩니다.
- **2류 — 앞에 일반 문자열:** 비어 있지 않은 줄이 정확히 하나여야 합니다. 그 문자열을 맨 앞에 놓고 뒤에 외부 문자를 붙입니다.
- **3류 — 뒤에 일반 문자열:** 비어 있지 않은 줄이 정확히 하나여야 합니다. 앞에 외부 문자를 놓고 그 문자열을 맨 뒤에 붙입니다.
- **4류 — 문장 사이·양끝 삽입:** 비어 있지 않은 줄이 두 개 이상이어야 합니다. 입력 순서를 유지하며 각 문장 앞·사이·뒤에 외부 문자 구간을 둡니다(기존 3류 알고리즘).

빈 줄은 무시합니다. 목표 길이는 20~120 코드 포인트이며 기본값은 80입니다. 2·3·4류에서 일반 문자열의 코드 포인트 총합이 목표 길이 이상이면 생성을 거부합니다. 4류는 각 문장 사이와 양 끝에 외부 문자를 최소 한 개씩 둘 수 있어야 합니다.

## 파일

- `generator.h` / `generator.c` — 외부 문자 집합과 순수 생성 로직(`build_string` 등). GTK 비의존, glib만 사용
- `main.c` — GTK 4 사용자 인터페이스. `generator.h`를 통해 생성 로직을 호출
- `selftest.c` — `build_string` 분기 구조 검증(glib만 링크)
- `Makefile` — GUI 빌드, `selftest`, `install`, `deb`, `rpm`, `packages` 타깃
- `nfkcgen.spec` — `.rpm` 빌드 명세
- `org.xisik.nfkcgen.desktop` — 데스크톱 메뉴 항목
- `dist/` — 빌드된 `.deb`, `.rpm`
