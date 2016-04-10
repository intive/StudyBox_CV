#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include "../RequestRouter.h"
#include "../../httpserver/Server.h"


BOOST_AUTO_TEST_SUITE(RequestRouter)

struct FuncObj
{
    std::pair<std::string, int> operator()(const std::string& s)
    {
        return std::make_pair("func_obj_response_string", 200);
    }
};


class TestRouter : public Router::RequestRouter
{
public:
    unsigned long size() { return services.size(); }
};


Http::Request getTestRequest(const std::string& uri, const std::string& content)
{
    std::string terminate = Http::CRLF;
    terminate += terminate;
    std::string input =
        "POST " + uri + " HTTP/1.0\r\nContent-Length: "
        + std::to_string(content.length())
        + terminate
        + content;

    Http::Request request;
    Http::RequestParser parser;

    std::string::iterator it;
    std::tie(std::ignore, it) = parser.parse(input.begin(), input.end(), request);
    parser.fill(it, input.end(), request);
    return request;
}


void registerServices(Router::RequestRouter& r)
{
    r.registerEndPointService("/api/test", [](const std::string& s) -> std::pair<std::string, int>
    {
        return{ R"({"request":"response"})", 200 };
    });

    r.registerEndPointService("/api/test2", [](const std::string& s)
    {
        return std::make_pair<std::string, int>("json_response2", 200);
    });

    r.registerEndPointService("/api/except", [](const std::string& s)
    {
        throw std::runtime_error("sample exception");
        return std::make_pair<std::string, int>("except", 200);
    });
}


BOOST_AUTO_TEST_CASE(RegisteringServices)
{
    TestRouter tr;
    tr.emitExceptionsToStdcerr = false;
    registerServices(tr);

    BOOST_CHECK(tr.size() == 3);

    // Podmiana handlera
    tr.registerEndPointService("/api/test2", FuncObj());

    BOOST_CHECK(tr.size() == 3);

    auto response = tr.routeRequest(getTestRequest("/api/test2", "hello"));
    BOOST_CHECK(response.raw().find("func_obj_response_string") != std::string::npos);
}


BOOST_AUTO_TEST_CASE(Routing)
{
    Router::RequestRouter rr;
    rr.emitExceptionsToStdcerr = false;
    registerServices(rr);


    auto request = getTestRequest("/api/test", "{\"test\":\"router\"}");
    auto response = rr.routeRequest(request);
    BOOST_CHECK(response.raw().find(R"({"request":"response"})") != std::string::npos);


    auto internal_error_request = getTestRequest("/api/except", "{\"test\":\"router\"}");
    auto internal_error_response = rr.routeRequest(internal_error_request);
    BOOST_CHECK(internal_error_response.raw().find(R"({"error":"internal server error"})") != std::string::npos);
    

    auto not_found_request = getTestRequest("/api/none", "{\"test\":\"router\"}");
    auto not_found_response = rr.routeRequest(not_found_request);
    BOOST_CHECK(not_found_response.raw().find(R"({"error":"no service for /api/none"})") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()