cmake_minimum_required(VERSION 3.21)
project(nfkcgen VERSION 1.0.0 LANGUAGES C)

# Windows(Visual Studio 2026/MSVC)용 빌드. GTK4는 vcpkg(vcpkg.json 매니페스트)가
# 제공하는 pkg-config(.pc) 파일로 찾는다. Linux 에서도 같은 CMakeLists 로 빌드할 수
# 있지만, Linux 기본 빌드는 별도 도구를 쓴다.

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 소스가 UTF-8(한글 주석·유니코드 문자열 리터럴)이다. MSVC 는 기본으로 시스템
# 코드페이지로 소스를 읽어 깨지므로, 소스/실행 문자셋을 UTF-8 로 지정한다.
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Windows/MSVC 에는 pkg-config 가 없다. vcpkg 가 설치한 pkgconf 를 쓰게 하고,
# GTK4 .pc 파일이 있는 vcpkg 설치 트리를 PKG_CONFIG_PATH 로 알려준다.
if(WIN32)
    set(_vcpkg_tri "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    if(NOT PKG_CONFIG_EXECUTABLE)
        find_program(PKG_CONFIG_EXECUTABLE NAMES pkgconf pkg-config
            PATHS "${_vcpkg_tri}/tools/pkgconf" NO_DEFAULT_PATH)
    endif()
    set(ENV{PKG_CONFIG_PATH} "${_vcpkg_tri}/lib/pkgconfig")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

set(SOURCES main.c generator.c)

# logo.ico 가 있으면 아이콘 + 버전 정보를 실행 파일 리소스로 넣는다(Windows 전용).
if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/logo.ico")
    enable_language(RC)
    list(APPEND SOURCES nfkcgen.rc)
endif()

add_executable(nfkcgen ${SOURCES})

target_compile_definitions(nfkcgen PRIVATE NFKCGEN_VERSION="1.0")
target_link_libraries(nfkcgen PRIVATE PkgConfig::GTK4)

if(WIN32)
    # GUI 서브시스템: 탐색기에서 띄울 때 콘솔 창이 뜨지 않는다.
    set_target_properties(nfkcgen PROPERTIES WIN32_EXECUTABLE TRUE)
    if(MSVC)
        # /SUBSYSTEM:WINDOWS 의 기본 진입점은 WinMain 이므로, 소스의 main() 을 쓰도록
        # 진입점을 되돌린다. 또 freopen 등 CRT 보안 경고를 끈다.
        target_link_options(nfkcgen PRIVATE "/ENTRY:mainCRTStartup")
        target_compile_definitions(nfkcgen PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
else()
    target_link_libraries(nfkcgen PRIVATE m)
endif()

# 설치/패키징 규칙(Windows). `cmake --install <build> --prefix <stage>` 하면 실행 파일,
# GTK4 런타임 DLL, 런타임 데이터(스키마·픽스버프 로더·아이콘 테마)를 한 폴더로 모은다.
# 이 stage 폴더를 Inno Setup(installer/nfkcgen.iss)이 그대로 설치기로 묶는다.
if(WIN32)
    set(_vcpkg_root "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    install(TARGETS nfkcgen RUNTIME DESTINATION ".")
    if(EXISTS "${_vcpkg_root}/bin")
        install(DIRECTORY "${_vcpkg_root}/bin/" DESTINATION "." FILES_MATCHING PATTERN "*.dll")
    endif()
    if(EXISTS "${_vcpkg_root}/share/glib-2.0/schemas")
        install(DIRECTORY "${_vcpkg_root}/share/glib-2.0/schemas" DESTINATION "share/glib-2.0")
        # vcpkg 는 .gschema.xml 원본만 설치한다. GTK4 런타임이 읽는 gschemas.compiled 를
        # 설치 단계에서 생성한다.
        find_program(GLIB_COMPILE_SCHEMAS NAMES glib-compile-schemas
            PATHS "${_vcpkg_root}/tools/glib" NO_DEFAULT_PATH)
        if(GLIB_COMPILE_SCHEMAS)
            install(CODE "execute_process(COMMAND \"${GLIB_COMPILE_SCHEMAS}\" \"\${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas\")")
        endif()
    endif()
    if(EXISTS "${_vcpkg_root}/lib/gdk-pixbuf-2.0")
        install(DIRECTORY "${_vcpkg_root}/lib/gdk-pixbuf-2.0" DESTINATION "lib")
    endif()
    if(EXISTS "${_vcpkg_root}/share/icons")
        install(DIRECTORY "${_vcpkg_root}/share/icons" DESTINATION "share")
    endif()
    if(EXISTS "${_vcpkg_root}/share/glib-2.0/schemas/gschemas.compiled")
        # 이미 위에서 설치됨(명시용 주석).
    endif()
endif()

# 선택: 생성기 단위 테스트(ctest). gtk4 데이터 환경 없이도 돈다.
enable_testing()
add_executable(test-generator test-generator.c generator.c)
target_link_libraries(test-generator PRIVATE PkgConfig::GTK4)
if(NOT WIN32)
    target_link_libraries(test-generator PRIVATE m)
endif()
add_test(NAME generator COMMAND test-generator)
