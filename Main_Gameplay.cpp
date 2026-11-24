#include <SFML/Graphics.hpp>
#include <iostream>
#include <optional>
#include <vector>

#include "Koordinat.hpp"
#include "Bentuk.hpp"
#include "Meja.hpp"
#include "Bola.hpp"
#include "Lubang.hpp"
#include "Cue.hpp"
#include "game_logic.H"

int main() {
    sf::RenderWindow Jendela(sf::VideoMode(sf::Vector2u{1920, 1080}), "Simulasi Billiard", sf::Style::Default, sf::State::Windowed);
    Jendela.setFramerateLimit(60);

    Meja mejaBilliard(Koordinat(260.f, 190.f), 700.f, 1400.f, 40.f);

    float kiri   = mejaBilliard.getbataskiri();
    float kanan  = mejaBilliard.getbataskanan();
    float atas   = mejaBilliard.getbatasatas();
    float bawah  = mejaBilliard.getbatasbawah();
    float tengah = (kiri + kanan) / 2.0f;

    std::vector<Lubang> lubangBilliard = {
        Lubang(Koordinat(kiri, atas),    30.f, sf::Color::Black, "Kiri Atas"),
        Lubang(Koordinat(tengah, atas),  30.f, sf::Color::Black, "Tengah Atas"),
        Lubang(Koordinat(kanan, atas),   30.f, sf::Color::Black, "Kanan Atas"),
        Lubang(Koordinat(kiri, bawah),   30.f, sf::Color::Black, "Kiri Bawah"),
        Lubang(Koordinat(tengah, bawah), 30.f, sf::Color::Black, "Tengah Bawah"),
        Lubang(Koordinat(kanan, bawah),  30.f, sf::Color::Black, "Kanan Bawah")
    };

    // Index 0 wajib bola putih (nomor 0), sisanya bola sasaran sementara buat ngetes skor & giliran
    Koordinat titikAwalPutih(610.f, 540.f);
    std::vector<Bola> daftarBola = {
        Bola(titikAwalPutih,        20.f, sf::Color::White,  "Bola Putih",  0, &mejaBilliard),
        Bola(Koordinat(1310.f, 540.f), 20.f, sf::Color::Red,    "Bola Merah",  1, &mejaBilliard),
        Bola(Koordinat(1350.f, 515.f), 20.f, sf::Color::Yellow, "Bola Kuning", 2, &mejaBilliard),
        Bola(Koordinat(1350.f, 565.f), 20.f, sf::Color::Blue,   "Bola Biru",   3, &mejaBilliard)
    };

    cue stick(&daftarBola[0]);
    GameLogic wasit(lubangBilliard, titikAwalPutih);
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
                        wasit.onShotTaken();
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
                if (wasit.wasFoul()) std::cout << "[FOUL!] Bola putih masuk lubang.\n";
                std::cout << "Giliran Player " << wasit.getCurrentPlayer()
                          << " | Skor P1: " << wasit.getScore(1)
                          << " - P2: " << wasit.getScore(2) << "\n";
            } else if (stateSebelumnya == GameState::GameOver) {
                std::cout << "=== GAME OVER === Pemenang: Player " << wasit.getWinner()
                          << " (Skor P1: " << wasit.getScore(1)
                          << " - P2: " << wasit.getScore(2) << ")\n";
            }
        }

        Jendela.clear(sf::Color(30, 30, 30));
        mejaBilliard.kirim_ke_jendela(Jendela);

        for (size_t i = 0; i < lubangBilliard.size(); ++i) {
            lubangBilliard[i].kirim_ke_jendela(Jendela);
        }
        for (size_t i = 0; i < daftarBola.size(); ++i) {
            daftarBola[i].kirim_ke_jendela(Jendela);
        }
        stick.kirim_ke_jendela(Jendela);

        Jendela.display();
    }

    return 0;
}