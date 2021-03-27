#include <windows.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include "ParserApi.h"

using namespace std;

bool ParseCVSFileByPattern( _In_ const wchar_t* Input, _In_ const wchar_t* Pattern, _Out_ const wchar_t* Output )
{
    if ( !Input || !Output || !Pattern )
    {
        return false;
    }

    FILE* in = NULL;
    FILE* out = NULL;

    char* readBuffer = new char [10240];
    char lowPattern [1024];
    bool result = false;


    if ( !ConvertToAnsi( Pattern, 1023, lowPattern ) )
    {
        return false;
    }

    __try
    {
        if ( !readBuffer )
        {
            __leave;
        }
        _wfopen_s( &in, Input, L"rt" );
        if ( !in )
        {
            __leave;
        }

        _wfopen_s( &out, Output, L"wt" );
        if ( !out )
        {
            __leave;
        }

        while ( fgets( readBuffer, 10239, in ) != NULL )
        {
            _strlwr_s( readBuffer, 10239 );
            auto pos = strstr( readBuffer, lowPattern );
            if ( pos != NULL)
            {
                fputs( readBuffer, out );
            }
        }
    }
    __finally
    {
        if ( readBuffer )
        {
            delete[] readBuffer;
        }

        if ( in )
        {
            fclose( in );
        }

        if ( out )
        {
            fclose( out );
        }
    }

    return result;

}

const wstring GetFilterPid( _In_ const wstring& PidStorage )
{
    wstring result( L"" );
    FILE* in = NULL;
    char* readBuffer = new char [10240];

    if ( readBuffer != nullptr )
    {
        _wfopen_s( &in, PidStorage.c_str(), L"rt" );
        if ( in != NULL )
        {
            while ( fgets( readBuffer, 10239, in ) != NULL )
            {
                auto ptr = strchr( readBuffer, '(' );
                if ( ptr != nullptr )
                {
                    ptr++;
                    auto endPtr = strchr( ptr, ')' );
                    if ( endPtr != nullptr )
                    {
                        *endPtr = 0;
                        wstring val;
                        ConvertToWide( ptr, val );
                        result = L"0x";
                        int filler = (int)(8 - val.size());
                        while ( filler > 0 )
                        {
                            result += L"0";
                            filler--;
                        }
                        result += val;
                    }
                    break;
                }
            }
            fclose( in );
        }
    }

    if ( readBuffer != nullptr )
    {
        delete[] readBuffer;
    }

    return result;
}

bool ParsePidFile( _In_ const wstring& file )
{
    auto pidVal = GetFilterPid( file );
    if ( !pidVal.empty() )
    {
        wstring input( file );
        wstring output( file );

        input.replace( input.end() - 8, input.end(), L".csv" );
        output.replace( output.end() - 8, output.end(), L".csv.filtered" );
        ParseCVSFileByPattern( input.c_str(), pidVal.c_str(), output.c_str() );
    }

    return true;
}

void FilterAllCSvFiles( _In_ const wchar_t* Input )
{
    if ( Input == nullptr )
    {
        return;
    }


    ParseFolderWithCallback( Input, NULL, NULL, L"\\*.etl.pid", ParsePidFile );
}
