#pragma once
#include "was/storage_account.h"
#include "was/blob.h"
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <ostream>
#include <stdexcept>

class AzureStorageError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

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
    AzureStorageManager(const std::string& accountName, const std::string& containerName, const std::string& accountKey);

    /// Tworzy obiekt z kluczem zawartym w pliku o podanej ścieżce.
    /*
     * @throw AzureStorageError w przypadku nie odnalezienia pliku.
     */
    AzureStorageManager(const std::string& keyPath);

    /// Domyślny destruktor
    ~AzureStorageManager();


    /// Metoda do pobierania zdjęcia z serwera Azure
    /**
    * Pobiera z serwera plik o podanej w parametrze nazwie
    * @throw w przypadku wystąpienia błędu
    * W przypadku pustego argumentu destination ustawia pole temporaryFileName na nazwę tymczasowego pliku zapisanego lokalnie
    */
    void downloadToFile(const std::string& fileAddr, std::string destination = "");
    /// Pobiera obiekt do bufora (std::vector)
    std::vector<unsigned  char> downloadToBuffer(const std::string& fileAddr);
    /// Pobiera obiekt do bufora tekstowego
    std::string downloadToString(const std::string& fileAddr);
    /// Metoda do wysyłania nowego pliku na serwer Azure
    /**
    * Wysyła na serwer azure plik o podanej ścieżce (pełna ścieżka np "tmp//plik.png")
    * @param path ścieżka do pliku
    * @return adres do tego pliku
    */
    std::string uploadToServer(const std::string& path);


    ///Zwraca nazwę ostatnio pobranego pliku tymczasowego
    std::string getTemporaryFileName();


private:
    /// Domyślny konstruktor
    /**
    * Nadaje domyślne wartości dla pól accountName, containerName oraz accountKey
    */
    AzureStorageManager();

    /// Tworzy obiekt wykorzystywany przez funkcje downloadTo...()
    azure::storage::cloud_block_blob prepareForDownload(const std::string& fileAddr) const;

    /// Nazwa konta Azure
    std::string accountName;
    /// Nazwa kontenera Azure
    std::string containerName;
    /// Klucz konta Azure
    std::string accountKey;
    /// Nazwa pliku tymczasowego pobranego z serwera
    std::string temporaryFileName;
};

