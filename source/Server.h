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



/// Przestrzeñ klas i funkcji oraz sta³ych zwi¹zanych z dzia³aniem serwera HTTP.
/**
 * Pozostaj¹ w du¿ym zwi¹zku z przestrzeni¹ TCP - komunikacja przebiega za pomoc¹ gniazd
 * przy wykorzystaniu serwisów reaktywnych. 
 * Komunikacja z frontendem u¿ytkownika przebiega asynchronicznie za pomoc¹ serwisów proaktywnych,
 * - wystêpuje wielow¹tkowoœæ w celu zredukowania czasu blokowania.
 */
namespace Http {

/// Sta³a okreœlaj¹ca now¹ linie wykorzystywan¹ przez HTTP.
constexpr auto CRLF = "\r\n";
/// Separator nag³ówków HTTP.
constexpr auto HEADER_SEPARATOR = ": ";




/// Klasa okreœlaj¹ca URI.
/**
 * Pozwala na dekodowanie, tokenizacjê i segmentacjê oraz szeregowanie
 * subdomen, katalogów, zapytañ GET.
 */
class Uri
{
public:
    typedef std::unordered_map<std::string, std::string> QueryType;

    /// Przeprowadza dekodowanie z formatu HTTP.
    static bool Decode(const std::string& in, std::string& out);

    /// Tworzy nowy URI z podanego ³añcucha znaków.
    Uri(const std::string& uri);
    /// Tworzy pusty URI.
    Uri();

    /// Zwraca surowy ³añcuch znaków w postaci, w jakiej je otrzyma³.
    std::string& raw();
    const std::string& raw() const;
    /// Zwraca argumenty zapytania GET
    /**
     * Tokenizacja przebiega po pierwszym znaku '?',
     * rodzielona '&' oraz '='.
     * Elementy nie s¹ dekodowane.
     */
    QueryType query() const;
    /// Zwraca rodzica katalogu.
    /**
     * Dla /foo/bar/ zwraca /foo/bar
     * Dla /foo/bar zwraca /foo
     * Elementy nie s¹ dekodowane.
     */
    Uri parent() const;
    /// Zwraca œcie¿kê bez argumentów GET.
    /**
     * Œcie¿ka nie jest dekodowana.
     */
    Uri absolutePath() const;
    /// Rozdziela œcie¿kê absolutn¹ na poszczególne katalogi.
    /**
     * Segmenty nie s¹ dekodowane.
     */
    std::vector<std::string> segments() const;

private:
    std::string uri;
};



/// Typ wykorzystywany jako cia³o dokumentu HTTP.
typedef std::string BodyType;
/// Typ wykorzystywany jako oznaczenie typu MIME.
typedef std::string MediaType;



/// Posiada dostêpne domyœlne metody HTTP.
namespace RequestMethod
{
#define MAKE_HTTP_METHOD(methodName) constexpr auto methodName = #methodName
    MAKE_HTTP_METHOD(OPTIONS);
    MAKE_HTTP_METHOD(GET);
    MAKE_HTTP_METHOD(HEAD);
    MAKE_HTTP_METHOD(POST);
    MAKE_HTTP_METHOD(PUT);
    MAKE_HTTP_METHOD(TRACE);
    MAKE_HTTP_METHOD(CONNECT);
    constexpr auto DELETE = "DELETE"; //< Ze wzglêdu na ekspansjê makra DELETE.
#undef MAKE_HTTP_METHOD
}



/// Struktura okreœlaj¹ca wersjê HTTP.
struct Version
{
    int major, minor;
};



/// Typ wykorzystywany do reprezentacji Nag³ówka.
typedef std::pair<std::string /* name */, std::string /* value */> Header;
/// Typ wykorzystywany do kolekcji nag³ówków.
//typedef std::unordered_map<Header::first_type, Header::second_type> HeaderContainer;
typedef std::vector<Header> HeaderContainer;



/// Klasa okreœlaj¹ca zapytanie HTTP.
class Request
{
public:
    /// Zwraca ci¹g znaków, jaki otrzymany by³ w zapytaniu.
    std::string raw() const;
    /// Zwraca metodê HTTP.
    const std::string& method() const;
    /// Zwraca URI.
    const Uri& uri() const;
    /// Zwraca wersjê w formacie {}.{}
    std::string version() const;
    /// Zwraca kolekcjê wszystkich nag³ówków.
    const HeaderContainer& headers() const;
    /// Zwraca cia³o dokumentu.
    const BodyType body() const;

private:
    /// Posiada informacje szczegó³owe o g³ówynm wierszu.
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



/// Parser zapytañ HTTP.
/**
 * Jest maszyn¹ stanów w celu optymalizacji asynchronicznego odczytywania zapytañ.
 */
class RequestParser
{
public:
    /// Wynik parsowania.
    enum class Result
    {
        Good, //< Zapytanie poprawne pod wzglêdem syntaktycznym.
        Bad, //< Zapytanie niepoprawne pod wzglêdem syntaktycznym.
        Indeterminate //< Zapytanie dotychczas nie zawiera³o b³êdów syntaktycznych.
    };

    /// Tworzy nowy obiekt.
    RequestParser();

    /// Przeprowadza parsowanie dla podanego zasiêgu.
    /**
     * Kontener musi mieæ value_type == char lub kompatybilny i
     * mo¿liwoœæ dostêpu sekwencyjnego.
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

    /// Dodaje do cia³a zapytania dany zasiêg.
    /**
     * Przeprowadza sprawdzenie z nag³ówkiem Content-Length w celu
     * okreslenia maksymalnej wielkoœci cia³a zapytania.
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
    /// Zwraca d³ugoœæ cia³a dla zapytania HTTP.
    static int contentLength(const Request& request);
    /// Sprawdza zgodnoœæ znaku w danej chwili dla zapytania.
    Result consume(char value, Request& request);

    /// Lista mo¿liwych stanów parsera.
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
    /// Lista mo¿liwych statusów odpowiedzi wraz z odpowiadaj¹cymi kodami.
    enum class Status
    {
        /* Informational */
        Continue = 100,
        SwitchingProtocols = 101,

        /* Success */
        Ok = 200,
        Accepted = 201,
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

    /// Tworzy odpowiedŸ z danym kodem i cia³em o podanym typie mediów.
    Response(Status code, const BodyType& content, const MediaType& mediaType);

    /// Zwraca odpowiedŸ w formie, jaka zostanie wys³ana do klienta.
    std::string raw() const;
    /// Zwraca status odpowiedzi.
    Status status() const;

    /// Posiada nag³ówki mo¿liwe do modyfikacji w zale¿noœci od potrzeb.
    /**
     * Nag³ówki nie przechodz¹ walidacji syntaktycznej.
     */
    HeaderContainer headers;

private:
    Status responseStatus;
    std::string response;
};



class HandlerStrategy;


/// Klasa enkapsuluj¹ca po³¹czenie z klientem.
/**
 * Odpowiedzialna za odczytanie zapytania i wywo³anie odpowiedzi.
 */
class Connection// : public std::enable_shared_from_this<Connection>
{
public:
    typedef std::array<char, 8192> BufferType;

    /// Tworzy nowe po³¹czenie dla gniazda oraz klasy obs³uguj¹cej zapytanie.
    /**
     * Klasa obs³uguj¹ca zapytanie ma rolê mened¿era.
     */
    Connection(Tcp::Socket socket, HandlerStrategy& handler);

    /// Otwiera po³¹czenie.
    void start();
    /// Zamyka po³¹czenie.
    void stop();

private:
    /// Odczytuje czêœæ nag³ówkow¹ zapytania HTTP.
    /**
     * W przypadku niepoprawnoœci mo¿e wczeœniej zakoñczyæ po³¹czenie z
     * komunikatem Bad Request.
     */
    void read();
    /// Dokonuje odczytu cia³a zapytania HTTP.
    void readBody();
    void write();

    BufferType buffer;

    Tcp::Socket socket;
    HandlerStrategy& globalHandler;
    RequestParser parser;
    Request request;
};

/// Klasa odpowiedzialna za rozdzia³ zadañ do odpowiednich w¹tków.
class ConnectionPool
{
public:
    typedef std::function<void()> RequestHandler;

    /// Tworzy nowy obiekt o okreœlonym maksymalnym obci¹¿eniu.
    /**
     * Liczba w¹tków dedukowana jest na podstawie wykrytej liczby procesorów.
     */
    ConnectionPool(std::size_t maxLoad = 500);
    /// Tworzy nowy obiekt o okreœlonych parametrach.
    ConnectionPool(std::size_t maxThreads, std::size_t maxLoad = 500);

    ~ConnectionPool();

    /// dodaje zadanie do rozdzielenia.
    /**
     * @return czy zadanie zosta³o dodane do kolejki czy nie.
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



/// Klasa okreœlaj¹ca strategiê podzia³u zadañ serwera HTTP.
class HandlerStrategy
{
public:
    typedef std::function<Response(Request)> RequestHandler;
    typedef std::function<void(RequestHandler)> ConnectionResponse;

    virtual ~HandlerStrategy() = default;

    /// Przekazuje sparsowane zapytanie do funkcji obs³uguj¹cej.
    /**
     * Sposób wywo³ania funkcji le¿y po stronie implementacji.
     */
    virtual void handle(ConnectionResponse response) = 0;
    /// Bezpoœrednio wysy³a odpowiedŸ do gniazda.
    virtual void respond(Tcp::Socket& socket, Response::Status stockResponse) = 0;

    /// Uruchamia po³¹czenie.
    virtual void start(ConnectionPtr connection) = 0;
    /// Koñczy po³¹czenie.
    virtual void stop(ConnectionPtr connection) = 0;
};



/// Klasa odpowiedzialna za strategiê podzia³u zadañ na ograniczon¹ liczbê w¹tków.
class ThreadedHandlerStrategy : public HandlerStrategy
{
public:

    ThreadedHandlerStrategy(RequestHandler handler);

    /// Przekazuje zadanie do oddzielnego w¹tku.
    void handle(ConnectionResponse response) override;
    /// Odpowiada na g³ównym w¹tku.
    void respond(Tcp::Socket& socket, Response::Status stockResponse) override;

    /// Implementuje HandlerStrategy.
    void start(ConnectionPtr connection) override;
    void stop(ConnectionPtr connection) override;

private:
    //std::unordered_set<ConnectionPtr> connections;
    RequestHandler handler;
    ConnectionPool pool;
};



/// Klasa bêd¹ca frontendem u¿ytkownika do serwera HTTP.
/**
 * Za jej pomoc¹ u¿ytkownik mo¿e ustaliæ strategiê 
 * rozdzielania zadañ oraz serwis demultipleksacji po³¹czeñ.
 */
class Server
{
public:
    typedef HandlerStrategy::RequestHandler RequestHandler;
    typedef std::unique_ptr<Tcp::StreamServiceInterface> ServicePtr;
    typedef std::unique_ptr<HandlerStrategy> StrategyPtr;
    /// Tworzy nowy obiekt na danym adresie i porcie.
    /**
     * Daje najwiêksze mo¿liwoœci dostosowania.
     */
    Server(const std::string& host, const std::string& port, ServicePtr service, StrategyPtr globalHandler);
    /// Tworzy nowy obiekt z zadan¹ funkcj¹ odpowiadaj¹c¹ na zapytania.
    /**
     * Obiekt wykorzystuje domyœln¹ strategiê ThreadedHandlerStrategy.
     */
    Server(const std::string& host, const std::string& port, RequestHandler handler, ServicePtr service = ServicePtr(new Tcp::StreamService()));

    /// Uruchamia serwer.
    /**
     * Serwer bêdzie dzia³aæ do czasu otrzymania sygna³u przerwania systemowego.
     * Zwrócona wartoœæ to wartoœæ sygna³u.
     */
    int run();

private:
    /// asynchronicznie akceptuje po³¹czenie.
    void accept();

    ServicePtr service;
    Tcp::Acceptor acceptor;
    Tcp::SignalSet signals;
    StrategyPtr globalHandler;
};

} // namespace Http

#endif // PATR_SERVER_H
