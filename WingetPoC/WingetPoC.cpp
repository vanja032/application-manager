// WingetPoC.cpp

#include <iostream>
#include <winrt/base.h>

#include "WingetService.h"

int main()
{
    try
    {
        winrt::init_apartment();

        WingetService service;
        const auto apps = service.SearchApps(L"whatsapp");

        std::cout << "Search results:\n";

        for (auto const &app : apps)
        {
            std::wcout << app.name << L" | ID: " << app.id << L'\n';
        }

        return 0;
    }
    catch (const winrt::hresult_error &ex)
    {
        std::wcerr << L"WinRT error: " << ex.message().c_str() << L'\n';
        return 1;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
