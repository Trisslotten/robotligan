#ifndef WINDOWS_API_HELPERS_HPP_
#define WINDOWS_API_HELPERS_HPP_
#include <Tlhelp32.h>
#include <Winsock2.h>
#include <iphlpapi.h>
#include <process.h>
#include <string.h>
#include <winbase.h>
#include <windows.h>

#include <string>
namespace helper {
	namespace misc {
		/*
			Gets the error description from error ID
			getlasterror, WSAlasterror etc
		*/
		static std::string GetErrorString(unsigned long error) {
			// Get the error message, if any.
			DWORD errorMessageID = error;
			LPSTR messageBuffer = nullptr;
			size_t size = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&messageBuffer, 0, NULL);

			std::string message(messageBuffer, size);

			// Free the buffer.
			LocalFree(messageBuffer);

			return message;
		}
	}  // namespace misc
	namespace ps {
		/*
			Kills ALL process by the name. Ex server.exe
		*/
		static void KillProcess(const char* filename) {
			HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
			PROCESSENTRY32 pEntry;
			pEntry.dwSize = sizeof(pEntry);
			BOOL hRes = Process32First(hSnapShot, &pEntry);
			while (hRes) {
				if (strcmp(pEntry.szExeFile, filename) == 0) {
					HANDLE hProcess =
						OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
					if (hProcess != NULL) {
						TerminateProcess(hProcess, 9);
						CloseHandle(hProcess);
					}
				}
				hRes = Process32Next(hSnapShot, &pEntry);
			}
			CloseHandle(hSnapShot);
		}
	}  // namespace ps
	namespace ws {
		/*
			Gets the index of the best index to access the outside web
			Aka the one that is active and can access the addr 1.1.1.1
		*/
		static unsigned long GetBestNIC() {
			sockaddr sa;
			sa.sa_family = AF_INET;
			inet_pton(AF_INET, "1.1.1.1", &(sa.sa_data));
			DWORD interfaceIndex = 1;
			int error = GetBestInterfaceEx(&sa, &interfaceIndex);
			if (error != NO_ERROR) {
				std::cout << "Error while getting GetBestNIC: "
					<< misc::GetErrorString(error) << std::endl;
			}
			else {
				std::cout << "GetBestNIC: interface ID " << interfaceIndex << std::endl;
			}
			return interfaceIndex;
		}
		/*
			Gets the IP of an interface by the index (For example given in GetBestNIC)
		*/
		static std::string GetIPByIndex(DWORD IfIndex) {
			PIP_ADAPTER_INFO pAdapterInfo;
			PIP_ADAPTER_INFO pAdapter = NULL;
			DWORD dwRetVal = 0;
			UINT i;
			std::string retvalue;
			/* variables used to print DHCP time info */

			ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
			pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
			if (pAdapterInfo == NULL) {
				printf("Error allocating memory needed to call GetAdaptersinfo\n");
				return "Error";
			}
			// Make an initial call to GetAdaptersInfo to get
			// the necessary size into the ulOutBufLen variable
			if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
				free(pAdapterInfo);
				pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
				if (pAdapterInfo == NULL) {
					printf("Error allocating memory needed to call GetAdaptersinfo\n");
					return "Error";
				}
			}

			if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
				pAdapter = pAdapterInfo;
				while (pAdapter) {
					if (pAdapter->Index == IfIndex)
					{
						retvalue = std::string(pAdapter->IpAddressList.IpAddress.String);
					}
					pAdapter = pAdapter->Next;
				}
			}
			else {
				printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);

			}
			if (pAdapterInfo)
				free(pAdapterInfo);
			return retvalue;
		}
	}
}
#endif
