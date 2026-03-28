#!/usr/bin/env bash
set -e

cd /project

echo "[1/5] Downloading headers..."
curl -fsSL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp      -o include/json.hpp
curl -fsSL https://raw.githubusercontent.com/WerWolv/libtesla/master/include/tesla.hpp -o include/tesla.hpp
curl -fsSL https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h     -o include/stb_truetype.h

echo "[2/5] Patching tesla.hpp for GCC 15 compatibility..."
python3 - <<'EOF'
content = open('include/tesla.hpp').read()
old = 'struct ScissoringConfig {'
new = ('struct ScissoringConfig {'
       ' ScissoringConfig(s32 x,s32 y,s32 w,s32 h):x(x),y(y),w(w),h(h){}'
       ' ScissoringConfig()=default;')
if old in content:
    content = content.replace(old, new, 1)
    open('include/tesla.hpp', 'w').write(content)
    print("  -> ScissoringConfig constructor added")
else:
    print("  -> already patched or not found, skipping")
EOF

echo "[3/5] Copying headers to system include path..."
cp include/tesla.hpp     /opt/devkitpro/portlibs/switch/include/
cp include/stb_truetype.h /opt/devkitpro/portlibs/switch/include/

echo "[4/5] Building..."
make

echo "[5/5] Locating OVL..."
# elf2nro drops the file in build/ without a proper name due to a Makefile quirk.
# Find the actual OVL (smallest file with .ovl extension or named .ovl) and copy it.
OVL=""
if   [ -f "build/.ovl" ];          then OVL="build/.ovl"
elif [ -f "k_limate.ovl" ];        then OVL="k_limate.ovl"
else
    OVL=$(find build -name "*.ovl" 2>/dev/null | head -1)
fi

if [ -z "$OVL" ]; then
    echo "ERROR: OVL file not found after build."
    exit 1
fi

cp "$OVL" k_limate.ovl
SIZE=$(du -h k_limate.ovl | cut -f1)

echo ""
echo "Done! k_limate.ovl ($SIZE)"
echo "  Copy to SD:/switch/.overlays/k_limate.ovl"
