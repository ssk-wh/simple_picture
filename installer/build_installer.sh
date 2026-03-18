#!/bin/bash
# SimplePicture Installer Build Script
# Usage: bash installer/build_installer.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
DIST_DIR="$SCRIPT_DIR/dist"

# Extract version from CMakeLists.txt
VERSION=$(grep -oP 'project\(SimplePicture VERSION \K[0-9.]+' "$PROJECT_DIR/CMakeLists.txt")
echo "=== SimplePicture Installer Builder (v$VERSION) ==="

# Step 1: Build Release version
echo "[1/3] Building Release version..."
cd "$PROJECT_DIR"
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build --config Release

# Step 2: Collect files to dist/
echo "[2/3] Collecting files..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR/platforms" "$DIST_DIR/imageformats"

# Copy exe
cp "$BUILD_DIR/SimplePicture.exe" "$DIST_DIR/"

# Copy DLLs - use ldd to find runtime dependencies
echo "  Collecting DLLs..."
ldd "$BUILD_DIR/SimplePicture.exe" | grep -i mingw64 | awk '{print $3}' | while read dll; do
    cp "$dll" "$DIST_DIR/"
done

# Also copy Qt DLLs from build dir (in case ldd missed some)
for dll in "$BUILD_DIR"/*.dll; do
    [ -f "$dll" ] && cp -n "$dll" "$DIST_DIR/"
done

# Copy Qt plugins
cp "$BUILD_DIR/platforms/"*.dll "$DIST_DIR/platforms/"
cp "$BUILD_DIR/imageformats/"*.dll "$DIST_DIR/imageformats/"

echo "  $(find "$DIST_DIR" -type f \( -name "*.dll" -o -name "*.exe" \) | wc -l) files collected"

# Step 3: Sync version to NSIS script and build installer
echo "[3/3] Building installer..."
cd "$SCRIPT_DIR"
sed -i "s/!define APP_VERSION \".*\"/!define APP_VERSION \"$VERSION\"/" SimplePicture.nsi
makensis SimplePicture.nsi

echo ""
echo "=== Done! ==="
echo "Installer: $SCRIPT_DIR/SimplePicture-$VERSION-Setup.exe"
