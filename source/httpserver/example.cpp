#include "Server.h"
#include "Socket.h"

#include <chrono>

void example()
{
    Http::Server server("0.0.0.0", "80", [](const Http::Request&) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Symulacja przeci��enia.
        return Http::Response(Http::Response::Status::Ok, "Ok", "text/plain");
    });
}