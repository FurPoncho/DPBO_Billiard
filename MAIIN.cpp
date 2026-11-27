#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <memory>
#include <stack>
#include <vector>
#include <string>
#include <optional>

#include <iostream>
// #include "test.hpp"

class Layar {
    public:
    Layar (){};
    virtual ~Layar (){};
    
    virtual void Init () = 0;
    virtual void Process () = 0;
    // rename to SPF later for consistency
    virtual void Update (const sf::Time& deltaTime) = 0;
    virtual void Draw () = 0;
    
    virtual void Pause (){};
    virtual void Start (){};
};

class Manage_Layar {
    public:
    Manage_Layar():
    di_tambah(false),
    di_ganti(false),
    di_hapus(false),
    di_hapusSemua(false)
    {}
    ~Manage_Layar(){}

    void Tambah(std::unique_ptr<Layar> layar_untuk_ditambah, bool ganti = false){
        di_tambah = true;
        the_layarBaru = std::move(layar_untuk_ditambah);
        di_ganti = ganti;
    }
    void Hapus () {
        di_hapus = true;
    }
    void HapusSemua (){
        di_hapusSemua = true;
        di_hapus = false ;
    }
    
    void ProsesLayar (){
        if (di_hapusSemua){
            while (!the_stack.empty())
            {
                the_stack.pop();
            }
            di_hapusSemua = false;
        }
        else if (di_hapus && (!the_stack.empty())){
            the_stack.pop();
            if (!the_stack.empty())
            {
                the_stack.top()->Start();
            }
            di_hapus = false;
        }
        if (di_tambah){
            if (di_ganti && (!the_stack.empty()))
            {
                the_stack.pop();
                di_ganti = false;
            }
            if (!the_stack.empty())
            {
                the_stack.top()->Pause();
            }
            the_stack.push(std::move(the_layarBaru));
            the_stack.top()->Init();
            the_stack.top()->Start();
            di_tambah = false;
        }
    }
    
    std::unique_ptr<Layar> &GetLayarTeratas(){
        return the_stack.top();
    }
    bool ApakahStackKosong() const {
        return the_stack.empty();
    }

    private:
    // need stack to maintain game after going into pause state. could've used vector, 
    // but since its LIFO might as well make use of the stack std header
    std::stack<std::unique_ptr<Layar>> the_stack;
    std::unique_ptr<Layar> the_layarBaru;

    bool di_tambah;
    bool di_ganti;
    bool di_hapus;
    bool di_hapusSemua;

};

class Manage_asset {
    public:
        std::vector<std::shared_ptr<sf::Sound>> sounds;

        Manage_asset(){}
        ~Manage_asset(){}

        void AddTexture(int id, const std::string &filePath, bool wantRepeated = false){
            auto texture = std::make_unique<sf::Texture>();
            if (texture->loadFromFile(filePath)){
                texture->setRepeated(wantRepeated);
                textures[id]=std::move(texture);
            }
        };
        void AddFont(int id, const std::string &filePath){
            auto font = std::make_unique<sf::Font>();
            if (font->openFromFile(filePath)){
                fonts[id]=std::move(font);
            }
        };
        void AddSoundBuffer(int id,const std::string &filePath){
            auto buffer = std::make_unique<sf::SoundBuffer>();
            if (buffer->loadFromFile(filePath)){
                buffers[id]=std::move(buffer);
            }
        }

        const sf::Texture &GetTexture(int id) const{
            return *(textures.at(id).get());
        };
        const sf::Font &GetFont(int id) const{
            return *(fonts.at(id).get());
        };
        const sf::SoundBuffer &GetBuffer(int id) const{
            return *(buffers.at(id).get());
        };
            
    private:
        std::map<int,std::unique_ptr<sf::Texture>> textures;
        std::map<int,std::unique_ptr<sf::Font>> fonts;
        std::map<int,std::unique_ptr<sf::SoundBuffer>> buffers; 
        
};

struct Context {
    std::unique_ptr<sf::RenderWindow> the_window;
    std::unique_ptr<Manage_Layar> the_layar;
    std::unique_ptr<Manage_asset> the_asset;
    Context(){
        the_window = std::make_unique<sf::RenderWindow>();
        the_layar = std::make_unique<Manage_Layar>();
        the_asset = std::make_unique<Manage_asset>();
    }
};  


class Layar_Bermain : public Layar {
    private:
    std::shared_ptr<Context> the_context;
    std::vector<sf::Text> texts;
    std::vector<std::string> strings;
    std::vector<sf::Sprite> sprites;

    public:
    Layar_Bermain(std::shared_ptr<Context> &context):
    the_context(context){
    };
    ~Layar_Bermain(){};

    void Init() override{
        // imports
        the_context->the_asset->AddTexture(50,"assets/bg/Layar_Bermain.png");
        sf::Sprite sprite1(the_context->the_asset->GetTexture(50));
        sprites.push_back(sprite1);


        if (the_context->the_asset->sounds[0]->getStatus() == sf::SoundSource::Status::Playing){
            the_context->the_asset->sounds[0]->stop();
        }
        
        strings.insert(strings.begin(),{"sedang main"});
        for (int i=0; i < strings.size();i++){
            sf::Text& ref = texts.emplace_back(the_context->the_asset->GetFont(10),strings[i],14);
            ref.setFillColor(sf::Color::Blue);
        }

        // always follow, size, origin, position 
        texts[0].setCharacterSize(180);
        // center that bitch
        texts[0].setOrigin({texts[0].getLocalBounds().size.x/2,texts[0].getLocalBounds().size.y/2});
        texts[0].setPosition({800,250});
        texts[0].setRotation(sf::degrees(-10));
    };
    
    void Process() override ;
    void Update(const sf::Time& deltaTime) override {
        
    };
    void Draw() override{
        the_context->the_window->clear();
        for (const auto& thing : sprites) {
            the_context->the_window->draw(thing);
        }
        for (const auto& thing : texts) {
            // std::cout<<thing.getString().toAnsiString()<<"\n";
            the_context->the_window->draw(thing);
        }
        the_context->the_window->display();
    };
};

class Layar_Setup : public Layar {
    private:
    std::shared_ptr<Context> the_context;
    std::vector<sf::Text> texts;
    std::vector<std::string> strings;
    std::vector<sf::Sprite> sprites;

    public:
    Layar_Setup(std::shared_ptr<Context> &context):
    the_context(context){
    };
    ~Layar_Setup(){};

    void Init() override{
        // imports
        the_context->the_asset->AddTexture(30,"assets/bg/Layar_Setup.png");
        sf::Sprite sprite1(the_context->the_asset->GetTexture(30));
        sprites.push_back(sprite1);
        
        strings.insert(strings.begin(),{"placeholder"});
        for (int i=0; i < strings.size();i++){
            sf::Text& ref = texts.emplace_back(the_context->the_asset->GetFont(10),strings[i],14);
            ref.setFillColor(sf::Color::Blue);
        }

        // always follow, size, origin, position 
        texts[0].setCharacterSize(180);
        // center that bitch
        texts[0].setOrigin({texts[0].getLocalBounds().size.x/2,texts[0].getLocalBounds().size.y/2});
        texts[0].setPosition({800,250});
        texts[0].setRotation(sf::degrees(-10));    
    };
    
    void Process() override;

    void Update(const sf::Time& deltaTime) override {
        
    };
    void Draw() override{
        the_context->the_window->clear();
        for (const auto& thing : sprites) {
            the_context->the_window->draw(thing);
        }
        for (const auto& thing : texts) {
            the_context->the_window->draw(thing);
        }
        the_context->the_window->display();
    };
};

// Layar Pengaturan dan layar Pause blm kelar
class Layar_Pengaturan : public Layar {
    private:
    std::shared_ptr<Context> the_context;
    std::vector<sf::Text> texts;
    std::vector<std::string> strings;
    std::vector<sf::Sprite> sprites;

    public:
    Layar_Pengaturan(std::shared_ptr<Context> &context):
    
    the_context(context){
    };
    ~Layar_Pengaturan(){};

    void Init() override{

    };
    void Process() override;

    void Update(const sf::Time& deltaTime) override {

    };
    void Draw() override{
        
    };
};
class Layar_Pause : public Layar {
    private:
    std::shared_ptr<Context> the_context;
    std::vector<sf::Text> texts;
    std::vector<std::string> strings;
    std::vector<sf::Sprite> sprites;

    public:
    Layar_Pause(std::shared_ptr<Context> &context):
    the_context(context){
    };
    ~Layar_Pause(){};

    void Init() override{
    };
    
    void Process() override;

    void Update(const sf::Time& deltaTime) override { 
    
    };

    void Draw() override{

    };
};

class Layar_Menu : public Layar {
    private:
    int mode_layar;   
    std::shared_ptr<Context> the_context;
    std::vector<sf::Text> texts;
    std::vector<std::string> strings;
    std::vector<sf::Sprite> sprites;

    public:
    Layar_Menu(std::shared_ptr<Context> &context):
    the_context(context){
    };
    ~Layar_Menu(){};

    void Init() override{
        mode_layar = 0;
        the_context->the_asset->AddTexture(10,"assets/bg/Layar_Menu_default.png");
        the_context->the_asset->AddTexture(11,"assets/bg/Layar_Menu_1.png");
        the_context->the_asset->AddTexture(12,"assets/bg/Layar_Menu_2.png");
        the_context->the_asset->AddTexture(13,"assets/bg/Layar_Menu_3.png");
        
        sf::Sprite sprite1(the_context->the_asset->GetTexture(10));
        sprites.push_back(sprite1);
        sprite1.setTexture(the_context->the_asset->GetTexture(11));
        sprites.push_back(sprite1);
        sprite1.setTexture(the_context->the_asset->GetTexture(12));
        sprites.push_back(sprite1);
        sprite1.setTexture(the_context->the_asset->GetTexture(13));
        sprites.push_back(sprite1);
               
        if (!(the_context->the_asset->sounds[0]->getStatus() == sf::SoundSource::Status::Playing)){
            the_context->the_asset->sounds[0]->play();
        }
        
        
        strings.insert(strings.begin(),{"Top Down Billiard","Mulai Permainan", "Pengaturan"});
        for (int i=0; i < strings.size();i++){
            sf::Text& ref = texts.emplace_back(the_context->the_asset->GetFont(10),strings[i],14);
            ref.setFillColor(sf::Color::Blue);
        }

        // always follow, size, origin, position 
        texts[0].setCharacterSize(180);
        // center that bitch
        texts[0].setOrigin({texts[0].getLocalBounds().size.x/2,texts[0].getLocalBounds().size.y/2});
        texts[0].setPosition({800,250});
        texts[0].setRotation(sf::degrees(-10));
        
        // center that bitch
        texts[1].setCharacterSize(80);
        texts[1].setOrigin({texts[1].getLocalBounds().size.x/2,texts[1].getLocalBounds().size.y/2});
        texts[1].setPosition({800,450});
        
        // center that bitch
        texts[2].setCharacterSize(80);
        texts[2].setOrigin({texts[2].getLocalBounds().size.x/2,texts[2].getLocalBounds().size.y/2});
        texts[2].setPosition({800,580});
        


    };
    
    void Process() override;
    void Update(const sf::Time& deltaTime) override {
        
    };
    void Draw() override{
        the_context->the_window->clear();

        the_context->the_window->draw(sprites[mode_layar]);
        // for (const auto& thing : texts) {
        //     the_context->the_window->draw(thing);
        // }
        the_context->the_window->display();
    };
};


void Layar_Bermain::Process (){
    while (const std::optional event = the_context->the_window->pollEvent()){                                
        if (event->is<sf::Event::Closed>()){                                                                 
        the_context->the_layar->HapusSemua();                                                                
        }                                                                                                    
        else if (const auto* terpencet = event->getIf<sf::Event::KeyPressed>()){                             
            std::cout<<"terpencet: "<<sf::Keyboard::getDescription(terpencet->scancode).toAnsiString()<<"\n";
            // check the scancodes                                                                           
            switch (terpencet->scancode)                                                                     
            {                                                                                                
            case sf::Keyboard::Scancode::Enter:                                                              
                the_context->the_layar->Tambah(std::make_unique<Layar_Menu>(the_context),true);                                                                                                        
                break;                                                                                   
            case sf::Keyboard::Scancode::P:
                if (the_context->the_asset->sounds[0]->getStatus() == sf::SoundSource::Status::Playing)
                {
                    the_context->the_asset->sounds[0]->pause();
                }
                else {
                    the_context->the_asset->sounds[0]->play();
                }
                break;                                                                                     
            default:                                                                                         
                break;                                                                                       
            }                                                                                                
                                                                                                             
                                                                                                             
        }                                                                                                    
                                                                                                             
    }                                                                                                        
}

void Layar_Setup::Process (){
    while (const std::optional event = the_context->the_window->pollEvent()){                                
        if (event->is<sf::Event::Closed>()){                                                                 
        the_context->the_layar->HapusSemua();                                                                
        }                                                                                                    
        else if (const auto* terpencet = event->getIf<sf::Event::KeyPressed>()){                             
            std::cout<<"terpencet: "<<sf::Keyboard::getDescription(terpencet->scancode).toAnsiString()<<"\n";
            // check the scancodes                                                                           
            switch (terpencet->scancode)                                                                     
            {                                                                                                
            case sf::Keyboard::Scancode::Enter:                                                              
                std::cout<<"yuk main";                                                                           
                the_context->the_layar->Tambah(std::make_unique<Layar_Bermain>(the_context),true);               
                break;                                                                                                             
            default:                                                                                         
                break;                                                                                       
            }                                                                                                
                                                                                                             
                                                                                                             
        }                                                                                                    
                                                                                                             
    }                                                                                                        
}

void Layar_Pengaturan::Process (){
    while (const std::optional event = the_context->the_window->pollEvent()){                                
        if (event->is<sf::Event::Closed>()){                                                                 
        the_context->the_layar->HapusSemua();                                                                
        }                                                                                                    
        else if (const auto* terpencet = event->getIf<sf::Event::KeyPressed>()){                             
            std::cout<<"terpencet: "<<sf::Keyboard::getDescription(terpencet->scancode).toAnsiString()<<"\n";
            // check the scancodes                                                                           
            switch (terpencet->scancode)                                                                     
            {                                                                                                
            case sf::Keyboard::Scancode::Enter:                                                              
            std::cout<<"yuk main";                                                                           
            the_context->the_layar->Tambah(std::make_unique<Layar_Bermain>(the_context),true);               
                                                                                                             
                break;                                                                                       
                                                                                                             
            // case sf::Keyboard::Scancode::P:                                                               
            //     if (sounds[0].getStatus() == sf::SoundSource::Status::Playing)                            
            //     {                                                                                         
            //         sounds[0].pause();                                                                    
            //     }                                                                                         
            //     else {                                                                                    
            //         sounds[0].play();                                                                     
            //     }                                                                                         
                                                                                                             
            default:                                                                                         
                break;                                                                                       
            }                                                                                                
                                                                                                             
                                                                                                             
        }                                                                                                    
                                                                                                             
    }                                                                                                        
}

void Layar_Pause::Process (){
    while (const std::optional event = the_context->the_window->pollEvent()){                                
        if (event->is<sf::Event::Closed>()){                                                                 
        the_context->the_layar->HapusSemua();                                                                
        }                                                                                                    
        else if (const auto* terpencet = event->getIf<sf::Event::KeyPressed>()){                             
            std::cout<<"terpencet: "<<sf::Keyboard::getDescription(terpencet->scancode).toAnsiString()<<"\n";
            // check the scancodes                                                                           
            switch (terpencet->scancode)                                                                     
            {                                                                                                
            case sf::Keyboard::Scancode::Enter:                                                              
            std::cout<<"yuk main";                                                                           
            the_context->the_layar->Tambah(std::make_unique<Layar_Bermain>(the_context),true);               
                                                                                                             
                break;                                                                                       
                                                                                                             
            // case sf::Keyboard::Scancode::P:                                                               
            //     if (sounds[0].getStatus() == sf::SoundSource::Status::Playing)                            
            //     {                                                                                         
            //         sounds[0].pause();                                                                    
            //     }                                                                                         
            //     else {                                                                                    
            //         sounds[0].play();                                                                     
            //     }                                                                                         
                                                                                                             
            default:                                                                                         
                break;                                                                                       
            }                                                                                                
                                                                                                             
                                                                                                             
        }                                                                                                    
                                                                                                             
    }                                                                                                        
}

void Layar_Menu::Process (){
    while (const std::optional event = the_context->the_window->pollEvent()){
        if (event->is<sf::Event::Closed>()){
        the_context->the_layar->HapusSemua();
        }
        else if (const auto* terpencet = event->getIf<sf::Event::KeyPressed>()){
            std::cout<<"terpencet: "<<sf::Keyboard::getDescription(terpencet->scancode).toAnsiString()<<"\n";
            // check the scancodes
            switch (terpencet->scancode)
            {
            case sf::Keyboard::Scancode::Enter:
                the_context->the_layar->Tambah(std::make_unique<Layar_Setup>(the_context),true);
                break;
            case sf::Keyboard::Scancode::P:
                if (the_context->the_asset->sounds[0]->getStatus() == sf::SoundSource::Status::Playing)
                {
                    the_context->the_asset->sounds[0]->pause();
                }
                else {
                    the_context->the_asset->sounds[0]->play();
                }
                break;
            case sf::Keyboard::Scancode::Num1:
                std::cout<<"Hover: Bermain\n";
                mode_layar = 1;
                break;
            case sf::Keyboard::Scancode::Num2:
                std::cout<<"Hover: Pengaturan\n";
                mode_layar = 2;
                break;
            case sf::Keyboard::Scancode::Num3:
                std::cout<<"Hover: Keluar\n";
                mode_layar = 3;
                break;
            case sf::Keyboard::Scancode::Num0:
                std::cout<<"Hover: None\n";
                mode_layar = 0;
                break;
            default:
                break;
            }            
        }  
        
        // else if (const auto* terklik = event->getIf<sf::Event::MouseButtonPressed>()){
        // }
    }                                                                                           
}

class Game{
public:
    Game(): 
    the_context(std::make_shared<Context>()) {
        the_context->the_window->create(sf::VideoMode({1600,900}),"Top Down Billiard",sf::Style::Close);
        the_context->the_window->setView(sf::View(sf::FloatRect({0.f, 0.f},{1600.f, 900.f})));
        the_context->the_layar->Tambah(std::make_unique<Layar_Menu>(the_context));
        // 3.1 newest feature, but we already implemented the old way, sunk cost fallacy
        // the_context->the_window->setFramerateLimit(30);

        the_context->the_asset->AddSoundBuffer(1,"assets/sfx_billiard/bgm_utama.ogg");
        auto sound1 = std::make_shared<sf::Sound>(the_context->the_asset->GetBuffer(1));
        the_context->the_asset->sounds.push_back(sound1);
        the_context->the_asset->sounds[0]->setLooping(true);
        the_context->the_asset->sounds[0]->play(); 

        the_context->the_asset->AddFont(10,"assets/font/ARIAL.TTF");
    }
    void Run(){
        // apparently 3.1 has new feature called framerate. 
        // these entire SPF thing is obsolete.
        sf::Clock clock;
        sf::Time TimeSinceLastFrame = sf::Time::Zero;
    
        while (the_context->the_window->isOpen()){
            TimeSinceLastFrame += clock.restart();
            while (TimeSinceLastFrame > SPF){
                TimeSinceLastFrame -= SPF;

                the_context->the_layar->ProsesLayar();
                if (!the_context->the_layar->ApakahStackKosong()){
                    the_context->the_layar->GetLayarTeratas()->Process();
                    the_context->the_layar->GetLayarTeratas()->Update(SPF);
                    the_context->the_layar->GetLayarTeratas()->Draw();
                }
                else{
                    the_context->the_window->close();
                }              
            } 
        }
    }
private:
    std::shared_ptr<Context> the_context;

    // note, const second per frame to make sure game dont update too often -pon
    const sf::Time SPF = sf::seconds(1.f/60.f);
    // for test sake, its 30fps for now, change before present -pon
};

int main (){
    // tes hmm;
    // std::cout<<hmm.wow(2,3);
    Game game;
    game.Run();
}