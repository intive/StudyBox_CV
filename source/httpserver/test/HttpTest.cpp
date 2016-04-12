#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../Server.h"
#include "../Socket.h"

/// Testy sprawdzające poprawność parsera zapytań HTTP.
BOOST_AUTO_TEST_SUITE(RequestParse)

/// Sprawdza czy prawidłowe zapytanie HTTP zostanie poprawnie zidentyfikowane metodą parse().
BOOST_AUTO_TEST_CASE(ValidRequest)
{
    Http::Request request;
    Http::RequestParser parser;
    std::string terminate = Http::CRLF;
    terminate += terminate;
    std::string input = "GET / HTTP/1.0" + terminate; // całkowity odczyt.
    Http::RequestParser::Result result;

    std::tie(result, std::ignore) = parser.parse(input.begin(), input.end(), request);
    BOOST_REQUIRE(result == Http::RequestParser::Result::Good);

    request = Http::Request();
    parser.reset();
    input = "POST /te"; // częściowy odczyt.
    std::tie(result, std::ignore) = parser.parse(input.begin(), input.end(), request);
    BOOST_CHECK(result == Http::RequestParser::Result::Indeterminate);

    input = "st?q1=42&q2=other HTTP/1.0\r\nContent-Length: 10" + terminate;
    std::tie(result, std::ignore) = parser.parse(input.begin(), input.end(), request);
    BOOST_CHECK(result == Http::RequestParser::Result::Good);
}

/// Sprawdza czy nieprawidłowe zapytanie HTTP zostanie poprawnie zidentyfikowane metodą parse().
BOOST_AUTO_TEST_CASE(InvalidRequest)
{
    Http::Request request;
    Http::RequestParser parser;
    std::string terminate = Http::CRLF;
    terminate += terminate;
    std::string input = "GET / HTTP/1.0 *" + terminate; // błędne zapytanie
    Http::RequestParser::Result result;

    std::tie(result, std::ignore) = parser.parse(input.begin(), input.end(), request);
    BOOST_CHECK(result == Http::RequestParser::Result::Bad);
}

/// Sprawdza czy zawartość poprawnego zapytania zgadza się z tą prawdziwą metodą fill().
BOOST_AUTO_TEST_CASE(RequestContents)
{
    Http::Request request;
    Http::RequestParser parser;
    Http::RequestParser::Result result;
    std::string terminate = Http::CRLF;
    terminate += terminate;
    std::string content = "\r\n\r\n\r\nContent\n\t\n\n   \\\"; ";
    std::string input = "POST /test?q1=42&q2=other HTTP/1.0\r\nContent-Length: " + std::to_string(content.length()) + terminate + content;
    std::string::iterator it;
    input += content;

    std::tie(result, it) = parser.parse(input.begin(), input.end(), request);
    BOOST_REQUIRE(result == Http::RequestParser::Result::Good);

    auto query = request.uri().query();
    auto& headers = request.headers();
    BOOST_REQUIRE(request.uri().raw() == "/test?q1=42&q2=other");
    BOOST_REQUIRE(request.uri().absolutePath().raw() == "/test");
    BOOST_REQUIRE(query["q1"] == "42" && query["q2"] == "other" && query.size() == 2);

    parser.fill(it, input.end(), request);

    BOOST_CHECK(request.body() == content);
}

BOOST_AUTO_TEST_SUITE_END()

/// Testy sprawdzające poprawność klasy odpowiedzialnej za odpowiedzi HTTP od serwera.
BOOST_AUTO_TEST_SUITE(ResponseValidity)

/// Sprawdza czy zawartość odpowiedzi jest poprawna.
BOOST_AUTO_TEST_CASE(ResponseContents)
{
    std::string content = "Content\n\t\n\n   \\\"; ";
    std::string type = "text/plain";
    Http::Response response(Http::Response::Status::Ok, content, type);
    BOOST_CHECK(response.headers[0].first == "Content-Length" && response.headers[0].second == std::to_string(content.length()));
    BOOST_CHECK(response.headers[1].first == "Content-Type" && response.headers[1].second == type);
    BOOST_CHECK(response.status() == Http::Response::Status::Ok);
}

BOOST_AUTO_TEST_SUITE_END()

#include <sstream>

/// Klasa imitująca StreamService.
class ServiceMock : public Tcp::StreamServiceInterface
{
    int run() override
    {
        return 0;
    }
    std::unique_ptr<Tcp::ServiceFactory> getFactory() override
    {
        return nullptr;
    }
};

/// Klasa imitująca implementację gniazda na podstawie std::stringstream.
class SocketMock : public Tcp::SocketInterface
{
public:
    SocketMock(const std::string& message, Tcp::StreamServiceInterface& s, bool* globalClosed = nullptr, std::ostringstream* globalWriter = nullptr)
        : SocketInterface(s, 0), closed(false), globalClosed(globalClosed),
          reader(message), globalWriter(globalWriter) {}

    ~SocketMock()
    {
        close();
    }

    const bool& isClosed() const
    {
        if (globalClosed)
            return *globalClosed;
        return closed;
    }

private:
    int read(BufferType & buffer) override
    {
        if (closed)
            return 0;
        reader.read(buffer.first, buffer.second);
        return (int)reader.gcount();
    }
    int readSome(BufferType & buffer) override
    {
        if (closed)
            return 0;
        auto ret = (int)reader.readsome(buffer.first, 5);
        if (!ret)
            close();

        return ret;
    }
    void asyncReadSome(BufferType buffer, ReadHandler handler) override
    {
        auto bytes = readSome(buffer);
        handler(bytes <= 0, bytes);
    }
    int write(const ConstBufferType & buffer) override
    {
        if (closed && !writer.str().empty())
            throw Tcp::SendError("");
        if (globalWriter)
            globalWriter->write(buffer.first, buffer.second);
        writer.write(buffer.first, buffer.second);
        return (int)buffer.second;
    }
    int writeSome(const ConstBufferType & buffer) override
    {
        throw Tcp::NotImplemented("");
    }
    void asyncWriteSome(const ConstBufferType& buffer, WriteHandler handler) override
    {
        throw Tcp::NotImplemented("");
    }
    void close() override
    {
        if (globalClosed)
            *globalClosed = true;
        closed = true;
    }

    bool closed;
    bool* globalClosed;
    std::istringstream reader;
    std::ostringstream writer;
    std::ostringstream* globalWriter;
};


/// Testy sprawdzające poprawność obsługi połączeń
BOOST_AUTO_TEST_SUITE(ConnectionHandling)

/// Sprawdza czy połączenie poprawnie wywołuje funkcję obsługującą zapytanie.
BOOST_AUTO_TEST_CASE(ConnectionHandlerCall)
{
    using Http::CRLF;
    ServiceMock service;
    bool called;
    auto socket = std::unique_ptr<Tcp::SocketInterface>(new SocketMock(std::string("GET / HTTP/1.0") + CRLF + CRLF, service));
    {
        Http::ThreadedHandlerStrategy handler([&called](const Http::Request& request)
        {
            called = true;
            return Http::Response(Http::Response::Status::Ok, "", "");
        }
        );

        BOOST_REQUIRE_NO_THROW(
            handler.start(std::make_shared<Http::Connection>(Tcp::Socket(std::move(socket)), handler)));
    }

    BOOST_CHECK(called);
}

/// Sprawdza czy serwer jest w stanie obsłużyć kilka połączeń przy obciążeniu.
BOOST_AUTO_TEST_CASE(MultipleConnections)
{
    using Http::CRLF;
    ServiceMock service;
    {
        Http::ThreadedHandlerStrategy handler([](const Http::Request& request)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return Http::Response(Http::Response::Status::Ok, "", "");
        }
        );
        for (int i = 0; i < 12; ++i)
        {
            auto socket = std::unique_ptr<Tcp::SocketInterface>(new SocketMock(std::string("GET / HTTP/1.0") + CRLF + CRLF, service));

            BOOST_REQUIRE_NO_THROW(
                handler.start(std::make_shared<Http::Connection>(Tcp::Socket(std::move(socket)), handler)));
        }
    }
}

/// Sprawdza czy połączenie odpowiada na zapytanie zgodnie z oczekiwaniami.
BOOST_AUTO_TEST_CASE(ConnectionValidity)
{
    using Http::CRLF;
    ServiceMock service;
    std::ostringstream globalWriter;
    Http::Response response(Http::Response::Status::Ok, "Content\n\t\n\n   \\\"; ", "text/plain");
    auto socket = std::unique_ptr<Tcp::SocketInterface>(new SocketMock(std::string("GET / HTTP/1.0") + CRLF + CRLF, service, nullptr, &globalWriter));
    {
        Http::ThreadedHandlerStrategy handler([&response](const Http::Request& request)
        {
            return response;
        }
        );

        BOOST_REQUIRE_NO_THROW(
            handler.start(std::make_shared<Http::Connection>(Tcp::Socket(std::move(socket)), handler)));
    }

    BOOST_CHECK(globalWriter.str() == response.raw());
}

/// Sprawdza czy połączenie poprawnie reaguje na niespodziewane zamknięcie po stronie klienta.
BOOST_AUTO_TEST_CASE(ConnectionSendClosed)
{
    using Http::CRLF;
    ServiceMock service;
    bool isClosed = false;
    auto socket = std::unique_ptr<Tcp::SocketInterface>(new SocketMock(std::string("GET / HTTP/1.0") + CRLF + CRLF, service, &isClosed));
    socket->close();
    BOOST_REQUIRE(isClosed);
    {
        Http::ThreadedHandlerStrategy handler([](const Http::Request& request)
        {
            return Http::Response(Http::Response::Status::Ok, "", "");
        }
        );

        handler.start(std::make_shared<Http::Connection>(Tcp::Socket(std::move(socket)), handler));
    }
    BOOST_CHECK(isClosed);
}

/// Sprawdza czy połączenie jest poprawnie zamknięte po zakończeniu obsługi i czy nie ma wycieków.
BOOST_AUTO_TEST_CASE(ConnectionClose)
{
    using Http::CRLF;
    ServiceMock service;
    bool isClosed = false;
    auto socket = std::unique_ptr<Tcp::SocketInterface>(new SocketMock(std::string("GET / HTTP/1.0") + CRLF + CRLF, service, &isClosed));
    BOOST_CHECK(!isClosed);
    {
        Http::ThreadedHandlerStrategy handler([](const Http::Request& request)
        { return Http::Response(Http::Response::Status::Ok, "", ""); }
        );

        BOOST_REQUIRE_NO_THROW(
            handler.start(std::make_shared<Http::Connection>(Tcp::Socket(std::move(socket)), handler)));
    }

    BOOST_CHECK(isClosed);
}

BOOST_AUTO_TEST_SUITE_END()
