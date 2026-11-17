# Poncho-First-public-repo
Our first public repository which is also our first ever repository made in Github.
# DPBO_Billiard
SFML based Game



# DPBO_Billiard

Game biliar 2D berbasis **C++ dan SFML**, dibuat untuk tugas mata kuliah Desain dan Pemrograman Berorientasi Objek (DPBO). Proyek ini menerapkan konsep OOP (inheritance, polymorphism, encapsulation) pada simulasi fisika bola biliar sederhana.

## Fitur

- Meja biliar dengan dinding dan 6 lubang
- Bola yang bergerak dengan gesekan kain meja
- Pantulan bola pada dinding meja
- Tabrakan antar-bola (pertukaran momentum)
- Stik biliar yang mengikuti kursor mouse
- Kekuatan pukulan diatur dengan menahan tombol mouse
- Bola yang masuk lubang terdeteksi lewat class `Lubang`

## Cara Bermain

| Aksi | Kontrol |
|------|---------|
| Mengarahkan stik | Gerakkan mouse |
| Mengisi kekuatan | Tahan klik kiri |
| Menembak | Lepas klik kiri |

Stik akan muncul kembali setelah semua bola berhenti bergerak.

## Struktur Class

```
Bentuk (abstract)
├── Meja      : meja dan dinding, menyimpan batas area bermain
├── Bola      : fisika gerak, pantulan dinding, tabrakan antar-bola
├── cue       : stik, arah, dan kekuatan pukulan
└── Lubang    : deteksi bola masuk lubang

Koordinat     : posisi (x, y)
```

## Struktur Proyek

```
.
├── Fisika_Game.cpp      # Program utama dan logika game
├── assets/images/       # Aset gambar (PNG)
├── index.html           # Template tampilan UI
├── globals.css
├── style.css
└── README.md
```

## Persyaratan

- Compiler C++17 atau lebih baru (g++)
- [SFML 3.x](https://www.sfml-dev.org/)

Instal SFML di Ubuntu/WSL:

```bash
sudo apt update
sudo apt install libsfml-dev
```

Pastikan versi SFML yang terpasang adalah 3.x, karena kode memakai API baru seperti `sf::State::Windowed` dan `Event::getIf`.

## Kompilasi dan Menjalankan

```bash
g++ -std=c++17 Fisika_Game.cpp -o Fisika_Game -lsfml-graphics -lsfml-window -lsfml-system
./Fisika_Game
```



