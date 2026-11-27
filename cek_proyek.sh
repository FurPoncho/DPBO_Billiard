#!/usr/bin/env bash
# Cek kelengkapan proyek DPBO_Billiard. Jalankan dari root proyek:  bash cek_proyek.sh
ok(){ echo "  [OK]  $1"; }; bad(){ echo "  [!!]  $1"; BAD=1; }
BAD=0

echo "== File kode yang dibutuhkan game utama"
for f in Main_Gameplay.cpp game_logic.cpp game_logic.H Bentuk.hpp Bola.hpp cue.hpp Koordinat.hpp Lubang.hpp Meja.hpp; do
  [ -f "$f" ] && ok "$f" || bad "$f TIDAK ADA"
done

echo "== Versi file (harus versi terbaru dari Claude)"
grep -q "ambilDanResetRel" Bola.hpp 2>/dev/null        && ok "Bola.hpp terbaru"        || bad "Bola.hpp masih versi lama"
grep -q "pasangTekstur" Meja.hpp 2>/dev/null           && ok "Meja.hpp terbaru"        || bad "Meja.hpp masih versi lama"
grep -q "void setTexture" cue.hpp 2>/dev/null          && ok "cue.hpp terbaru"         || bad "cue.hpp masih versi lama"
grep -q "onShotTaken(const std::vector<Bola>" game_logic.H 2>/dev/null && ok "game_logic.H terbaru" || bad "game_logic.H masih versi lama"
grep -q "susunBola" Main_Gameplay.cpp 2>/dev/null      && ok "Main_Gameplay.cpp terbaru" || bad "Main_Gameplay.cpp masih versi lama"

echo "== Huruf besar/kecil nama include (Linux/WSL membedakan)"
for inc in $(grep -hoE '#include "[^"]+"' Main_Gameplay.cpp game_logic.cpp *.hpp game_logic.H 2>/dev/null | sed 's/#include "//; s/"//' | sort -u); do
  [ -f "$inc" ] && ok "$inc" || bad "#include \"$inc\" -> file tidak ditemukan (cek huruf besar/kecil)"
done

echo "== Aset gambar"
for f in background table cue cue_ball $(seq -f "ball_%g" 1 15); do
  [ -f "assets/images/$f.png" ] && true || { bad "assets/images/$f.png TIDAK ADA"; }
done
[ $BAD -eq 0 ] && ok "semua gambar ada"

echo "== SFML"
if command -v pkg-config >/dev/null && pkg-config --exists sfml-graphics; then
  v=$(pkg-config --modversion sfml-graphics); echo "  versi terpasang: $v"
  case "$v" in 3.*) ok "SFML 3.x";; *) bad "kode ini butuh SFML 3.x (sf::State, Event::getIf); yang terpasang $v";; esac
else
  bad "pkg-config tidak menemukan sfml-graphics (SFML belum terpasang / bukan di /usr)"
fi

echo "== File .cpp dengan main() (jangan dikompilasi bersamaan)"
grep -lE '^\s*int\s+main\s*\(' *.cpp 2>/dev/null | sed 's/^/  - /'

echo; [ $BAD -eq 0 ] && echo "Semua beres. Jalankan: make run" || echo "Ada yang perlu dibereskan (lihat [!!] di atas)."
