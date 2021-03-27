#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include "ParserApi.h"

using namespace std;

extern vector<wstring> g_LabelsMapping;
extern vector<wstring> g_FileNamesMapping;

bool RemoveIfNoPid( _In_ const wstring& file )
{
    wstring inFile( file );
    wstring inFile2( file );
    wstring inFile3( file );
    inFile2 += L".syslog";
    inFile3 += L".pid";

    if ( !filesystem::exists( inFile2 ) || !filesystem::exists( inFile3 ) )
    {
        ::DeleteFile( inFile.c_str() );
        ::DeleteFile( inFile2.c_str() );
        ::DeleteFile( inFile3.c_str() );
    }
    return true;
}

bool ConvertEtlFile( _In_ const wstring& file )
{
    wstring inFile( file );
    wstring inFile2( inFile );
    wstring outFile( inFile );
    inFile2 += L".syslog";

    auto const pos = outFile.find_last_of( '.' );
    outFile.replace( pos, 4, L".csv" );

    wstring commandLine = L"tracerpt ";
    commandLine += inFile;
    commandLine += L" ";
    commandLine += inFile2;
    commandLine += L" -o ";
    commandLine += outFile;
    commandLine += L" -of CSV -pdb srv*DownstreamStore*https:////msdl.microsoft.com//download//symbols";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int exit_code = 0;

    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );
    ZeroMemory( &pi, sizeof( pi ) );

    if ( !::CreateProcess( NULL,   // No module name (use command line)
        (wchar_t*) commandLine.c_str(),        // Command line
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
        return true;;
    }

    // Wait until child process exits.
    auto result = WaitForSingleObject( pi.hProcess, INFINITE );
    // Close process and thread handles.
    ::CloseHandle( pi.hProcess );
    ::CloseHandle( pi.hThread );

    RecordFileName( file );
    return true;
}

void RecordFileName( _In_ const wstring& path )
{
    auto pos = path.find_last_of( '\\' );
    if ( pos != wstring::npos )
    {
        wstring parsedName = path.substr( pos+1 );
        pos = parsedName.find_last_of( '.' );
        if ( pos != wstring::npos )
        {
            parsedName.erase( pos );
        }
        g_FileNamesMapping.push_back( parsedName );
    }

}

void ParseEtlFolders( _In_ const wchar_t* output_folder, _In_ vector<wstring>& inputs )
{
    int index = 0;
    for ( auto& var : inputs )
    {
        ParseFolderWithCallback( var.c_str(), NULL, NULL, L"\\*.etl", RemoveIfNoPid );
    }

    for ( auto& var : inputs )
    {

        if ( index < 1 )
        {
            ParseFolderWithCallback( var.c_str(), L"\\output\\label.all", L"Bening", L"\\*.etl", ConvertEtlFile );
        }
        else
        {
            ParseFolderWithCallback( var.c_str(), L"\\output\\label.all", L"Malware", L"\\*.etl", ConvertEtlFile );
        }

        index++;

        FilterAllCSvFiles( var.c_str() );
        AggregateAllTraces( var.c_str() );
    }
    AggregateFilteredTraces( output_folder, inputs, NULL );
}

void ParseAggregateFolders( _In_ const wchar_t* output_folder, _In_ vector<wstring>& inputs )
{
    AggregateFilteredTraces( output_folder, inputs, L"\\output\\label.all" );
}