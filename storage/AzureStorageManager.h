#pragma once
#include "was/storage_account.h"
#include "was/blob.h"
#include <string>
#include <vector>
#include <sstream>
#include <time.h>

/// Klasa obsluguj¹ca komunikacjê z Azure
/**
* W chwili obecnej pozwala na wys³anie i odebranie pliku z Azure Cloud
*/
class AzureStorageManager
{
public:
    /// Domyœlny konstruktor
    /**
    * Nadaje domyœlne wartoœci dla pól accountName, containerName oraz accountKey
    */
    AzureStorageManager();


    /// Konstruktor z parametrami
    /**
    * Ustawia wartoœci accountName, containerName oraz accountKey na podstawie otrzymanych parametrów
    */
    AzureStorageManager(std::string _accountName, std::string _containerName, std::string _accountKey);


    /// Domyœlny destruktor
    ~AzureStorageManager();


    /// Metoda do pobierania zdjêcia z serwera Azure
    /**
    * Pobiera z serwera plik o podanej w parametrze nazwie
    * true jeœli uda³o siê pobraæ zdjêcie lub false kiedy wyst¹pi³ jakiœ b³¹d
    * Ustawia pole temporaryFileName na nazwê tymczasowego pliku zapisanego lokalnie
    */
    bool downloadFromServer(std::string _fileAddr);


    /// Metoda do wysy³ania nowego pliku na serwer Azure
    /**
    * Wysy³a na serwer azure plik o œcie¿ce (pe³na œcie¿ka np "tmp//plik.png") podanej w parametrze
    * Zwraca adres do tego pliku
    */
    std::string uploadToServer(std::string _path);


    ///Zwraca nazwê ostatnio pobranego pliku tymczasowego
    std::string getTemporaryFileName();


private:
    /// Nazwa konta Azure
    std::string accountName;
    /// Nazwa kontenera Azure
    std::string containerName;
    /// Klucz konta Azure
    std::string accountKey;
    /// Nazwa pliku tymczasowego pobranego z serwera
    std::string temporaryFileName;
};

