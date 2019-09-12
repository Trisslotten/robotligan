// dllmain.cpp : Defines the entry point for the DLL application.
#include <pch.h>
#include <NetAPI/common.h>
#include <NetAPI/helper/netinitialization.h>

/*
	Skapa ny tråd i dllMain som initierar variabeln.

*/
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		NetAPI::initialization::GlobalSocketInternals::GetInstance().initializeWsock();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
		WSACleanup();
        break;
    }
	
    return TRUE;
}

