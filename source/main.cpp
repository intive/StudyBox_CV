#include "Server.h"
#include "Socket.h"

#include <chrono>
#include <iostream>

int main()
{
    Http::Server server("0.0.0.0", "80", [](const Http::Request&) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Symulacja przeci¹¿enia.
        return Http::Response(Http::Response::Status::Ok, "Ok", "text/plain");
    });

    std::cout << server.run() << std::endl;
}