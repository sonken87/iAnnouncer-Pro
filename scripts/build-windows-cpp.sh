#!/usr/bin/env bash
# Cross-compiles the C++ component gallery for Windows (mingw-w64) and packs it
# as dist/skeuo-ui-cpp-gallery-windows-x64.zip (exe + WebView2Loader.dll).
#
# Needs: cmake, mingw-w64 (x86_64-w64-mingw32-g++-posix), zip.
set -euo pipefail

cd "$(dirname "$0")/.."
BUILD="${BUILD_DIR:-cpp/build-win}"
OUT=dist/skeuo-ui-cpp-gallery

cmake -S cpp -B "$BUILD" -DCMAKE_TOOLCHAIN_FILE="$(pwd)/cpp/cmake/mingw-w64.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DSKEUO_BUILD_TESTS=OFF
cmake --build "$BUILD" -j"$(nproc)"

rm -rf "$OUT" "$OUT-windows-x64.zip"
mkdir -p "$OUT"
cp "$BUILD/gallery.exe" "$OUT/skeuo-ui-cpp-gallery.exe"
cp cpp/third_party/webview2/x64/WebView2Loader.dll "$OUT/"
cat > "$OUT/README.txt" <<'TXT'
skeuo-ui component gallery (C++)
================================

Double-click skeuo-ui-cpp-gallery.exe. Keep WebView2Loader.dll in the same folder.

Requires the Microsoft Edge WebView2 Runtime, which ships with Windows 10
(April 2018 update or later) and Windows 11. If the window does not open,
install the "Evergreen" runtime from
https://developer.microsoft.com/microsoft-edge/webview2/
TXT
(cd dist && zip -qr "skeuo-ui-cpp-gallery-windows-x64.zip" skeuo-ui-cpp-gallery)
echo "built dist/skeuo-ui-cpp-gallery-windows-x64.zip"
