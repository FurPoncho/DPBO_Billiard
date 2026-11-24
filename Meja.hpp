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
using std::cout;
using std::endl;



class Meja : public Bentuk {
    private:
    float panjang;
    float lebar;
    float tebal_dinding;
    sf::RectangleShape wujud_meja;
    // Tekstur meja (opsional). Kalau terpasang, sprite ini yang digambar, bukan persegi hijau.
    std::optional<sf::Sprite> sprite_;
    // Area "mulut" lubang: di dalam area ini dinding tidak memantulkan bola,
    // supaya bola bisa lewat ke lubang yang letaknya di luar tepi kain.
    struct MulutLubang { float x, y, r; };
    std::vector<MulutLubang> mulut_;
    public:
    Meja(Koordinat posisi_kiri_atas, float p, float l, float tb= 30.f ) : 
    Bentuk(posisi_kiri_atas, sf::Color(20, 108, 44)), panjang(p), lebar(l), tebal_dinding(tb) 
    {
        wujud_meja.setSize({lebar, panjang});
        wujud_meja.setFillColor(Warna);
        wujud_meja.setPosition(getPosisiSFML());
        wujud_meja.setOutlineThickness(tebal_dinding);
        wujud_meja.setOutlineColor(sf::Color(101, 55, 24));
    }
    void update(float deltatime) override {}
    void kirim_ke_jendela(sf::RenderWindow& Jendela) override {
        if (sprite_) Jendela.draw(*sprite_);
        else         Jendela.draw(wujud_meja);
    }
    // 'tekstur' harus tetap hidup selama Meja dipakai (sprite hanya menyimpan pointer).
    void pasangTekstur(const sf::Texture& tekstur, Koordinat posisiGambar) {
        sprite_.emplace(tekstur);
        sprite_->setPosition({posisiGambar.x, posisiGambar.y});
    }
    bool punyaTekstur() const { return sprite_.has_value(); }
    void tambahMulutLubang(Koordinat pusat, float radius) { mulut_.push_back({pusat.x, pusat.y, radius}); }
    // True kalau titik (x,y) ada di mulut sebuah lubang DAN bergerak menuju pusat lubang itu
    // (sudut antara arah gerak dan arah ke lubang <= ~45 derajat). Bola yang hanya lewat
    // atau menjauh tetap dipantulkan dinding seperti biasa.
    bool dekatMulutLubang(float x, float y, float vx, float vy) const {
        const float kecepatan2 = vx * vx + vy * vy;
        for (size_t i = 0; i < mulut_.size(); ++i) {
            float dx = mulut_[i].x - x, dy = mulut_[i].y - y;      // arah ke pusat lubang
            float jarak2 = dx * dx + dy * dy;
            if (jarak2 > mulut_[i].r * mulut_[i].r) continue;
            if (kecepatan2 < 1.f) return true;                      // bola diam di mulut: jangan dipantulkan
            float dot = dx * vx + dy * vy;
            // cos(sudut) >= 0.7  <=>  dot >= 0.7 * |d| * |v|
            if (dot > 0.f && dot * dot >= 0.49f * jarak2 * kecepatan2) return true;
        }
        return false;
    }
    float getbataskiri() const{return Posisi_dia.x;}
    float getbataskanan() const{return Posisi_dia.x + lebar;}
    float getbatasatas() const{return Posisi_dia.y;}
    float getbatasbawah() const {return Posisi_dia.y + panjang;}
};