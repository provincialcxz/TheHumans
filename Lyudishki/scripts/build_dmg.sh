#!/bin/bash
set -e

# --- Lyudishki macOS DMG Installer Builder (Mr. Robot style) ---

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${1:-$PROJECT_DIR/build-release}"
APP_NAME="Lyudishki"
DMG_NAME="${APP_NAME}-macOS"
VERSION="1.0.0"
QT_DIR="$(brew --prefix qt)"
DMG_BG="$PROJECT_DIR/resources/icons/dmg_background.png"
DMG_BG_2X="$PROJECT_DIR/resources/icons/dmg_background@2x.png"

echo "=== $APP_NAME DMG Builder (Mr. Robot Edition) ==="
echo ""

# Step 1: Build Release
echo "[1/6] Building Release..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$PROJECT_DIR" \
    -DCMAKE_PREFIX_PATH="$QT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0
cmake --build . --config Release -- -j$(sysctl -n hw.ncpu)
echo "  ✓ Build OK"

# Step 2: macdeployqt
echo "[2/6] Bundling Qt frameworks..."
"$QT_DIR/bin/macdeployqt" "$BUILD_DIR/$APP_NAME.app" \
    -always-overwrite \
    -libpath="$QT_DIR/lib" 2>&1 | grep -v "^ERROR:" || true
echo "  ✓ Frameworks bundled"

# Step 3: Set icon (must happen BEFORE signing — editing Info.plist after
# codesign invalidates the signature)
echo "[3/7] Setting app icon..."
ICNS="$PROJECT_DIR/resources/icons/Lyudishki.icns"
if [ -f "$ICNS" ]; then
    cp "$ICNS" "$BUILD_DIR/$APP_NAME.app/Contents/Resources/Lyudishki.icns"
    /usr/libexec/PlistBuddy -c "Set :CFBundleIconFile Lyudishki" \
        "$BUILD_DIR/$APP_NAME.app/Contents/Info.plist" 2>/dev/null || \
    /usr/libexec/PlistBuddy -c "Add :CFBundleIconFile string Lyudishki" \
        "$BUILD_DIR/$APP_NAME.app/Contents/Info.plist"
fi
echo "  ✓ Icon set"

# Step 4: Re-sign the app (macdeployqt invalidates signature; this must be
# the last modification to the bundle before signing)
echo "[4/7] Re-signing app bundle..."
chmod -R u+w "$BUILD_DIR/$APP_NAME.app"
xattr -cr "$BUILD_DIR/$APP_NAME.app"
codesign --force --deep --sign - "$BUILD_DIR/$APP_NAME.app"
echo "  ✓ Signed (ad-hoc)"

# Step 5: Create temp R/W DMG
echo "[5/7] Creating styled DMG..."
DMG_STAGING="$BUILD_DIR/dmg-staging"
DMG_TMP="$BUILD_DIR/tmp_rw.dmg"
DMG_OUTPUT="$BUILD_DIR/$DMG_NAME-$VERSION.dmg"

rm -rf "$DMG_STAGING" "$DMG_TMP" "$DMG_OUTPUT"
mkdir -p "$DMG_STAGING/.background"

cp -R "$BUILD_DIR/$APP_NAME.app" "$DMG_STAGING/"
ln -s /Applications "$DMG_STAGING/Applications"
cp "$DMG_BG" "$DMG_STAGING/.background/background.png"
cp "$DMG_BG_2X" "$DMG_STAGING/.background/background@2x.png"

# Create R/W DMG
hdiutil create -srcfolder "$DMG_STAGING" -volname "$APP_NAME" \
    -fs HFS+ -fsargs "-c c=64,a=16,e=16" \
    -format UDRW "$DMG_TMP" -ov

# Step 6: Apply styling via AppleScript
echo "[6/7] Applying Mr. Robot styling..."
MOUNT_DIR="/Volumes/$APP_NAME"

# Unmount if already mounted
hdiutil detach "$MOUNT_DIR" 2>/dev/null || true

hdiutil attach "$DMG_TMP" -readwrite -noverify -noautoopen

# Wait for mount
sleep 2

osascript <<APPLESCRIPT
tell application "Finder"
    tell disk "$APP_NAME"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set bounds of container window to {100, 100, 760, 500}

        set theViewOptions to the icon view options of container window
        set arrangement of theViewOptions to not arranged
        set icon size of theViewOptions to 96
        set background picture of theViewOptions to file ".background:background.png"

        -- Position icons: App on left circle, Applications on right circle
        set position of item "$APP_NAME.app" of container window to {200, 220}
        set position of item "Applications" of container window to {460, 220}

        close
        open
        update without registering applications
        delay 2
        close
    end tell
end tell
APPLESCRIPT

# Sync and detach
sync
hdiutil detach "$MOUNT_DIR"

# Step 7: Convert to compressed DMG
echo "[7/7] Compressing final DMG..."
hdiutil convert "$DMG_TMP" -format UDZO -imagekey zlib-level=9 -o "$DMG_OUTPUT"
rm -f "$DMG_TMP"
rm -rf "$DMG_STAGING"

echo ""
echo "Verifying signature..."
if codesign -v "$BUILD_DIR/$APP_NAME.app" 2>&1; then
    echo "  ✓ Signature valid"
else
    echo "  ✗ SIGNATURE INVALID — the app will be killed by Gatekeeper on launch!"
    exit 1
fi

echo ""
echo "=== Done ==="
echo "DMG: $DMG_OUTPUT"
echo "Size: $(du -h "$DMG_OUTPUT" | cut -f1)"
