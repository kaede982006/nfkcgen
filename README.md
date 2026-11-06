# NFKC 연구용 문자열 생성기 (GTK 4 / C)

고전·신규 외부 문자 집합에 대해 1~4류 실험용 문자열을 생성하고, NFKC 정규화 전후 코드 포인트 길이를 비교하는 데스크톱 프로그램입니다.

| 폴더 | 내용 |
|---|---|
| [`linux/`](linux/README.md) | Linux 소스, `Makefile`, 빌드된 `.deb`·`.rpm`(`linux/dist/`) |
| [`windows/`](windows/README.md) | Visual Studio 2026에서 빌드되는 소스와 프로젝트. GTK 4는 vcpkg로 받습니다 |

두 폴더의 `main.c`, `generator.c`, `generator.h`는 같은 내용입니다. 한쪽을 고치면 다른 쪽에도 반영하세요.
