#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <fstream>
#include <stdexcept>

class Fiszka {
private:
    std::string pytanie;
    std::string odpowiedz;
    bool opanowana = false;

public:
    Fiszka(std::string fiszka_pytanie, std::string fiszka_odpowiedz)
        : pytanie(fiszka_pytanie), odpowiedz(fiszka_odpowiedz)
    {
    }

    std::string pokazPytanie() const {
        return pytanie;
    }

    std::string pokazOdp() const {
        return odpowiedz;
    }

    bool sprawdzOdpowiedz(std::string odpowiedzUzytkownika) const {
        return odpowiedzUzytkownika == odpowiedz;
    }

    void oznaczJakoOpanowana() {
        opanowana = true;
    }

    bool czyOpanowana()  const {
        return opanowana;
    }
};

class ZestawFiszek {
private:
    std::vector<Fiszka> zestaw;
public:
    void dodajFiszke(Fiszka fiszka) {
        zestaw.push_back(fiszka);
    }

    bool czyPusty() const {
        return zestaw.empty();
    }

    int losujFiszke() const {
        if (zestaw.empty()) {
            throw std::runtime_error("Brak fiszek w zestawie!");
        };

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<> dist(0, zestaw.size() - 1);
        int i = dist(rng);
        return i;
    }

    bool wczytaj_z_pliku(std::string nazwaPliku) {
        std::ifstream plik(nazwaPliku);
        if (!plik.is_open()) {
            return false;
        }

        std::string linia;
        while (std::getline(plik, linia)) {
            size_t srednik = linia.find(";");
            if (srednik == std::string::npos) {
                continue;
            }
            std::string pytanie = linia.substr(0, srednik);
            std::string odp = linia.substr(srednik + 1);
            dodajFiszke(Fiszka(pytanie, odp));
        }

        plik.close();
        return true;
    }

    void usunFiszke(int indeks) {
        zestaw.erase(zestaw.begin() + indeks);
    }

    std::vector<int> pobierzNiepoprawne() const {
        std::vector<int> doNauki;
        for (int i = 0; i < zestaw.size(); i++) {
            if (!zestaw[i].czyOpanowana()) {
                doNauki.push_back(i);
            }
        }
        return doNauki;
    }

    Fiszka& getFiszka(int indeks) {
        return zestaw[indeks];
    }

    void wyczysc() {
        zestaw.clear();
    }
};

class Przycisk {
private:
    sf::Text text;
    sf::Color color;
    sf::Color hover_color;
public:
    Przycisk() {}

    void rysuj(sf::RenderWindow& window) const {
        window.draw(text);
    }

    void aktualizuj(sf::Vector2i mousePos) {
        if (text.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            text.setFillColor(hover_color);
        }
        else {
            text.setFillColor(color);
        }
    }

    bool klikniety(sf::Vector2i mousePos) const {
        return text.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }

    void inicjalizuj(sf::Font& font, sf::String napis, int size, sf::Vector2f position, sf::Color color, sf::Color hover_color) {
        this->color = color;
        this->hover_color = hover_color;
        text.setFont(font);
        text.setString(napis);
        text.setCharacterSize(size);
        text.setPosition(position);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2, bounds.height / 2);
        text.setFillColor(color);
    }
};

class Menu {
private:
    std::vector<Przycisk> przyciski;
public:
    Menu() {}

    void rysuj(sf::RenderWindow& window) const {
        for (auto& p : przyciski) {
            p.rysuj(window);
        }
    }

    void aktualizuj(sf::Vector2i mousePos) {
        for (auto& p : przyciski) {
            p.aktualizuj(mousePos);
        }
    }

    int klikniety(sf::Vector2i mousePos) const {
        for (int i = 0; i < przyciski.size(); i++) {
            if (przyciski[i].klikniety(mousePos)) {
                return i;
            }
        }
        return -1;
    }

    void inicjalizuj(std::vector<Przycisk> menu_przyciski) {
        this->przyciski = menu_przyciski;
    }
};

class SesjaNauki {
private:
    ZestawFiszek& zestaw;
    std::vector<int> kolejka;
    int aktualnyIndeks;
    bool czyPokazacOdpowiedz;
    bool czyKoniec;
    bool czyBrakFiszek;

public:
    SesjaNauki(ZestawFiszek& z)
        : zestaw(z), aktualnyIndeks(-1), czyPokazacOdpowiedz(false), czyKoniec(false),  czyBrakFiszek(false)
    {}

    void przygotujNowaFiszke() {
        if (kolejka.empty() || aktualnyIndeks >= kolejka.size() - 1) {

            kolejka = zestaw.pobierzNiepoprawne();

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(kolejka.begin(), kolejka.end(), g);

            aktualnyIndeks = 0;

            if (kolejka.empty()) {
                if (zestaw.czyPusty()) {
                    czyBrakFiszek = true;
                }
                else {
                    czyKoniec = true;
                }
                return;
            }
        }
        else {
            aktualnyIndeks++;
        }

        czyPokazacOdpowiedz = false;
        czyKoniec = false;
    }

    std::string pobierzTresc() const {
        if (czyKoniec) {
            return "MATERIAŁ OPANOWANY!";
        }
        else if (czyBrakFiszek) {
            return "BRAK FISZEK W ZESTAWIE!";
        }


        int indeksFiszki = kolejka[aktualnyIndeks];

        if (czyPokazacOdpowiedz) {
            return zestaw.getFiszka(indeksFiszki).pokazOdp();
        }
        else {
            return zestaw.getFiszka(indeksFiszki).pokazPytanie();
        }
    }

    void obrocFiszke() { 
        czyPokazacOdpowiedz = !czyPokazacOdpowiedz; 
    }

    void oznaczJakoOpanowana() {
        if (!czyKoniec && !kolejka.empty()) {
            int indeksFiszki = kolejka[aktualnyIndeks];
            zestaw.getFiszka(indeksFiszki).oznaczJakoOpanowana();
            przygotujNowaFiszke();
        }
    }

    void oznaczJakoNieopanowana() {
        if (!czyKoniec) {
            przygotujNowaFiszke();
        }
    }

    bool czyObrocona() const {
        return czyPokazacOdpowiedz;
    }

    bool czyPokazacTlo() {
        if (czyKoniec || czyBrakFiszek) {
            return false;
        }
        return true;
    }
};

struct Kafelek {
    int x;
    int y;
};

class Snake {
public:
    enum class Kierunki {
        GORA,
        PRAWO,
        DOL,
        LEWO
    };

private:
    int zycia;
    std::vector<Kafelek> cialo;
    Kierunki kierunek;
    
public:
    Snake()
    : zycia(3), kierunek(Kierunki::PRAWO)
    {
        cialo.push_back({ 8, 8 });
        cialo.push_back({ 7,8 });
        cialo.push_back({ 6,8 });
    }

    void idz() {
        Kafelek nowaGlowa = cialo[0]; 
        if (kierunek == Kierunki::PRAWO) {
            nowaGlowa.x += 1;
        }
        else if (kierunek == Kierunki::LEWO) {
            nowaGlowa.x -= 1;
        }
        else if (kierunek == Kierunki::GORA) {
            nowaGlowa.y -= 1;
        }
        else if (kierunek == Kierunki::DOL) {
            nowaGlowa.y += 1;
        }
        cialo.insert(cialo.begin(), nowaGlowa);
        cialo.pop_back();
        
    }

    void zmienKierunek(Kierunki kierunek) {
        if (!((this->kierunek == Kierunki::PRAWO && kierunek == Kierunki::LEWO) || (this->kierunek == Kierunki::LEWO && kierunek == Kierunki::PRAWO) || (this->kierunek == Kierunki::GORA && kierunek == Kierunki::DOL) || (this->kierunek == Kierunki::DOL && kierunek == Kierunki::GORA))) {
            this->kierunek = kierunek;
        }
    }

    void rosnij() {
        Kafelek nowyKafelek = cialo[cialo.size()-1];
        cialo.push_back(nowyKafelek);
    }

    bool czyKolizja(sf::Vector2i rozmiarPlanszy) {
        if (cialo[0].x >= rozmiarPlanszy.x || cialo[0].x < 0 || cialo[0].y >= rozmiarPlanszy.y || cialo[0].y < 0) {
            return true;
        }

        for (int i = 1; i < cialo.size(); i++) {
            if (cialo[i].x == cialo[0].x && cialo[i].y == cialo[0].y) {
                return true;
            }
        }
        return false;
    }

    void usunZycie() {
        zycia -= 1;
    }
};

class Owoc {
private:
    int wartosc;
    sf::Vector2i pozycja;
public:
    Owoc() {}
    
    bool czyZebrany(sf::Vector2i pozycjaGlowy) {

    }

    void nowaPozycja(sf::Vector2i rozmiarOkna) {

    }
};

class GraSnake {
private:
    sf::RenderWindow& window;
    ZestawFiszek& zestaw;
    Snake snake;
    Owoc owoc;
    const int szerokoscKafelka;
    const int iloscKafelkow;
    sf::Clock zegar;

    bool czyPokazujePytanie;
    bool gra;
public:
    GraSnake(sf::RenderWindow& window, ZestawFiszek& zestaw)
        : window(window), zestaw(zestaw), szerokoscKafelka(50), iloscKafelkow(800/szerokoscKafelka), czyPokazujePytanie(false), gra(true)
    {}

    void obsluzEventy() {

    }

    void rysuj() {

    }

    void aktualizuj() {
        if (zegar.getElapsedTime().asSeconds() > 0.2f) {
            snake.idz();
            zegar.restart();
        }
    }

};

class Aplikacja {
private:
    ZestawFiszek zestaw;
    SesjaNauki sesja;
    sf::Font font;
    sf::RenderWindow window;

    Przycisk graj;
    Przycisk ustawienia;
    Przycisk wyjdz;
    Przycisk fiszki;
    Przycisk snake;
    Przycisk umiem;
    Przycisk nieUmiem;
    Przycisk powrot;

    Menu main_menu;
    Menu games_menu;

    enum class Menus {
        GLOWNE,
        GRY,
        EKRAN_FISZEK,
        SNAKE
    };
    Menus aktualnyStan = Menus::GLOWNE;

    GraSnake graSnake;

    sf::Vector2i mousePos;
public:
    Aplikacja() 
        : sesja(zestaw), graSnake(window, zestaw)
    {
        window.create(sf::VideoMode(800, 800), "Gra edukacyjna");
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf")) {
            throw std::runtime_error("Nie udało się wczytać czcionki!");
        }

        umiem.inicjalizuj(font, "UMIEM", 30, sf::Vector2f(250, 600), sf::Color::Green, sf::Color::White);
        nieUmiem.inicjalizuj(font, "NIE UMIEM", 30, sf::Vector2f(550, 600), sf::Color::Red, sf::Color::White);
        powrot.inicjalizuj(font, L"POWRÓT", 35, sf::Vector2f(400, 700), sf::Color::White, sf::Color::Cyan);
        wyjdz.inicjalizuj(font, L"WYJDŹ", 35, sf::Vector2f(400, 700), sf::Color::White, sf::Color::Red);
        graj.inicjalizuj(font, "GRAJ", 50, sf::Vector2f(400, 100), sf::Color::White, sf::Color::Blue);
        ustawienia.inicjalizuj(font, "USTAWIENIA", 50, sf::Vector2f(400, 200), sf::Color::White, sf::Color::Blue);
        fiszki.inicjalizuj(font, "FISZKI", 50, sf::Vector2f(400, 100), sf::Color::White, sf::Color::Blue);
        snake.inicjalizuj(font, "SNAKE", 50, sf::Vector2f(400, 200), sf::Color::White, sf::Color::Blue);

        main_menu.inicjalizuj({graj, ustawienia, wyjdz});
        games_menu.inicjalizuj({fiszki, snake, powrot});
    } 
    
    void obsluzEventy() {
        sf::Event event;
        mousePos = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (aktualnyStan == Menus::GLOWNE) {
                    if (main_menu.klikniety(mousePos) == 0) {
                        aktualnyStan = Menus::GRY;
                    }
                    else if (main_menu.klikniety(mousePos) == 1) {
                        std::cout << "USTAWEINIA";
                    }
                    else if (main_menu.klikniety(mousePos) == 2) {
                        window.close();
                    }
                }

                else if (aktualnyStan == Menus::GRY) {
                    if (games_menu.klikniety(mousePos) == 0) {
                        aktualnyStan = Menus::EKRAN_FISZEK;
                        zestaw.wyczysc();
                        zestaw.wczytaj_z_pliku("text.txt");
                        sesja.przygotujNowaFiszke();
                    }
                    else if (games_menu.klikniety(mousePos) == 1) {
                        std::cout << "SNAKE";
                    }
                    else if (games_menu.klikniety(mousePos) == 2) {
                        aktualnyStan = Menus::GLOWNE;
                    }
                }
            }

            if (aktualnyStan == Menus::EKRAN_FISZEK) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space) {
                        sesja.obrocFiszke();
                    }
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (powrot.klikniety(mousePos)) {
                        aktualnyStan = Menus::GRY;
                    }
                    else if (umiem.klikniety(mousePos)) {
                        sesja.oznaczJakoOpanowana();
                    }
                    else if (nieUmiem.klikniety(mousePos)) {
                        sesja.oznaczJakoNieopanowana();
                    }
                }
            }
        }
    }

    void aktualizuj() {

    }

    void rysuj() {
        mousePos = sf::Mouse::getPosition(window);
        if (aktualnyStan == Menus::GLOWNE) {
            main_menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            main_menu.rysuj(window);
        }
        else if (aktualnyStan == Menus::GRY) {
            games_menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            games_menu.rysuj(window);
        }
        else if (aktualnyStan == Menus::EKRAN_FISZEK) {
            powrot.aktualizuj(mousePos);
            window.clear(sf::Color::Black);

            sf::Text tekst;
            tekst.setFont(font);
            std::string string = sesja.pobierzTresc();
            tekst.setString(sf::String::fromUtf8(string.begin(), string.end()));
            tekst.setCharacterSize(40);
            tekst.setFillColor(sf::Color::Black);

            sf::RectangleShape tloFiszki;
            tloFiszki.setSize(sf::Vector2f(600, 300));
            tloFiszki.setOrigin(300, 150);
            tloFiszki.setPosition(400, 400);

            if (sesja.czyObrocona()) {
                tloFiszki.setFillColor(sf::Color(255, 165, 0));
            }
            else {
                tloFiszki.setFillColor(sf::Color::Yellow);
            }

            sf::FloatRect textBounds = tekst.getLocalBounds();
            tekst.setOrigin(textBounds.left + textBounds.width / 2, textBounds.top + textBounds.height / 2);
            tekst.setPosition(400, 400);

            if (sesja.czyPokazacTlo()) {
                window.draw(tloFiszki);
            }
            else {
                tekst.setFillColor(sf::Color::Cyan);
            }
            
            window.draw(tekst);

            umiem.aktualizuj(mousePos);
            nieUmiem.aktualizuj(mousePos);
            umiem.rysuj(window);
            nieUmiem.rysuj(window);
            powrot.rysuj(window);
        }
    }

    void uruchom() {
        while (window.isOpen()) {

            obsluzEventy();

            rysuj();

            window.display();
        }
    }
};

int main() {
    Aplikacja aplikacja;
    aplikacja.uruchom();
    return 0;
}