#include "../httpserver/Predef.h"

#if defined(PATR_OS_WINDOWS)
#    define _CRT_SECURE_NO_WARNINGS
#    include <ws2tcpip.h>
#    define CLOSE_SOCKET closesocket

#elif defined(PATR_OS_UNIX)
#    include <unistd.h>
#    include <sys/socket.h>
#    include <netdb.h>
#    define CLOSE_SOCKET close

#else
#    error "Unrecognised OS"
#endif

#include <tuple>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>


using namespace std;

namespace
{
    tuple<string, string, string> getDomainAndEndpoint(const string& url)
    {
        size_t domain_end;
        size_t domain_start = url.find("http") == string::npos ? 0 : url.find_first_of("//") + 2;
        size_t port_s = 0, port_e = 0;

        auto endpoint_start = url.find("/", domain_start);
        if (endpoint_start == string::npos)
        {
            return getDomainAndEndpoint(url + "/");
        }

        auto colon = url.find_first_of(":", domain_start);
        if (colon != string::npos && colon < endpoint_start)
        {
            domain_end = colon;
            port_s = colon + 1;
            port_e = endpoint_start;
        }
        else
        {
            domain_end = endpoint_start;
        }

        return make_tuple<string, string, string>(
        { url.begin() + domain_start,  url.begin() + domain_end },
            port_s == port_e ? "80" : string{ url.begin() + port_s, url.begin() + port_e },
            { url.begin() + endpoint_start, url.end() }
        );
    };


    int connect(const string& domain, const string& port)
    {
        int sock = 0;
        struct addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        struct addrinfo *servinfo = nullptr;
        struct addrinfo *s = nullptr;


        auto get_addrinfo_result = getaddrinfo(domain.c_str(), port.c_str(), &hints, &servinfo);
        if (get_addrinfo_result != 0)
        {
            throw runtime_error("Host couldn't be resolved: " + string{ strerror(get_addrinfo_result) });
        }

        for (s = servinfo; s != nullptr; s = s->ai_next)
        {
            if ((sock = socket(s->ai_family, s->ai_socktype, s->ai_protocol)) == -1)
            {
                continue;
            }

            if (connect(sock, s->ai_addr, s->ai_addrlen) == -1)
            {
                CLOSE_SOCKET(sock);
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo);

        if (s == nullptr || !sock)
            throw runtime_error{ "Couldn't connect to server" };

        return sock;
    }


    enum class ResponseStatus
    {
        Good,
        Error,
        Redirect
    };


    pair<ResponseStatus, string> inspectHeader(const string& header)
    {
        string first_line{ begin(header), begin(header) + header.find_first_of("\r\n") };

        regex rgx{ R"(^HTTP\/[\d.]+\s+(\d+)\s+(.*)$)" };
        smatch result;
        if (!regex_search(first_line, result, rgx))
        {
            return{ ResponseStatus::Error, "Couldn't parse header" };
        }

        int response_code = stoi(result[1]);
        if (response_code >= 400)
        {
            return{ ResponseStatus::Error, result[2] };
        }

        if (response_code > 300)
        {
            regex loc_rgx{ R"((?:Location\:)\s+(.+[^\r\n])(?:\r\n))" };
            smatch loc_result;
            if (regex_search(header, loc_result, loc_rgx))
            {
                return{ ResponseStatus::Redirect, loc_result[1] };
            }
        }

        return{ ResponseStatus::Good, "" };
    }
}



namespace Utility
{
    void dlFileToBuffer(const string& url, vector<unsigned char>& buffer)
    {
        auto result = getDomainAndEndpoint(url);
        string domain = get<0>(result);
        string port = get<1>(result);
        string endpoint = get<2>(result);

        int sock = connect(domain, port);

        stringstream request;
        request << "GET " << endpoint << " HTTP/1.0\r\n"
            << "Host: " << domain << "\r\n"
            << "Accept: */*\r\n\r\n";

        auto req = request.str();

        auto sent = send(sock, req.c_str(), req.length(), 0);
        if (sent == 0)
        {
            CLOSE_SOCKET(sock);
            throw runtime_error("Request fail: " + string{ strerror(errno) });
        }

        char b[1024] = { 0 };
        size_t content_len = 0;
        size_t recvd;

        bool header_complete = false;
        stringstream header;

        while ((recvd = recv(sock, b, 1024, 0)) != 0)
        {
            if (!header_complete)
            {
                header << b;

                auto headstr = header.str();
                auto end_of_header = headstr.find("\r\n\r\n");

                if (end_of_header == string::npos)
                {
                    continue;
                }


                header_complete = true;

                ResponseStatus status;
                string msg;
                tie(status, msg) = inspectHeader(headstr);
                if (status == ResponseStatus::Error)
                {
                    throw runtime_error(msg);
                }
                else if (status == ResponseStatus::Redirect)
                {
                    CLOSE_SOCKET(sock);
                    return dlFileToBuffer(msg, buffer);
                }

                auto header_content_len = strstr(b, "Content-Length: ");
                if (header_content_len)
                {
                    auto r_after_content_len = strchr(header_content_len, '\r');
                    string n{ header_content_len + 16, r_after_content_len };
                    sscanf(n.c_str(), "%zu", &content_len);
                    buffer.reserve(content_len);
                }

                auto body_start = strstr(b, "\r\n\r\n");
                buffer.insert(end(buffer), body_start + 4, &b[recvd]);
            }
            else
            {
                buffer.insert(end(buffer), begin(b), begin(b) + recvd);
            }

            memset(b, 0, sizeof(b));
        };

        CLOSE_SOCKET(sock);
    }


    void dlFileToFile(const string& url, const string& path)
    {
        vector<unsigned char> buffer;
        dlFileToBuffer(url, buffer);

        ofstream file(path, ios::binary);

        if (!file.good())
            throw runtime_error("Couldn't open file to save. Path: " + path);

        file.write(reinterpret_cast<char*>(&buffer[0]), buffer.size());

        file.flush();
    }
}
