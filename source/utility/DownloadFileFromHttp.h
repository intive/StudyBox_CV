#ifndef PATR_DLHTTPFILE_UTILITY_H
#define PATR_DLHTTPFILE_UTILITY_H


#include <vector>

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
}

#endif //PATR_DLHTTPFILE_UTILITY_H
