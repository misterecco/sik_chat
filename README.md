    Simple client-server chat for university classes

    Author: Tomasz Kępa <tk359746@students.mimuw.edu.pl>

Zadanie 1
=========

**Termin oddania: poniedziałek 9 maja 2016, godzina 19.00
                (liczy się czas na serwerze SVN)**

Należy przygotować parę programów - klient i serwer - służącą do komunikacji
tekstowej pomiędzy użytkownikami (czat). Klient powinien wczytywać tekst z
wejścia i każdą wczytaną linię przesyłać do serwera. Jednocześnie powinien on
odbierać wiadomości (linie) od serwera i wypisywać je na standardowe wyjście.
Serwer powinien odbierać poszczególne wiadomości od klientów i rozsyłać każdą
do wszystkich podłączonych klientów z wyjątkiem nadawcy wiadomości.


Wywołanie programu
------------------

Serwer uruchamiamy poleceniem:

    ./server [port]

Parametr:

port - opcjonalny numer portu, na którym serwer ma odbierać połączenia od
       klientów (liczba dziesiętna); jeśli nie podano argumentu, jako numer
       portu powinna być przyjęta liczba 20160.

Klienta uruchamiamy poleceniem:

    ./client host [port]

Parametry:

    host - nazwa serwera, z którym należy się połączyć;
    port - opcjonalny numer portu serwera, z którym należy się połączyć (liczba
       dziesiętna); jeśli nie podano argumentu, jako numer portu powinna być
       przyjęta liczba 20160.


Protokół
--------

Komunikacja pomiędzy klientem i serwerem odbywa się po TCP. Klient po
uruchomieniu powinien nawiązać połączenie z serwerem, które powinno trwać aż do
zamknięcia programu klienta.

Komunikaty przesyłane pomiędzy serwerem i klientem powinny być następującej
postaci:

1) Liczba 16-bitowa bez znaku, podana w sieciowej kolejności bajtów, nie
większa niż 1000, będąca długością tekstu.

2) Tekst, o długości (w bajtach) podanej we wcześniejszym polu. Jest to jedna
linia wejścia wprowadzonego przez użytkownika, bez końcowego znaku 0 oraz bez
końcowego znaku przejścia do nowej linii.


Wymagania szczegółowe
---------------------

Programy powinny być odporne na podanie niepoprawnych parametrów lub złej liczby
parametrów. W takim przypadku należy wypisać stosowny komunikat o błędzie i
zakończyć program z kodem powrotu 1.

W przypadku otrzymania nieprawidłowych (złośliwych) danych przysłanych przez
sieć:

* program serwera powinien zamknąć problematyczne połączenie, w dalszym
  ciągu poprawnie obsługując pozostałe połączenia;

* program klienta powinien wypisać stosowny komunikat o błędzie i zakończyć się
  z kodem powrotu 100.

W tym zadaniu nie przejmujemy się problemem chwilowego zawieszania się
serwera w sytuacji przepełnienia bufora wysyłania dla gniazda. Innymi słowy,
można założyć, że każde wywołanie funkcji write szybko się zakończy.

Serwer powinien odsyłać klientom (innym niż nadawca) otrzymane wiadomości w
całości; niedozwolone jest dzielenie przez serwer komunikatów na krótsze.

W serwerze można przyjąć ograniczenie 20 na liczbę jednocześnie obsługiwanych
klientów.

W kliencie na standardowe wyjście należy wypisywać tylko komunikaty
otrzymane od serwera, bez żadnych dodatkowych informacji.

Rozwiązanie powinno:
- działać w środowisku Linux;
- być napisane w języku C lub C++ z wykorzystaniem interfejsu gniazd oraz
  mechanizmu poll lub select (nie wolno tworzyć wielu wątków ani procesów,
  jak również korzystać z libevent ani boost::asio);
- kompilować się za pomocą GCC (polecenie gcc lub g++) - wśród parametrów
  należy użyć -Wall.

Ponieważ rozwiązanie będzie automatycznie testowane, należy ściśle
przestrzegać podanej specyfikacji (nazwy plików, protokół komunikacji,
komunikaty na standardowym wyjściu, kody powrotu).


Oddawanie rozwiązania
---------------------

Jako rozwiązanie należy dostarczyć pliki źródłowe oraz plik makefile,
które należy umieścić w repozytorium SVN

    https://svn.mimuw.edu.pl/repos/SIK/

w katalogu

    students/ab123456/zadanie1/

gdzie ab123456 to standardowy login osoby oddającej rozwiązanie, używany
na maszynach wydziału, wg schematu: inicjały, nr indeksu.

W wyniku wykonania polecenia make powinny powstać pliki wykonywalne o nazwach
'client' i 'server'.


Ocena
-----

Ocena zadania będzie składała się z trzech składników:
- ocena wzrokowa działania programu (20%);
- testy automatyczne (60%);
- jakość kodu źródłowego (20%).

Każda rozpoczęta minuta spóźnienia będzie kosztowała 0.01 punktu.

