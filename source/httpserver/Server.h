#ifndef PATR_SERVER_H
#define PATR_SERVER_H

#include <memory>

#include <list>
#include <array>
#include <queue>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "Socket.h"


/// Przestrzeń klas i funkcji oraz stałych związanych z działaniem serwera HTTP.
/**
 * Pozostają w dużym związku z przestrzenią TCP - komunikacja przebiega za pomocą gniazd
 * przy wykorzystaniu serwisów reaktywnych. 
 * Komunikacja z frontendem użytkownika przebiega asynchronicznie za pomocą serwisów proaktywnych,
 * - występuje wielowątkowość w celu zredukowania czasu blokowania.
 */
namespace Http {

/// Stała określająca nową linie wykorzystywaną przez HTTP.
constexpr auto CRLF = "\r\n";
/// Separator nagłówków HTTP.
constexpr auto HEADER_SEPARATOR = ": ";




/// Klasa określająca URI.
/**
 * Pozwala na dekodowanie, tokenizację i segmentację oraz szeregowanie
 * subdomen, katalogów, zapytań GET.
 */
class Uri
{
public:
    typedef std::unordered_map<std::string, std::string> QueryType;

    /// Przeprowadza dekodowanie z formatu HTTP.
    static bool Decode(const std::string& in, std::string& out);

    /// Tworzy nowy URI z podanego łańcucha znaków.
    Uri(const std::string& uri);
    /// Tworzy pusty URI.
    Uri();

    /// Zwraca surowy łańcuch znaków w postaci, w jakiej je otrzymał.
    std::string& raw();
    const std::string& raw() const;
    /// Zwraca argumenty zapytania GET
    /**
     * Tokenizacja przebiega po pierwszym znaku '?',
     * rodzielona '&' oraz '='.
     * Elementy nie są dekodowane.
     */
    QueryType query() const;
    /// Zwraca rodzica katalogu.
    /**
     * Dla /foo/bar/ zwraca /foo/bar
     * Dla /foo/bar zwraca /foo
     * Elementy nie są dekodowane.
     */
    Uri parent() const;
    /// Zwraca ścieżkę bez argumentów GET.
    /**
     * Ścieżka nie jest dekodowana.
     */
    Uri absolutePath() const;
    /// Rozdziela ścieżkę absolutną na poszczególne katalogi.
    /**
     * Segmenty nie są dekodowane.
     */
    std::vector<std::string> segments() const;

private:
    std::string uri;
};



/// Typ wykorzystywany jako ciało dokumentu HTTP.
typedef std::string BodyType;
/// Typ wykorzystywany jako oznaczenie typu MIME.
typedef std::string MediaType;



/// Posiada dostępne domyślne metody HTTP.
namespace RequestMethod
{
    constexpr auto OPTIONS = "OPTIONS";
    constexpr auto GET = "GET";
    constexpr auto HEAD = "HEAD";
    constexpr auto POST = "POST";
    constexpr auto PUT = "PUT";
    constexpr auto TRACE = "TRACE";
    constexpr auto CONNECT = "CONNECT";
    constexpr auto DELETE = "DELETE";
}



/// Struktura określająca wersję HTTP.
struct Version
{
    int major, minor;
};



/// Typ wykorzystywany do reprezentacji Nagłówka.
typedef std::pair<std::string /* name */, std::string /* value */> Header;
/// Typ wykorzystywany do kolekcji nagłówków.
//typedef std::unordered_map<Header::first_type, Header::second_type> HeaderContainer;
typedef std::vector<Header> HeaderContainer;



/// Klasa określająca zapytanie HTTP.
class Request
{
public:
    /// Zwraca ciąg znaków, jaki otrzymany był w zapytaniu.
    std::string raw() const;
    /// Zwraca metodę HTTP.
    const std::string& method() const;
    /// Zwraca URI.
    const Uri& uri() const;
    /// Zwraca wersję w formacie {}.{}
    std::string version() const;
    /// Zwraca kolekcję wszystkich nagłówków.
    const HeaderContainer& headers() const;
    /// Zwraca ciało dokumentu.
    const BodyType body() const;

private:
    /// Posiada informacje szczegółowe o główynm wierszu.
    struct RequestLine
    {
        std::string method;
        Uri uri;
        Version version;
    } requestLine;
    
    HeaderContainer headerCollection;
    BodyType content;

    friend class RequestParser;
};



/// Parser zapytań HTTP.
/**
 * Jest maszyną stanów w celu optymalizacji asynchronicznego odczytywania zapytań.
 */
class RequestParser
{
public:
    /// Wynik parsowania.
    enum class Result
    {
        Good, //< Zapytanie poprawne pod względem syntaktycznym.
        Bad, //< Zapytanie niepoprawne pod względem syntaktycznym.
        Indeterminate //< Zapytanie dotychczas nie zawierało błędów syntaktycznych.
    };

    /// Tworzy nowy obiekt.
    RequestParser();

    /// Przeprowadza parsowanie dla podanego zasięgu.
    /**
     * Kontener musi mieć value_type == char lub kompatybilny i
     * możliwość dostępu sekwencyjnego.
     */
    template<typename InputIt>
    std::pair<Result, InputIt> parse(InputIt begin, InputIt end, Request& request)
    {
        while (begin != end)
        {
            Result result = consume(*begin++, request);
            if (result == Result::Good || result == Result::Bad)
                return std::make_pair(result, begin);
        }
        return std::make_pair(Result::Indeterminate, begin);
    }

    /// Dodaje do ciała zapytania dany zasięg.
    /**
     * Przeprowadza sprawdzenie z nagłówkiem Content-Length w celu
     * okreslenia maksymalnej wielkości ciała zapytania.
     */
    template<typename InputIt>
    static bool fill(InputIt begin, InputIt end, Request& request)
    {
        std::size_t max = contentLength(request);

        if (request.content.length() == max)
            return true;

        while (begin != end)
        {
            request.content += *begin++;
            if (request.content.length() == max)
                return true;
        }
        return false;
    }

    void reset();

private:
    /// Zwraca długość ciała dla zapytania HTTP.
    static int contentLength(const Request& request);
    /// Sprawdza zgodność znaku w danej chwili dla zapytania.
    Result consume(char value, Request& request);

    /// Lista możliwych stanów parsera.
    enum State
    {
        MethodStart,
        Method,
        Uri,
        HttpVersionH,
        HttpVersionT1,
        HttpVersionT2,
        HttpVersionP,
        HttpVersionSlash,
        HttpVersionMajorStart,
        HttpVersionMajor,
        HttpVersionMinorStart,
        HttpVersionMinor,
        ExpectingNewline1,
        HeaderLineStart,
        HeaderLws,
        HeaderName,
        SpaceBeforeHeaderValue,
        HeaderValue,
        ExpectingNewline2,
        ExpectingNewline3
    } state;
};



/// Klasa odpowiedzi HTTP.
class Response
{
public:
    /// Lista możliwych statusów odpowiedzi wraz z odpowiadającymi kodami.
    enum class Status
    {
        /* Informational */
        Continue = 100,
        SwitchingProtocols = 101,

        /* Success */
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NonAuthoritativeInformation =  203,
        NoContent = 204,
        ResetContent = 205,
        PartialContent = 206,

        /* Redirection */
        MultipleChoices = 300,
        MovedPermanently = 301,
        Found = 302,
        SeeOther = 303,
        NotModified = 304,
        UseProxy = 305,
        TemporaryRedirect = 307,

        /* Client Error */
        BadRequest = 400,
        Unauthorized = 401,
        PaymentRequired = 402,
        Forbidden = 403,
        NotFound = 404,
        MethodNotAllowed = 405,
        NotAcceptable = 406,
        ProxyAuthenticationRequired = 407,
        RequestTimeout = 408,
        Conflict = 409,
        Gone = 410,
        LengthRequired = 411,
        PreconditionFailed = 412,
        RequestEntityTooLarge = 413,
        RequestUriTooLarge = 414,
        UnsupportedMediaType = 415,
        RequestedRangeNotSatisfiable = 416,
        ExpectationFailed = 417,

        /* Server Error */
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
        GatewayTimeout = 504,
        HttpVersionNotSupported = 505
    };

    /// Tworzy odpowiedź z danym kodem i ciałem o podanym typie mediów.
    Response(Status code, const BodyType& content, const MediaType& mediaType);

    /// Zwraca odpowiedź w formie, jaka zostanie wysłana do klienta.
    std::string raw() const;
    /// Zwraca status odpowiedzi.
    Status status() const;

    /// Posiada nagłówki możliwe do modyfikacji w zależności od potrzeb.
    /**
     * Nagłówki nie przechodzą walidacji syntaktycznej.
     */
    HeaderContainer headers;

private:
    Status responseStatus;
    std::string response;
};



class HandlerStrategy;

/// Klasa enkapsulująca połączenie z klientem.
/**
 * Odpowiedzialna za odczytanie zapytania i wywołanie odpowiedzi.
 */
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    typedef std::array<char, 8192> BufferType;

    /// Tworzy nowe połączenie dla gniazda oraz klasy obsługującej zapytanie.
    /**
     * Klasa obsługująca zapytanie ma rolę menedżera.
     */
    Connection(Tcp::Socket socket, HandlerStrategy& handler);

    /// Otwiera połączenie.
    void start();
    /// Zamyka połączenie.
    void stop();

private:
    /// Odczytuje część nagłówkową zapytania HTTP.
    /**
     * W przypadku niepoprawności może wcześniej zakończyć połączenie z
     * komunikatem Bad Request.
     */
    void read();
    /// Dokonuje odczytu ciała zapytania HTTP.
    void readBody();
    void write();

    BufferType buffer;

    Tcp::Socket socket;
    HandlerStrategy& globalHandler;
    RequestParser parser;
    Request request;
};

/// Klasa odpowiedzialna za rozdział zadań do odpowiednich wątków.
class ConnectionPool
{
public:
    typedef std::function<void()> RequestHandler;

    /// Tworzy nowy obiekt o określonym maksymalnym obciążeniu.
    /**
     * Liczba wątków dedukowana jest na podstawie wykrytej liczby procesorów.
     */
    ConnectionPool(std::size_t maxLoad = 5000);
    /// Tworzy nowy obiekt o określonych parametrach.
    ConnectionPool(std::size_t maxThreads, std::size_t maxLoad = 5000);

    ~ConnectionPool();

    /// dodaje zadanie do rozdzielenia.
    /**
     * @return czy zadanie zostało dodane do kolejki czy nie.
     */
    bool add(RequestHandler handler);

private:
    std::size_t maxThreads;
    std::size_t maxLoad;

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;

    // synchronization
    std::mutex mutex;
    std::condition_variable condition;
    bool stop;
};



using ConnectionPtr = std::shared_ptr<Connection>;



/// Klasa określająca strategię podziału zadań serwera HTTP.
class HandlerStrategy
{
public:
    typedef std::function<Response(Request)> RequestHandler;
    typedef std::function<void(RequestHandler)> ConnectionResponse;

    virtual ~HandlerStrategy() = default;

    /// Przekazuje sparsowane zapytanie do funkcji obsługującej.
    /**
     * Sposób wywołania funkcji leży po stronie implementacji.
     */
    virtual void handle(ConnectionResponse response) = 0;
    /// Bezpośrednio wysyła odpowiedź do gniazda.
    virtual void respond(Tcp::Socket& socket, Response::Status stockResponse) = 0;

    /// Uruchamia połączenie.
    virtual void start(ConnectionPtr connection) = 0;
    /// Kończy połączenie.
    virtual void stop(ConnectionPtr connection) = 0;
};



/// Klasa odpowiedzialna za strategię podziału zadań na ograniczoną liczbę wątków.
class ThreadedHandlerStrategy : public HandlerStrategy
{
public:

    ThreadedHandlerStrategy(RequestHandler handler);

    /// Przekazuje zadanie do oddzielnego wątku.
    void handle(ConnectionResponse response) override;
    /// Odpowiada na głównym wątku.
    void respond(Tcp::Socket& socket, Response::Status stockResponse) override;

    /// Implementuje HandlerStrategy.
    void start(ConnectionPtr connection) override;
    void stop(ConnectionPtr connection) override;

private:
    std::unordered_set<ConnectionPtr> connections;
    RequestHandler handler;
    ConnectionPool pool;
};



/// Klasa będąca frontendem użytkownika do serwera HTTP.
/**
 * Za jej pomocą użytkownik może ustalić strategię 
 * rozdzielania zadań oraz serwis demultipleksacji połączeń.
 */
class Server
{
public:
    typedef HandlerStrategy::RequestHandler RequestHandler;
    typedef std::unique_ptr<Tcp::StreamServiceInterface> ServicePtr;
    typedef std::unique_ptr<HandlerStrategy> StrategyPtr;
    /// Tworzy nowy obiekt na danym adresie i porcie.
    /**
     * Daje największe możliwości dostosowania.
     */
    Server(const std::string& host, const std::string& port, ServicePtr service, StrategyPtr globalHandler);
    /// Tworzy nowy obiekt z zadaną funkcją odpowiadającą na zapytania.
    /**
     * @param handler funkcja lub obiekt funkcyjny obsługujący argument Http::Request i zwracający Http::Response.
     * Obiekt wykorzystuje domyślną strategię ThreadedHandlerStrategy.
     */
    Server(const std::string& host, const std::string& port, RequestHandler handler, ServicePtr service = ServicePtr(new Tcp::StreamService()));

    /// Uruchamia serwer.
    /**
     * Serwer będzie działać do czasu otrzymania sygnału przerwania systemowego.
     * Zwraca 0 w przypadku złapania wyjątku, w przeciwnym razie nie wraca.
     */
    int run();

private:
    /// asynchronicznie akceptuje połączenie.
    void accept();

    ServicePtr service;
    Tcp::Acceptor acceptor;
    Tcp::SignalSet signals;
    StrategyPtr globalHandler;
};

} // namespace Http

#endif // PATR_SERVER_H
