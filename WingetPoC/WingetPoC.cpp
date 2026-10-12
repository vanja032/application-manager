// WingetPoC.cpp

#include <iostream>
#include <cwctype>
#include <winrt/base.h>

#include "WingetService.h"

int main()
{
    try
    {
        winrt::init_apartment();

        WingetService service;
        const auto apps = service.SearchApps(L"whatsapp");
        if (apps.empty())
        {
            std::wcout << L"No applications found.\n";
            return 0;
        }

        const auto &app = apps.front();

        std::wcout << L"App: " << app.name << L'\n';
        std::wcout << L"Package ID: " << app.id << L'\n';

        std::wcout << L"Install this application? (y/n): ";

        std::wstring answer;
        std::wcin >> answer;

        if (answer.empty() || std::towlower(answer[0]) != L'y')
        {
            return 0;
        }

        std::wcout << L"Installing...\n";

        service.InstallApp(app.id);

        std::wcout << L"Installation completed successfully.\n";

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
