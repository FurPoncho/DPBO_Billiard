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
using std::cout;
using std::endl;

class Bentuk {
    protected:
    Koordinat Posisi_dia;
    sf::Color Warna;
    float radius;
    public:
    Bentuk(Koordinat posisi = Koordinat(), sf::Color W = sf::Color::White) : Posisi_dia(posisi), Warna(W) {}
    virtual ~Bentuk() = default;
    virtual void kirim_ke_jendela(sf::RenderWindow& Jendela) = 0;
    virtual void update(float deltatime) = 0;
    Koordinat getposisi() const {return Posisi_dia;}
    void setposisi(const Koordinat& pos) {Posisi_dia = pos;}
    sf::Color getwarna() const {return Warna;}
    void setwarna(const sf::Color& w) {Warna = w;}
    sf::Vector2f getPosisiSFML() const {return sf::Vector2f{Posisi_dia.x, Posisi_dia.y};}
};