#ifndef PATR_SOCKET_H
#define PATR_SOCKET_H

#include <memory>

#include <set>
#include <map>
#include <queue>
#include <string>
#include <unordered_set>

#include <functional>
#include <chrono>

struct addrinfo;

/// Przestrzeń dla klas i funkcji związanych z niskopoziomową komunikacją TCP/IP.
/**
 * Będące tu klasy można podzielić na 4 części:
 * Service - odpowiedzialne za demultipleksację i komunikację między elementami,
 * Interface - odpowiedzialne za udostępnianie interfejsów w celu rozszerzania funkcjonalności,
 * Implementation - odpowiedzialne za szczegóły implementacyjne, spełniające interfejs powyżej,
 * Pozostałe - służące jako frontend użytkownika.
 *
 * Użytkownik musi powołać obiekt StreamService aby móc wykorzystywać asynchroniczne
 * aspekty api. W przeciwnym wypadku musi spełnić StreamServiceInterface w stopniu,
 * który by na to pozwalał.
 */
namespace Tcp {

// Oznacza, że wykorzystywana funkcja nie jest zaimplementowana.
class NotImplemented : public std::logic_error
{
public:
    using std::logic_error::logic_error;
};

// Bazowa klasa do oznaczania błędów związanych z komunikacją TCP.
class TcpError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

// Błąd związany ze specyficzną platformą
class PlatformError : public TcpError
{
public:
    using TcpError::TcpError;
};

// Błąd związany z gniazdami.
class SocketError : public TcpError
{
public:
    using TcpError::TcpError;
};

// Błąd związany z i/o gniazd.
class StreamError : public SocketError
{
public:
    using SocketError::SocketError;
};

// Błąd wyjścia gniazda.
class SendError : public StreamError
{
public:
    using StreamError::StreamError;
};

// Błąd wejścia gniazda.
class ReceiveError : public StreamError
{
public:
    using StreamError::StreamError;
};

// Błąd w nasłuchiwaniu połączeń.
class AcceptorError : public SocketError
{
public:
    using SocketError::SocketError;
};

// Błąd w związaniu gniazda z adresem.
class BindError : public AcceptorError
{
public:
    using AcceptorError::AcceptorError;
};

// Błąd w przygotowaniu gniazda do nasłuchiwania.
class ListenError : public AcceptorError
{
public:
    using AcceptorError::AcceptorError;
};

// Błąd otwarcia połączenia.
class AcceptError : public AcceptorError
{
public:
    using AcceptorError::AcceptorError;
};

// Błąd demultipleksacji / asynchronicznego wywołania.
class ServiceError : public TcpError
{
public:
    using TcpError::TcpError;
};

// Błąd gniazda wyjściowego.
class EndpointError : public TcpError
{
public:
    using TcpError::TcpError;
};

// Błąd ustawienia opcji gniazda.
class SocketOptionError : public SocketError
{
public:
    using SocketError::SocketError;
};

// Typ wykorzystywany do przesyłu informacji przez gniazda.
typedef std::pair<char* /* data */, int /* size */> Buffer;
typedef std::pair<const char* /* data */, int /* size */> ConstBuffer;






/// Klasa do blokowania kopiowania obiektów.
/**
 * Nie należy usuwać obiektów przez wskaźnik do tej klasy ze względu
 * na brak konstruktora.
 */
class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator =(NonCopyable&&) = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator =(const NonCopyable&) = delete;
};






/// Służy zamianie kontenera na Tcp::Buffer.
/**
 * Zamianie może podlegać dowolny kontener spełniający wymogi
 * ContiguousContainer posiadający value_type char lub równoważny. 
 */
template<typename T>
Buffer MakeBuffer(T& container)
{
    return std::make_pair(container.data(), static_cast<int>(container.size()));
}





/// Służy zamianie kontenera na Tcp::ConstBuffer.
/**
* Zamianie może podlegać dowolny kontener spełniający wymogi
* ContiguousContainer posiadający value_type char lub równoważny.
*/
template<typename T>
ConstBuffer MakeBuffer(const T& container)
{
    return std::make_pair(container.data(), static_cast<int>(container.size()));
}

/// Przestrzeń opcji gniazd tcp.
namespace Option {
    /// Główna struktura, z jakiej korzystaj Tcp::Socket.
    struct Option
    {
        int option;
        int value;
    };

    /// Specjalizacja Tcp::Option dla opcji SO_REUSEADDR.
    struct ReuseAddress : public Option
    {
        ReuseAddress(bool value);
    };

    // Możliwa rozbudowa, jeżeli zajdzie taka potrzeba.
}




class StreamService;
class Socket;
class AcceptorImplementation;
class SocketImplementation;
class SocketInterface;
class AcceptorInterface;
class SocketInterface;
class StreamServiceInterface;




/// Klasa bazowa obiektów zawierających uchwyty.
/**
 * Tutaj uchwytami są deskryptory plików, ze względu na wybrany
 * rodzaj implementacji.
 */
class Service
{
public:
    typedef int HandleType;

    // Gwarantuje dostęp do uchwytu.
    HandleType getHandle() const;
    void setHandle(HandleType handle);

    // porównuje obiekty ze względu na rozmiar ich uchwytu.
    bool operator ==(const Service&) const;
    bool operator !=(const Service&) const;
    bool operator <(const Service&) const;
    bool operator >(const Service&) const;

protected:
    ~Service() = default;

    HandleType handle;
};




/// Interfejs służący do komunikacji z StreamService.
class SignalServiceInterface
{
public:
    typedef int SignalType;

    virtual ~SignalServiceInterface() = default;

    /// W celu sprawdzenia jaki sygnał został wywołany.
    /**
     * @return wartość sygnału jeżeli ten został wywołany.
     */
    virtual int get() const = 0;
    /// W celu sprawdzenia czy sygnał został wywołany.
    virtual bool received() const = 0;
};




/// Służy do sprawdzania zdarzeń przerwań systemowych.
class SignalService : public SignalServiceInterface
{
public:
    SignalService(int& sigVal, bool& sigFlag);

    /// Zwraca jaki sygnał został wywołany.
    /**
    * W przypadku, gdy żaden nie został wywołany,
    * zwracana wartość jest nieokreślona.
    */
    int get() const override;
    /// Zwraca, czy sygnał został wywołany.
    bool received() const override;

private:
    int& sigVal;
    bool& sigFlag;
};





/// Służy do obsługi asynchronicznych wywołań ::accept().
/**
 * Działa jako backend operacji asynchronicznych dla dowolnych
 * implementacji spełniających AcceptorInterface.
 */
class AcceptorService : public Service
{
public:
    /// Funkcja wywołana przy asynchronicznym zdarzezniu.
    /**
     * Socket to gniazdo, dla którego zostało otwarte połączenie.
     */
    typedef std::function<void(Socket)> Handler;
    typedef Service::HandleType HandleType;

    /// Tworzy obiekt z implementacją o interfejsie AcceptorInterface.
    AcceptorService(AcceptorInterface& implementation);

    /// Wybudza implementację w celu dokonania asynchronicznej akcji.
    void acceptReady();
    /// Wprowadza nową funkcję do wywołania w kolejce.
    void enqueue(Handler handler);

private:
    AcceptorInterface& implementation;
    std::queue<Handler> handlers;
};




/// Służy do obsługi asynchronicznych odczytów i zapisów do gniazd.
/**
 * Działa jako backend operacji asynchronicznych dla dowolnych
 * implementacji spełniających SocketInterface.
 */
class SocketService : public Service
{
public:
    typedef Service::HandleType HandleType;
    /// Prototyp funkcji służącej do obsługi asynchronicznego odczytu z gniazda.
    typedef std::function<void(int /*error code */, std::size_t /* bytes read */)> ReadHandler;
    /// Prototyp funkcji służącej do obsługi asynchronicznego zapisu z gniazda.
    /**
     * Asynchroniczny zapis nie jest obecnie zaimplementowany.
     */
    typedef std::function<void(int, int)> WriteHandler;
    /// Bufor wywkorzystywany do komunikacji z implementacją.
    typedef Buffer BufferType;
    typedef ConstBuffer ConstBufferType;

    /// Tworzy obiekt z implementacją o interfejsie SocketInterface.
    SocketService(StreamServiceInterface& service, SocketInterface& implementation, HandleType handle);
    /// Oznacza, że gniazdo jest gotowe do nieblokującego odczytu.
    int readReady();
    /// Oznacza, że gniazdo jest gotowe do nieblokującego zapisu.
    int writeReady();

    /// Służy wprowadzeniu nowych funkcji obsługujących asynchroniczny odczyt.
    void enqueue(BufferType& buffer, ReadHandler handler);
    /// Służy wprowadzeniu nowych funkcji obsługujących asynchroniczny zapis.
    void enqueue(const ConstBufferType& buffer, WriteHandler handler);

    /// Zwraca pozostały czas, po upływie którego nastąpi timeout w [ms].
    int remainingTime() const;
    /// Uaktualnia pozostały czas do timeoutu w [ms].
    void updateRemainingTime(int milliseconds);

    /// Oznacza gniazdo jako gotowe do zamknięcia.
    /**
     * Zamknięcie może nie nastąpić natychmiast.
     */
    void shutdown();

private:
    int shut;
    int timeout, currentTime;
    std::chrono::high_resolution_clock::time_point begin;
    std::queue<std::pair<BufferType, ReadHandler>> readHandlers;
    std::queue<std::pair<ConstBufferType, WriteHandler>> writeHandlers;
    SocketInterface& implementation;
    StreamServiceInterface& service;
};




/// Interfejs dla informacji o gniazdu docelowym.
class EndpointInterface
{
public:
    typedef addrinfo* AddressType;
    typedef addrinfo* ProtocolType;

    virtual ~EndpointInterface() = default;
    /// W celu uzyskania informacji o adresie.
    virtual AddressType address() const = 0;
    /// W celu uzyskania informacji o protokole.
    virtual ProtocolType protocol() const = 0;

    virtual Socket connect(StreamServiceInterface& service) const = 0;
};





/// Klasa zawierająca szczegóły implementacyjne dla wybierania informacji docelowym miejscu.
class EndpointImplementation : public EndpointInterface
{
public:
    typedef addrinfo* AddressType;
    typedef addrinfo* ProtocolType;

    /// Tworzy obiekt o danym adresie i porcie.
    /** 
     * address może przyjmować formę nazwy domeny (np. www.example.com),
     * mnemonikę (np. localhost), IPv4 (np. 0.0.0.0), IPv6 (np 0::0).
     * Port może przyjmować wartości od 1 do 49150 w postaci łańcucha znaków.
     */
    EndpointImplementation(const std::string& address, const std::string& port);
    ~EndpointImplementation();

    /// Zwraca informacje o adresie niezbędne do wykonania operacji.
    AddressType address() const override;
    /// Zwraca informacje o protokole w celu otwarcia gniazda.
    ProtocolType protocol() const override;

    Socket connect(StreamServiceInterface& service) const override;

private:
    AddressType addressVal;
};




/// Służy jako frontend użytkownika.
/**
 * Przekierowuje odpowiedzialność do implementacji.
 */
class Endpoint
{
public:
    typedef EndpointImplementation::AddressType AddressType;
    typedef EndpointImplementation::ProtocolType ProtocolType;

    /// Tworzy obiekt z adresem i portem o domyślnej implementacji.
    Endpoint(const std::string& address, const std::string& port);
    /// Tworzy obiekt o danej implementacji, zawierającej już informacje szczegółowe.
    Endpoint(std::unique_ptr<EndpointInterface> implementation);

    /// Przekierowuje do implementacji.
    AddressType address() const;
    /// Przekierowuje do implementacji.
    ProtocolType protocol() const;

    Socket connect() const;

private:
    std::unique_ptr<EndpointInterface> implementation;
};




/// Jako interfejs klas nasłuchujących połączeń.
class AcceptorInterface : public NonCopyable
{
public:
    typedef AcceptorService::HandleType HandleType;
    typedef AcceptorService::Handler Handler;

    /// Tworzy nowy obiekt, który do asynchronicznej komunikacji wykorzysta
    /// obiekt implementujący StreamServiceInterface.
    AcceptorInterface(StreamServiceInterface&);
    virtual ~AcceptorInterface() = default;

    /// Otwiera gniazdo o danym protokole.
    virtual void open(Endpoint::ProtocolType protocol) = 0;
    /// Ustawia opcję dla gniazda.
    virtual void setOption(const Option::Option& option) = 0;
    /// Przypisuje gniazdo do adresu.
    virtual void bind(Endpoint::AddressType address) = 0;
    /// Flaguje gniazdo do nasłuchiwania na połaczenia.
    /**
     * backlog oznacza maksymalną ilość połączeń oczekujących w kolejce do podłączenia,
     * które nie zostały jeszcze obsłużone.
     */
    virtual void listen(int backlog) = 0;

    /// Blokująca funkcja do przyjęcia połączenia.
    /**
     * Funkcja będzize blokować, dopóki nie zostanie wybudzona przez
     * nadchodzące połączenie.
     */
    virtual Socket accept() = 0;
    /// Asynchroniczna wersja przyjmuje funkcję do obsługi zdarzenia, gdy ono nadejdzie.
    /**
     * Przekazywana funkcja zostanie wykonana, gdy obiekt zostanie wybudzony.
     * Funkcja wraca bez blokowania.
     */
    void asyncAccept(Handler handler);

protected:
    /// Celem ograniczenia interakcji frontendu z obiektami typu *Service.
    HandleType handle() const;
    void handle(HandleType handle);

private:
    AcceptorService service;
};




/// Interfejs gniazd TCP.
class SocketInterface
{
public:
    typedef SocketService::ReadHandler ReadHandler;
    typedef SocketService::WriteHandler WriteHandler;
    typedef SocketService::BufferType BufferType;
    typedef SocketService::ConstBufferType ConstBufferType;
    typedef SocketService::HandleType HandleType;

    /// Tworzy obiekt związany z obiektem typu *Service.
    SocketInterface(StreamServiceInterface& service, HandleType handle);
    virtual ~SocketInterface() = default;

    /// Blokuje, dopóki nie odczyta określonej liczby bajtów.
    /**
     * @return ilość odczytanych bajtów - powinna wynosić rozmiar bufora.
     */
    virtual int read(BufferType& buffer) = 0;
    /// Odczytuje dostępną w tej chwili liczbę bajtów.
    /**
     * Może blokować, jeżeli w danej chwili bufor gniazda jest pusty.
     * @return ilość odczytanych bajtów - powinna być równa ilości dostępnych bajtów na gnieździe.
     */
    virtual int readSome(BufferType& buffer) = 0;
    /// Oznacza asynchroniczne wywołanie funkcji ReadHandler.
    /**
     * Funkcja powinna wracać bez blokowania.
     */
    virtual void asyncReadSome(BufferType buffer, ReadHandler handler);
    /// Odpowiednik read dla zapisu.
    /**
     * @return ilość faktycznie zapisanych bajtów - powinna wynosić rozmiar bufora.
     */
    virtual int write(const ConstBufferType& buffer) = 0;
    /// Odpowiednik readSome dla zapisu.
    /**
     * @return ilość faktycznie zapisanych bajtów.
     */
    virtual int writeSome(const ConstBufferType& buffer) = 0;
    /// Odpowiednik asyncReadSome dla zapisu.
    virtual void asyncWriteSome(const ConstBufferType& buffer, WriteHandler handler);

    /// Bezpośrednio zamyka gniazdo.
    virtual void close() = 0;
    /// Oznacza gniazdo do zamknięcia.
    void shutdown();

protected:
    HandleType handle() const;

private:
    SocketService service;
};


/// Interfejs do tworzenia implementacji interfejsów dla danego serwisu.
class ServiceFactory
{
public:
    /// Tworzy nowy obiekt implementujący AcceptorInterface.
    virtual std::unique_ptr<AcceptorInterface> getImplementation() = 0;
    /// Tworzy nowy obiekt implementujący EndpointInterface.
    virtual std::unique_ptr<EndpointInterface> resolve(const std::string& host, const std::string& port) = 0;
};


/// Interfejs głównego serwisu reaktywnego.
class StreamServiceInterface
{
public:
    virtual ~StreamServiceInterface() = default;

    /// Uruchamia serwis.
    /**
     * Wywołanie powinno blokować do momentu wywołania obsługiwanego przerwania
     * systemowego lub wyzerowania licznika otwartych gniazd.
     */
    virtual int run() = 0;
    /// Dodaje istniejący serwis gniazda.
    virtual void add(SocketService* service);
    /// Usuwa serwis gniazda.
    /**
     * Gniazdo powinno samo zagwarantować poprawną komunikację z serwisem.
     */
    virtual void remove(SocketService* service);
    /// Dodaje istniejący serwis akceptora.
    virtual void add(AcceptorService* service);
    /// Dodaje serwis do obsługi sygnałów.
    /**
     * Obsługiwany jest tylko jeden serwis sygnałów ze względów implementacyjnych.
     */
    virtual void add(SignalService* service);

    /// Zwraca nowy obiekt do tworzenia obiektów klasy spełniających wymagania danego serwisu.
    virtual std::unique_ptr<ServiceFactory> getFactory() = 0;

protected:
    std::set<SocketService*> sockets;
    std::set<AcceptorService*> acceptors;
    SignalService* signal;
};





/// Implementacja fabryki dla klasy StreamService.
/**
 * Tworzy obiekty klasy obsługiwanej przez StreamService.
 */
class StreamServiceFactory : public ServiceFactory, public NonCopyable
{
public:
    StreamServiceFactory(StreamService& service);

    /// Zwraca implementację akceptora zgodną z wymaganiami StreamService.
    virtual std::unique_ptr<AcceptorInterface> getImplementation() override;
    /// Zwraca informacje o gnieździe końcowym o implementacji związanej z StreamService.
    virtual std::unique_ptr<EndpointInterface> resolve(const std::string & host, const std::string & port) override;

private:
    StreamService& service;
};

/// Klasa do demultipleksacji połączeń oraz asynchronicznej komunikacji z innymi serwisami.
/**
 * Demultipleksacji podlegają sygnały gotowości do odczytu, zapisu oraz nowych połaczeń.
 */
class StreamService : public StreamServiceInterface
{
public:
    /// Tworzy nowy obiekt serwisu.
    StreamService();
    /// Zamyka pozostałe otwarte połączenia.
    ~StreamService();

    /// Implementuje interfejs StreamServiceInterface.
    int run() override;
    void add(SocketService* service) override;
    void remove(SocketService* service) override;

    /// Zwraca fabrykę do tworzenia obiektów klas obsługiwanych przez serwis.
    std::unique_ptr<ServiceFactory> getFactory() override;

private:
    /// Wybiera możliwe do odczytu gniazda.
    /**
     * Aktualizuje czas od ostatniego wywołania.
     * Odpowiedzialna za timeout gniazd.
     * @return liczba wybranych gniazd.
     */
    int select(const std::chrono::high_resolution_clock::time_point& start, int& timeSinceLastUpdate);

    struct StreamServicePimpl;
    std::unique_ptr<StreamServicePimpl> pimpl;
};



/// Frontend użytkownika dla klasy nasłuchującej nowych połączeń.
/**
 * Oddelegowuje odpowiedzialność implementacji do klasy implementacyjnej spełniających
 * interfejs AcceptorInterface
 */
class Acceptor : public NonCopyable
{
public:
    typedef AcceptorInterface::Handler Handler;

    /// Tworzy nowy obiekt związany z danym serwisem.
    Acceptor(StreamServiceInterface& service);
    /// Tworzy nowy obiekt o określonej implementacji.
    Acceptor(std::unique_ptr<AcceptorInterface> implementation);

    /// Przekierowuje do implementacji.
    void open(Endpoint::ProtocolType protocol);
    void setOption(const Option::Option& option);
    void bind(Endpoint::AddressType address);
    void listen(int backlog);

    Socket accept();
    void asyncAccept(Handler handler);

private:
    std::unique_ptr<AcceptorInterface> implementation;
};


/// Frontend użytkownika dla klasy gniazda.
/**
 * Udostępnia funkcje odczytu i zapisu blokującego oraz nie dla gniazda TCP.
 */
class Socket : public NonCopyable
{
public:
    typedef SocketInterface::ReadHandler ReadHandler;
    typedef SocketInterface::WriteHandler WriteHandler;
    typedef SocketInterface::BufferType BufferType;
    typedef SocketInterface::ConstBufferType ConstBufferType;
    typedef SocketInterface::HandleType HandleType;

    //Socket(StreamService& service, HandleType handle);
    /// Tworzy nowy obiekt z określoną implementacją.
    Socket(std::unique_ptr<SocketInterface> implementation);

    /// Przekierowuje wywołanie do implementacji.
    int read(BufferType& buffer);
    int readSome(BufferType& buffer);
    void asyncReadSome(BufferType buffer, ReadHandler handler);
    int write(const ConstBufferType& buffer);
    int writeSome(const ConstBufferType& buffer);
    void asyncWriteSome(const ConstBufferType& buffer, WriteHandler handler);

    void close();
    void shutdown();

private:
    std::unique_ptr<SocketInterface> implementation;
};






/// Frontend użytkownika do obsługi sygnałów.
/**
 * Standardowe sygnały, które powinny być 
 * zdefiniowane na wszystkich platformach to:
 * SIGABRT
 * SIGFPE
 * SIGILL
 * SIGINT
 * SIGSEGV
 * SIGTERM
 */
class SignalSet
{
public:
    typedef int SignalType;

    /// Tworzy nowy obiekt współdziałający z serwisem.
    SignalSet(StreamServiceInterface&);

    /// Dodaje sygnał do sygnałów obsługiwanych przez obiekt.
    /**
     * Ze względów implementacyjnych sygnały są dzielone przez różne obiekty tego typu.
     */
    void add(SignalType signal);

private:
    SignalService service;
    static int Signal;
    static bool Occured;
    static void SignalHandler(int signal);
};





/// Klasa zawierająca szczegóły implementacyjne na akceptora.
class AcceptorImplementation : public AcceptorInterface
{
public:
    AcceptorImplementation(StreamServiceInterface& service);

    /// Implementuje zgodnie z AcceptorInterface.
    void open(Endpoint::ProtocolType protocol) override;
    void setOption(const Option::Option& option) override;
    void bind(Endpoint::AddressType address) override;
    void listen(int backlog) override;

    Socket accept() override;

private:
    StreamServiceInterface& streamService;
};



/// Klasa zawierająca szczegóły implementacyjne gniazda TCP.
class SocketImplementation : public SocketInterface
{
public:
    typedef SocketService::HandleType HandleType;
    typedef SocketService::ReadHandler ReadHandler;
    typedef SocketService::WriteHandler WriteHandler;
    typedef SocketService::BufferType BufferType;
    typedef SocketService::ConstBufferType ConstBufferType;

    /// Tworzy nowy obiekt związany z StreamServiceInterface oraz przydzielonym uchwytem.
    SocketImplementation(StreamServiceInterface& service, HandleType handle);
    ~SocketImplementation();

    /// Implementuje zgodnie z SocketInterface.
    int read(BufferType& buffer) override;
    int readSome(BufferType& buffer) override;
    int write(const ConstBufferType& buffer) override;
    int writeSome(const ConstBufferType& buffer) override;

    /// Zamyka gniazdo.
    void close() override;

private:
    bool closed;
};

} // namespace Tcp

#endif // PATR_SOCKET_H
