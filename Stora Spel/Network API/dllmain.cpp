// dllmain.cpp : Defines the entry point for the DLL application.
#include <pch.h>
#include <NetAPI/common.hpp>
#include <NetAPI/helper/netinitialization.hpp>

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
		NetAPI::Initialization::GlobalSocketInternals::GetInstance().InitializeWsock();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
	
    return TRUE;
}

