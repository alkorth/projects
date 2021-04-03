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
#include <unordered_map>

using namespace std;

#define MAX_PARAMETERS_SIZE 512 // maximum size of parameter

shared_ptr<wstring> powerShellPath = make_shared<wstring>(L"c:\\windows\\system32\\WindowsPowerShell\\v1.0\\powershell.exe");
shared_ptr<wstring> powerShellPolicy = make_shared<wstring>(L" Set - ExecutionPolicy RemoteSigned");

//
// Power shell scripts
//
shared_ptr<wstring> powerShellStartScript = make_shared<wstring>(L"d:\\dataset\\scripts\\vmstart.ps1");
shared_ptr<wstring> powerShellStopScript = make_shared<wstring>(L"d:\\dataset\\scripts\\vmstop.ps1");
shared_ptr<wstring> powerShellMountScript = make_shared<wstring>(L"d:\\dataset\\scripts\\hdmount.ps1");
shared_ptr<wstring> powerShellRevertScript = make_shared<wstring>(L"d:\\dataset\\scripts\\vmrevert.ps1");
shared_ptr<wstring> powerShellDisMountScript = make_shared<wstring>(L"d:\\dataset\\scripts\\hddismount.ps1");
shared_ptr<wstring> powerShellCopyResultsScript = make_shared<wstring>(L"d:\\dataset\\scripts\\copyresults.ps1");

//
// Trace collector execution template
//
shared_ptr<wstring> traceCollectorTemplate = make_shared<wstring>(L"t:\\scripts\\tracecollector \"%s\" 120000 t:\\data\\%s.etl >> t:\\data\\%s.log\n");

//
// Output batch file path
//
shared_ptr<wstring> outputBatchFilePath = make_shared<wstring>(L"f:\\data\\onerun.bat");

//
// Collection of power shell commands locations
//
vector< shared_ptr<wstring>> powerShellParametersCollection = { powerShellStartScript, powerShellStopScript, powerShellMountScript, powerShellRevertScript, powerShellDisMountScript, powerShellCopyResultsScript };

//
// Binding table for parameters to resolve after configuration file read complete
// Note: some or all parameters may be missing in the configuration file, default values mentioned above will be used
// Note: all parameters names must be lower case ones
//

vector < pair<wstring, shared_ptr<wstring>>> totalParametersCollection = { make_pair<>(L"powershellpath", powerShellPath)
    , make_pair<>(L"powershellstartscript", powerShellStartScript)
    , make_pair<>(L"powershellstopscript", powerShellStopScript)
    , make_pair<>(L"powershellmountscript", powerShellMountScript)
    , make_pair<>(L"powershellrevertscript", powerShellRevertScript)
    , make_pair<>(L"powershelldismountscript", powerShellDisMountScript)
    , make_pair<>(L"powershellcopyresultsscript", powerShellCopyResultsScript)
    , make_pair<>(L"tracecollectortemplate", traceCollectorTemplate)
    , make_pair<>(L"outputbatchfilepath", outputBatchFilePath) };

//
// Forward declarations
//
unordered_map<wstring, wstring> ParseParameters(_In_ const wchar_t* configFile, _In_ ULONG MaxParameterSize);
void ToLower(_In_ wstring& result);

//
// Bind located parameters
//
void BindParameters(_In_ unordered_map<wstring, wstring>& inputMap)
{
    for (auto val : totalParametersCollection)
    {
        auto result = inputMap.find(val.first);
        if (result != inputMap.end())
        {
            *val.second = result->second;
        }
    }
}

//
// Power shell scripts paths preparations
//
void PreparePowerShellCommands()
{
    wstring separator = L" -f ";
    for (auto var : powerShellParametersCollection)
    {
        *var = *powerShellPath + separator  + *var;
    }
}

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
    wstring fullPowerShellPolicyString = *powerShellPath + *powerShellPolicy;

    auto ret_code = Execute(powerShellPath->c_str(), fullPowerShellPolicyString.c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS configuration failed with %x error\n", ret_code );
        return ret_code;
    }

    // start the VM
    ret_code = Execute(powerShellPath->c_str(), powerShellStartScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM start failed with %x error\n", ret_code );
        return ret_code;
    }

    // wait for 10 minutes
    ::SleepEx( 60000, FALSE );

    // stop the VM
    ret_code = Execute(powerShellPath->c_str(), powerShellStopScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // Mount VM volume
    ret_code = Execute(powerShellPath->c_str(), powerShellMountScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // Copy results
    ret_code = Execute(powerShellPath->c_str(), powerShellCopyResultsScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // UnMount VM volume
    ret_code = Execute(powerShellPath->c_str(), powerShellDisMountScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    // Revert VM
    ret_code = Execute(powerShellPath->c_str(), powerShellRevertScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    return ret_code;
}

HRESULT PrepareExecutionScript( const wchar_t* exe_name, const wchar_t* out )
{
    wstring fullPowerShellPolicyString = *powerShellPath + *powerShellPolicy;
    auto ret_code = Execute(powerShellPath->c_str(), fullPowerShellPolicyString.c_str());

    if ( ret_code != S_OK )
    {
        printf( "PS configuration failed with %x error\n", ret_code );
        return ret_code;
    }
    // Mount VM volume
    ret_code = Execute(powerShellPath->c_str(), powerShellMountScript->c_str());
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

        wsprintf( buffer, traceCollectorTemplate->c_str(), exe_name, copy, copy );
        fputws( buffer, ofile );
        fputws( L"timeout /t 5\n", ofile );

        fflush( ofile );
        fclose( ofile );
    }
    // UnMount VM volume
    ret_code = Execute(powerShellPath->c_str(), powerShellDisMountScript->c_str());
    if ( ret_code != S_OK )
    {
        printf( "PS VM stop failed with %x error\n", ret_code );
        return ret_code;
    }

    return ret_code;
}

int wmain(int argc, const wchar_t** argv)
{
    unordered_map<wstring, wstring> parametersCollection;
    printf( "\nCycleExec v1.2\n" );
    // declaring argument of time()
    time_t my_time = time( NULL );

    char buffer [MAX_PATH];

    if ( argc != 3 )
    {
        printf( "Format: %S <samples path aggregated file> <configuration file>\n", argv[0] );
        printf("Configuration file format:\n");
        printf("\tParameter=Data\n");
        printf("Where <Parameter> can be one of next:\n");
        printf("\tpowershellpath - fully qualified path to power shell\n");
        printf("\tpowershellstartscript - fully qualified path to power shell script to start the test VM\n");
        printf("\tpowershellstopscript - fully qualified path to power shell script to stop the test VM\n");
        printf("\tpowershellmountscript - fully qualified path to power shell script to mount the VM HDD\n");
        printf("\tpowershellrevertscript - fully qualified path to power shell script to revert the VM HDD to initial state\n");
        printf("\tpowershelldismountscript - fully qualified path to power shell script to dismount the VM HDD\n");
        printf("\tpowershellcopyresultsscript - fully qualified path to power shell script to copy test results from mounted VM HDD\n");
        printf("\ttracecollectortemplate - template string used within auto generated execution script to create trace collector command line\n");
        printf("\toutputbatchfilepath - fully qualified path to location where the auto generated batch file should be stored\n");
        return 1;
    }

    parametersCollection = ParseParameters(argv[2], MAX_PARAMETERS_SIZE);

    if (parametersCollection.size() == 0)
    {
        printf("FATAL: failed to read input parameters from %S", argv[2]);
        return 1;
    }

    BindParameters(parametersCollection);
    PreparePowerShellCommands();

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

        auto retval = PrepareExecutionScript(sbuffer, outputBatchFilePath->c_str());
        retval = ExecutionCycle();

        my_time = time( NULL );
        ctime_s( buffer, MAX_PATH - 1, &my_time );
        printf( "Ended at %s with %x\n", buffer, retval );
    }
    fclose( file );
    return 0;
}

