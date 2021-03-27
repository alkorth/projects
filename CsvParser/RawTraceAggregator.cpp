#include <windows.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include "ParserApi.h"

using namespace std;

bool AggregateCsvFile( _In_ const wstring& file )
{
    FILE* in;
    FILE* out;

    wstring output( file );
    output.replace( output.end() - 13, output.end(), L".csv.aggr" );

    _wfopen_s( &in, file.c_str(), L"rt" );
    _wfopen_s( &out, output.c_str(), L"wt" );

    if ( in && out )
    {
        char* readBuffer = new char [10240];

        if ( readBuffer != nullptr )
        {
            while ( fgets( readBuffer, 10239, in ) != NULL )
            {
                auto val = AggregateString( readBuffer, '_', 4 );

                if ( !val.empty() )
                {
                    val += ",";
                    fputs( val.c_str(), out );
                }
            }
        }

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

    return true;
}

void AggregateAllTraces( _In_ const wchar_t* Input )
{
    if ( Input == nullptr )
    {
        return;
    }

    ParseFolderWithCallback( Input, NULL, NULL, L"\\*.csv.filtered", AggregateCsvFile );
}
