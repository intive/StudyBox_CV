#ifndef PATR_REQUESTROUTER_H
#define PATR_REQUESTROUTER_H

#include <map>
#include <string>
#include <functional>


namespace Http
{
    class Request;
    class Response;
}

namespace Router
{
    class RequestRouter
    {
    protected:
        /// Szablon lambdy
        /*
         * @param ciało zapytania http (np. JSON)
         * @return odpowiedź na zapytanie (np. JSON) w postaci std::pair<std::string, int>. int określa kod odpowiedzi Http.
         */
        using EndpointHandler = std::function<std::pair<std::string, int>(const std::string&)>;

        std::map<std::string, EndpointHandler> services;

    public:
        RequestRouter()                                = default;
        ~RequestRouter()                               = default;
        RequestRouter(RequestRouter&&)                 = default;
        RequestRouter& operator=(RequestRouter&&)      = default;

        RequestRouter(const RequestRouter&)            = delete;
        RequestRouter& operator=(const RequestRouter&) = delete;


        /// Flaga określająca, czy router powinien wypisać komunikat na std::cerr w przypadku złapania wyjątku podczas działania handlera.
        bool emitExceptionsToStdcerr = true;


        /// Rejestruje handler do wykonania w przypadku żądania http dla podanego endpointa
        /*
         * @param endpoint - ciąg znaków określający dla jakiego endpointa jest przeznaczony handler, np. "/api/ocr"
         * @param handler - lambda lub obiekt funkcyjny przyjmujący const std::string& i zwracający std::pair<std::string, int>.
         * Jeśli podany end point jest już zarejestrowany, handler jest nadpisywany.
         */
        void registerEndPointService(const std::string& endpoint, EndpointHandler handler);


        /// Wywołuje odpowiedni handler (na podstawie URI zapytania)
        /*
         * @param Request od serwera
         * @return Odpowiedź do serwera z ciałem zawierającym:
         *   wynik działania handlera lub
         *   json z wiadomością o "not found" jeżeli nie ma obsługi żądanego endpointa lub
         *   json z wiadomością o "internal server error" jeżeli złapano wyjątek podczas działania handlera.
         */
        Http::Response routeRequest(const Http::Request& request);
    };
}

#endif //PATR_REQUESTROUTER_H
