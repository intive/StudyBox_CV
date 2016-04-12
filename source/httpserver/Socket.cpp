#include "Socket.h"
#include <csignal>
#include <cassert>
#include <errno.h>
#include <sstream>

#include "Predef.h"

#if defined(PATR_OS_WINDOWS)
#pragma comment(lib, "ws2_32.lib")
#    include <winsock2.h>
#    include <ws2tcpip.h>
#    include <windows.h>
#elif defined(PATR_OS_UNIX)
#    include <unistd.h>
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <netdb.h>
#    include <arpa/inet.h>
#    include <netinet/in.h>
#else
#    error "Unrecognised OS"
#endif

namespace {
    int GetLastSocketError()
    {
#if defined(PATR_OS_WINDOWS)
        return WSAGetLastError();
#elif defined(PATR_OS_UNIX)
        return errno;
#endif
    }
}

struct Tcp::StreamService::StreamServicePimpl
{
    StreamServicePimpl();
    ~StreamServicePimpl();

    fd_set readFdsMaster;
    fd_set readFds;
};

Tcp::StreamService::StreamServicePimpl::StreamServicePimpl() : readFdsMaster()
{
#if defined(PATR_OS_WINDOWS)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        throw PlatformError("WSA failed to initialize");
    }
#endif
}

Tcp::StreamService::StreamServicePimpl::~StreamServicePimpl()
{
#if defined(PATR_OS_WINDOWS)
    WSACleanup();
#endif
}

Tcp::StreamService::StreamService() : pimpl(new StreamServicePimpl())
{
}

Tcp::StreamService::~StreamService()
{
}

int Tcp::StreamService::run()
{
    fd_set& readFdsMaster = pimpl->readFdsMaster;
    FD_ZERO(&readFdsMaster);

    for (auto& service : acceptors)
    {
        FD_SET(service->getHandle(), &readFdsMaster);
    }

    while (!signal || !signal->received())
    {
        auto start = std::chrono::high_resolution_clock::now();
        int timeSinceLastUpdate = 0;

        fd_set& readFds = pimpl->readFds;

        auto result = select(start, timeSinceLastUpdate);
        if (result < 0)
            break;


        if (signal->received())
            break;

        for (auto socket = sockets.begin(); socket != sockets.end();)
        {
            auto next = socket;
            ++next;
            if (FD_ISSET((*socket)->getHandle(), &readFds))
            {
                const_cast<SocketService*>(*socket)->readReady();
            }
            else
            {
                const_cast<SocketService*>(*socket)->updateRemainingTime(timeSinceLastUpdate);
            }
            socket = next;
        }

        for (auto& acceptor : acceptors)
        {
            if (FD_ISSET(acceptor->getHandle(), &readFds))
            {
                const_cast<AcceptorService*>(acceptor)->acceptReady();
            }
        }
    }

    for (auto& socket : sockets)
    {
        const_cast<SocketService*>(socket)->shutdown();
    }

    return signal->get();
}

void Tcp::StreamService::add(SocketService* service)
{
    StreamServiceInterface::add(service);
    FD_SET(service->getHandle(), &pimpl->readFdsMaster);
}

void Tcp::StreamService::remove(SocketService * service)
{
    StreamServiceInterface::remove(service);
    FD_CLR(service->getHandle(), &pimpl->readFdsMaster);
}

std::unique_ptr<Tcp::ServiceFactory> Tcp::StreamService::getFactory()
{
    return std::unique_ptr<ServiceFactory>(new StreamServiceFactory(*this));
}

int Tcp::StreamService::select(const std::chrono::high_resolution_clock::time_point& start, int& timeSinceLastUpdate)
{
    for (;;)
    {
        auto& readFds = pimpl->readFds;
        readFds = pimpl->readFdsMaster;
        int result = (*acceptors.rbegin())->getHandle();

        if (!sockets.empty())
            result = (std::max)((*sockets.rbegin())->getHandle(), result);

        auto comp = [](const SocketService* left, const SocketService* right) { return left->remainingTime() < right->remainingTime(); };

        std::set<SocketService*, decltype(comp)> timevals(comp);
        timevals.insert(sockets.begin(), sockets.end());

        while (!timevals.empty() && (*timevals.begin())->remainingTime() <= 0)
            timevals.erase(timevals.begin());

        auto remainingTime = timevals.empty() ? 0 : (*timevals.begin())->remainingTime() - timeSinceLastUpdate;
        auto timeout = timeval();
        timeout.tv_sec = remainingTime / 1000;
        timeout.tv_usec = (remainingTime * 1000) % 1000000;

        int retval = 0;

        if (remainingTime > 0)
            retval = ::select(result + 1, &readFds, nullptr, nullptr, &timeout);
        else
            retval = ::select(result + 1, &readFds, nullptr, nullptr, nullptr); // w przypadku braku oczekujących gniazd, blokuj bez przerwy.

        if (retval < 0)
        {
            if (signal->received())
                return retval;
            throw ServiceError("select failed");
        }

        auto stop = std::chrono::high_resolution_clock::now();
        timeSinceLastUpdate += (int)std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

        if (retval != 0)
            return retval; // nie został przekroczony czas oczekiwania.

        while (!timevals.empty() && (*timevals.begin())->remainingTime() <= timeSinceLastUpdate)
        {
            (*timevals.begin())->updateRemainingTime(timeSinceLastUpdate); // gniazdo przekroczyło swój czas oczekiwania.
            timevals.erase(timevals.begin());
        }
    }
}

Tcp::Service::HandleType Tcp::Service::getHandle() const
{
    return handle;
}

void Tcp::Service::setHandle(HandleType handle)
{
    this->handle = handle;
}

bool Tcp::Service::operator==(const Service& other) const
{
    return handle == other.handle;
}

bool Tcp::Service::operator!=(const Service& other) const
{
    return handle != other.handle;
}

bool Tcp::Service::operator<(const Service& other) const
{
    return handle < other.handle;
}

bool Tcp::Service::operator>(const Service& other) const
{
    return handle > other.handle;
}

Tcp::SocketImplementation::SocketImplementation(StreamServiceInterface& service, HandleType handle) : SocketInterface(service, handle), closed(false)
{
}

Tcp::SocketImplementation::~SocketImplementation()
{
    if (!closed)
        close();
}

int Tcp::SocketImplementation::read(BufferType & buffer)
{
    int total = 0;
    int bytesleft = buffer.second;
    int n;

    while (total < buffer.second) {
        n = recv(handle(), buffer.first + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    return total;

}

int Tcp::SocketImplementation::readSome(BufferType & buffer)
{
    auto result = ::recv(handle(), buffer.first, buffer.second, 0);
    if (result == -1)
    {
#if defined(PATR_OS_UNIX)
        auto val = errno;
        if (val != EWOULDBLOCK && val != ECONNREFUSED && val != EWOULDBLOCK && val != EAGAIN)
            throw ReceiveError("recv failed");
#elif defined(PATR_OS_WINDOWS)
        auto val = WSAGetLastError();
        if (val != WSAETIMEDOUT && val != WSAEINPROGRESS && val != WSAEWOULDBLOCK)
            throw ReceiveError("recv failed");
#endif
    }
    return result;
}

int Tcp::SocketImplementation::write(const ConstBufferType & buffer)
{
    int total = 0;
    int bytesleft = buffer.second;
    int n;

    while (total < buffer.second) {
        n = ::send(handle(), buffer.first + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    return total;
}

int Tcp::SocketImplementation::writeSome(const ConstBufferType & buffer)
{
    auto result = ::send(handle(), buffer.first, buffer.second, 0);
    if (result == -1)
    {
        throw SendError("send failed");
    }
    return result;
}

void Tcp::SocketImplementation::close()
{
#if defined(PATR_OS_WINDOWS)
    ::closesocket(handle());
#elif defined(PATR_OS_UNIX)
    ::close(handle());
#endif
}

Tcp::SocketService::SocketService(StreamServiceInterface& service, SocketInterface& implementation, HandleType handle) : shut(0), implementation(implementation), service(service)
{
    this->handle = handle;
    currentTime = 0;
    timeout = 30000; // 30 sekund
}

int Tcp::SocketService::readReady()
{
    currentTime = 0;
    if (readHandlers.empty()) return 0;
    int s = 0;
    if (!shut)
        s = implementation.readSome(readHandlers.front().first);
    if (s <= 0)
        shut += 1 - s;
    auto copy = readHandlers.front().second;
    readHandlers.pop();
    copy(shut, s);

    return s;
}

int Tcp::SocketService::writeReady()
{
    if (writeHandlers.empty()) return 0;

    int s = implementation.writeSome(writeHandlers.front().first);
    writeHandlers.front().second(shut, s);

    if (shut)
    {
        writeHandlers.front().second(shut, -1);
        return 0;
    }
    writeHandlers.pop();

    return s;
}

void Tcp::SocketService::enqueue(BufferType& buffer, ReadHandler handler)
{
    readHandlers.push(std::make_pair(buffer, std::move(handler)));
}

void Tcp::SocketService::enqueue(const ConstBufferType& buffer, WriteHandler handler)
{
    writeHandlers.push(std::make_pair(buffer, std::move(handler)));
}

int Tcp::SocketService::remainingTime() const
{
    return timeout - currentTime;
}

void Tcp::SocketService::updateRemainingTime(int milliseconds)
{
    currentTime += milliseconds;
    if (currentTime >= timeout)
    {
        shutdown();
        readReady();
    }
}

void Tcp::SocketService::shutdown()
{
    shut = 1;
    service.remove(this);
}

Tcp::Socket::Socket(std::unique_ptr<SocketInterface> implementation) : implementation(std::move(implementation))
{
    assert(this->implementation);
}

int Tcp::Socket::read(BufferType & buffer)
{
    return implementation->read(buffer);
}

int Tcp::Socket::readSome(BufferType & buffer)
{
    return implementation->readSome(buffer);
}

void Tcp::Socket::asyncReadSome(BufferType buffer, ReadHandler handler)
{
    implementation->asyncReadSome(buffer, std::move(handler));
}

int Tcp::Socket::write(const ConstBufferType & buffer)
{
    return implementation->write(buffer);
}

int Tcp::Socket::writeSome(const ConstBufferType & buffer)
{
    return implementation->writeSome(buffer);
}

void Tcp::Socket::asyncWriteSome(const ConstBufferType & buffer, WriteHandler handler)
{
    implementation->asyncWriteSome(buffer, std::move(handler));
}

void Tcp::Socket::close()
{
    shutdown();
    implementation->close();
}

void Tcp::Socket::shutdown()
{
    implementation->shutdown();
}

Tcp::AcceptorImplementation::AcceptorImplementation(StreamServiceInterface& service) : AcceptorInterface(service), streamService(service)
{
}

void Tcp::AcceptorImplementation::open(Endpoint::ProtocolType protocol)
{
    handle(static_cast<HandleType>(::socket(protocol->ai_family, protocol->ai_socktype, protocol->ai_protocol)));
}

void Tcp::AcceptorImplementation::setOption(const Option::Option& option)
{
    if (::setsockopt(handle(), SOL_SOCKET, option.option, reinterpret_cast<const char*>(&option.value), sizeof option.value) == -1)
    {
        throw SocketOptionError("setsockopt failed");
    }
}

void Tcp::AcceptorImplementation::bind(Endpoint::AddressType address)
{
    if (::bind(handle(), address->ai_addr, static_cast<int>(address->ai_addrlen)) == -1)
    {

        throw BindError("bind failed (" + std::to_string(GetLastSocketError()) + ')');
    }
}

void Tcp::AcceptorImplementation::listen(int backlog)
{
    if (::listen(handle(), backlog) == -1)
    {
        throw ListenError("listen failed (" + std::to_string(GetLastSocketError()) + ')');
    }
}

Tcp::Socket Tcp::AcceptorImplementation::accept()
{
    auto result = ::accept(handle(), nullptr, nullptr);
    if (result == -1)
    {
        throw AcceptError("accept failed (" + std::to_string(GetLastSocketError()) + ')');
    }
    return Socket(std::unique_ptr<SocketImplementation>(new SocketImplementation(streamService, static_cast<HandleType>(result))));
}

Tcp::AcceptorService::AcceptorService(AcceptorInterface& implementation) : implementation(implementation)
{
}

void Tcp::AcceptorService::acceptReady()
{
    auto socket = implementation.accept();
    if (!handlers.empty())
    {
        handlers.front()(std::move(socket));
        handlers.pop();
    }
}

void Tcp::AcceptorService::enqueue(Handler handler)
{
    handlers.push(std::move(handler));
}

Tcp::Acceptor::Acceptor(StreamServiceInterface& service) : implementation(service.getFactory()->getImplementation())
{
}

Tcp::Acceptor::Acceptor(std::unique_ptr<AcceptorInterface> implementation) : implementation(std::move(implementation))
{
    assert(this->implementation);
}

void Tcp::Acceptor::open(Endpoint::ProtocolType protocol)
{
    implementation->open(protocol);
}

void Tcp::Acceptor::setOption(const Option::Option& option)
{
    implementation->setOption(option);
}

void Tcp::Acceptor::bind(Endpoint::AddressType address)
{
    implementation->bind(address);
}

void Tcp::Acceptor::listen(int backlog)
{
    implementation->listen(backlog);
}

Tcp::Socket Tcp::Acceptor::accept()
{
    return implementation->accept();
}

void Tcp::Acceptor::asyncAccept(Handler handler)
{
    implementation->asyncAccept(std::move(handler));
}

Tcp::EndpointImplementation::EndpointImplementation(const std::string & address, const std::string & port)
{
    auto hints = addrinfo();
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    auto result = getaddrinfo(address.c_str(), port.c_str(), &hints, &addressVal);
    if (result != 0)
    {
        std::string errinfo = "getaddrinfo failed: ";
        //errinfo += gai_strerror(result);
        throw EndpointError(errinfo);
    }
}

Tcp::EndpointImplementation::~EndpointImplementation()
{
    freeaddrinfo(addressVal);
}

Tcp::EndpointImplementation::AddressType Tcp::EndpointImplementation::address() const
{
    return addressVal;
}

Tcp::EndpointImplementation::ProtocolType Tcp::EndpointImplementation::protocol() const
{
    return addressVal;
}

Tcp::Endpoint::Endpoint(const std::string & address, const std::string & port) : implementation(new EndpointImplementation(address, port))
{
}

Tcp::Endpoint::Endpoint(std::unique_ptr<EndpointInterface> implementation) : implementation(std::move(implementation))
{
}

Tcp::Endpoint::AddressType Tcp::Endpoint::address() const
{
    return implementation->address();
}

Tcp::Endpoint::ProtocolType Tcp::Endpoint::protocol() const
{
    return implementation->protocol();
}


int Tcp::SignalSet::Signal;
bool Tcp::SignalSet::Occured;

Tcp::SignalSet::SignalSet(StreamServiceInterface& service) : service(Signal, Occured)
{
    service.add(&this->service);
}

void Tcp::SignalSet::add(SignalType signal)
{
    std::signal(signal, SignalHandler);
}

void Tcp::SignalSet::SignalHandler(int signal)
{
    SignalSet::Signal = signal;
    SignalSet::Occured = true;
}

Tcp::SignalService::SignalService(int & sigVal, bool& sigFlag) : sigVal(sigVal), sigFlag(sigFlag)
{
}

bool Tcp::SignalService::received() const
{
    return sigFlag;
}

int Tcp::SignalService::get() const
{
    return sigVal;
}

Tcp::Option::ReuseAddress::ReuseAddress(bool value) : Option{ SO_REUSEADDR, value }
{
}

Tcp::SocketInterface::HandleType Tcp::SocketInterface::handle() const
{
    return service.getHandle();
}

Tcp::SocketInterface::SocketInterface(StreamServiceInterface& service, HandleType handle) : service(service, *this, handle)
{
    service.add(&this->service);
}

void Tcp::SocketInterface::asyncReadSome(BufferType buffer, ReadHandler handler)
{
    service.enqueue(buffer, std::move(handler));
}

void Tcp::SocketInterface::asyncWriteSome(const ConstBufferType & buffer, WriteHandler handler)
{
    service.enqueue(buffer, std::move(handler));
}

void Tcp::SocketInterface::shutdown()
{
    service.shutdown();
}

Tcp::AcceptorInterface::AcceptorInterface(StreamServiceInterface& service) : service(*this)
{
    service.add(&this->service);
}

void Tcp::AcceptorInterface::asyncAccept(Handler handler)
{
    service.enqueue(std::move(handler));
}

Tcp::AcceptorInterface::HandleType Tcp::AcceptorInterface::handle() const
{
    return service.getHandle();
}

void Tcp::AcceptorInterface::handle(HandleType handle)
{
    service.setHandle(handle);
}

void Tcp::StreamServiceInterface::add(SocketService* service)
{
    sockets.insert(service);
}

void Tcp::StreamServiceInterface::remove(SocketService * service)
{
    sockets.erase(service);
}

void Tcp::StreamServiceInterface::add(AcceptorService* service)
{
    acceptors.insert(service);
}

void Tcp::StreamServiceInterface::add(SignalService* service)
{
    signal = service;
}

Tcp::StreamServiceFactory::StreamServiceFactory(StreamService & service) : service(service)
{
}

std::unique_ptr<Tcp::AcceptorInterface> Tcp::StreamServiceFactory::getImplementation()
{
    return std::unique_ptr<AcceptorInterface>(new AcceptorImplementation(service));;
}

std::unique_ptr<Tcp::EndpointInterface> Tcp::StreamServiceFactory::resolve(const std::string & host, const std::string & port)
{
    return std::unique_ptr<EndpointInterface>(new EndpointImplementation(host, port));
}
