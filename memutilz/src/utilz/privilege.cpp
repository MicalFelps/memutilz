#include "utilz/privilege.h"
#include <QtWidgets/QMessageBox>

namespace utilz {
	namespace privilege {
		bool isRunningAsAdmin() {
			BOOL bIsAdmin = FALSE;
			HANDLE hToken = NULL;

			if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
				TOKEN_ELEVATION tElevation;
				DWORD dwSize;

				if (GetTokenInformation(hToken, TokenElevation, &tElevation, sizeof(TOKEN_ELEVATION), &dwSize))
					bIsAdmin = tElevation.TokenIsElevated;
				
				CloseHandle(hToken);
			}
			return bIsAdmin == TRUE;
		}
		bool tryElevate() {
			wchar_t szPath[MAX_PATH];
			if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
				SHELLEXECUTEINFOW sei = { sizeof(sei) };
				sei.lpVerb = L"runas";
				sei.lpFile = szPath;
				sei.hwnd = NULL;
				sei.nShow = SW_NORMAL;

				if (ShellExecuteExW(&sei))
					return true;
			}
			return false;
		}
		bool CheckAndRequestAdmin() {
			if (isRunningAsAdmin())
				return true;

            auto reply = QMessageBox::question(
               nullptr,
               "Administrator Required",
               "This application requires administrator privileges to access memory.\n\n"
               "Would you like to restart as administrator?",
               QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No
            );

			if (reply == QMessageBox::Yes) {
				if (!tryElevate())
					QMessageBox::critical(nullptr, "Error", "Failed to elevate privileges.");
				return false;
			}
			return false; // User declined or failure
		}
	}
}