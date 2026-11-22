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
        Jendela.draw(wujud_meja);
    }
    float getbataskiri() const{return Posisi_dia.x;}
    float getbataskanan() const{return Posisi_dia.x + lebar;}
    float getbatasatas() const{return Posisi_dia.y;}
    float getbatasbawah() const {return Posisi_dia.y + panjang;}
};