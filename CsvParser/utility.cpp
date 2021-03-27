#include <Windows.h>
#include <string>
#include <algorithm>
#include "ParserApi.h"

using namespace std;
extern vector<wstring> g_LabelsMapping;

void ParseFolderWithCallback( _In_ const wchar_t* folder, _In_ const wchar_t* LabelFile, _In_ const wchar_t* Label, _In_ const wchar_t* postFix , _In_ ParseCallback callback )
{
    wstring pattern( folder );
    pattern += postFix;

    wstring path( folder );

    WIN32_FIND_DATA findData { 0 };
    char labelBuffer [64];
    FILE* out = NULL;

    if ( ( LabelFile != nullptr ) && ( Label != nullptr ) )
    {
        path += LabelFile;
        _wfopen_s( &out, path.c_str(), L"at" );

        if ( !out )
        {
            return;
        }
    }

    auto hFind = ::FindFirstFile( pattern.c_str(), &findData );
    while ( hFind != INVALID_HANDLE_VALUE )
    {
        wstring dest( folder );
        dest += L"\\";
        dest += findData.cFileName;
        if ( !callback( dest.c_str() ) )
        {
            break;
        }

        if ( out )
        {
            sprintf( labelBuffer, "%S\n", Label );
            fwrite( labelBuffer, 1, strlen( labelBuffer ), out );
            g_LabelsMapping.push_back(Label);
        }

        if ( !::FindNextFile( hFind, &findData ) )
        {
            break;
        }

    }

    if ( out )
    {
        fflush( out );
        fclose( out );
    }
}

CHAR wide_to_narrow( WCHAR w )
{
    // simple typecast
    // works because UNICODE incorporates ASCII into itself
    return tolower( CHAR( w ) );
}

bool ConvertToAnsi( _In_ const wchar_t* Pattern, _In_ ULONG size, _Out_ char* Output )
{
    if ( !Pattern || !Output || !size )
    {
        return false;
    }

    std::wstring wstr { Pattern };
    std::string dest;
    dest.resize( wstr.size() );
    std::transform( wstr.begin(), wstr.end(), dest.begin(), wide_to_narrow );
    strncpy( Output, dest.c_str(), size );
    return true;
}

bool ConvertToWide( _In_ const char* val, _Out_ wstring& string )
{

    if ( !val )
    {
        return false;
    }

    const size_t cSize = strlen( val ) + 1;
    wchar_t* buffer  = new wchar_t[cSize];
    if ( buffer == nullptr )
    {
        return false;
    }

    mbstowcs( buffer, val, cSize );
    string = buffer;
    delete[] buffer;
    return true;
}


string AggregateString( _In_ const char* line, char splitChar, int limit )
{
    string buffer;
    const char* ptr = line;

    while ( *ptr != 0 )
    {
        if ( IsCharAlphaNumericA( *ptr ) )
        {
            buffer.push_back( *ptr );
            ptr++;
            continue;
        }

        if ( *ptr == ' ' )
        {
            ptr++;
            continue;
        }

        if ( *ptr == ',' )
        {
            buffer.push_back( splitChar );
            limit--;
        }

        ptr++;

        if ( limit == 0 )
        {
            break;
        }

    }

    if ( !buffer.empty() )
    {
        buffer.pop_back(); // remove last char, obsolete
    }
    return buffer;
}

int fsize( FILE* fp )
{
    int prev = ftell( fp );
    fseek( fp, 0L, SEEK_END );
    int sz = ftell( fp );
    fseek( fp, prev, SEEK_SET ); //go back to where we were
    return sz;
}