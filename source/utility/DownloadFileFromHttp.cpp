#define _CRT_SECURE_NO_WARNINGS

#include "../httpserver/Socket.h"
#include "DownloadFileFromHttp.h"

#include <tuple>
#include <regex>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <array>


using namespace std;

namespace
{
    enum class ResponseStatus
    {
        Good,
        Error,
        Redirect
    };

    const string content_len_str = "Content-Length: ";
    const string header_separator = "\r\n\r\n";

    
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
    }


    pair<ResponseStatus, string> checkResponse(const string& header)
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


    string prepareRequest(const string& domain, const string& endpoint)
    {
        return "GET " + endpoint + " HTTP/1.0\r\n"
            + "Host: " + domain + "\r\n"
            + "Accept: */*\r\n\r\n";;
    }
}



namespace Utility
{
    void fetchData(vector<unsigned char>& buffer, function<int(Tcp::Buffer&)> func)
    {
        array<char, 1024> b = { 0 };
        Tcp::Buffer tb = Tcp::MakeBuffer(b);

        size_t recvd;

        bool header_complete = false;
        string header;

        while ((recvd = func(tb)) != 0)
        {
            if (!header_complete)
            {
                copy(begin(b), begin(b) + recvd, back_inserter(header));

                auto end_of_header = header.find("\r\n\r\n");
                if (end_of_header == string::npos)
                {
                    continue;
                }


                header_complete = true;

                ResponseStatus status;
                string msg;
                tie(status, msg) = checkResponse(header);
                if (status == ResponseStatus::Error)
                {
                    throw runtime_error(msg);
                }
                else if (status == ResponseStatus::Redirect)
                {
                    return dlFileToBuffer(msg, buffer);
                }

                auto header_content_len = search(begin(header), end(header), begin(content_len_str), end(content_len_str));
                if (header_content_len != end(header))
                {
                    auto r_after_content_len = find(header_content_len, end(header), '\r');
                    string n{ header_content_len + 16, r_after_content_len };
                    buffer.reserve(stoul(n));
                }

                auto body_start = search(begin(header), end(header), begin(header_separator), end(header_separator));
                buffer.insert(end(buffer), body_start + 4, begin(header) + recvd);
            }
            else
            {
                buffer.insert(end(buffer), begin(b), begin(b) + recvd);
            }
        }
    }


    void dlFileToBuffer(const string& url, vector<unsigned char>& buffer)
    {
        auto result = getDomainAndEndpoint(url);
        string domain = get<0>(result);
        string port = get<1>(result);
        string endpoint = get<2>(result);

        Tcp::StreamService service;
        auto sock = service.getFactory()->resolve(domain, port)->connect(service);

        const auto req = prepareRequest(domain, endpoint);

        auto sent = sock.write(Tcp::MakeBuffer(req));
        if (sent == 0)
        {
            throw runtime_error("Request fail: " + string{ strerror(errno) });
        }

        fetchData(buffer, [&sock](Tcp::Buffer& b) -> int
        {
            return sock.readSome(b);
        });
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
