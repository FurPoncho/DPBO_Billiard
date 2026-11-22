#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include <vector>
#include <string>
#include <optional>

#include <iostream>

// soo many include, it's like endgame in here (nah its doomsday)

// note on how to deal with this whole multiscreen BS,
// edit switch case whenever editing screens list

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

// do last cause messy is better than unfinished
class Manage_asset {
    public:
        Manage_asset(){
            // sf::Font font1;
            // if (!font1.openFromFile("font/ARIAL.TTF")){
            //     exit(9);   
            //     //error handling later
            // }
            // the_fonts.push_back(font1);
        }
        ~Manage_asset(){};

        // std::unique_ptr<sf::Font> &GetFont(int i = 0){
            
        // }
    private:
        std::vector<sf::Font> the_fonts;
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

class Layar_Pengaturan : public Layar {
    private:
    sf::Font font1;
    sf::Texture texture1;

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
        // FONT MUST BE STORED SOMEWHERE PERM, 
        if (!font1.openFromFile("font/ARIAL.TTF")){
            exit(9);   
            //error handling later
        }
        if (!texture1.loadFromFile("img/bg1.jpg")){
            exit(8);   
            //error handling later
        }
        sf::Sprite a(texture1);
        a.setScale(sf::Vector2f(4,4));
        sprites.push_back(a);
        
        strings.insert(strings.begin(),{"kata","kiti","kutu"});

        for (int i=0; i < strings.size();i++){
            sf::Text& ref = texts.emplace_back(font1,strings[i],14);
            ref.setFillColor(sf::Color::Blue);
            ref.setPosition({20.f,20.f+(i*10.f)});
        }
    };
    void Process() override {
        while (const std::optional event = the_context->the_window->pollEvent()){
            if (event->is<sf::Event::Closed>()){
                the_context->the_layar->HapusSemua();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                
            }
            
        }
        
    };
    void Update(const sf::Time& deltaTime) override {
        
    };
    void Draw() override{
        the_context->the_window->clear();
        // for (const auto& thing : sprites) {
        //     the_context->the_window->draw(thing);
        // }
        for (const auto& thing : texts) {
            // std::cout<<thing.getString().toAnsiString()<<"\n";
            the_context->the_window->draw(thing);
        }
        the_context->the_window->display();
    };
};

class Layar_Menu : public Layar {
    private:
    sf::Font font1;
    sf::Texture texture1;
    sf::RectangleShape ruler;

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
        // imports
        // FONT MUST BE STORED SOMEWHERE PERM, 
        if (!font1.openFromFile("font/ARIAL.TTF")){
            exit(9);   
            //error handling later
        }
        if (!texture1.loadFromFile("img/ponbg1.png")){
            exit(8);   
            //error handling later
        }
        sf::Sprite a(texture1);
        sprites.push_back(a);
        
        strings.insert(strings.begin(),{"Top Down Billiard","Mulai Permainan", "Pengaturan"});
        
        for (int i=0; i < strings.size();i++){
            sf::Text& ref = texts.emplace_back(font1,strings[i],14);
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
        
        
        // ruler.setSize(texts[0].getGlobalBounds().size);
        // ruler.setOrigin(ruler.getLocalBounds().size/2.f);
        // ruler.setPosition(texts[0].getPosition());
        // ruler.setPosition(texts[0].getOrigin());
        // ruler.setFillColor(sf::Color::Red);


    };
    
    void Process() override {
        while (const std::optional event = the_context->the_window->pollEvent()){
            if (event->is<sf::Event::Closed>()){
                the_context->the_layar->HapusSemua();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
                if (keyPressed->code == sf::Keyboard::Key::Up) {
                    // TODO: pindah pilihan menu ke atas
                }
            }
            
        }
        
    };
    void Update(const sf::Time& deltaTime) override {
        
    };
    void Draw() override{
        the_context->the_window->clear();
        for (const auto& thing : sprites) {
            the_context->the_window->draw(thing);
        }
        the_context->the_window->draw(ruler);
        for (const auto& thing : texts) {
            // std::cout<<thing.getString().toAnsiString()<<"\n";
            the_context->the_window->draw(thing);
        }
        the_context->the_window->display();
    };
};


// EVENTUALLY well make all this 
// enum layars{
//     Layar_Menu = 0,
//     Layar_Pengaturan,
//     Layar_Maumain,
//     Layar_Main,
//     Layar_Pause
// };

// class keadaan {
//     public:
//     keadaan():
//     is_tambah(false), is_tukar(false), is_hapus(false)
// };

class Game{
public:
    Game(): 
    the_context(std::make_shared<Context>()) {
        the_context->the_window->create(sf::VideoMode({1600,900}),"Top Down Billiard",sf::Style::Close);
        the_context->the_window->setView(sf::View(sf::FloatRect({0.f, 0.f},{1600.f, 900.f})));
        // the_context->the_window
        the_context->the_layar->Tambah(std::make_unique<Layar_Menu>(the_context));
        // 3.1 newest feature, but we already implemented the old way, sunk cost fallacy
        // the_context->the_window->setFramerateLimit(30);
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
    const sf::Time SPF = sf::seconds(1.f/30.f);
    // for test sake, its 30fps for now, change before present -pon
};

int main (){
    Game game;
    game.Run();
}