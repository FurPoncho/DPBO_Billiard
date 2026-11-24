# DPBO_Billiard

Game biliar 8-Ball 2D berbasis **C++17 dan SFML 3**, dibuat untuk tugas mata kuliah Desain dan Pemrograman Berorientasi Objek (DPBO). Proyek ini menerapkan konsep OOP (inheritance, polymorphism, encapsulation) pada simulasi fisika bola biliar beserta aturan permainan 8-Ball.

## Fitur

- Meja biliar dengan dinding dan 6 lubang
- Bola yang bergerak dengan gesekan kain meja dan pantulan dinding
- Tabrakan antar-bola (pertukaran momentum)
- Stik biliar yang mengikuti kursor mouse, kekuatan pukulan diatur dengan menahan klik kiri
- Deteksi bola masuk lubang lewat class `Lubang`
- Logika aturan 8-Ball lengkap (`GameLogic`): giliran pemain, penentuan grup Polos/Stripes, deteksi foul, skor, dan kondisi menang/kalah

## Cara Bermain

| Aksi | Kontrol |
|------|---------|
| Mengarahkan stik | Gerakkan mouse |
| Mengisi kekuatan | Tahan klik kiri |
| Menembak | Lepas klik kiri |

Stik akan muncul kembali setelah semua bola berhenti bergerak dan giliran pemain berikutnya dimulai. Status giliran, foul, dan skor dicetak ke terminal setiap kali fase permainan berubah.

## Struktur Class

```
Bentuk (abstract)
├── Meja      : meja, dinding, dan mulut lubang (batas area bermain)
├── Bola      : fisika gerak, gesekan, pantulan dinding, tabrakan antar-bola
├── cue       : stik, arah bidikan, dan kekuatan pukulan
└── Lubang    : deteksi bola masuk lubang

Koordinat     : posisi (x, y)
GameLogic     : wasit permainan (giliran, grup bola, foul, skor, menang/kalah)
```

## Struktur Proyek

```
.
├── Main_Gameplay.cpp    # Program utama (entry point game)
├── game_logic.cpp/.H    # Aturan permainan 8-Ball (GameLogic)
├── Bentuk.hpp           # Class dasar abstrak
├── Bola.hpp             # Bola biliar
├── Meja.hpp             # Meja dan dinding
├── Lubang.hpp           # Lubang meja
├── cue.hpp              # Stik pemukul
├── Koordinat.hpp        # Struktur posisi (x, y)
├── assets/images/       # Aset gambar meja, bola, dan stik (PNG)
├── font/                # Font yang dipakai UI
├── pasang_sfml3.sh      # Script sekali-jalan untuk memasang SFML 3 ke ~/sfml-3
├── cek_proyek.sh        # Script diagnosis kelengkapan proyek
└── Makefile
```

> `MAIIN.cpp` dan `Template_UI_Code_For_Pasha_To_Use.cpp` adalah prototipe/eksperimen lama, masing-masing punya `main()` sendiri dan **tidak** boleh dikompilasi bersamaan dengan `Main_Gameplay.cpp`.

## Persyaratan

- Compiler C++17 atau lebih baru (g++)
- [SFML 3.x](https://www.sfml-dev.org/) — kode memakai API SFML 3 (`sf::State`, `Event::getIf`, dll), **tidak kompatibel** dengan SFML 2

### Memasang SFML 3 (WSL/Ubuntu)

Paket `libsfml-dev` di repo Ubuntu umumnya masih versi 2.x, jadi pasang SFML 3 dari source memakai script yang sudah disediakan (dibangun ke `~/sfml-3`, tidak mengganggu SFML 2 yang mungkin sudah terpasang dari apt):

```bash
bash pasang_sfml3.sh
```

## Kompilasi dan Menjalankan

```bash
make run SFML_PREFIX=$HOME/sfml-3
```

Atau manual:

```bash
g++ -std=c++17 -I$HOME/sfml-3/include Main_Gameplay.cpp game_logic.cpp -o billiard \
    -L$HOME/sfml-3/lib -Wl,-rpath,$HOME/sfml-3/lib -lsfml-graphics -lsfml-window -lsfml-system
./billiard
```

Jalankan `bash cek_proyek.sh` untuk memeriksa kelengkapan file dan aset sebelum build.