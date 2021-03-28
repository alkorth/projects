// cycleexec.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <wchar.h>

using namespace std;

wstring powerShellPath = L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe";
wstring powerShellPolicy = L" Set - ExecutionPolicy RemoteSigned";

HRESULT Execute( const wchar_t* process, const wchar_t* arg )
{
    HRESULT result { S_OK };

    STARTUPINFO info = { sizeof( info ) };
    PROCESS_INFORMATION processInfo;

    if ( !::CreateProcessW( process, (LPWSTR)arg, NULL, NULL, FALSE, 0 , NULL, NULL, &info, &processInfo ) )
    {
        result = HRESULT_FROM_WIN32( ::GetLastError() );
    }
    else
    {
        ::WaitForSingleObject( processInfo.hProcess, INFINITE );
        ::CloseHandle( processInfo.hProcess );
        ::CloseHandle( processInfo.hThread );
    }
    return result;
}

HRESULT ExecutionCycle()
{
    auto fullPowerShellPolicyString = powerShellPath + powerShellPolicy;
    auto ret_code = Execute(powerShellPath.c_str(), fullPowerShellPolicyString.c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS configuration failed with %x error\n", ret_code );
        return ret_code;
    }

    // start the VM
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\vmstart.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM start failed with %x error\n", ret_code );
        return ret_code;
    }

    // wait for 10 minutes
    ::SleepEx( 60000, FALSE );

    // stop the VM
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\vmstop.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // Mount VM volume
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\hdmount.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // Copy results
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\copyresults.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // UnMount VM volume
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\hddismount.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // Revert VM
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\vmrevert.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    return ret_code;
}

HRESULT PrepareExecutionScript( const wchar_t* exe_name, const wchar_t* out )
{
    auto ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe Set-ExecutionPolicy RemoteSigned" );
    if ( ret_code != S_OK )
    {
        printf( "PS configuration failed with %x error\n", ret_code );
        return ret_code;
    }
    // Mount VM volume
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\hdmount.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }
    FILE* ofile;
    _wfopen_s( &ofile, out, L"wt" );
    if ( ofile != NULL )
    {
        wchar_t buffer [MAX_PATH + 1];
        wchar_t copy [MAX_PATH + 1];
        const wchar_t* ptr = wcsrchr( exe_name, '\n' );

        if ( ptr != NULL )
        {
            *(wchar_t*) ptr = 0;
        }

        ptr = wcsrchr( exe_name, '\\' );
        if ( ptr == NULL )
        {
            ptr = exe_name;
        }
        else
        {
            ptr++;
        }
        wcscpy_s( copy, ptr );
        ptr = wcsrchr( copy, '.' );

        if ( ptr != NULL )
        {
            *(wchar_t*)ptr = 0;
        }

        wsprintf( buffer, L"t:\\scripts\\tracecollector \"%s\" 120000 t:\\data\\%s.etl >> t:\\data\\%s.log\n", exe_name, copy, copy );
        fputws( buffer, ofile );
        fputws( L"timeout /t 5\n", ofile );

        fflush( ofile );
        fclose( ofile );
    }
    // UnMount VM volume
    ret_code = Execute(powerShellPath.c_str(), L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe -f d:\\dataset\\scripts\\hddismount.ps1" );
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    return ret_code;
}
int wmain(int argc, const wchar_t** argv)
{
    printf( "\nCycleExec v1.1\n" );
    // declaring argument of time()
    time_t my_time = time( NULL );

    char buffer [MAX_PATH];

    if ( argc != 3 )
    {
        printf( "Format: %S <samples path aggregated file> <configuration file>\n", argv[0] );
        return 1;
    }

    FILE* file;
    _wfopen_s( &file, argv [1], L"rt" );

    if ( !file )
    {
        printf( "Failed to open input file %S\n", argv[1] );
        return 1;
    }

    wchar_t sbuffer [MAX_PATH + 1];
    while ( fgetws( sbuffer, MAX_PATH, file ) != NULL )
    {
        ctime_s( buffer, MAX_PATH - 1, &my_time );

        printf( "Started at %s for %S\n", buffer, sbuffer );

        auto retval = PrepareExecutionScript(sbuffer,L"f:\\data\\onerun.bat");
        retval = ExecutionCycle();

        my_time = time( NULL );
        ctime_s( buffer, MAX_PATH - 1, &my_time );
        printf( "Ended at %s with %x\n", buffer, retval );
    }
    fclose( file );
    return 0;
}

