#!/bin/bash
# Builds the macOS installer package for Lumora Synth.
#
#   packaging/macos/build_pkg.sh <dist-dir> <version> <output.pkg>
#
# <dist-dir> must contain "Lumora Synth.vst3", "Lumora Synth.component" and
# "Lumora Synth.app" (already signed). The installer lets the user pick which
# of the three to install; all of them go to the system-wide locations.

set -euo pipefail

DIST="$1"
VERSION="$2"
OUT="$3"

HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# Builds one component package. Bundles are marked non-relocatable, otherwise
# the installer would "upgrade" a copy it finds elsewhere on disk instead of
# installing into the plug-in folder.
component() {
    local id="$1" bundle="$2" location="$3" scripts="${4:-}"
    local root="$WORK/root-$id"
    local plist="$WORK/$id.plist"

    mkdir -p "$root"
    ditto "$DIST/$bundle" "$root/$bundle"

    pkgbuild --analyze --root "$root" "$plist"
    local i=0
    while /usr/libexec/PlistBuddy -c "Print :$i" "$plist" >/dev/null 2>&1; do
        # plutil -replace also creates the key, which newer pkgbuild no longer writes.
        plutil -replace "$i.BundleIsRelocatable" -bool NO "$plist"
        i=$((i + 1))
    done

    local args=(--root "$root" --component-plist "$plist"
                --identifier "com.lumora.lumorasynth.$id" --version "$VERSION"
                --install-location "$location")
    if [ -n "$scripts" ]; then
        args+=(--scripts "$scripts")
    fi
    pkgbuild "${args[@]}" "$WORK/$id.pkg"
}

component vst3 "Lumora Synth.vst3"      "/Library/Audio/Plug-Ins/VST3"       "$HERE/scripts"
component au   "Lumora Synth.component" "/Library/Audio/Plug-Ins/Components" "$HERE/scripts"
component app  "Lumora Synth.app"       "/Applications"

mkdir -p "$WORK/resources"
cp "$HERE/resources/welcome.html" "$WORK/resources/"
cp "$HERE/../../LICENSE" "$WORK/resources/LICENSE.txt"
sed "s/@VERSION@/$VERSION/g" "$HERE/distribution.xml" > "$WORK/distribution.xml"

productbuild --distribution "$WORK/distribution.xml" \
             --resources "$WORK/resources" \
             --package-path "$WORK" \
             "$OUT"

echo "Built $OUT"
