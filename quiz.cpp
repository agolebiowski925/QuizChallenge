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
    {}
    
    std::string pokazPytanie() {
        return pytanie;
    }

    std::string pokazOdp() {
        return odpowiedz;
    }

    bool sprawdzOdpowiedz(std::string odpowiedzUzytkownika) {
        return odpowiedzUzytkownika == odpowiedz;
    }

    void oznaczJakoOpanowana() {
        opanowana = true;
    }

    bool czyOpanowana() {
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

    bool czyPusty() {
        return zestaw.empty();
    }

    int losujFiszke() {
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
            int srednik = linia.find(";");
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

    std::vector<int> pobierzNiepoprawne() {
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
    Przycisk(sf::Font& font, sf::String napis, int size, sf::Vector2f position, sf::Color color, sf::Color hover_color)
        : color(color), hover_color(hover_color)
    {   
        text.setFont(font);
        text.setString(napis);
        text.setCharacterSize(size);
        text.setPosition(position);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2, bounds.height / 2);
        text.setFillColor(color);
    }

    void rysuj(sf::RenderWindow& window) {
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

    bool klikniety(sf::Vector2i mousePos) {
        return text.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    }
};

class Menu {
private:
    std::vector<Przycisk> przyciski;
public:
    Menu(std::vector<Przycisk> menu_przyciski)
        : przyciski(menu_przyciski)
    {}

    void rysuj(sf::RenderWindow& window) {
        for (auto p : przyciski) {
            p.rysuj(window);
        }
    }

    void aktualizuj(sf::Vector2i mousePos) {
        for (auto& p : przyciski) {
            p.aktualizuj(mousePos);
        }
    }

    int klikniety(sf::Vector2i mousePos) {
        for (int i = 0; i < przyciski.size(); i++) {
            if (przyciski[i].klikniety(mousePos)) {
                return i;
            }
        }
    }
};

class SesjaNauki {
private:
    ZestawFiszek& zestaw;
    std::vector<int> kolejka; 
    int aktualnyIndeks;        
    bool czyPokazacOdpowiedz;
    bool czyKoniec;

public:
    SesjaNauki(ZestawFiszek& z)
        : zestaw(z), aktualnyIndeks(-1), czyPokazacOdpowiedz(false), czyKoniec(false)
    {
    }

    void przygotujNowaFiszke() {
        if (kolejka.empty() || aktualnyIndeks >= kolejka.size() - 1) {

            kolejka = zestaw.pobierzNiepoprawne(); 

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(kolejka.begin(), kolejka.end(), g);

            aktualnyIndeks = 0;

            if (kolejka.empty()) {
                czyKoniec = true;
                return;
            }
        }
        else {
            aktualnyIndeks++;
        }

        czyPokazacOdpowiedz = false;
        czyKoniec = false;
    }

    std::string pobierzTresc() {
        if (czyKoniec) {
            return "MATERIAŁ OPANOWANY!";
        }
        

        int indeksFiszki = kolejka[aktualnyIndeks];

        if (czyPokazacOdpowiedz) return zestaw.getFiszka(indeksFiszki).pokazOdp();
        else return zestaw.getFiszka(indeksFiszki).pokazPytanie();
    }

    void obrocFiszke() { czyPokazacOdpowiedz = !czyPokazacOdpowiedz; }

    void oznaczJakoOpanowana() {
        if (!czyKoniec && !kolejka.empty()) {
            int indeksFiszki = kolejka[aktualnyIndeks];
            zestaw.getFiszka(indeksFiszki).oznaczJakoOpanowana();
            przygotujNowaFiszke();
        }
    }

    void oznaczJakoNieopanowana() {
        if (!czyKoniec) przygotujNowaFiszke();
    }

    bool czyObrocona() {
        return czyPokazacOdpowiedz;
    }
};

int main() {
    sf::RenderWindow window{ sf::VideoMode(800, 800), "Gra edukacyjna" };
    window.setFramerateLimit(60);

    sf::Font font1;
    if (!font1.loadFromFile("arial.ttf")) {
        std::cout << "Nie udało się wczytać czcionki!\n";
        return 1;
    }

    Przycisk graj(font1, "GRAJ", 50, sf::Vector2f(400, 100), sf::Color::White, sf::Color::Blue);
    Przycisk ustawienia(font1, "USTAWIENIA", 50, sf::Vector2f(400, 200), sf::Color::White, sf::Color::Blue);
    Przycisk wyjdz(font1, L"WYJDŹ", 35, sf::Vector2f(400, 700), sf::Color::White, sf::Color::Red);

    Przycisk fiszki(font1, "FISZKI", 50, sf::Vector2f(400, 100), sf::Color::White, sf::Color::Blue);
    Przycisk snake(font1, "SNAKE", 50, sf::Vector2f(400, 200), sf::Color::White, sf::Color::Blue);
    Przycisk powrot(font1, L"POWRÓT", 35, sf::Vector2f(400, 700), sf::Color::White, sf::Color::Cyan);

   
    Przycisk umiem(font1, "UMIEM", 30, sf::Vector2f(250, 600), sf::Color::Green, sf::Color::White);
    Przycisk nieUmiem(font1, "NIE UMIEM", 30, sf::Vector2f(550, 600), sf::Color::Red, sf::Color::White);

    Menu main_menu({graj, ustawienia, wyjdz});
    Menu games_menu({fiszki, snake, powrot});

    int menu = 1;
    ZestawFiszek zestaw1;
    
    SesjaNauki sesja(zestaw1);

    while (window.isOpen()) {
        sf::Event event;
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (menu == 1) {
                    if (main_menu.klikniety(mousePos) == 0) {
                        menu = 2;
                    }
                    else if (main_menu.klikniety(mousePos) == 1) {
                        std::cout << "USTAWEINIA";
                    }
                    else if (main_menu.klikniety(mousePos) == 2) {
                        window.close();
                    }
                }

                else if (menu == 2) {
                    if (games_menu.klikniety(mousePos) == 0) {
                        menu = 3;
                        zestaw1.wyczysc();
                        zestaw1.wczytaj_z_pliku("text.txt");
                        sesja.przygotujNowaFiszke(); 
                    }
                    else if (games_menu.klikniety(mousePos) == 1) {
                        std::cout << "SNAKE";
                    }
                    else if (games_menu.klikniety(mousePos) == 2) {
                        menu = 1;
                    }
                }
            }

            if (menu == 3) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space) {
                        sesja.obrocFiszke();
                    }
                }

                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (powrot.klikniety(mousePos)) {
                        menu = 2;
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

        if (menu == 1) {
            main_menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            main_menu.rysuj(window);
        }
        else if (menu == 2) {
            games_menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            games_menu.rysuj(window);
        }
        else if (menu == 3) {
            powrot.aktualizuj(mousePos);
            window.clear(sf::Color::Black);

            sf::Text tekst;
            tekst.setFont(font1);
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
            tekst.setOrigin(textBounds.left + textBounds.width / 2,
                textBounds.top + textBounds.height / 2);
            tekst.setPosition(400, 400);

            window.draw(tloFiszki); 
            window.draw(tekst);     

            umiem.aktualizuj(mousePos);
            nieUmiem.aktualizuj(mousePos);
            umiem.rysuj(window);
            nieUmiem.rysuj(window);
            powrot.rysuj(window);
        }
        
        window.display();
    }

    return 0;
}