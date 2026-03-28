#!/usr/bin/env bash
set -euo pipefail

# 스크립트 위치로 이동
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 색상 코드
RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[1;33m'
NC='\033[0m'

ok()  { echo -e " ${GRN}[OK]${NC} $*"; }
err() { echo -e " ${RED}[X]${NC}  $*"; }
inf() { echo -e " ${YLW}[..]${NC} $*"; }

echo ""
echo " ================================================"
echo "  Klimate - Nintendo Switch Weather Overlay"
echo "  Linux 빌드 스크립트"
echo " ================================================"
echo ""

# ── devkitPro 확인 / 설치 ──────────────────────────────────────────────────────
if [ -z "${DEVKITPRO:-}" ]; then
    export DEVKITPRO=/opt/devkitpro
fi

if [ ! -d "$DEVKITPRO" ]; then
    echo ""
    inf "devkitPro가 설치되어 있지 않습니다. 설치를 시작합니다..."
    echo ""

    if command -v apt-get &>/dev/null; then
        # Ubuntu / Debian
        inf "패키지 목록 업데이트 중..."
        sudo apt-get update -qq
        sudo apt-get install -y wget curl build-essential

        inf "devkitPro 설치 프로그램 다운로드 중..."
        wget -q https://apt.devkitpro.org/install-devkitpro-pacman -O /tmp/install-dkp.sh
        chmod +x /tmp/install-dkp.sh
        sudo /tmp/install-dkp.sh

    elif command -v pacman &>/dev/null; then
        # Arch Linux
        inf "devkitPro (Arch) 설치 중..."
        sudo pacman -S --noconfirm base-devel curl wget
        wget -q https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.xz
        sudo pacman -U --noconfirm devkitpro-keyring.pkg.tar.xz
        sudo pacman-key --recv BC26F752D25B92CE272E0F44F7FD5492264BB9D0 --keyserver keyserver.ubuntu.com
        sudo pacman-key --lsign BC26F752D25B92CE272E0F44F7FD5492264BB9D0
        echo "[devkitpro]
Server = https://pkg.devkitpro.org/packages" | sudo tee -a /etc/pacman.conf
        sudo pacman -Sy --noconfirm

    else
        err "지원하지 않는 배포판입니다."
        echo ""
        echo " 직접 설치: https://devkitpro.org/wiki/Getting_Started"
        echo ""
        exit 1
    fi

    # 환경 변수 적용
    export DEVKITPRO=/opt/devkitpro
fi

ok "devkitPro: $DEVKITPRO"

# devkitPro 환경 변수 로드
if [ -f /etc/profile.d/devkit-env.sh ]; then
    # shellcheck disable=SC1091
    source /etc/profile.d/devkit-env.sh
else
    export PATH="$DEVKITPRO/tools/bin:$DEVKITPRO/devkitA64/bin:$PATH"
fi

# ── 필요한 Switch 패키지 설치 ─────────────────────────────────────────────────
inf "필요한 Switch 패키지 확인 및 설치 중..."
echo "  (처음 실행 시 시간이 걸릴 수 있습니다)"

sudo dkp-pacman -S --needed --noconfirm \
    switch-dev switch-libnx \
    switch-curl switch-mbedtls switch-zlib \
    switch-libtesla

ok "패키지 준비 완료"

# ── json.hpp 다운로드 ──────────────────────────────────────────────────────────
if [ ! -f "include/json.hpp" ]; then
    inf "nlohmann/json.hpp 다운로드 중..."
    curl -fsSL -o include/json.hpp \
        "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
    ok "json.hpp 다운로드 완료"
else
    ok "json.hpp 준비됨"
fi

# ── 빌드 ──────────────────────────────────────────────────────────────────────
inf "빌드 중..."
echo ""

make

echo ""
echo " ================================================"
ok "빌드 성공! weather_overlay.ovl 생성됨"
echo " ================================================"
echo ""
echo " ── 스위치에 설치하는 방법 ────────────────────────────"
echo ""
echo "  1. SD카드에 파일 복사:"
echo "     weather_overlay.ovl → SD:/switch/.overlays/"
echo ""
echo "  2. 설정 파일 생성 (없을 경우):"
echo "     SD:/config/weather.json"
echo "     (이 폴더의 weather.json.example 참고)"
echo ""
echo "  3. Tesla Menu를 실행하면 오버레이 목록에 표시됩니다."
echo " ───────────────────────────────────────────────────────"
echo ""
