#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
BIN_DIR="${ROOT_DIR}/bin"
BUILD_TYPE="Release"
CLEAN=0
PACKAGE=0

for arg in "$@"; do
    case "$arg" in
        --clean)
            CLEAN=1
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --release)
            BUILD_TYPE="Release"
            ;;
        --package)
            PACKAGE=1
            ;;
        --help|-h)
            echo "Usage: $0 [--clean] [--debug|--release] [--package]"
            exit 0
            ;;
        *)
            echo "Unknown option: $arg" >&2
            exit 1
            ;;
    esac
done

if [[ "$CLEAN" -eq 1 ]]; then
    echo "Cleaning build/ and bin/..."
    rm -rf "$BUILD_DIR" "$BIN_DIR"
fi

echo "Configuring UltraCalc ($BUILD_TYPE)..."
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
if [[ -f "${BUILD_DIR}/compile_commands.json" ]]; then
    ln -sf "build/compile_commands.json" "${ROOT_DIR}/compile_commands.json"
fi

JOBS="$(nproc 2>/dev/null || echo 4)"
echo "Building targets with -j${JOBS}..."
cmake --build "$BUILD_DIR" -j"$JOBS"

mkdir -p "$BIN_DIR"

TARGETS=(
    "ultracalc-cli"
    "ultracalc-ext"
    "ultracalc-server"
    "ultracalc-gui"
    "ultracalc-tui"
)

for target in "${TARGETS[@]}"; do
    if [[ -f "${BUILD_DIR}/${target}" ]]; then
        cp "${BUILD_DIR}/${target}" "${BIN_DIR}/${target}"
        chmod +x "${BIN_DIR}/${target}"
    else
        echo "Warning: expected target ${BUILD_DIR}/${target} not found" >&2
    fi
done

cp "${ROOT_DIR}/guiqt.py" "${BIN_DIR}/guiqt.py"
chmod +x "${BIN_DIR}/guiqt.py"
cp "${ROOT_DIR}/uigen.html" "${BIN_DIR}/uigen.html"

echo "Build complete. Staged artifacts in ${BIN_DIR}/:"
ls -lh "$BIN_DIR"

if [[ "$PACKAGE" -eq 1 ]]; then
    DIST_DIR="${ROOT_DIR}/dist"
    ARCH="$(uname -m)"
    PKG_NAME="ultracalc-v1.0.0-linux-${ARCH}"
    PKG_STAGE="${DIST_DIR}/${PKG_NAME}"

    echo "Packaging release into ${DIST_DIR}..."
    rm -rf "$PKG_STAGE"
    mkdir -p "${PKG_STAGE}/bin"

    cp -a "${BIN_DIR}/"* "${PKG_STAGE}/bin/"
    rm -rf "${PKG_STAGE}/bin/__pycache__"
    cp "${ROOT_DIR}/README.md" "${PKG_STAGE}/"
    cp "${ROOT_DIR}/ultracalcAPI.md" "${PKG_STAGE}/"
    cp "${ROOT_DIR}/CALCULATOR_GUIDE.md" "${PKG_STAGE}/"
    cp "${ROOT_DIR}/exmanifest.json" "${PKG_STAGE}/"

    cd "$DIST_DIR"
    tar -czf "${PKG_NAME}.tar.gz" "${PKG_NAME}"
    if command -v zip >/dev/null 2>&1; then
        zip -rq "${PKG_NAME}.zip" "${PKG_NAME}"
    else
        python3 -m zipfile -c "${PKG_NAME}.zip" "${PKG_NAME}"
    fi
    rm -rf "${PKG_NAME}"
    cd "$ROOT_DIR"

    echo "Release packages created in ${DIST_DIR}:"
    ls -lh "${DIST_DIR}"
fi
