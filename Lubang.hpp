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



class Lubang : public Bentuk{
    private:
    float radius;
    sf::CircleShape wujud_lubang;
    std:: string posisi_lubang;
    public:
    Lubang(Koordinat pos = Koordinat(), float r = 30.f, sf::Color w = sf::Color::Black, std::string nama = "Lubang")
    : Bentuk(pos, w), radius(r), posisi_lubang(nama) {
        wujud_lubang.setRadius(radius);
        wujud_lubang.setFillColor(Warna);
        wujud_lubang.setOrigin({radius, radius});
        wujud_lubang.setPosition(getPosisiSFML());

        wujud_lubang.setOutlineThickness(3.f);
        wujud_lubang.setOutlineColor(sf::Color(45, 25, 12));

    }
    void update(float deltatime) override {}
    void kirim_ke_jendela(sf::RenderWindow& Jendela) override {
        Jendela.draw(wujud_lubang);
    }

    bool  Apakah_bola_masuk(const Bola& bola) const{
        if (!bola.isAktif()) return false;
        float dx = bola.getposisi().x - Posisi_dia.x;
        float dy = bola.getposisi().y - Posisi_dia.y;
        float jarakKuadrat = (dx * dx) + (dy * dy);

        return jarakKuadrat <= (radius * radius);
    }
    float getRadius() const { return radius; }
    std::string getPosisiLubang() const { return posisi_lubang; }
};
