/******************************************************************************
 * Tytuł       : QuizChallenge
 * Autor       : Adam Gołębiowski
 * Klasa       : 3 TIP
 * Data        : 2025-03-01
 * Wersja      : 1.0
 * Opis        : Program tworzy grę edukacyjną opartą na fiszkach. Z menu 
 *               głównego można przejść do przeglądania fiszek, gry Snake lub
 *               ustawień. W przeglądaniu fiszek wyświetlane jest pytanie na
 *               żółtej karcie, po naciśnięciu spacji fiszka obraca się
 *               i pokazuje odpowiedź. Użytkownik zaznacza czy opanował materiał
 *               przyciskami "UMIEM" i "NIE UMIEM". W grze Snake poruszamy się
 *               strzałkami, po zebraniu owocu pojawia się pytanie z aktualnego
 *               zestawu fiszek - poprawna odpowiedź daje punkt, błędna odbiera
 *               życie. W ustawieniach można załadować własny zestaw fiszek
 *               z pliku .txt umieszczonego w folderze zestawy, gdzie każda
 *               linia ma format "pytanie;odpowiedź". Przy pisaniu programu
 *               korzystałem z poradników na YouTube, stron poświęconych
 *               bibliotece SFML i pomocy Sztucznej Inteligencji (po to, żeby
 *               lepiej zrozumieć działanie funkcji i komend). Kod pisałem
 *               samodzielnie.
 * Wersja SFML : 2.5.1.
 * Zależności  : C++17 lub nowszy
 * Kompilacja  : g++ quiz.cpp -o quiz -lsfml-graphics -lsfml-window -lsfml-system
 * Uruchomienie: ./quiz
 * Kontakt     : a.golebiowski@zset.leszno.pl
 ******************************************************************************/
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

// Reprezentuje pojedynczą fiszkę z pytaniem i odpowiedzią
class Fiszka
{
  private:
    std::string pytanie;
    std::string odpowiedz;
    bool opanowana = false;

  public:
    Fiszka(std::string fiszka_pytanie, std::string fiszka_odpowiedz)
        : pytanie(fiszka_pytanie), odpowiedz(fiszka_odpowiedz)
    {
    }

    std::string pokazPytanie() const
    {
        return pytanie;
    }

    std::string pokazOdp() const
    {
        return odpowiedz;
    }

    bool sprawdzOdpowiedz(std::string odpowiedzUzytkownika) const
    {
        std::string odp1 = odpowiedzUzytkownika;
        std::string odp2 = odpowiedz;
        for (char &c : odp1)
        {
            c = std::tolower((unsigned char)c);
        }
        for (char &c : odp2)
        {
            c = std::tolower((unsigned char)c);
        }
        return odp1 == odp2;
    }

    void oznaczJakoOpanowana()
    {
        opanowana = true;
    }

    bool czyOpanowana() const
    {
        return opanowana;
    }
};

// Przechowuje i zarządza zestawem fiszek
class ZestawFiszek
{
  private:
    std::vector<Fiszka> zestaw;

  public:
    void dodajFiszke(Fiszka fiszka)
    {
        zestaw.push_back(fiszka);
    }

    bool czyPusty() const
    {
        return zestaw.empty();
    }

    // Losuje indeks fiszki z zestawu
    int losujFiszke() const
    {
        if (zestaw.empty())
        {
            throw std::runtime_error("Brak fiszek w zestawie!");
        };

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<> dist(0, zestaw.size() - 1);
        int i = dist(rng);
        return i;
    }

    // Wczytuje fiszki z pliku txt w formacie "pytanie;odpowiedź"
    bool wczytaj_z_pliku(std::string nazwaPliku)
    {
        std::ifstream plik(nazwaPliku);
        if (!plik.is_open())
        {
            return false;
        }

        std::string linia;
        while (std::getline(plik, linia))
        {
            size_t srednik = linia.find(";");
            if (srednik == std::string::npos)
            {
                continue;
            }
            std::string pytanie = linia.substr(0, srednik);
            std::string odp = linia.substr(srednik + 1);
            dodajFiszke(Fiszka(pytanie, odp));
        }

        plik.close();
        return true;
    }

    void usunFiszke(int indeks)
    {
        zestaw.erase(zestaw.begin() + indeks);
    }

    std::vector<int> pobierzNiepoprawne() const
    {
        std::vector<int> doNauki;
        for (int i = 0; i < zestaw.size(); i++)
        {
            if (!zestaw[i].czyOpanowana())
            {
                doNauki.push_back(i);
            }
        }
        return doNauki;
    }

    Fiszka &getFiszka(int indeks)
    {
        return zestaw[indeks];
    }

    void wyczysc()
    {
        zestaw.clear();
    }
};

// Obsługuje interaktywny przycisk tekstowy z efektem hover
class Przycisk
{
  private:
    sf::Text text;
    sf::Color color;
    sf::Color hover_color;

  public:
    Przycisk()
    {
    }

    void rysuj(sf::RenderWindow &window) const
    {
        window.draw(text);
    }

    void aktualizuj(sf::Vector2i mousePos)
    {
        if (text.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                            static_cast<float>(mousePos.y)))
        {
            text.setFillColor(hover_color);
        }
        else
        {
            text.setFillColor(color);
        }
    }

    bool klikniety(sf::Vector2i mousePos) const
    {
        return text.getGlobalBounds().contains(static_cast<float>(mousePos.x),
                                               static_cast<float>(mousePos.y));
    }

    void inicjalizuj(sf::Font &font, sf::String napis, int size,
                     sf::Vector2f position, sf::Color color,
                     sf::Color hover_color)
    {
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

// Grupa przycisków tworzących menu nawigacyjne
class Menu
{
  private:
    std::vector<Przycisk> przyciski;

  public:
    Menu()
    {
    }

    void rysuj(sf::RenderWindow &window) const
    {
        for (auto &p : przyciski)
        {
            p.rysuj(window);
        }
    }

    void aktualizuj(sf::Vector2i mousePos)
    {
        for (auto &p : przyciski)
        {
            p.aktualizuj(mousePos);
        }
    }

    int klikniety(sf::Vector2i mousePos) const
    {
        for (int i = 0; i < przyciski.size(); i++)
        {
            if (przyciski[i].klikniety(mousePos))
            {
                return i;
            }
        }
        return -1;
    }

    void inicjalizuj(std::vector<Przycisk> menu_przyciski)
    {
        this->przyciski = menu_przyciski;
    }
};

// Zarządza sesją nauki - losuje fiszki i śledzi postęp
class SesjaNauki
{
  private:
    ZestawFiszek &zestaw;
    std::vector<int> kolejka;
    int aktualnyIndeks;
    bool czyPokazacOdpowiedz;
    bool czyKoniec;
    bool czyBrakFiszek;

  public:
    SesjaNauki(ZestawFiszek &z)
        : zestaw(z), aktualnyIndeks(-1), czyPokazacOdpowiedz(false),
          czyKoniec(false), czyBrakFiszek(false)
    {
    }

    void przygotujNowaFiszke()
    {
        if (kolejka.empty() || aktualnyIndeks >= kolejka.size() - 1)
        {

            kolejka = zestaw.pobierzNiepoprawne();

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(kolejka.begin(), kolejka.end(), g);

            aktualnyIndeks = 0;

            if (kolejka.empty())
            {
                if (zestaw.czyPusty())
                {
                    czyBrakFiszek = true;
                }
                else
                {
                    czyKoniec = true;
                }
                return;
            }
        }
        else
        {
            aktualnyIndeks++;
        }

        czyPokazacOdpowiedz = false;
        czyKoniec = false;
    }

    std::string pobierzTresc() const
    {
        if (czyKoniec)
        {
            return "MATERIAŁ OPANOWANY!";
        }
        else if (czyBrakFiszek)
        {
            return "BRAK FISZEK W ZESTAWIE!";
        }

        int indeksFiszki = kolejka[aktualnyIndeks];

        if (czyPokazacOdpowiedz)
        {
            return zestaw.getFiszka(indeksFiszki).pokazOdp();
        }
        else
        {
            return zestaw.getFiszka(indeksFiszki).pokazPytanie();
        }
    }

    void obrocFiszke()
    {
        czyPokazacOdpowiedz = !czyPokazacOdpowiedz;
    }

    void oznaczJakoOpanowana()
    {
        if (!czyKoniec && !kolejka.empty())
        {
            int indeksFiszki = kolejka[aktualnyIndeks];
            zestaw.getFiszka(indeksFiszki).oznaczJakoOpanowana();
            przygotujNowaFiszke();
        }
    }

    void oznaczJakoNieopanowana()
    {
        if (!czyKoniec)
        {
            przygotujNowaFiszke();
        }
    }

    bool czyObrocona() const
    {
        return czyPokazacOdpowiedz;
    }

    bool czyPokazacTlo()
    {
        if (czyKoniec || czyBrakFiszek)
        {
            return false;
        }
        return true;
    }

    void reset()
    {
        kolejka.clear();
        aktualnyIndeks = 0;
        czyPokazacOdpowiedz = false;
        czyKoniec = false;
        czyBrakFiszek = false;
    }
};

struct Kafelek
{
    int x;
    int y;
};

// Logika węża - ruch, kierunki, kolizje, życia
class Snake
{
  public:
    enum class Kierunki
    {
        GORA,
        PRAWO,
        DOL,
        LEWO
    };

  private:
    int zycia;
    std::vector<Kafelek> cialo;
    Kierunki kierunek;
    Kierunki nastepnyKierunek;

  public:
    Snake()
        : zycia(3), kierunek(Kierunki::PRAWO), nastepnyKierunek(Kierunki::PRAWO)
    {
        cialo.push_back({8, 8});
        cialo.push_back({7, 8});
        cialo.push_back({6, 8});
    }

    void idz()
    {
        kierunek = nastepnyKierunek;
        Kafelek nowaGlowa = cialo[0];
        if (kierunek == Kierunki::PRAWO)
        {
            nowaGlowa.x += 1;
        }
        else if (kierunek == Kierunki::LEWO)
        {
            nowaGlowa.x -= 1;
        }
        else if (kierunek == Kierunki::GORA)
        {
            nowaGlowa.y -= 1;
        }
        else if (kierunek == Kierunki::DOL)
        {
            nowaGlowa.y += 1;
        }
        cialo.insert(cialo.begin(), nowaGlowa);
        cialo.pop_back();
    }

    void zmienKierunek(Kierunki nowyKierunek)
    {
        if (!((kierunek == Kierunki::PRAWO && nowyKierunek == Kierunki::LEWO) ||
              (kierunek == Kierunki::LEWO && nowyKierunek == Kierunki::PRAWO) ||
              (kierunek == Kierunki::GORA && nowyKierunek == Kierunki::DOL) ||
              (kierunek == Kierunki::DOL && nowyKierunek == Kierunki::GORA)))
        {
            this->nastepnyKierunek = nowyKierunek;
        }
    }

    void rosnij()
    {
        Kafelek nowyKafelek = cialo[cialo.size() - 1];
        cialo.push_back(nowyKafelek);
    }

    // Sprawdza kolizję węża ze ścianą i samym sobą
    bool czyKolizja(sf::Vector2i rozmiarPlanszy)
    {
        if (cialo[0].x >= rozmiarPlanszy.x || cialo[0].x < 0 ||
            cialo[0].y >= rozmiarPlanszy.y || cialo[0].y < 0)
        {
            return true;
        }

        for (int i = 1; i < cialo.size(); i++)
        {
            if (cialo[i].x == cialo[0].x && cialo[i].y == cialo[0].y)
            {
                return true;
            }
        }
        return false;
    }

    void usunZycie()
    {
        zycia -= 1;
    }

    const std::vector<Kafelek> &getCialo() const
    {
        return cialo;
    }

    int getZycia() const
    {
        return zycia;
    }

    void reset()
    {
        zycia = 3;
        kierunek = Kierunki::PRAWO;
        nastepnyKierunek = Kierunki::PRAWO;
        cialo.clear();
        cialo.push_back({8, 8});
        cialo.push_back({7, 8});
        cialo.push_back({6, 8});
    }
};

// Reprezentuje owoc na planszy gry Snake
class Owoc
{
  private:
    int wartosc;
    sf::Vector2i pozycja;

  public:
    Owoc(int wartosc_owocu, sf::Vector2i rozmiarPlanszy) : wartosc(wartosc_owocu)
    {
        nowaPozycja(rozmiarPlanszy);
    }

    bool czyZebrany(sf::Vector2i pozycjaGlowy)
    {
        if (pozycjaGlowy == pozycja)
        {
            return true;
        }
        return false;
    }

    void nowaPozycja(sf::Vector2i rozmiarOkna)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<> dist1(0, rozmiarOkna.x - 1);
        std::uniform_int_distribution<> dist2(0, rozmiarOkna.y - 1);
        int x = dist1(rng);
        int y = dist2(rng);
        pozycja = {x, y};
    }

    sf::Vector2i getPozycja() const
    {
        return pozycja;
    }
};

// Gra Snake zintegrowana z zestawem fiszek
class GraSnake
{
  private:
    sf::RenderWindow &window;
    ZestawFiszek &zestaw;

    sf::Font &font;
    Snake snake;
    const int szerokoscKafelka;
    const int iloscKafelkow;
    Owoc owoc;
    sf::Clock zegar;
    sf::Clock zegarBlednejOdp;

    bool czyPokazujePytanie;
    bool czyPrzegrana;
    bool wrocDoMenu;
    bool czyPokazujeBlednaOdp;

    std::string odpowiedzGracza;
    int aktualnaFiszka;
    int punkty;

    Przycisk powrot;
    Przycisk zagrajPonownie;

    sf::Texture teksturaOwoc;
    sf::Sprite spriteOwoc;
  public:
    GraSnake(sf::RenderWindow &window, ZestawFiszek &zestaw, sf::Font &font)
        : window(window), zestaw(zestaw), font(font), szerokoscKafelka(50),
          iloscKafelkow(800 / szerokoscKafelka), czyPokazujePytanie(false),
          czyPrzegrana(false),
          owoc(1, sf::Vector2i(iloscKafelkow, iloscKafelkow)),
          aktualnaFiszka(-1), wrocDoMenu(false), punkty(0),
          czyPokazujeBlednaOdp(false)
    {
    }

    void obsluzEventy(sf::Event &event, sf::Vector2i &mousePos)
    {
        if (event.type == sf::Event::KeyPressed && czyPokazujePytanie == false)
        {
            if (event.key.code == sf::Keyboard::Up)
            {
                snake.zmienKierunek(Snake::Kierunki::GORA);
            }
            else if (event.key.code == sf::Keyboard::Down)
            {
                snake.zmienKierunek(Snake::Kierunki::DOL);
            }
            else if (event.key.code == sf::Keyboard::Right)
            {
                snake.zmienKierunek(Snake::Kierunki::PRAWO);
            }
            else if (event.key.code == sf::Keyboard::Left)
            {
                snake.zmienKierunek(Snake::Kierunki::LEWO);
            }
        }
        else if (czyPokazujePytanie)
        {
            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode == 8)
                {
                    if (!odpowiedzGracza.empty())
                    {
                        odpowiedzGracza.pop_back();
                    }
                }
                else if (event.text.unicode == 13)
                {
                    if (zestaw.getFiszka(aktualnaFiszka)
                            .sprawdzOdpowiedz(odpowiedzGracza))
                    {
                        punkty += 1;
                        owoc.nowaPozycja({iloscKafelkow, iloscKafelkow});
                        czyPokazujePytanie = false;
                        odpowiedzGracza = "";
                    }
                    else
                    {
                        czyPokazujeBlednaOdp = true;
                        zegarBlednejOdp.restart();
                        snake.usunZycie();
                        czyPokazujePytanie = false;
                        odpowiedzGracza = "";
                    }
                }
                else
                {
                    odpowiedzGracza += static_cast<char>(event.text.unicode);
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonPressed &&
                 event.mouseButton.button == sf::Mouse::Left &&
                 czyPrzegrana == true)
        {
            if (powrot.klikniety(mousePos))
            {
                wrocDoMenu = true;
            }
            else if (zagrajPonownie.klikniety(mousePos))
            {
                reset();
            }
        }
    }

    void rysuj(sf::Vector2i &mousePos)
    {
        for (int i = 0; i < snake.getCialo().size(); i++)
        {
            sf::RectangleShape segment;
            segment.setSize(sf::Vector2f(szerokoscKafelka, szerokoscKafelka));
            segment.setPosition(snake.getCialo()[i].x * szerokoscKafelka,
                                snake.getCialo()[i].y * szerokoscKafelka);
            if (i == 0)
            {
                segment.setFillColor(sf::Color(0, 150, 0));
            }
            else
            {
                segment.setFillColor(sf::Color::Green);
            }
            window.draw(segment);
        }

        spriteOwoc.setPosition(this->owoc.getPozycja().x * szerokoscKafelka,
                               this->owoc.getPozycja().y * szerokoscKafelka);
        window.draw(spriteOwoc);

        if (czyPokazujePytanie)
        {
            sf::Text pytanie;
            pytanie.setFont(font);
            std::string stringPytanie =
                zestaw.getFiszka(aktualnaFiszka).pokazPytanie();
            pytanie.setString(sf::String::fromUtf8(stringPytanie.begin(),
                                                   stringPytanie.end()));
            pytanie.setCharacterSize(40);
            pytanie.setFillColor(sf::Color::Black);
            sf::FloatRect pytanieBounds = pytanie.getLocalBounds();
            pytanie.setOrigin(pytanieBounds.left + pytanieBounds.width / 2,
                              pytanieBounds.top + pytanieBounds.height / 2);
            pytanie.setPosition(400, 400);

            sf::Text odp;
            odp.setFont(font);
            std::string stringOdp = odpowiedzGracza;
            
            odp.setString(
                sf::String::fromUtf8(stringOdp.begin(), stringOdp.end()));
            odp.setCharacterSize(40);
            odp.setFillColor(sf::Color::Black);
            sf::FloatRect odpBounds = odp.getLocalBounds();
            odp.setOrigin(odpBounds.left + odpBounds.width / 2,
                          odpBounds.top + odpBounds.height / 2);
            odp.setPosition(400, 500);

            sf::RectangleShape tloPytania;
            tloPytania.setSize(sf::Vector2f(600, 300));
            tloPytania.setOrigin(300, 150);
            tloPytania.setPosition(400, 400);
            tloPytania.setFillColor(sf::Color::Yellow);

            window.draw(tloPytania);
            window.draw(pytanie);
            window.draw(odp);
        }

        if (czyPrzegrana)
        {
            sf::RectangleShape tloKomunikat;
            tloKomunikat.setFillColor(sf::Color::Yellow);
            tloKomunikat.setSize(sf::Vector2f(600, 600));
            sf::FloatRect tloKomunikatBounds = tloKomunikat.getLocalBounds();
            tloKomunikat.setOrigin(
                tloKomunikatBounds.left + tloKomunikatBounds.width / 2,
                tloKomunikatBounds.top + tloKomunikatBounds.height / 2);
            tloKomunikat.setPosition(400, 400);

            sf::Text komunikat;
            komunikat.setFont(font);
            komunikat.setString(L"PRZEGRAŁEŚ!");
            komunikat.setCharacterSize(55);
            komunikat.setFillColor(sf::Color::Red);
            sf::FloatRect komunikatBounds = komunikat.getLocalBounds();
            komunikat.setOrigin(komunikatBounds.left + komunikatBounds.width / 2,
                                komunikatBounds.top +
                                    komunikatBounds.height / 2);
            komunikat.setPosition(400, 150);

            sf::Text liczba_punktow;
            liczba_punktow.setFont(font);
            liczba_punktow.setString("PUNKTY: " + std::to_string(punkty));
            liczba_punktow.setCharacterSize(50);
            liczba_punktow.setFillColor(sf::Color::Green);
            sf::FloatRect punktyBounds = liczba_punktow.getLocalBounds();
            liczba_punktow.setOrigin(punktyBounds.left + punktyBounds.width / 2,
                                     punktyBounds.top + punktyBounds.height / 2);
            liczba_punktow.setPosition(400, 300);

            window.draw(tloKomunikat);
            window.draw(komunikat);
            window.draw(liczba_punktow);
            zagrajPonownie.rysuj(window);
            powrot.rysuj(window);
            powrot.aktualizuj(mousePos);
            zagrajPonownie.aktualizuj(mousePos);
        }

        if (czyPokazujeBlednaOdp)
        {
            sf::RectangleShape tloPoprawnejOdp;
            tloPoprawnejOdp.setFillColor(sf::Color(255, 165, 0));
            tloPoprawnejOdp.setSize(sf::Vector2f(600, 600));
            sf::FloatRect tloOdpBounds = tloPoprawnejOdp.getLocalBounds();
            tloPoprawnejOdp.setOrigin(tloOdpBounds.left + tloOdpBounds.width / 2,
                                      tloOdpBounds.top +
                                          tloOdpBounds.height / 2);
            tloPoprawnejOdp.setPosition(400, 400);

            sf::Text poprawnaOdp1;
            poprawnaOdp1.setFont(font);
            poprawnaOdp1.setString(L"POPRAWNA ODPOWIEDŹ:");
            poprawnaOdp1.setCharacterSize(30);
            poprawnaOdp1.setFillColor(sf::Color::Black);
            sf::FloatRect poprawnaOdpBounds1 = poprawnaOdp1.getLocalBounds();
            poprawnaOdp1.setOrigin(
                poprawnaOdpBounds1.left + poprawnaOdpBounds1.width / 2,
                poprawnaOdpBounds1.top + poprawnaOdpBounds1.height / 2);
            poprawnaOdp1.setPosition(400, 400);

            sf::Text poprawnaOdp2;
            poprawnaOdp2.setFont(font);
            std::string tekst = zestaw.getFiszka(aktualnaFiszka).pokazOdp();
            poprawnaOdp2.setString(
                sf::String::fromUtf8(tekst.begin(), tekst.end()));
            poprawnaOdp2.setCharacterSize(40);
            poprawnaOdp2.setFillColor(sf::Color::Black);
            sf::FloatRect poprawnaOdpBounds2 = poprawnaOdp2.getLocalBounds();
            poprawnaOdp2.setOrigin(
                poprawnaOdpBounds2.left + poprawnaOdpBounds2.width / 2,
                poprawnaOdpBounds2.top + poprawnaOdpBounds2.height / 2);
            poprawnaOdp2.setPosition(400, 460);

            sf::Text pozostaleZycia;
            pozostaleZycia.setFont(font);
            std::string tekstZycia =
                "POZOSTAŁE ŻYCIA: " + std::to_string(snake.getZycia());
            pozostaleZycia.setString(
                sf::String::fromUtf8(tekstZycia.begin(), tekstZycia.end()));
            pozostaleZycia.setCharacterSize(30);
            pozostaleZycia.setFillColor(sf::Color::Blue);
            sf::FloatRect pozostaleZyciaBounds = pozostaleZycia.getLocalBounds();
            pozostaleZycia.setOrigin(
                pozostaleZyciaBounds.left + pozostaleZyciaBounds.width / 2,
                pozostaleZyciaBounds.top + pozostaleZyciaBounds.height / 2);
            pozostaleZycia.setPosition(400, 300);

            window.draw(tloPoprawnejOdp);
            window.draw(poprawnaOdp1);
            window.draw(poprawnaOdp2);
            window.draw(pozostaleZycia);
        }
    }

    void aktualizuj()
    {
        if (zegar.getElapsedTime().asSeconds() > 0.2f &&
            czyPokazujePytanie == false && czyPrzegrana == false &&
            czyPokazujeBlednaOdp == false)
        {
            snake.idz();
            zegar.restart();
        }

        if (snake.czyKolizja(sf::Vector2i(iloscKafelkow, iloscKafelkow)) ||
            snake.getZycia() <= 0)
        {
            czyPrzegrana = true;
        }

        if (owoc.czyZebrany({snake.getCialo()[0].x, snake.getCialo()[0].y}) &&
            czyPokazujePytanie == false && czyPokazujeBlednaOdp == false)
        {
            snake.rosnij();
            czyPokazujePytanie = true;
            aktualnaFiszka = zestaw.losujFiszke();
        }

        if (czyPokazujeBlednaOdp &&
            zegarBlednejOdp.getElapsedTime().asSeconds() > 4.0f)
        {
            czyPokazujeBlednaOdp = false;
            owoc.nowaPozycja({iloscKafelkow, iloscKafelkow});
        }
    }

    void reset()
    {
        czyPrzegrana = false;
        czyPokazujePytanie = false;
        odpowiedzGracza = "";
        owoc.nowaPozycja({iloscKafelkow, iloscKafelkow});
        snake.reset();
        wrocDoMenu = false;
        punkty = 0;
    }

    bool czyWrocic() const
    {
        return wrocDoMenu;
    }

    void inicjalizujPrzyciski()
    {
        zagrajPonownie.inicjalizuj(font, "ZAGRAJ PONOWNIE", 50,
                                   sf::Vector2f(400, 450), sf::Color::Black,
                                   sf::Color::Blue);
        powrot.inicjalizuj(font, L"POWRÓT", 35, sf::Vector2f(400, 650),
                           sf::Color::Black, sf::Color::Cyan);
        teksturaOwoc.loadFromFile("owoc.png");
        spriteOwoc.setTexture(teksturaOwoc);
    }
};

// Ekran ustawień - ładowanie zestawów fiszek z pliku
class Ustawienia
{
  private:
    sf::Font &font;
    sf::RenderWindow &window;
    ZestawFiszek &zestaw;

    std::vector<std::string> znalezionePliki;
    std::vector<Przycisk> pliki_przyciski;
    std::string wybranyPlik;

    Przycisk wgrajZestaw;
    Przycisk zaladujZestaw;
    Przycisk powrot;
    Menu menu;

    bool wrocDoMenu;

    enum class Menus
    {
        GLOWNE,
        WGRAJ,
        ZALADUJ
    };
    Menus aktualnyStan = Menus::GLOWNE;

  public:
    Ustawienia(sf::RenderWindow &window, ZestawFiszek &zestaw, sf::Font &font)
        : window(window), zestaw(zestaw), font(font), wrocDoMenu(false)
    {
    }

    // Skanuje folder zestawy/ w poszukiwaniu plików .txt
    void skanujFolder()
    {
        znalezionePliki.clear();
        pliki_przyciski.clear();

        for (auto &plik : std::filesystem::directory_iterator("zestawy/"))
        {
            if (plik.path().extension() == ".txt")
            {
                std::string sciezka = plik.path().string();
                int indeks = znalezionePliki.size();
                znalezionePliki.push_back(sciezka);
                Przycisk przycisk;
                int wiersz = indeks % 14;
                int kolumna = indeks / 14;

                float x = 200 + kolumna * 400;
                float y = 50 + wiersz * 50;

                przycisk.inicjalizuj(font, plik.path().filename().string(), 25,
                                     sf::Vector2f(x, y), sf::Color::White,
                                     sf::Color::Blue);
                pliki_przyciski.push_back(przycisk);
            }
        }
    }

    void obsluzEvent(sf::Event &event, sf::Vector2i &mousePos)
    {
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            if (aktualnyStan == Menus::GLOWNE)
            {
                if (wgrajZestaw.klikniety(mousePos))
                {
                    aktualnyStan = Menus::WGRAJ;
                }
                else if (zaladujZestaw.klikniety(mousePos))
                {
                    aktualnyStan = Menus::ZALADUJ;
                }
                else if (powrot.klikniety(mousePos))
                {
                    wrocDoMenu = true;
                }
            }
            else if (aktualnyStan == Menus::WGRAJ)
            {
                if (powrot.klikniety(mousePos))
                {
                    aktualnyStan = Menus::GLOWNE;
                }
            }
            else if (aktualnyStan == Menus::ZALADUJ)
            {
                if (powrot.klikniety(mousePos))
                {
                    aktualnyStan = Menus::GLOWNE;
                }

                for (int i = 0; i < pliki_przyciski.size(); i++)
                {
                    if (pliki_przyciski[i].klikniety(mousePos))
                    {
                        zestaw.wyczysc();
                        zestaw.wczytaj_z_pliku(znalezionePliki[i]);
                        wybranyPlik = znalezionePliki[i];
                        wrocDoMenu = true;
                    }
                }
            }
        }
    }

    void rysuj(sf::Vector2i &mousePos)
    {
        if (aktualnyStan == Menus::GLOWNE)
        {
            menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            menu.rysuj(window);
        }
        else if (aktualnyStan == Menus::WGRAJ)
        {
            powrot.aktualizuj(mousePos);
            window.clear(sf::Color::Black);

            sf::Text wgrajKomunikat;
            wgrajKomunikat.setFont(font);
            wgrajKomunikat.setString(
                L"Żeby wgrać zestaw fiszek umieść plik .txt w folderze "
                L"/zestawy!\nNazwa pliku nie powinna mieć więcej niż 25 "
                L"znaków!\nW folderze nie powinno umieszczać się więcej niż 28 "
                L"zestawów!\n\n\nFiszki należy zapisać w "
                L"formacie:\n\npytanie1;odpowiedź1\npytanie2;odpowiedz2\n...");
            wgrajKomunikat.setCharacterSize(25);
            wgrajKomunikat.setFillColor(sf::Color::Red);
            sf::FloatRect wgrajKomunikatBounds = wgrajKomunikat.getLocalBounds();
            wgrajKomunikat.setOrigin(
                wgrajKomunikatBounds.left + wgrajKomunikatBounds.width / 2,
                wgrajKomunikatBounds.top + wgrajKomunikatBounds.height / 2);
            wgrajKomunikat.setPosition(400, 200);

            window.draw(wgrajKomunikat);
            powrot.rysuj(window);
        }
        else if (aktualnyStan == Menus::ZALADUJ)
        {
            window.clear(sf::Color::Black);
            for (auto &p : pliki_przyciski)
            {
                p.aktualizuj(mousePos);
                p.rysuj(window);
            }
            powrot.aktualizuj(mousePos);
            powrot.rysuj(window);
        }
    }

    bool czyWrocic() const
    {
        return wrocDoMenu;
    }

    void inicjalizujPrzyciski()
    {
        wgrajZestaw.inicjalizuj(font, "WGRAJ ZESTAW", 50, sf::Vector2f(400, 100),
                                sf::Color::White, sf::Color::Blue);
        zaladujZestaw.inicjalizuj(font, L"ZAŁADUJ ZESTAW", 50,
                                  sf::Vector2f(400, 200), sf::Color::White,
                                  sf::Color::Blue);
        powrot.inicjalizuj(font, L"POWRÓT", 35, sf::Vector2f(400, 750),
                           sf::Color::White, sf::Color::Cyan);
        menu.inicjalizuj({wgrajZestaw, zaladujZestaw, powrot});
        skanujFolder();
    }

    void reset()
    {
        wrocDoMenu = false;
        aktualnyStan = Menus::GLOWNE;
    }

    std::string pobierzWybranyPlik() const
    {
        return wybranyPlik;
    }
};

// Główna klasa aplikacji - zarządza stanami i główną pętlą
class Aplikacja
{
  private:
    sf::Font font;
    sf::RenderWindow window;

    ZestawFiszek zestaw;
    SesjaNauki sesja;

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

    enum class Menus
    {
        GLOWNE,
        GRY,
        EKRAN_FISZEK,
        SNAKE,
        USTAWIENIA
    };
    Menus aktualnyStan = Menus::GLOWNE;

    GraSnake graSnake;
    Ustawienia menuUstawienia;

    sf::Vector2i mousePos;

  public:
    Aplikacja()
        : sesja(zestaw), graSnake(window, zestaw, font),
          menuUstawienia(window, zestaw, font)
    {
        window.create(sf::VideoMode(800, 800), "Gra edukacyjna");
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf"))
        {
            throw std::runtime_error("Nie udało się wczytać czcionki!");
        }

        umiem.inicjalizuj(font, "UMIEM", 30, sf::Vector2f(250, 600),
                          sf::Color::Green, sf::Color::White);
        nieUmiem.inicjalizuj(font, "NIE UMIEM", 30, sf::Vector2f(550, 600),
                             sf::Color::Red, sf::Color::White);
        powrot.inicjalizuj(font, L"POWRÓT", 35, sf::Vector2f(400, 700),
                           sf::Color::White, sf::Color::Cyan);
        wyjdz.inicjalizuj(font, L"WYJDŹ", 35, sf::Vector2f(400, 700),
                          sf::Color::White, sf::Color::Red);
        graj.inicjalizuj(font, "GRAJ", 50, sf::Vector2f(400, 100),
                         sf::Color::White, sf::Color::Blue);
        ustawienia.inicjalizuj(font, "USTAWIENIA", 50, sf::Vector2f(400, 200),
                               sf::Color::White, sf::Color::Blue);
        fiszki.inicjalizuj(font, "FISZKI", 50, sf::Vector2f(400, 100),
                           sf::Color::White, sf::Color::Blue);
        snake.inicjalizuj(font, "SNAKE", 50, sf::Vector2f(400, 200),
                          sf::Color::White, sf::Color::Blue);

        main_menu.inicjalizuj({graj, ustawienia, wyjdz});
        games_menu.inicjalizuj({fiszki, snake, powrot});

        graSnake.inicjalizujPrzyciski();
        menuUstawienia.inicjalizujPrzyciski();
    }

    void obsluzEventy()
    {
        sf::Event event;
        mousePos = sf::Mouse::getPosition(window);

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                if (aktualnyStan == Menus::GLOWNE)
                {
                    if (main_menu.klikniety(mousePos) == 0)
                    {
                        aktualnyStan = Menus::GRY;
                    }
                    else if (main_menu.klikniety(mousePos) == 1)
                    {
                        aktualnyStan = Menus::USTAWIENIA;
                        menuUstawienia.reset();
                    }
                    else if (main_menu.klikniety(mousePos) == 2)
                    {
                        window.close();
                    }
                }

                else if (aktualnyStan == Menus::GRY)
                {
                    if (games_menu.klikniety(mousePos) == 0)
                    {
                        aktualnyStan = Menus::EKRAN_FISZEK;
                        zestaw.wyczysc();
                        zestaw.wczytaj_z_pliku(
                            menuUstawienia.pobierzWybranyPlik());
                        sesja.reset();
                        sesja.przygotujNowaFiszke();
                    }
                    else if (games_menu.klikniety(mousePos) == 1)
                    {
                        if (!(menuUstawienia.pobierzWybranyPlik() == ""))
                        {
                            aktualnyStan = Menus::SNAKE;
                            zestaw.wyczysc();
                            zestaw.wczytaj_z_pliku(
                                menuUstawienia.pobierzWybranyPlik());
                        }
                    }
                    else if (games_menu.klikniety(mousePos) == 2)
                    {
                        aktualnyStan = Menus::GLOWNE;
                    }
                }

                else if (aktualnyStan == Menus::USTAWIENIA)
                {
                    menuUstawienia.obsluzEvent(event, mousePos);
                }
            }

            if (aktualnyStan == Menus::EKRAN_FISZEK)
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Space)
                    {
                        sesja.obrocFiszke();
                    }
                }

                if (event.type == sf::Event::MouseButtonPressed &&
                    event.mouseButton.button == sf::Mouse::Left)
                {
                    if (powrot.klikniety(mousePos))
                    {
                        aktualnyStan = Menus::GRY;
                    }
                    else if (umiem.klikniety(mousePos))
                    {
                        sesja.oznaczJakoOpanowana();
                    }
                    else if (nieUmiem.klikniety(mousePos))
                    {
                        sesja.oznaczJakoNieopanowana();
                    }
                }
            }
            if (aktualnyStan == Menus::SNAKE)
            {
                graSnake.obsluzEventy(event, mousePos);
            }
        }
    }

    void aktualizuj()
    {
        if (aktualnyStan == Menus::SNAKE)
        {
            graSnake.aktualizuj();
        }
        if (aktualnyStan == Menus::SNAKE && graSnake.czyWrocic())
        {
            aktualnyStan = Menus::GRY;
            graSnake.reset();
        }
        if (aktualnyStan == Menus::USTAWIENIA && menuUstawienia.czyWrocic())
        {
            aktualnyStan = Menus::GLOWNE;
        }
    }

    void rysuj()
    {
        mousePos = sf::Mouse::getPosition(window);
        if (aktualnyStan == Menus::GLOWNE)
        {
            main_menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            main_menu.rysuj(window);
        }
        else if (aktualnyStan == Menus::GRY)
        {
            games_menu.aktualizuj(mousePos);
            window.clear(sf::Color::Black);
            games_menu.rysuj(window);
        }
        else if (aktualnyStan == Menus::EKRAN_FISZEK)
        {
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

            if (sesja.czyObrocona())
            {
                tloFiszki.setFillColor(sf::Color(255, 165, 0));
            }
            else
            {
                tloFiszki.setFillColor(sf::Color::Yellow);
            }

            sf::FloatRect textBounds = tekst.getLocalBounds();
            tekst.setOrigin(textBounds.left + textBounds.width / 2,
                            textBounds.top + textBounds.height / 2);
            tekst.setPosition(400, 400);

            if (sesja.czyPokazacTlo())
            {
                window.draw(tloFiszki);
            }
            else
            {
                tekst.setFillColor(sf::Color::Cyan);
            }

            window.draw(tekst);

            umiem.aktualizuj(mousePos);
            nieUmiem.aktualizuj(mousePos);
            umiem.rysuj(window);
            nieUmiem.rysuj(window);
            powrot.rysuj(window);
        }
        else if (aktualnyStan == Menus::SNAKE)
        {
            window.clear(sf::Color::Black);
            graSnake.rysuj(mousePos);
        }
        else if (aktualnyStan == Menus::USTAWIENIA)
        {
            menuUstawienia.rysuj(mousePos);
        }
    }

    void uruchom()
    {
        // Główna pętla gry - obsługa eventów, aktualizacja i rysowanie
        while (window.isOpen())
        {
            obsluzEventy();
            aktualizuj();
            rysuj();
            window.display();
        }
    }
};

int main()
{
    Aplikacja aplikacja;
    aplikacja.uruchom();
    return 0;
}