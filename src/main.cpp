#include <chrono>
#include <iostream>
#include <Windows.h>
#include <thread>
#include "easywsclient.hpp"
#include "json.hpp"

using easywsclient::WebSocket;

void sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

int main() {
    #ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
    #endif

    const std::string uri = "ws://localhost:6123";

    while (true) {
        WebSocket::pointer ws = WebSocket::from_url(uri);
        if (!ws) {
            std::cerr << "Couldn't connect to GlazeWM, retrying in 5 seconds" << std::endl;
            sleep(5);
            continue;
        }
        
        std::cout << "Connected successfully." << std::endl;
        ws->send("sub -e focus_changed -e focused_container_moved -e window_managed");

        while (ws->getReadyState() != WebSocket::CLOSED) {
            ws->poll();
            ws->dispatch([ws](std::string const& message) {
                auto json = nlohmann::json::parse(message);
                auto event = json["data"];
    
                if (event.empty() || event["focusedContainer"].empty()) {
                    return;
                }
    
                auto container = event["focusedContainer"];
                int width = container["width"];
                int height = container["height"];
    
                if (width == 0 || height == 0) return;
    
                std::string command = "command set-tiling-direction ";
                command.append(width > height ? "horizontal" : "vertical");
    
                ws->send(command);
            });
    
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        delete ws;
        ws = nullptr;
        std::cout << "Lost connection, will retry in 3 seconds" << std::endl;
        sleep(3);
    }

    WSACleanup();
    return 0;
}