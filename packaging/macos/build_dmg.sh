#!/usr/bin/env bash
# macOS DMG Packaging Script for ARCLINE
set -euo pipefail

APP_NAME="ARCLINE"
VERSION="0.1.0"
BUILD_DIR="build-macos"
DMG_OUTPUT="arcline-${VERSION}-macos-universal.dmg"

echo "==> Configuring ARCLINE Universal Binary build..."
cmake -B "${BUILD_DIR}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" \
    -DARCLINE_BUILD_TESTS=ON

echo "==> Compiling ARCLINE..."
cmake --build "${BUILD_DIR}" --config Release

echo "==> Running Automated Tests..."
ctest --test-dir "${BUILD_DIR}" --output-on-failure

APP_BUNDLE="${BUILD_DIR}/Arcline_artefacts/Release/${APP_NAME}.app"
if [ ! -d "${APP_BUNDLE}" ]; then
    echo "ERROR: Bundle not found at ${APP_BUNDLE}"
    exit 1
fi

echo "==> Creating DMG image..."
hdiutil create -volname "${APP_NAME}" -srcfolder "${APP_BUNDLE}" -ov -format UDZO "${DMG_OUTPUT}"
echo "==> Created ${DMG_OUTPUT} successfully."
