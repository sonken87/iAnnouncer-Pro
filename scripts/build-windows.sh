#!/usr/bin/env bash
# Cross-compiles the component gallery into a Windows desktop app and packs
# it as dist/skeuo-ui-gallery-windows-x64.zip (exe + WebView2Loader.dll).
#
# Needs: rustup target x86_64-pc-windows-gnu, mingw-w64 (x86_64-w64-mingw32-gcc), zip.
# Runs on Windows 10/11 out of the box (they ship the WebView2 runtime).
set -euo pipefail

cd "$(dirname "$0")/.."
TARGET=x86_64-pc-windows-gnu
TARGET_DIR="${CARGO_TARGET_DIR:-target}"
OUT=dist/skeuo-ui-gallery

cargo build -p gallery --release --no-default-features --features desktop --target "$TARGET"

rm -rf "$OUT" "$OUT-windows-x64.zip"
mkdir -p "$OUT"
cp "$TARGET_DIR/$TARGET/release/gallery.exe" "$OUT/skeuo-ui-gallery.exe"
# wry's WebView2 loader, built by webview2-com-sys for this target.
loader=$(ls -t "$TARGET_DIR/$TARGET"/release/build/webview2-com-sys-*/out/x64/WebView2Loader.dll | head -1)
cp "$loader" "$OUT/"
cat > "$OUT/README.txt" <<'TXT'
skeuo-ui component gallery
==========================

Double-click skeuo-ui-gallery.exe. Keep WebView2Loader.dll in the same folder.

Requires the Microsoft Edge WebView2 Runtime, which ships with Windows 10
(April 2018 update or later) and Windows 11. If the window does not open,
install the "Evergreen" runtime from
https://developer.microsoft.com/microsoft-edge/webview2/
TXT
(cd dist && zip -qr "skeuo-ui-gallery-windows-x64.zip" skeuo-ui-gallery)
echo "built dist/skeuo-ui-gallery-windows-x64.zip"
