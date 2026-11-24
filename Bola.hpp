#pragma once
#include "Bentuk.hpp"
#include "Meja.hpp"
#include <string>
#include <cmath>

class Bola : public Bentuk {
private:
    float radius;
    sf::Vector2f Kecepatan;
    std::string nama_bola;
    int nomor_bola;
    bool status_bola;
    sf::CircleShape wujud_bola;
    const Meja* meja_ref;

public:
    Bola(Koordinat pos = Koordinat(), float r = 20.0f, sf::Color w = sf::Color::White,
         std::string nama = "Bola", int nomor = 0, const Meja* meja = nullptr, const sf::Texture* tekstur = nullptr)
        : Bentuk(pos, w), radius(r), nama_bola(nama), nomor_bola(nomor), status_bola(true),
          Kecepatan{0.f, 0.f}, meja_ref(meja)
    {
        wujud_bola.setRadius(radius);
        wujud_bola.setOrigin({radius, radius});
        wujud_bola.setPosition(getPosisiSFML());

        if (tekstur != nullptr) {
            wujud_bola.setTexture(tekstur);
            wujud_bola.setFillColor(sf::Color::White);
        } else {
            wujud_bola.setFillColor(Warna);
        }
    }

    void setTexture(const sf::Texture* tekstur) {
        if (tekstur != nullptr) {
            wujud_bola.setTexture(tekstur);
            wujud_bola.setFillColor(sf::Color::White);
        }
    }

    void update(float deltatime) override {
        if (!status_bola) return;

        Posisi_dia.x += Kecepatan.x * deltatime;
        Posisi_dia.y += Kecepatan.y * deltatime;

        float gesekan_kain = 0.992f;
        Kecepatan.x *= gesekan_kain;
        Kecepatan.y *= gesekan_kain;

        if (std::abs(Kecepatan.x) < 3.f && std::abs(Kecepatan.y) < 3.f) {
            Kecepatan = {0.f, 0.f};
        }

        if (meja_ref != nullptr) {
            float koefisien_pantul = -0.85f;

            if (Posisi_dia.x - radius <= meja_ref->getbataskiri()) {
                Posisi_dia.x = meja_ref->getbataskiri() + radius;
                Kecepatan.x *= koefisien_pantul;
            }
            else if (Posisi_dia.x + radius >= meja_ref->getbataskanan()) {
                Posisi_dia.x = meja_ref->getbataskanan() - radius;
                Kecepatan.x *= koefisien_pantul;
            }

            if (Posisi_dia.y - radius <= meja_ref->getbatasatas()) {
                Posisi_dia.y = meja_ref->getbatasatas() + radius;
                Kecepatan.y *= koefisien_pantul;
            }
            else if (Posisi_dia.y + radius >= meja_ref->getbatasbawah()) {
                Posisi_dia.y = meja_ref->getbatasbawah() - radius;
                Kecepatan.y *= koefisien_pantul;
            }
        }

        wujud_bola.setPosition(getPosisiSFML());
    }

    void cekTabrakanBola(Bola& bolaLain) {
        if (!this->status_bola || !bolaLain.status_bola || this == &bolaLain) {
            return;
        }

        float dx = bolaLain.Posisi_dia.x - this->Posisi_dia.x;
        float dy = bolaLain.Posisi_dia.y - this->Posisi_dia.y;
        float jarakKuadrat = (dx * dx) + (dy * dy);
        float totalRadius = this->radius + bolaLain.radius;

        if (jarakKuadrat <= (totalRadius * totalRadius) && jarakKuadrat > 0.0001f) {
            float jarak = std::sqrt(jarakKuadrat);
            float nx = dx / jarak;
            float ny = dy / jarak;

            float overlap = 0.5f * (totalRadius - jarak);
            this->Posisi_dia.x -= nx * overlap;
            this->Posisi_dia.y -= ny * overlap;
            bolaLain.Posisi_dia.x += nx * overlap;
            bolaLain.Posisi_dia.y += ny * overlap;

            float tx = -ny;
            float ty = nx;

            float dpTan1 = this->Kecepatan.x * tx + this->Kecepatan.y * ty;
            float dpTan2 = bolaLain.Kecepatan.x * tx + bolaLain.Kecepatan.y * ty;

            float dpNorm1 = this->Kecepatan.x * nx + this->Kecepatan.y * ny;
            float dpNorm2 = bolaLain.Kecepatan.x * nx + bolaLain.Kecepatan.y * ny;

            float elastisitas = 0.95f;
            float normBaru1 = dpNorm2 * elastisitas;
            float normBaru2 = dpNorm1 * elastisitas;

            this->Kecepatan.x = (tx * dpTan1) + (nx * normBaru1);
            this->Kecepatan.y = (ty * dpTan1) + (ny * normBaru1);

            bolaLain.Kecepatan.x = (tx * dpTan2) + (nx * normBaru2);
            bolaLain.Kecepatan.y = (ty * dpTan2) + (ny * normBaru2);

            this->wujud_bola.setPosition(this->getPosisiSFML());
            bolaLain.wujud_bola.setPosition(bolaLain.getPosisiSFML());
        }
    }

    void kirim_ke_jendela(sf::RenderWindow& Jendela) override {
        if (status_bola) {
            Jendela.draw(wujud_bola);
        }
    }

    void pukul(sf::Vector2f dorongan) { Kecepatan = dorongan; }
    sf::Vector2f getkecepatan() const { return Kecepatan; }
    void setKecepatan(sf::Vector2f v) { Kecepatan = v; }
    float getRadius() const { return radius; }
    bool isAktif() const { return status_bola; }
    void setStatus(bool status) { status_bola = status; }
    std::string getNama() const { return nama_bola; }
    int getNomor() const { return nomor_bola; }
    bool sedangBergerak() const { return (Kecepatan.x != 0.f || Kecepatan.y != 0.f); }
};