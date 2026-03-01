# QuizChallenge

Edukacyjna aplikacja łącząca naukę z fiszek z mechaniką gry Snake. Pozwala na interaktywne utrwalanie wiedzy oraz importowanie własnych zestawów pytań.

---

### Funkcje programu
* **Tryb Fiszki:** Przeglądanie pytań na interaktywnych kartach z systemem samooceny (UMIEM / NIE UMIEM).
* **Tryb Snake:** Klasyczna gra zręcznościowa, w której zebranie owocu wymaga odpowiedzi na pytanie z aktualnego zestawu.
* **Własne zestawy:** Możliwość ładowania plików `.txt` z folderu `zestawy` (format: `pytanie;odpowiedź`).

---

### Wymagania techniczne
* **Język:** C++17 (lub nowszy).
* **Biblioteka:** SFML 2.5.1.
* **Kompilacja:** `g++ quiz.cpp -o quiz -lsfml-graphics -lsfml-window -lsfml-system`
* **Uruchomienie:** `./quiz`

---

### Sterowanie
* **Menu:** Myszka
* **Fiszki:** `Spacja` (obrót karty), przyciski ekranowe do oceny
* **Snake:** `Strzałki` (ruch), `Enter` (zatwierdzenie odpowiedzi)

---

### Kontakt
* **Autor:** Adam Gołębiowski
* **Email:** a.golebiowski@zset.leszno.pl
