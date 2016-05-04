#ifndef PATR_DLHTTPFILE_UTILITY_H
#define PATR_DLHTTPFILE_UTILITY_H

#include <string>
#include <vector>
#include <functional>

namespace Utility 
{
    /// Funkcja pobiera dane z danego adresu http i wrzuca do podanego bufora.
    /*
     * @param url adres http w postaci std::string
     * @param buffer std::vector<unsigned char>&
     * @throw std::runtime_error what() zawiera przyczynę niepowodzenia
     * wyjątek jest rzucany przy błędach http (4xx i 5xx)
     * przy > 300 dokonywana jest próba odnalezienie zasobu pod podanym adresem
    */
    void dlFileToBuffer(const std::string& url, std::vector<unsigned char>& buffer);


    /// Funkcja zapisuje plik z podanego adresu http do pliku o podanej ścieżce
    /*
     * @param url adres http w postaci std::string
     * @param path ścieżka do zapisania pliku
     * @throw std::runtime_error what() zawiera przyczynę niepowodzenia
     */
    void dlFileToFile(const std::string& url, const std::string& path);



    /// Funkcja pobiera dane z podanej lambdy i wrzuca do bufora
    /*
     * @param buffer bufor danych
     * @param func lambda, która przyjmuje Tcp::Buffer&, uzupełnia go i zwraca ilość wrzuconych danych
     * Przeznaczenie raczej testowe - napisanie lambdy, która podaje dane celem sprawdzenia wyodrębnienia ciała
     */
    void fetchData(std::vector<unsigned char>& buffer, std::function<int(std::pair<char* /* data */, int /* size */>&)> func);
}

#endif //PATR_DLHTTPFILE_UTILITY_H
