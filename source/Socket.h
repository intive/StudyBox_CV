#ifndef PATR_SOCKET_H
#define PATR_SOCKET_H

#include <memory>

#include <set>
#include <map>
#include <queue>
#include <string>
#include <unordered_set>

#include <functional>

struct addrinfo;

/// Przestrzeñ dla klas i funkcji zwi¹zanych z niskopoziomow¹ komunikacj¹ TCP/IP.
/**
 * Bêd¹ce tu klasy mo¿na podzieliæ na 4 czêœci:
 * Service - odpowiedzialne za demultipleksacjê i komunikacjê miêdzy elementami,
 * Interface - odpowiedzialne za udostêpnianie interfejsów w celu rozszerzania funkcjonalnoœci,
 * Implementation - odpowiedzialne za szczegó³y implementacyjne, spe³niaj¹ce interfejs powy¿ej,
 * Pozosta³e - s³u¿¹ce jako frontend u¿ytkownika.
 *
 * U¿ytkownik musi powo³aæ obiekt StreamService aby móc wykorzystywaæ asynchroniczne
 * aspekty api. W przeciwnym wypadku musi spe³niæ StreamServiceInterface w stopniu,
 * który by na to pozwala³.
 */
namespace Tcp {

// Oznacza, ¿e wykorzystywana funkcja nie jest zaimplementowana.
class NotImplemented : public std::logic_error
{
public:
    using std::logic_error::logic_error;
};

// Bazowa klasa do oznaczania b³êdów zwi¹zanych z komunikacj¹ TCP.
class TcpError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

#define DEFINE_ERROR(errorName, baseClass) \
class errorName : public baseClass \
{ \
public: \
using baseClass::baseClass; \
}
// B³¹d zwi¹zany ze specyficzn¹ platform¹
DEFINE_ERROR(PlatformError, TcpError);
// B³¹d zwi¹zany z gniazdami.
DEFINE_ERROR(SocketError, TcpError);
// B³¹d zwi¹zany z i/o gniazd.
DEFINE_ERROR(StreamError, SocketError);
// B³¹d wyjœcia gniazda.
DEFINE_ERROR(SendError, StreamError);
// B³¹d wejœcia gniazda.
DEFINE_ERROR(ReceiveError, StreamError);
// B³¹d w nas³uchiwaniu po³¹czeñ.
DEFINE_ERROR(AcceptorError, SocketError);
// B³¹d w zwi¹zaniu gniazda z adresem.
DEFINE_ERROR(BindError, AcceptorError);
// B³¹d w przygotowaniu gniazda do nas³uchiwania.
DEFINE_ERROR(ListenError, AcceptorError);
// B³¹d otwarcia po³¹czenia.
DEFINE_ERROR(AcceptError, AcceptorError);
// B³¹d demultipleksacji / asynchronicznego wywo³ania.
DEFINE_ERROR(ServiceError, TcpError);
// B³¹d gniazda wyjœciowego.
DEFINE_ERROR(EndpointError, TcpError);
// B³¹d ustawienia opcji gniazda.
DEFINE_ERROR(SocketOptionError, SocketError);
#undef DEFINE_ERROR

// Typ wykorzystywany do przesy³u informacji przez gniazda.
typedef std::pair<char* /* data */, int /* size */> Buffer;
typedef std::pair<const char* /* data */, int /* size */> ConstBuffer;






/// Klasa do blokowania kopiowania obiektów.
/**
 * Nie nale¿y usuwaæ obiektów przez wskaŸnik do tej klasy ze wzglêdu
 * na brak konstruktora.
 */
struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator =(const NonCopyable&) = delete;
    NonCopyable& operator =(NonCopyable&&) = delete;
};






/// S³u¿y zamianie kontenera na Tcp::Buffer.
/**
 * Zamianie mo¿e podlegaæ dowolny kontener spe³niaj¹cy wymogi
 * ContiguousContainer posiadaj¹cy value_type char lub równowa¿ny. 
 */
template<typename T>
Buffer MakeBuffer(T& container)
{
    return std::make_pair(container.data(), static_cast<int>(container.size()));
}





/// S³u¿y zamianie kontenera na Tcp::ConstBuffer.
/**
* Zamianie mo¿e podlegaæ dowolny kontener spe³niaj¹cy wymogi
* ContiguousContainer posiadaj¹cy value_type char lub równowa¿ny.
*/
template<typename T>
ConstBuffer MakeBuffer(const T& container)
{
    return std::make_pair(container.data(), static_cast<int>(container.size()));
}

/// Przestrzeñ opcji gniazd tcp.
namespace Option {
    /// G³ówna struktura, z jakiej korzystaj Tcp::Socket.
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

    // Mo¿liwa rozbudowa, je¿eli zajdzie taka potrzeba.
}




class StreamService;
class Socket;
class AcceptorImplementation;
class SocketImplementation;
class SocketInterface;
class AcceptorInterface;
class SocketInterface;
class StreamServiceInterface;




/// Klasa bazowa obiektów zawieraj¹cych uchwyty.
/**
 * Tutaj uchwytami s¹ deskryptory plików, ze wzglêdu na wybrany
 * rodzaj implementacji.
 */
class Service
{
public:
    typedef int HandleType;

    // Gwarantuje dostêp do uchwytu.
    HandleType getHandle() const;
    void setHandle(HandleType handle);

    // porównuje obiekty ze wzglêdu na rozmiar ich uchwytu.
    bool operator ==(const Service&) const;
    bool operator !=(const Service&) const;
    bool operator <(const Service&) const;
    bool operator >(const Service&) const;

protected:
    ~Service() = default;

    HandleType handle;
};




/// Interfejs s³u¿¹cy do komunikacji z StreamService.
class SignalServiceInterface
{
public:
    typedef int SignalType;

    virtual ~SignalServiceInterface() = default;

    /// W celu sprawdzenia jaki sygna³ zosta³ wywo³any.
    /**
     * @return wartoœæ sygna³u je¿eli ten zosta³ wywo³any.
     */
    virtual int get() const = 0;
    /// W celu sprawdzenia czy sygna³ zosta³ wywo³any.
    virtual bool received() const = 0;
};




/// S³u¿y do sprawdzania zdarzeñ przerwañ systemowych.
class SignalService : public SignalServiceInterface
{
public:
    SignalService(int& sigVal, bool& sigFlag);

    /// Zwraca jaki sygna³ zosta³ wywo³any.
    /**
    * W przypadku, gdy ¿aden nie zosta³ wywo³any,
    * zwracana wartoœæ jest nieokreœlona.
    */
    int get() const override;
    /// Zwraca, czy sygna³ zosta³ wywo³any.
    bool received() const override;

private:
    int& sigVal;
    bool& sigFlag;
};





/// S³u¿y do obs³ugi asynchronicznych wywo³añ ::accept().
/**
 * Dzia³a jako backend operacji asynchronicznych dla dowolnych
 * implementacji spe³niaj¹cych AcceptorInterface.
 */
class AcceptorService : public Service
{
public:
    /// Funkcja wywo³ana przy asynchronicznym zdarzezniu.
    /**
     * Socket to gniazdo, dla którego zosta³o otwarte po³¹czenie.
     */
    typedef std::function<void(Socket)> Handler;
    typedef Service::HandleType HandleType;

    /// Tworzy obiekt z implementacj¹ o interfejsie AcceptorInterface.
    AcceptorService(AcceptorInterface& implementation);

    /// Wybudza implementacjê w celu dokonania asynchronicznej akcji.
    void acceptReady();
    /// Wprowadza now¹ funkcjê do wywo³ania w kolejce.
    void enqueue(Handler handler);

private:
    AcceptorInterface& implementation;
    std::queue<Handler> handlers;
};




/// S³u¿y do obs³ugi asynchronicznych odczytów i zapisów do gniazd.
/**
 * Dzia³a jako backend operacji asynchronicznych dla dowolnych
 * implementacji spe³niaj¹cych SocketInterface.
 */
class SocketService : public Service
{
public:
    typedef Service::HandleType HandleType;
    /// Prototyp funkcji s³u¿¹cej do obs³ugi asynchronicznego odczytu z gniazda.
    typedef std::function<void(int /*error code */, std::size_t /* bytes read */)> ReadHandler;
    /// Prototyp funkcji s³u¿¹cej do obs³ugi asynchronicznego zapisu z gniazda.
    /**
     * Asynchroniczny zapis nie jest obecnie zaimplementowany.
     */
    typedef std::function<void(int, int)> WriteHandler;
    /// Bufor wywkorzystywany do komunikacji z implementacj¹.
    typedef Buffer BufferType;
    typedef ConstBuffer ConstBufferType;

    /// Tworzy obiekt z implementacj¹ o interfejsie SocketInterface.
    SocketService(StreamServiceInterface& service, SocketInterface& implementation, HandleType handle);
    /// Oznacza, ¿e gniazdo jest gotowe do nieblokuj¹cego odczytu.
    int readReady();
    /// Oznacza, ¿e gniazdo jest gotowe do nieblokuj¹cego zapisu.
    int writeReady();

    /// S³u¿y wprowadzeniu nowych funkcji obs³uguj¹cych asynchroniczny odczyt.
    void enqueue(BufferType& buffer, ReadHandler handler);
    /// S³u¿y wprowadzeniu nowych funkcji obs³uguj¹cych asynchroniczny zapis.
    void enqueue(const ConstBufferType& buffer, WriteHandler handler);

    /// Oznacza gniazdo jako gotowe do zamkniêcia.
    /**
     * Zamkniêcie mo¿e nie nast¹piæ natychmiast.
     */
    void shutdown();

private:
    bool shut;

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
};





/// Klasa zawieraj¹ca szczegó³y implementacyjne dla wybierania informacji docelowym miejscu.
class EndpointImplementation : public EndpointInterface
{
public:
    typedef addrinfo* AddressType;
    typedef addrinfo* ProtocolType;

    /// Tworzy obiekt o danym adresie i porcie.
    /** 
     * address mo¿e przyjmowaæ formê nazwy domeny (np. www.example.com),
     * mnemonikê (np. localhost), IPv4 (np. 0.0.0.0), IPv6 (np 0::0).
     * Port mo¿e przyjmowaæ wartoœci od 1 do 49150 w postaci ³añcucha znaków.
     */
    EndpointImplementation(const std::string& address, const std::string& port);
    ~EndpointImplementation();

    /// Zwraca informacje o adresie niezbêdne do wykonania operacji.
    AddressType address() const override;
    /// Zwraca informacje o protokole w celu otwarcia gniazda.
    ProtocolType protocol() const override;

private:
    AddressType addressVal;
};




/// S³u¿y jako frontend u¿ytkownika.
/**
 * Przekierowuje odpowiedzialnoœæ do implementacji.
 */
class Endpoint
{
public:
    typedef EndpointImplementation::AddressType AddressType;
    typedef EndpointImplementation::ProtocolType ProtocolType;

    /// Tworzy obiekt z adresem i portem o domyœlnej implementacji.
    Endpoint(const std::string& address, const std::string& port);
    /// Tworzy obiekt o danej implementacji, zawieraj¹cej ju¿ informacje szczegó³owe.
    Endpoint(std::unique_ptr<EndpointInterface> implementation);

    /// Przekierowuje do implementacji.
    AddressType address() const;
    /// Przekierowuje do implementacji.
    ProtocolType protocol() const;

private:
    std::unique_ptr<EndpointInterface> implementation;
};




/// Jako interfejs klas nas³uchuj¹cych po³¹czeñ.
class AcceptorInterface : public NonCopyable
{
public:
    typedef AcceptorService::HandleType HandleType;
    typedef AcceptorService::Handler Handler;

    /// Tworzy nowy obiekt, który do asynchronicznej komunikacji wykorzysta
    /// obiekt implementuj¹cy StreamServiceInterface.
    AcceptorInterface(StreamServiceInterface&);
    virtual ~AcceptorInterface() = default;

    /// Otwiera gniazdo o danym protokole.
    virtual void open(Endpoint::ProtocolType protocol) = 0;
    /// Ustawia opcjê dla gniazda.
    virtual void setOption(const Option::Option& option) = 0;
    /// Przypisuje gniazdo do adresu.
    virtual void bind(Endpoint::AddressType address) = 0;
    /// Flaguje gniazdo do nas³uchiwania na po³aczenia.
    /**
     * backlog oznacza maksymaln¹ iloœæ po³¹czeñ oczekuj¹cych w kolejce do pod³¹czenia,
     * które nie zosta³y jeszcze obs³u¿one.
     */
    virtual void listen(int backlog) = 0;

    /// Blokuj¹ca funkcja do przyjêcia po³¹czenia.
    /**
     * Funkcja bêdzize blokowaæ, dopóki nie zostanie wybudzona przez
     * nadchodz¹ce po³¹czenie.
     */
    virtual Socket accept() = 0;
    /// Asynchroniczna wersja przyjmuje funkcjê do obs³ugi zdarzenia, gdy ono nadejdzie.
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

    /// Tworzy obiekt zwi¹zany z obiektem typu *Service.
    SocketInterface(StreamServiceInterface& service, HandleType handle);
    virtual ~SocketInterface() = default;

    /// Blokuje, dopóki nie odczyta okreœlonej liczby bajtów.
    /**
     * @return iloœæ odczytanych bajtów - powinna wynosiæ rozmiar bufora.
     */
    virtual int read(BufferType& buffer) = 0;
    /// Odczytuje dostêpn¹ w tej chwili liczbê bajtów.
    /**
     * Mo¿e blokowaæ, je¿eli w danej chwili bufor gniazda jest pusty.
     * @return iloœæ odczytanych bajtów - powinna byæ równa iloœci dostêpnych bajtów na gnieŸdzie.
     */
    virtual int readSome(BufferType& buffer) = 0;
    /// Oznacza asynchroniczne wywo³anie funkcji ReadHandler.
    /**
     * Funkcja powinna wracaæ bez blokowania.
     */
    void asyncReadSome(BufferType buffer, ReadHandler handler);
    /// Odpowiednik read dla zapisu.
    /**
     * @return iloœæ faktycznie zapisanych bajtów - powinna wynosiæ rozmiar bufora.
     */
    virtual int write(const ConstBufferType& buffer) = 0;
    /// Odpowiednik readSome dla zapisu.
    /**
     * @return iloœæ faktycznie zapisanych bajtów.
     */
    virtual int writeSome(const ConstBufferType& buffer) = 0;
    /// Odpowiednik asyncReadSome dla zapisu.
    void asyncWriteSome(const ConstBufferType& buffer, WriteHandler handler);

    /// Bezpoœrednio zamyka gniazdo.
    virtual void close() = 0;
    /// Oznacza gniazdo do zamkniêcia.
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
    /// Tworzy nowy obiekt implementuj¹cy AcceptorInterface.
    virtual std::unique_ptr<AcceptorInterface> getImplementation() = 0;
    /// Tworzy nowy obiekt implementuj¹cy EndpointInterface.
    virtual std::unique_ptr<EndpointInterface> resolve(const std::string& host, const std::string& port) = 0;
};


/// Interfejs g³ównego serwisu reaktywnego.
class StreamServiceInterface
{
public:
    virtual ~StreamServiceInterface() = default;

    /// Uruchamia serwis.
    /**
     * Wywo³anie powinno blokowaæ do momentu wywo³ania obs³ugiwanego przerwania
     * systemowego lub wyzerowania licznika otwartych gniazd.
     */
    virtual int run() = 0;
    /// Dodaje istniej¹cy serwis gniazda.
    virtual void add(SocketService* service);
    /// Usuwa serwis gniazda.
    /**
     * Gniazdo powinno samo zagwarantowaæ poprawn¹ komunikacjê z serwisem.
     */
    virtual void remove(SocketService* service);
    /// Dodaje istniej¹cy serwis akceptora.
    virtual void add(AcceptorService* service);
    /// Dodaje serwis do obs³ugi sygna³ów.
    /**
     * Obs³ugiwany jest tylko jeden serwis sygna³ów ze wzglêdów implementacyjnych.
     */
    virtual void add(SignalService* service);

    /// Zwraca nowy obiekt do tworzenia obiektów klasy spe³niaj¹cych wymagania danego serwisu.
    virtual std::unique_ptr<ServiceFactory> getFactory() = 0;

protected:
    std::set<SocketService*> sockets;
    std::set<AcceptorService*> acceptors;
    SignalService* signal;
};





/// Implementacja fabryki dla klasy StreamService.
/**
 * Tworzy obiekty klasy obs³ugiwanej przez StreamService.
 */
class StreamServiceFactory : public ServiceFactory, public NonCopyable
{
public:
    StreamServiceFactory(StreamService& service);

    /// Zwraca implementacjê akceptora zgodn¹ z wymaganiami StreamService.
    virtual std::unique_ptr<AcceptorInterface> getImplementation() override;
    /// Zwraca informacje o gnieŸdzie koñcowym o implementacji zwi¹zanej z StreamService.
    virtual std::unique_ptr<EndpointInterface> resolve(const std::string & host, const std::string & port) override;

private:
    StreamService& service;
};



/// Klasa do demultipleksacji po³¹czeñ oraz asynchronicznej komunikacji z innymi serwisami.
/**
 * Demultipleksacji podlegaj¹ sygna³y gotowoœci do odczytu, zapisu oraz nowych po³aczeñ.
 */
class StreamService : public StreamServiceInterface
{
public:
    /// Tworzy nowy obiekt serwisu.
    StreamService();
    /// Zamyka pozosta³e otwarte po³¹czenia.
    ~StreamService();

    /// Implementuje interfejs StreamServiceInterface.
    int run() override;
    void add(SocketService* service) override;
    void remove(SocketService* service) override;

    /// Zwraca fabrykê do tworzenia obiektów klas obs³ugiwanych przez serwis.
    std::unique_ptr<ServiceFactory> getFactory() override;

private:
    struct StreamServicePimpl;
    std::unique_ptr<StreamServicePimpl> pimpl;
};



/// Frontend u¿ytkownika dla klasy nas³uchuj¹cej nowych po³¹czeñ.
/**
 * Oddelegowuje odpowiedzialnoœæ implementacji do klasy implementacyjnej spe³niaj¹cych
 * interfejs AcceptorInterface
 */
class Acceptor : public NonCopyable
{
public:
    typedef AcceptorInterface::Handler Handler;

    /// Tworzy nowy obiekt zwi¹zany z danym serwisem.
    Acceptor(StreamServiceInterface& service);
    /// Tworzy nowy obiekt o okreœlonej implementacji.
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


/// Frontend u¿ytkownika dla klasy gniazda.
/**
 * Udostêpnia funkcje odczytu i zapisu blokuj¹cego oraz nie dla gniazda TCP.
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
    /// Tworzy nowy obiekt z okreœlon¹ implementacj¹.
    Socket(std::unique_ptr<SocketInterface> implementation);

    /// Przekierowuje wywo³anie do implementacji.
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






/// Frontend u¿ytkownika do obs³ugi sygna³ów.
/**
 * Standardowe sygna³y, które powinny byæ 
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

    /// Tworzy nowy obiekt wspó³dzia³aj¹cy z serwisem.
    SignalSet(StreamServiceInterface&);

    /// Dodaje sygna³ do sygna³ów obs³ugiwanych przez obiekt.
    /**
     * Ze wzglêdów implementacyjnych sygna³y s¹ dzielone przez ró¿ne obiekty tego typu.
     */
    void add(SignalType signal);

private:
    SignalService service;
    static int Signal;
    static bool Occured;
    static void SignalHandler(int signal);
};





/// Klasa zawieraj¹ca szczegó³y implementacyjne na akceptora.
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



/// Klasa zawieraj¹ca szczegó³y implementacyjne gniazda TCP.
class SocketImplementation : public SocketInterface
{
public:
    typedef SocketService::HandleType HandleType;
    typedef SocketService::ReadHandler ReadHandler;
    typedef SocketService::WriteHandler WriteHandler;
    typedef SocketService::BufferType BufferType;
    typedef SocketService::ConstBufferType ConstBufferType;

    /// Tworzy nowy obiekt zwi¹zany z StreamServiceInterface oraz przydzielonym uchwytem.
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
