#include <chrono>
#include <print>
#include <iostream>
#include <thread>
#include <Windows.h>
#include <wsx/Client.hpp>
#include "json.hpp"

void sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    HANDLE hMutex = CreateMutex(NULL, TRUE, "gwm-autotiler");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        HWND hwnd = GetConsoleWindow();
        MessageBoxA(hwnd, "The program is already open.", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    if (std::string(lpCmdLine) == "console") {
        if (AllocConsole()) {
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
            freopen_s(&fp, "CONIN$", "r", stdin);
        }
    }

    std::string const url = "ws://127.0.0.1:6123";
    std::unique_ptr<wsx::Client> client;

    while (true) {
        auto result = wsx::connect(url);
        if (!result) {
            std::println("Connection failed, will retry in 5 seconds: {}", result.unwrapErr());
            sleep(5);
            continue;
        }

        client = std::make_unique<wsx::Client>(std::move(result).unwrap());
        auto sendResult = client->send("sub -e focus_changed -e focused_container_moved -e window_managed");
        if (!sendResult) {
            std::println("Subscription failed: {}", sendResult.unwrapErr());
            break;
        }

        while (client->isConnected()) {
            auto recvResult = client->recv();
            if (!recvResult) {
                std::println("Receive failed: {}", recvResult.unwrapErr());
                break;
            }

            wsx::Message msg = std::move(recvResult).unwrap();
            auto json = nlohmann::json::parse(msg.text());
            auto event = json["data"];

            if (event.empty() || event["focusedContainer"].empty()) {
                continue;
            }

            auto container = event["focusedContainer"];
            int width = container["width"];
            int height = container["height"];

            if (width == 0 || height == 0) continue;

            std::string command = "command set-tiling-direction ";
            std::string direction = width > height ? "horizontal" : "vertical";
            command.append(direction);

            auto sendResult = client->send(command);
            if (!sendResult) {
                std::println("Send failed: {}", sendResult.unwrapErr());
                break;
            }

            std::println("Changing direction to {}", direction);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        std::cout << "Lost connection, will retry in 3 seconds" << std::endl;
        sleep(3);
    }

    // if it fails it's whatever i think
    (void)client->close();
    client.reset();
    FreeConsole();
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    return 0;
}