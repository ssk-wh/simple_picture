#!/bin/bash
# SimplePicture Installer Build Script
# Usage: bash installer/build_installer.sh [--skip-build]
#   --skip-build  Skip cmake build (use existing build/SimplePicture.exe)

set -e

SKIP_BUILD=0
for arg in "$@"; do
    [ "$arg" = "--skip-build" ] && SKIP_BUILD=1
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
DIST_DIR="$SCRIPT_DIR/dist"

# Extract version from CMakeLists.txt
VERSION=$(sed -n 's/.*project(SimplePicture VERSION \([0-9.]*\).*/\1/p' "$PROJECT_DIR/CMakeLists.txt")
if [ -z "$VERSION" ]; then
    echo "ERROR: Failed to extract version from CMakeLists.txt"
    exit 1
fi
echo "=== SimplePicture Installer Builder (v$VERSION) ==="

# Detect Qt plugins directory
QT_PLUGIN_DIR=$(qmake -query QT_INSTALL_PLUGINS 2>/dev/null || echo "")
if [ -z "$QT_PLUGIN_DIR" ] || [ ! -d "$QT_PLUGIN_DIR" ]; then
    # Fallback to common MSYS2 path
    QT_PLUGIN_DIR="/c/msys64/mingw64/share/qt5/plugins"
fi
echo "  Qt plugins: $QT_PLUGIN_DIR"

# Step 1: Build Release version (skipped if --skip-build)
if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "[1/3] Building Release version..."
    cd "$PROJECT_DIR"
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
    cmake --build build --config Release
else
    echo "[1/3] Build skipped (--skip-build)."
fi

# Step 2: Collect files to dist/
echo "[2/3] Collecting files..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR/platforms" "$DIST_DIR/imageformats"

# Copy exe
cp "$BUILD_DIR/SimplePicture.exe" "$DIST_DIR/"

# Copy DLLs - use ldd to find runtime dependencies
echo "  Collecting exe dependencies..."
ldd "$BUILD_DIR/SimplePicture.exe" | grep -i mingw64 | awk '{print $3}' | while read dll; do
    cp "$dll" "$DIST_DIR/"
done

# Copy Qt plugins from Qt installation directory
echo "  Collecting Qt plugins..."
cp "$QT_PLUGIN_DIR/platforms/qwindows.dll" "$DIST_DIR/platforms/"
for plugin in qgif qico qjpeg qsvg qwebp qtiff; do
    src="$QT_PLUGIN_DIR/imageformats/${plugin}.dll"
    if [ ! -f "$src" ]; then
        echo "ERROR: 缺少图片格式插件 $src（请安装 mingw-w64-x86_64-qt5-imageformats）"
        exit 1
    fi
    cp "$src" "$DIST_DIR/imageformats/"
done

# Collect dependencies of ALL DLLs in dist (including plugins)
# This ensures indirect dependencies like libjpeg-8.dll are not missed
echo "  Collecting plugin dependencies..."
# 循环到不再新增 DLL 为止，处理传递依赖
while :; do
    added=0
    while IFS= read -r target; do
        while IFS= read -r dll; do
            [ -n "$dll" ] && [ -f "$dll" ] || continue
            if [ ! -f "$DIST_DIR/$(basename "$dll")" ]; then
                cp "$dll" "$DIST_DIR/"
                added=1
            fi
        done < <(ldd "$target" 2>/dev/null | grep -i mingw64 | awk '{print $3}' || true)
    done < <(find "$DIST_DIR" -name "*.dll")
    [ "$added" -eq 0 ] && break
done

echo "  $(find "$DIST_DIR" -type f \( -name "*.dll" -o -name "*.exe" \) | wc -l) files collected"

# Verify no missing dependencies
echo "  Verifying dependencies..."
MISSING=$(
    for target in "$DIST_DIR/"*.dll "$DIST_DIR/"*.exe "$DIST_DIR/imageformats/"*.dll "$DIST_DIR/platforms/"*.dll; do
        [ -f "$target" ] || continue
        ldd "$target" 2>/dev/null | grep "not found" | while read line; do
            echo "  $(basename "$target"): $line"
        done
    done
)
if [ -n "$MISSING" ]; then
    echo "  WARNING: Missing dependencies:"
    echo "$MISSING"
    exit 1
else
    echo "  All dependencies satisfied."
fi

# Step 3: Sync version to NSIS script and build installer
echo "[3/3] Building installer..."
cd "$SCRIPT_DIR"
sed -i "s/!define APP_VERSION \".*\"/!define APP_VERSION \"$VERSION\"/" SimplePicture.nsi
makensis SimplePicture.nsi

echo ""
echo "=== Done! ==="
echo "Installer: $SCRIPT_DIR/SimplePicture-$VERSION-Setup.exe"
