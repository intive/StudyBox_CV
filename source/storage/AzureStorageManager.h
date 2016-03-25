#pragma once
#include "was/storage_account.h"
#include "was/blob.h"
#include <string>
#include <vector>
#include <sstream>
#include <time.h>

/// Klasa obslugująca komunikację z Azure
/**
* W chwili obecnej pozwala na wysłanie i odebranie pliku z Azure Cloud
*/
class AzureStorageManager
{
public:
    /// Konstruktor z parametrami
    /**
    * Ustawia wartości accountName, containerName oraz accountKey na podstawie otrzymanych parametrów
    */
    AzureStorageManager(std::string accountName, std::string containerName, std::string accountKey);


    /// Domyślny destruktor
    ~AzureStorageManager();


    /// Metoda do pobierania zdjęcia z serwera Azure
    /**
    * Pobiera z serwera plik o podanej w parametrze nazwie
    * true jeśli udało się pobrać zdjęcie lub false kiedy wystąpił jakiś błąd
    * Ustawia pole temporaryFileName na nazwę tymczasowego pliku zapisanego lokalnie
    */
    bool downloadFromServer(std::string fileAddr);


    /// Metoda do wysyłania nowego pliku na serwer Azure
    /**
    * Wysyła na serwer azure plik o ścieżce (pełna ścieżka np "tmp//plik.png") podanej w parametrze
    * Zwraca adres do tego pliku
    */
    std::string uploadToServer(std::string path);


    ///Zwraca nazwę ostatnio pobranego pliku tymczasowego
    std::string getTemporaryFileName();


private:
    /// Domyślny konstruktor
    /**
    * Nadaje domyślne wartości dla pól accountName, containerName oraz accountKey
    */
    AzureStorageManager();

    /// Nazwa konta Azure
    std::string accountName;
    /// Nazwa kontenera Azure
    std::string containerName;
    /// Klucz konta Azure
    std::string accountKey;
    /// Nazwa pliku tymczasowego pobranego z serwera
    std::string temporaryFileName;
};

