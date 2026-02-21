#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <fstream>

class Fiszka {
private:
    std::string pytanie;
    std::string odpowiedz;

public:
    Fiszka(std::string fiszka_pytanie, std::string fiszka_odpowiedz)
    : pytanie(fiszka_pytanie), odpowiedz(fiszka_odpowiedz)
    {}
    
    std::string pokazPytanie() {
        return pytanie;
    }

    bool sprawdzOdpowiedz(std::string odpowiedzUzytkownika) {
        return odpowiedzUzytkownika == odpowiedz;
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

    Fiszka losujFiszke() {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<> dist(0, zestaw.size() - 1);
        int i = dist(rng);
        return zestaw[i];
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
};

int main() {
    sf::RenderWindow window{ sf::VideoMode(800, 800), "Gra edukacyjna" };
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        sf::Event event;
        window.pollEvent(event);
        if (event.type == sf::Event::Closed) {
            window.close();
            break;
        }
        window.clear(sf::Color::Black);
        window.display();
    }

    Fiszka fiszka("Stolica Polski", "Warszawa");
    std::cout << fiszka.pokazPytanie() << '\n';
    std::string odp;
    std::cin >> odp;
    if (fiszka.sprawdzOdpowiedz(odp)) {
        std::cout << "Poprawna odpowiedź!";
    }
    else {
        std::cout << "Niepoprawna odpowiedź";
    }

    while (true) {

    }

    return 0;
}