#!/usr/bin/env bash
# Memasang SFML 3 dari source ke ~/sfml-3 (tidak menyentuh SFML 2 dari apt).
# Jalankan sekali:  bash pasang_sfml3.sh      (butuh sudo untuk apt install, +-5-10 menit)
set -euo pipefail

VERSI=3.0.2
PREFIX="$HOME/sfml-3"
SRC="$HOME/.cache/sfml-src"

echo "== 1/4 Memasang paket pendukung (butuh sudo)"
sudo apt update
sudo apt install -y build-essential cmake git pkg-config \
    libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libgl1-mesa-dev
sudo apt install -y libfreetype-dev || sudo apt install -y libfreetype6-dev

echo "== 2/4 Mengunduh SFML $VERSI"
rm -rf "$SRC"
git clone --depth 1 --branch "$VERSI" https://github.com/SFML/SFML.git "$SRC"

echo "== 3/4 Konfigurasi dan kompilasi (audio & network dimatikan agar tidak butuh OpenAL)"
cmake -S "$SRC" -B "$SRC/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=ON \
    -DSFML_BUILD_AUDIO=OFF -DSFML_BUILD_NETWORK=OFF \
    -DSFML_BUILD_EXAMPLES=OFF -DSFML_BUILD_DOC=OFF -DSFML_BUILD_TEST_SUITE=OFF
cmake --build "$SRC/build" -j"$(nproc)"

echo "== 4/4 Memasang ke $PREFIX"
cmake --install "$SRC/build"

echo
echo "Selesai. SFML 3 ada di $PREFIX"
echo "Sekarang jalankan di folder proyek:  make run"
