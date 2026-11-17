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
#include "Meja.hpp"
using std::cout;
using std::endl;


class Bola : public Bentuk {
    private:
    float radius;
    sf::Vector2f Kecepatan;
    std::string nama_bola;
    bool status_bola;
    sf::CircleShape wujud_bola;
    const Meja* meja_ref;
    
    public:
    Bola(Koordinat pos = Koordinat(), float r = 15.0f, sf::Color w = sf::Color::White, std::string nama = "Bola", const Meja* meja = nullptr) 
    : Bentuk(pos, w), radius(r), nama_bola(nama), status_bola(true), Kecepatan{0.f, 0.f}, meja_ref(meja) {
    wujud_bola.setRadius(radius);
    wujud_bola.setFillColor(Warna);
    wujud_bola.setOrigin({radius, radius});
    wujud_bola.setPosition(getPosisiSFML());
    }
void update(float deltatime) override {
        if (!status_bola) return;

        // 1. Pindahkan posisi bola sesuai kecepatan
        Posisi_dia.x += Kecepatan.x * deltatime;
        Posisi_dia.y += Kecepatan.y * deltatime;

        // 2. Gesekan kain meja (perlambatan bertahap setiap frame)
        float gesekan_kain = 0.992f;
        Kecepatan.x *= gesekan_kain;
        Kecepatan.y *= gesekan_kain;

        // Hentikan total jika kecepatan sudah sangat pelan agar stik bisa muncul lagi
        if (std::abs(Kecepatan.x) < 3.f && std::abs(Kecepatan.y) < 3.f) {
            Kecepatan = {0.f, 0.f};
        }

        // 3. Cek pantulan 4 dinding meja
        if (meja_ref != nullptr) {
            float koefisien_pantul = -0.85f;

            // Dinding Kiri & Kanan (Sumbu X)
            if (Posisi_dia.x - radius <= meja_ref->getbataskiri()) {
                Posisi_dia.x = meja_ref->getbataskiri() + radius;
                Kecepatan.x *= koefisien_pantul;
            } 
            else if (Posisi_dia.x + radius >= meja_ref->getbataskanan()) {
                Posisi_dia.x = meja_ref->getbataskanan() - radius;
                Kecepatan.x *= koefisien_pantul;
            }

            // Dinding Atas & Bawah (Sumbu Y)
            if (Posisi_dia.y - radius <= meja_ref->getbatasatas()) {
                Posisi_dia.y = meja_ref->getbatasatas() + radius;
                Kecepatan.y *= koefisien_pantul;
            } 
            else if (Posisi_dia.y + radius >= meja_ref->getbatasbawah()) {
                Posisi_dia.y = meja_ref->getbatasbawah() - radius;
                Kecepatan.y *= koefisien_pantul;
            }
        }

        // 4. Sinkronkan posisi akhir ke wujud visual SFML
        wujud_bola.setPosition(getPosisiSFML());
    }
    void cekTabrakanBola(Bola& bolaLain) {
        // Abaikan jika salah satu bola sudah masuk lubang atau mengecek dirinya sendiri
        if (!this->status_bola || !bolaLain.status_bola || this == &bolaLain) {
            return;
        }

        // 1. Hitung selisih jarak sumbu X dan Y antar-pusat bola
        float dx = bolaLain.Posisi_dia.x - this->Posisi_dia.x;
        float dy = bolaLain.Posisi_dia.y - this->Posisi_dia.y;
        float jarakKuadrat = (dx * dx) + (dy * dy);
        float totalRadius = this->radius + bolaLain.radius;

        // 2. Cek apakah kedua lingkaran bersentuhan / tumpang tindih
        if (jarakKuadrat <= (totalRadius * totalRadius) && jarakKuadrat > 0.0001f) {
            float jarak = std::sqrt(jarakKuadrat);

            // Vektor satuan normal (arah garis tabrakan antar-pusat bola)
            float nx = dx / jarak;
            float ny = dy / jarak;

            // 3. KOREKSI POSISI (Dorong kedua bola agar tidak saling menempel)
            float overlap = 0.5f * (totalRadius - jarak);
            this->Posisi_dia.x -= nx * overlap;
            this->Posisi_dia.y -= ny * overlap;
            bolaLain.Posisi_dia.x += nx * overlap;
            bolaLain.Posisi_dia.y += ny * overlap;

            // Vektor satuan tangensial (tegak lurus garis tabrakan)
            float tx = -ny;
            float ty = nx;

            // Proyeksi kecepatan masing-masing bola ke sumbu tangensial & normal
            float dpTan1 = this->Kecepatan.x * tx + this->Kecepatan.y * ty;
            float dpTan2 = bolaLain.Kecepatan.x * tx + bolaLain.Kecepatan.y * ty;

            float dpNorm1 = this->Kecepatan.x * nx + this->Kecepatan.y * ny;
            float dpNorm2 = bolaLain.Kecepatan.x * nx + bolaLain.Kecepatan.y * ny;

            // 4. TUKAR KECEPATAN NORMAL (Hukum Kekekalan Momentum untuk massa sama)
            float elastisitas = 0.95f; // Sedikit energi hilang saat benturan keras
            float normBaru1 = dpNorm2 * elastisitas;
            float normBaru2 = dpNorm1 * elastisitas;

            // Gabungkan kembali kecepatan tangensial (tetap) + normal (tertukar)
            this->Kecepatan.x = (tx * dpTan1) + (nx * normBaru1);
            this->Kecepatan.y = (ty * dpTan1) + (ny * normBaru1);

            bolaLain.Kecepatan.x = (tx * dpTan2) + (nx * normBaru2);
            bolaLain.Kecepatan.y = (ty * dpTan2) + (ny * normBaru2);

            // Sinkronkan posisi visual terbaru ke SFML
            this->wujud_bola.setPosition(this->getPosisiSFML());
            bolaLain.wujud_bola.setPosition(bolaLain.getPosisiSFML());
        }
    }
    void kirim_ke_jendela(sf::RenderWindow& Jendela) override {
        if (status_bola) {
            Jendela.draw(wujud_bola);
        }
    }
    void pukul(sf::Vector2f dorongan) {
        Kecepatan = dorongan;
    }
    sf::Vector2f getkecepatan() const {return Kecepatan;}
    void setKecepatan(sf::Vector2f v) {Kecepatan = v;}
    float getRadius() const { return radius; }
    bool isAktif() const { return status_bola; }
    void setStatus(bool status) { status_bola = status; }
    std::string getNama() const { return nama_bola; }
    bool sedangBergerak() const {return (Kecepatan.x != 0.f || Kecepatan.y != 0.f);}
};