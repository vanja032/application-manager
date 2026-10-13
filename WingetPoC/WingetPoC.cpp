// WingetPoC.cpp

#include <cwctype>
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

        service.InstallApp(app.id,
                           [](const InstallProgressInfo &progress)
                           {
                               std::wstring state;

                               switch (progress.state)
                               {
                               case InstallState::Queued:
                                   state = L"Queued";
                                   break;

                               case InstallState::Downloading:
                                   state = L"Downloading";
                                   break;

                               case InstallState::Installing:
                                   state = L"Installing";
                                   break;

                               case InstallState::Completed:
                                   state = L"Completed";
                                   break;
                               }

                               std::wcout << state << L": " << progress.percent << L"%";

                               // Bytes prikazujemo samo tokom preuzimanja.
                               if (progress.state == InstallState::Downloading)
                               {
                                   constexpr double MB = 1024.0 * 1024.0;

                                   std::wcout << L" (" << progress.downloadedBytes / MB << L" / "
                                              << progress.totalBytes / MB << L" MB)";
                               }

                               std::wcout << L'\n';
                           });

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
