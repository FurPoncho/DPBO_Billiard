#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <string>
#include <cmath>
#include <vector>
#include "Koordinat.hpp"
#include "Bentuk.hpp"
#include "Bola.hpp"
using std::cout;
using std::endl;





class cue : public Bentuk {
    private:
    float panjang;
    float lebar;
    sf::RectangleShape wujud_cue;
    float sudut;
    float power;
    float jarak_mundur;
    float max_power;
    bool aktif_tusuk;
    bool sedang_ditarik;
    const Bola* Bola_Putih;

    public:
    // PERBAIKAN 1: Pakai != (bukan =)
    cue(const Bola* bola_putih, float pjg = 320.0f, float lbr = 8.0f, sf::Color w = sf::Color(193, 132, 68), float mp = 1500.f) 
    : Bentuk((bola_putih != nullptr) ? bola_putih->getposisi() : Koordinat(), w), 
      panjang(pjg), lebar(lbr), sudut(0.f), jarak_mundur(0.f), power(0.f), 
      max_power(mp), aktif_tusuk(true), sedang_ditarik(false), Bola_Putih(bola_putih) 
    {
        wujud_cue.setSize({panjang, lebar});
        wujud_cue.setFillColor(Warna);
        float radiusbola = (Bola_Putih != nullptr) ? Bola_Putih->getRadius() : 20.f;
        wujud_cue.setOrigin({panjang + radiusbola + 5.f, lebar / 2.f});
        wujud_cue.setPosition(getPosisiSFML());
    }

    void update(float deltatime) override {
        if(!aktif_tusuk || Bola_Putih == nullptr) return;
        Posisi_dia = Bola_Putih->getposisi();
        if(sedang_ditarik) {
            power += 1100.f * deltatime;
            if (power > max_power) {
                power = max_power;
            }
            jarak_mundur = (power / max_power) * 90.f;
        } else {
            jarak_mundur = 0.f;
        }
        float radiusBola = Bola_Putih->getRadius();
        wujud_cue.setOrigin({panjang + radiusBola + 5.f + jarak_mundur, lebar / 2.f});
        wujud_cue.setPosition(getPosisiSFML());
        wujud_cue.setRotation(sf::radians(sudut));
    }

    void kirim_ke_jendela(sf::RenderWindow& Jendela) override {
        if (aktif_tusuk && Bola_Putih != nullptr && Bola_Putih->isAktif()) {
            Jendela.draw(wujud_cue);
        }
    }

    // PERBAIKAN 2: Pakai tanda kurang (-) bukan (=)
    void ArahkankeMouse(sf::Vector2f PosisiMouse) {
        if (!aktif_tusuk || Bola_Putih == nullptr) return;

        float dx = PosisiMouse.x - Posisi_dia.x;
        float dy = PosisiMouse.y - Posisi_dia.y;
        sudut = std::atan2(dy, dx);
    }

    // PERBAIKAN 3: Cek !aktif_tusuk (bukan !sedang_ditarik)
    void mulaiTarik() {
        if (!aktif_tusuk) return;
        sedang_ditarik = true;
        power = 150.f;
    }

    sf::Vector2f lepas_tembak() {
        if(!aktif_tusuk || !sedang_ditarik) {
            return {0.f, 0.f};
        }
        sf::Vector2f Vektor_Dorong = {std::cos(sudut) * power, std::sin(sudut) * power};
        sedang_ditarik = false;
        power = 0.f;
        jarak_mundur = 0.f;
        aktif_tusuk = false;
        return Vektor_Dorong;
    }

    // Tekstur stik: ujung pukul harus di sisi KANAN gambar (sesuai cue.png).
    void setTexture(const sf::Texture* tekstur) {
        if (tekstur != nullptr) {
            wujud_cue.setTexture(tekstur);
            wujud_cue.setFillColor(sf::Color::White);
        }
    }

    void setaktif(bool status) {aktif_tusuk = status;}
    bool isaktif() const {return aktif_tusuk;}
    float getKekuatan() const {return power;}
    float getkekuatanmaks() const {return max_power;}
};