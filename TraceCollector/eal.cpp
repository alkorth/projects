// eal.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <stdio.h>
#include <evntrace.h>
#include <string>
#include "eal.h"

// {86BC8F33-8ABE-4CD0-BE25-1CE666238EFA}
static const GUID ealSessionGuid =
{ 0x86bc8f33, 0x8abe, 0x4cd0, { 0xbe, 0x25, 0x1c, 0xe6, 0x66, 0x23, 0x8e, 0xfa } };

// Microsoft-Windows-Win32k {8C416C79-D49B-4F01-A467-E56D3AA8234C}
// Microsoft-Windows-Kernel-Memory {D1D93EF7-E1F2-4F45-9943-03D245FE6C00}
// Microsoft-Windows-WMI-Activity {1418EF04-B0B4-4623-BF7E-D74AB47BBDAA}
// Microsoft - Windows - Kernel - File { EDD08927 - 9CC4 - 4E65 - B970 - C2560FB5C289 }
// Microsoft-JScript  {57277741-3638-4A4B-BDBA-0AC6E45DA56C}
// Microsoft-Windows-PowerShell {A0C1853B-5C40-4B15-8766-3CF1C58F985A}
// Microsoft-Windows-Kernel-Audit-API-Calls {E02A841C-75A3-4FA7-AFC8-AE09CF9B7F23}
// Microsoft-Windows-TCP-IP {2F07E2EE-15DB-40F1-90EF-9D7BA282188A}


// Thread Pool  {C861D0E2-A2C1-4D36-9F9C-970BAB943A12}
// Windows Kernel Trace {9E814AAD-3204-11D2-9A82-006008A86939} - won't work this way
// Microsoft-Windows-WMI-Activity {1418EF04-B0B4-4623-BF7E-D74AB47BBDAA}

static const GUID ealRecordingGuid[] = {

    { 0x8C416C79, 0xD49B, 0x4F01, { 0xa4, 0x67, 0xe5, 0x6d, 0x3a, 0xa8, 0x23, 0x4c } },
    { 0xD1D93EF7, 0xE1F2, 0x4F45, { 0x99, 0x43, 0x03, 0xd2, 0x45, 0xfe, 0x6c, 0x00 } },
    { 0x1418EF04, 0xB0B4, 0x4623, { 0xbf, 0x7e, 0xd7, 0x4a, 0xb4, 0x7b, 0xbd, 0xaa } },
    //{ 0xEDD08927, 0x9CC4, 0x4E65, { 0xb9, 0x70, 0xc2, 0x56, 0x0f, 0xb5, 0xc2, 0x89 } },
    { 0x57277741, 0x3638, 0x4A4B, { 0xbd, 0xba, 0x0a, 0xc6, 0xe4, 0x5d, 0xa5, 0x6c } },
    { 0xA0C1853B, 0x5C40, 0x4B15, { 0x87, 0x66, 0x3c, 0xf1, 0xc5, 0x8f, 0x98, 0x5a } },
    { 0xE02A841C, 0x75A3, 0x4FA7, { 0xaf, 0xc8, 0xae, 0x09, 0xcf, 0x9b, 0x7f, 0x23 } },
    { 0x2F07E2EE, 0x15DB, 0x40F1, { 0x90, 0xef, 0x9d, 0x7b, 0xa2, 0x82, 0x18, 0x8a } },

    //SystemTraceControlGuid
    //{ 0xC861D0E2, 0xA2C1, 0x4D36, { 0x9f, 0x9c, 0x97, 0x0b, 0xab, 0x94, 0x3a, 0x12 } },
    //{ 0x22fb2cd6, 0x0e7b, 0x422b, { 0xa0, 0xc7, 0x2f, 0xad, 0x1f, 0xd0, 0xe7, 0x16 } },
    //{ 0x70EB4F03, 0xC1DE, 0x4F73, { 0xa0, 0x51, 0x33, 0xd1, 0x3d, 0x54, 0x13, 0xbd } },
    //{ 0x7DD42A49, 0x5329, 0x4832, { 0x8d, 0xfd, 0x43, 0xd9, 0x79, 0x15, 0x3a, 0x88 } },
    //{ 0x9E814AAD, 0x3204, 0x11D2, { 0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39 } },
};

static const wchar_t* sessionName { L"Recording session" };

int wmain( int argc, const wchar_t** argv )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int exit_code = 0;

    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );
    ZeroMemory( &pi, sizeof( pi ) );

    if ( argc != 4 )
    {
        printf( "Usage: %S <command line> <execution delay in msec> <log file location>\n", argv[0] );
        return 0;
    }

    auto delay = _wtoi( argv[2] );
    if ( delay == 0 )
    {
        delay = 120000; // 2 minutes default
    }

    HANDLE hSession = 0;
    UINT guidsCount = sizeof( ealRecordingGuid ) / sizeof( ealRecordingGuid [0] );
    auto hRes = CreateLoggingSession( sessionName, ealSessionGuid, ealRecordingGuid, guidsCount, argv [3], hSession );

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
        DestroyLoggingSession( hSession, sessionName, argv [3], ealSessionGuid, ealRecordingGuid, guidsCount );
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
    DestroyLoggingSession( hSession, sessionName, argv [3], ealSessionGuid, ealRecordingGuid, guidsCount );

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