// CsvParser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include "ParserApi.h"

#pragma comment( lib, "user32" )

using namespace std;

int wmain(int argc, const wchar_t** argv)
{
    /*if ( argc == 5 )
    {
        if ( _wcsicmp( argv [1], L"-filter" ) == 0 )
        {
            if ( ParseCVSFileByPattern( argv [2], argv [4], argv [3] ) )
            {
                return 0;
            }

            return 1;
        }
    }*/

    //if ( argc == 3 )
    {
        if ( _wcsicmp( argv [1], L"-convert" ) == 0 )
        {
            vector<wstring> inputs;

            for ( int i = 2; i < argc; i++ )
            {
                inputs.push_back( argv [i] );
            }
            ParseEtlFolders( argv [2], inputs );
            return 0;
        }

        if ( _wcsicmp( argv [1], L"-aggregate" ) == 0 )
        {
            vector<wstring> inputs;

            for ( int i = 2; i < argc; i++ )
            {
                inputs.push_back( argv [i] );
            }
            ParseAggregateFolders( argv [2], inputs );
            return 0;
        }

    }

    return 0;
}


