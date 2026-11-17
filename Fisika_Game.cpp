#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <optional>
#include <string>
#include <cmath>
#include <vector>
using std::cout;
using std::endl;

class Koordinat {
    public:
    float x;
    float y;
    public:
    Koordinat(float x = 0.0f, float y=0.0f) : x(x), y(y) {}
};

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

    void setaktif(bool status) {aktif_tusuk = status;}
    bool isaktif() const {return aktif_tusuk;}
    float getKekuatan() const {return power;}
    float getkekuatanmaks() const {return max_power;}
};

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

int main() {
    sf::RenderWindow Jendela_Fisika(sf::VideoMode(sf::Vector2u{1920, 1080}), "Jendela Fisika", sf::Style::Default, sf::State::Windowed); //Bikin Jendela FullScreen atau Windowed
    Jendela_Fisika.setFramerateLimit(60);
   
    Meja mejaBilliard(Koordinat(260.f, 190.f), 700.f, 1400.f, 40.f);
    Bola bolaPutih(Koordinat(610.f, 540.f), 20.f, sf::Color::White, "Bola Putih", &mejaBilliard);
    Bola bolaMerah(Koordinat(1310.f, 540.f), 20.f, sf::Color::Red, "Bola Merah", &mejaBilliard);
    float kiri   = mejaBilliard.getbataskiri();
    float kanan  = mejaBilliard.getbataskanan();
    float atas   = mejaBilliard.getbatasatas();
    float bawah  = mejaBilliard.getbatasbawah();
    float tengah = (kiri + kanan) / 2.0f;
    std::vector<Lubang> lubangBilliard = {
        Lubang(Koordinat(kiri, atas), 30.f, sf::Color::Black, "Lubang Kiri Atas"),
        Lubang(Koordinat(tengah, atas), 30.f, sf::Color::Black, "Lubang Tengah Atas"),
        Lubang(Koordinat(kanan, atas), 30.f, sf::Color::Black, "Lubang Kanan Atas"),
        Lubang(Koordinat(kiri, bawah), 30.f, sf::Color::Black, "Lubang Kiri Bawah"),
        Lubang(Koordinat(tengah, bawah), 30.f, sf::Color::Black, "Lubang Tengah Bawah"),
        Lubang(Koordinat(kanan, bawah), 30.f, sf::Color::Black, "Lubang Kanan Bawah")
    };

    cue stick(&bolaPutih);
    sf::Clock pewaktu;
    while (Jendela_Fisika.isOpen()) {
        sf::Vector2i posisiPixelmouse = sf::Mouse::getPosition(Jendela_Fisika);
        sf::Vector2f posisiduniamouse = Jendela_Fisika.mapPixelToCoords(posisiPixelmouse);

        // Panggil fungsi arahkan stik ke kursor mouse
        stick.ArahkankeMouse(posisiduniamouse);

        float deltatime = pewaktu.restart().asSeconds();
        while (const std::optional<sf::Event> Kejadian = Jendela_Fisika.pollEvent()) {
            if (Kejadian->is<sf::Event::Closed>()) {
                Jendela_Fisika.close();
            } else if (const auto* Tekan_Mouse = Kejadian->getIf<sf::Event::MouseButtonPressed>()) {
                if (Tekan_Mouse->button == sf::Mouse::Button::Left) {
                    stick.mulaiTarik();
                }
            } else if (const auto* Lepas = Kejadian->getIf<sf::Event::MouseButtonReleased>()) {
                if (Lepas->button == sf::Mouse::Button::Left) {
                    sf::Vector2f dorongan = stick.lepas_tembak();
                    bolaPutih.pukul(dorongan);
                }
            }
        }

        // Update fisika bola cukup 1x per frame
        bolaPutih.update(deltatime);
        bolaMerah.update(deltatime);
        bolaPutih.cekTabrakanBola(bolaMerah);

        // Munculkan stik lagi setelah kedua bola berhenti total
        if (!bolaPutih.sedangBergerak() && !bolaMerah.sedangBergerak()) {
            stick.setaktif(true);
        }
        stick.update(deltatime);

        // Render berurutan per layer
        Jendela_Fisika.clear(sf::Color(30, 30, 30));
        mejaBilliard.kirim_ke_jendela(Jendela_Fisika);

        size_t i = 0;
        while (i < lubangBilliard.size()) {
            lubangBilliard[i].kirim_ke_jendela(Jendela_Fisika);
            i++;
        }

        bolaPutih.kirim_ke_jendela(Jendela_Fisika);
        bolaMerah.kirim_ke_jendela(Jendela_Fisika);
        stick.kirim_ke_jendela(Jendela_Fisika);

        Jendela_Fisika.display();
    }
    
    return 0;
    

    
}