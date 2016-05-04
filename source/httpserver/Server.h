#ifndef PATR_SERVER_H
#define PATR_SERVER_H

#include <memory>

#include <array>
#include <queue>
#include <vector>
#include <unordered_set>

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

//#include "Socket.h"

namespace Tcp {

class Socket;
class StreamServiceInterface;
}

/// Przestrzeń klas i funkcji oraz stałych związanych z działaniem serwera HTTP.
/**
 * Pozostają w dużym związku z przestrzenią TCP - komunikacja przebiega za pomocą gniazd
 * przy wykorzystaniu serwisów reaktywnych. 
 * Komunikacja z frontendem użytkownika przebiega asynchronicznie za pomocą serwisów proaktywnych,
 * - występuje wielowątkowość w celu zredukowania czasu blokowania.
 */
namespace Http {

enum class ResponseStatus;
class Response;
class Request;

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
    ~Connection();

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

    HandlerStrategy& globalHandler;
    struct ConnectionPimpl;
    std::unique_ptr<ConnectionPimpl> pimpl;
    //Tcp::Socket socket;
    //RequestParser parser;
    //Request request;
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
    virtual void respond(Tcp::Socket& socket, ResponseStatus stockResponse) = 0;

    /// Uruchamia połączenie.
    virtual void start(ConnectionPtr connection) = 0;
    /// Kończy połączenie.
    virtual void stop(ConnectionPtr connection) = 0;
};



/// Klasa odpowiedzialna za strategię podziału zadań na ograniczoną liczbę wątków.
class ThreadedHandlerStrategy : public HandlerStrategy
{
public:
    /// Tworzy obiekt obsługujący kolejkę maxRequests zapytań za pomocą podanej funkcji na nThreads wątkach.
    /**
     * @param nThreads wartość 0 ustawia liczbę wątków równą wykrytej liczbie procesorów.
     */
    ThreadedHandlerStrategy(RequestHandler handler, std::size_t maxRequests = 5000U,  std::size_t nThreads = 0U);

    /// Przekazuje zadanie do oddzielnego wątku.
    void handle(ConnectionResponse response) override;
    /// Odpowiada na głównym wątku.
    void respond(Tcp::Socket& socket, ResponseStatus stockResponse) override;

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
    Server(const std::string& host, const std::string& port, RequestHandler handler, ServicePtr service);
    Server(const std::string& host, const std::string& port, RequestHandler handler);
    ~Server();
    /// Uruchamia serwer.
    /**
     * Serwer będzie działać do czasu otrzymania sygnału przerwania systemowego.
     * Zwraca 0 w przypadku złapania wyjątku, w przeciwnym razie nie wraca.
     */
    int run();

private:
    /// asynchronicznie akceptuje połączenie.
    void accept();

    struct ServerPimpl;
    std::unique_ptr<ServerPimpl> pimpl;

    //ServicePtr service;
    //Tcp::Acceptor acceptor;
    //Tcp::SignalSet signals;
    StrategyPtr globalHandler;
};

} // namespace Http

#endif // PATR_SERVER_H
