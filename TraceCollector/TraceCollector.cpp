// TraceCollector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <stdio.h>
#include <evntrace.h>
#include <string>
#include <vector>
#include "TraceCollector.h"

using namespace std;

#define FUNCTION_ORDINAL 703 // ordinal of GUIDFromStringA
typedef BOOL (WINAPI *GUIDFromStringAType)( char*, LPGUID );

GUIDFromStringAType GUIDFromStringA = NULL;
// {86BC8F33-8ABE-4CD0-BE25-1CE666238EFA}
static const GUID ealSessionGuid =
{ 0x86bc8f33, 0x8abe, 0x4cd0, { 0xbe, 0x25, 0x1c, 0xe6, 0x66, 0x23, 0x8e, 0xfa } };

static const wchar_t* sessionName{ L"Trace collector recording session" };
bool BindConversionFunction()
{
    auto hModule = ::LoadLibrary(L"shell32.dll");
    if (hModule == NULL)
    {
        return false;
    }

    GUIDFromStringA = (GUIDFromStringAType)GetProcAddress(hModule, MAKEINTRESOURCEA(FUNCTION_ORDINAL));
    if (GUIDFromStringA == NULL)
    {
        return false;
    }
    return true;
}

bool ReadTrackingGuids(const wchar_t* file, vector<GUID>& output)
{
    if (!BindConversionFunction())
    {
        return false;
    }
    output.clear();

    FILE* pFile = NULL;
    _wfopen_s(&pFile, file, L"rt");

    if (pFile == NULL)
    {
        return false;
    }

    char buffer[MAX_PATH];
    GUID guid;

    while (!feof(pFile))
    {
        if (fgets(buffer, MAX_PATH - 1, pFile) == NULL)
        {
            break;
        }

        if (!GUIDFromStringA(buffer, &guid))
        {
            continue; // ignore parsing mistakes
        }
        output.push_back(guid);
    }
    fclose(pFile);
    return ( output.size() > 0 );
}

int wmain(int argc, const wchar_t** argv)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int exit_code = 0;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (argc != 5)
    {
        printf("Usage: %S <command line> <execution delay in msec> <log file location> <GUID list file>\n", argv[0]);
        return 0;
    }
    vector<GUID> ealRecordingGuid;
    if (!ReadTrackingGuids(argv[4], ealRecordingGuid))
    {
        printf("FATAL: failed to parse tracing guids file %S", argv[4]);
        return -1; // indicate error
    }
    auto delay = _wtoi( argv[2] );
    if ( delay == 0 )
    {
        delay = 120000; // 2 minutes default
    }

    HANDLE hSession = 0;
    UINT guidsCount = (UINT)ealRecordingGuid.size();
    auto hRes = CreateLoggingSession( sessionName, ealSessionGuid, ealRecordingGuid.data(), guidsCount, argv [3], hSession );

    if ( hRes != S_OK )
    {
        printf( "CreateLoggingSession for %S failed with (%d).\n", argv [1], hRes );
        return 1;

    }

    // Start the child process.
    if ( !CreateProcess( NULL,   // No module name (use command line)
        (wchar_t*)argv[1],        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf( "CreateProcess for %S failed with (%d).\n", argv[1], GetLastError() );
        DestroyLoggingSession( hSession, sessionName, argv [3], ealSessionGuid, ealRecordingGuid.data(), guidsCount );
        return 1;
    }

    // Wait until child process exits.
    auto result = WaitForSingleObject( pi.hProcess, delay );

    if ( result == WAIT_TIMEOUT ) // timeout, process not terminated
    {
        if ( !::TerminateProcess( pi.hProcess, 1 ) )
        {
            exit_code = 2;
        }
    }

    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    DestroyLoggingSession( hSession, sessionName, argv [3], ealSessionGuid, ealRecordingGuid.data(), guidsCount );

    std::wstring outPath( argv [3] );
    outPath += L".pid";
    FILE* of = NULL;
    _wfopen_s( &of, outPath.c_str(), L"wt" );

    if ( of )
    {
        fprintf( of, "\n\nExecution of %S(%x) finished\n", argv [1], pi.dwProcessId );
        fclose( of );
    }
    return exit_code;
}