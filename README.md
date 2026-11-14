# nfkcgen

**nfkcgen 1.0**은 NFKC 정규화 연구용 GTK4 문자열 생성기입니다.
문자열만 생성하며 TTS 호출, 음성 재생, 음량·발화 시간 측정은 하지 않습니다.

## 사용

1. 기존 고전/신규 × 1~4류 중 유형을 고르고 목표 길이(20~120 코드 포인트)를 정합니다.
   기본값은 신규 1류, 120자입니다. 2·3류는 일반 문장 한 줄, 4류는 두 줄 이상을 입력합니다.
2. **생성**을 누르면 원문과 NFKC 전후 코드 포인트 수가 표시됩니다. **복사**로 원문을 가져옵니다.

NFKC 확장 길이는 텍스트 지표일 뿐이며 음량이나 발화 시간의 예측값이 아닙니다.
`build_candidate()`는 같은 시드·설정·문자 집합으로 결과를 재현할 수 있으며, GUI는 매번 새 시드를 표시합니다.

## 빌드·검증

Linux, C 컴파일러, make, pkg-config, GTK4 개발 파일(4.12 이상)이 필요합니다.

```sh
make
make check
make check-ui  # Xvfb, dbus-run-session 필요
./nfkcgen
./nfkcgen --version
sudo make install PREFIX=/usr/local
```

**한글 입력:** 세션이 IBus(`XMODIFIERS=@im=ibus`)를 쓰고 `GTK_IM_MODULE`이 비어 있으면
nfkcgen은 GTK4 IBus 모듈을 직접 선택합니다. GNOME Wayland(Rocky Linux 10 등)의 기본 경로에서
한글이 입력되지 않는 문제를 피하기 위한 것이며, 사용자가 지정한 `GTK_IM_MODULE`은 그대로 둡니다.
이 모듈은 `ibus-gtk4` 패키지에 있습니다(Debian·Ubuntu: `apt`, Fedora·Rocky: `dnf`로 설치).
DEB/RPM 패키지는 이를 권장 의존성으로 설치합니다.

DEB/RPM은 x86_64 Linux용이며 빌드 환경의 라이브러리 ABI 요구사항을 따릅니다.
이 릴리스는 사용자가 지정한 연구 버전 **1.0**이며 배포 태그는 `v1.0.0`입니다.
구버전 `1.0`, `1.1`, `1.1.0`의 릴리스·태그·배포물은 삭제되었습니다. 기존 1.1 패키지에서 설치하면
패키지 관리자는 버전 하향으로 취급할 수 있습니다.

## 범위

TTS 엔진 연동, 음량·발화 시간 측정, 세션 저장·불러오기는 포함하지 않습니다.
프로그램 이름·실행 파일·창 제목·데스크톱 항목은 모두 `nfkcgen`입니다.
