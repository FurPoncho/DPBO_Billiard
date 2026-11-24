#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <iostream>
#include <optional>
#include <vector>
#include "Koordinat.hpp"
#include "Bentuk.hpp"
#include "Meja.hpp"
#include "Bola.hpp"
#include "Lubang.hpp"
#include "cue.hpp"
#include "game_logic.H"

namespace {

constexpr float RADIUS_BOLA = 20.f;

// ====== Geometri sesuai assets/images/table.png (1820x1028 px) ======
// Gambar meja digambar apa adanya (skala 1:1) di tengah jendela 1920x1080.
// Semua angka di bawah hasil ukur piksel pada gambar, lalu digeser OFFSET_MEJA.
const Koordinat OFFSET_MEJA(50.f, 26.f);

// Area kain (batas pantul bola) = tepi dalam bantalan hijau muda: x 117..1703, y 117..911 di gambar.
const float KAIN_KIRI = OFFSET_MEJA.x + 117.f;    // 167
const float KAIN_ATAS = OFFSET_MEJA.y + 117.f;    // 143
const float KAIN_LEBAR  = 1586.f;
const float KAIN_TINGGI = 794.f;

// Garis putih di gambar (x=1348) ada di SISI KANAN, jadi break dilakukan dari kanan
// dan segitiga disusun di sisi kiri (foot spot = 1/4 panjang kain dari kiri).
const float GARIS_BREAK_X = OFFSET_MEJA.x + 1348.f;                  // 1398
const float FOOT_SPOT_X   = KAIN_KIRI + 0.25f * KAIN_LEBAR;         // 563.5
const float TENGAH_Y      = KAIN_ATAS + KAIN_TINGGI / 2.f;           // 540

struct DataLubang { float x, y, rTangkap, rMulut; const char* nama; };
// rTangkap = bola dianggap masuk kalau pusatnya sedekat ini; rMulut = area di mana dinding dimatikan.
const DataLubang DATA_LUBANG[6] = {
    { OFFSET_MEJA.x +   85.f, OFFSET_MEJA.y +  97.f, 44.f, 110.f, "Kiri Atas"    },
    { OFFSET_MEJA.x + 899.5f, OFFSET_MEJA.y +  74.f, 38.f, 100.f, "Tengah Atas"  },
    { OFFSET_MEJA.x + 1722.f, OFFSET_MEJA.y +  96.f, 44.f, 110.f, "Kanan Atas"   },
    { OFFSET_MEJA.x +   85.f, OFFSET_MEJA.y + 934.f, 44.f, 110.f, "Kiri Bawah"   },
    { OFFSET_MEJA.x + 899.5f, OFFSET_MEJA.y + 953.f, 38.f, 100.f, "Tengah Bawah" },
    { OFFSET_MEJA.x + 1722.f, OFFSET_MEJA.y + 934.f, 44.f, 110.f, "Kanan Bawah"  }
};

bool muatTekstur(sf::Texture& tekstur, const std::string& path) {
    if (!tekstur.loadFromFile(path)) {
        std::cerr << "[ASET] Gagal memuat " << path
                  << " (jalankan program dari folder yang berisi assets/)\n";
        return false;
    }
    tekstur.setSmooth(true);
    return true;
}

// Warna dasar bola 1-7 (polos). Bola 9-15 memakai warna yang sama tapi dicerahkan
// sebagai penanda sementara sebelum tekstur stripes dipasang (Langkah 5).
sf::Color warnaDasar(int nomor) {
    switch (nomor) {
        case 1: return sf::Color(255, 205, 0);   // kuning
        case 2: return sf::Color(0, 70, 180);    // biru
        case 3: return sf::Color(210, 30, 30);   // merah
        case 4: return sf::Color(90, 40, 140);   // ungu
        case 5: return sf::Color(240, 120, 20);  // oranye
        case 6: return sf::Color(0, 120, 60);    // hijau
        case 7: return sf::Color(120, 20, 30);   // marun
        default: return sf::Color::Black;        // bola 8
    }
}

sf::Color warnaBola(int nomor) {
    if (nomor == 8) return sf::Color(20, 20, 20);
    if (nomor <= 7) return warnaDasar(nomor);
    sf::Color c = warnaDasar(nomor - 8);
    // campur 55% putih supaya terlihat beda dari bola polos
    auto cerah = [](std::uint8_t v) { return static_cast<std::uint8_t>(v + (255 - v) * 0.55f); };
    return sf::Color(cerah(c.r), cerah(c.g), cerah(c.b));
}

std::string namaBola(int nomor) {
    if (nomor == 0) return "Bola Putih";
    if (nomor == 8) return "Bola 8";
    return (nomor < 8 ? "Bola Polos " : "Bola Stripes ") + std::to_string(nomor);
}

// Susunan nomor bola per baris (apex -> belakang).
// Aturan 8-Ball: bola 8 di tengah baris ke-3, dua sudut belakang
// harus satu polos dan satu stripes.
const int FORMASI[5][5] = {
    { 1 },
    { 9, 7 },
    { 12, 8, 2 },
    { 10, 6, 13, 3 },
    { 4, 14, 15, 5, 11 }
};

// Mengisi (ulang) daftarBola: indeks 0 = bola putih, indeks 1..15 = bola sasaran.
// Dipakai juga untuk Restart (Langkah 4).
// PENTING: reserve(16) menjaga alamat memori elemen tetap sama, jadi pointer
// &daftarBola[0] yang dipegang objek cue tidak jadi dangling setelah clear().
// 'tekstur'[n] = tekstur bola nomor n (0 = putih); nullptr -> pakai warna polos.
// 'arah' = +1 kalau baris segitiga melebar ke kanan dari apex, -1 kalau ke kiri.
void susunBola(std::vector<Bola>& daftarBola, const Meja* meja,
               Koordinat titikAwalPutih, Koordinat titikApex,
               const std::array<const sf::Texture*, 16>& tekstur, float arah) {
    daftarBola.reserve(16);
    daftarBola.clear();

    daftarBola.emplace_back(titikAwalPutih, RADIUS_BOLA, sf::Color::White,
                            namaBola(0), 0, meja, tekstur[0]);

    const float jarakKolom = std::sqrt(3.f) * RADIUS_BOLA;  // jarak antar baris segitiga
    const float jarakBola  = 2.f * RADIUS_BOLA + 0.5f;      // celah kecil biar tidak overlap

    for (int kolom = 0; kolom < 5; ++kolom) {
        const int jumlah = kolom + 1;
        for (int k = 0; k < jumlah; ++k) {
            const int nomor = FORMASI[kolom][k];
            const float x = titikApex.x + arah * kolom * (jarakKolom + 0.5f);
            const float y = titikApex.y + (k - kolom / 2.f) * jarakBola;
            daftarBola.emplace_back(Koordinat(x, y), RADIUS_BOLA, warnaBola(nomor),
                                    namaBola(nomor), nomor, meja, tekstur[nomor]);
        }
    }
}

}  // namespace

int main() {
    sf::RenderWindow Jendela(sf::VideoMode(sf::Vector2u{1920, 1080}), "Simulasi Billiard", sf::Style::Default, sf::State::Windowed);
    Jendela.setFramerateLimit(60);

    // ---- Muat aset gambar ----
    sf::Texture texLatar, texMeja, texCue;
    std::array<sf::Texture, 16> texBola;
    std::array<const sf::Texture*, 16> ptrBola;
    ptrBola.fill(nullptr);

    const bool adaLatar = muatTekstur(texLatar, "assets/images/background.png");
    const bool adaMeja  = muatTekstur(texMeja,  "assets/images/table.png");
    const bool adaCue   = muatTekstur(texCue,   "assets/images/cue.png");
    for (int n = 0; n < 16; ++n) {
        std::string file = (n == 0) ? "assets/images/cue_ball.png"
                                    : "assets/images/ball_" + std::to_string(n) + ".png";
        if (muatTekstur(texBola[n], file)) ptrBola[n] = &texBola[n];
    }

    std::optional<sf::Sprite> latar;
    if (adaLatar) latar.emplace(texLatar);

    // ---- Meja & lubang ----
    Meja mejaBilliard(Koordinat(KAIN_KIRI, KAIN_ATAS), KAIN_TINGGI, KAIN_LEBAR, 40.f);
    if (adaMeja) mejaBilliard.pasangTekstur(texMeja, OFFSET_MEJA);

    std::vector<Lubang> lubangBilliard;
    for (int i = 0; i < 6; ++i) {
        const DataLubang& d = DATA_LUBANG[i];
        lubangBilliard.emplace_back(Koordinat(d.x, d.y), d.rTangkap, sf::Color::Black, d.nama);
        mejaBilliard.tambahMulutLubang(Koordinat(d.x, d.y), d.rMulut);
    }

    // Bola putih di garis break (kanan), apex segitiga di foot spot (kiri), baris melebar ke kiri.
    Koordinat titikAwalPutih(GARIS_BREAK_X, TENGAH_Y);
    Koordinat titikApex(FOOT_SPOT_X, TENGAH_Y);

    std::vector<Bola> daftarBola;
    susunBola(daftarBola, &mejaBilliard, titikAwalPutih, titikApex, ptrBola, -1.f);

    cue stick(&daftarBola[0], 480.f, 9.f);   // 480x9 = proporsi cue.png (640x12)
    if (adaCue) stick.setTexture(&texCue);
    GameLogic wasit(lubangBilliard, titikAwalPutih, titikApex);
    GameState stateSebelumnya = GameState::BallsMoving;
    sf::Clock pewaktu;

    while (Jendela.isOpen()) {
        sf::Vector2i posisiPixelMouse = sf::Mouse::getPosition(Jendela);
        sf::Vector2f posisiDuniaMouse = Jendela.mapPixelToCoords(posisiPixelMouse);

        if (wasit.getState() == GameState::Aiming) {
            stick.setaktif(true);
            stick.ArahkankeMouse(posisiDuniaMouse);
        }

        float deltatime = pewaktu.restart().asSeconds();
        while (const std::optional<sf::Event> Kejadian = Jendela.pollEvent()) {
            if (Kejadian->is<sf::Event::Closed>()) {
                Jendela.close();
            } else if (const auto* Tekan = Kejadian->getIf<sf::Event::MouseButtonPressed>()) {
                if (Tekan->button == sf::Mouse::Button::Left && wasit.getState() == GameState::Aiming) {
                    stick.mulaiTarik();
                }
            } else if (const auto* Lepas = Kejadian->getIf<sf::Event::MouseButtonReleased>()) {
                if (Lepas->button == sf::Mouse::Button::Left && wasit.getState() == GameState::Aiming) {
                    sf::Vector2f dorongan = stick.lepas_tembak();
                    if (dorongan.x != 0.f || dorongan.y != 0.f) {
                        daftarBola[0].pukul(dorongan);
                        wasit.onShotTaken(daftarBola);
                    }
                }
            }
        }

        wasit.update(daftarBola, deltatime);
        stick.update(deltatime);

        // Log status ke terminal setiap kali fase giliran berubah
        if (wasit.getState() != stateSebelumnya) {
            stateSebelumnya = wasit.getState();
            if (stateSebelumnya == GameState::Aiming) {
                if (wasit.wasFoul()) std::cout << "[FOUL!] " << wasit.getFoulReason() << "\n";
                std::cout << "Giliran Player " << wasit.getCurrentPlayer() << " | "
                          << (wasit.isTableOpen() ? "Meja terbuka"
                              : (wasit.getGrupPemain(wasit.getCurrentPlayer()) == GrupBola::Polos
                                     ? "Grup: Polos (1-7)" : "Grup: Stripes (9-15)"))
                          << " | Skor P1: " << wasit.getScore(1)
                          << " - P2: " << wasit.getScore(2) << "\n";
            } else if (stateSebelumnya == GameState::GameOver) {
                std::cout << "=== GAME OVER === Pemenang: Player " << wasit.getWinner()
                          << " | " << wasit.getGameOverReason()
                          << " (Skor P1: " << wasit.getScore(1)
                          << " - P2: " << wasit.getScore(2) << ")\n";
            }
        }

        Jendela.clear(sf::Color(30, 30, 30));
        if (latar) Jendela.draw(*latar);
        mejaBilliard.kirim_ke_jendela(Jendela);

        // Lubang sudah tergambar di table.png; gambar manual hanya kalau tekstur meja gagal dimuat
        if (!adaMeja) {
            for (size_t i = 0; i < lubangBilliard.size(); ++i) {
                lubangBilliard[i].kirim_ke_jendela(Jendela);
            }
        }
        for (size_t i = 0; i < daftarBola.size(); ++i) {
            daftarBola[i].kirim_ke_jendela(Jendela);
        }
        stick.kirim_ke_jendela(Jendela);

        Jendela.display();
    }

    return 0;
}