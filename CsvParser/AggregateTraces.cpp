#include <windows.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>
#include "ParserApi.h"

using namespace std;
using UnMappedTrace = vector<string>;
using MappedTrace = vector<uint32_t>;

vector< UnMappedTrace> g_UnmappedTraces;
vector< MappedTrace> g_MappedTraces;
unordered_map<string, uint32_t> g_NamesMapping;
unordered_map<string, uint32_t> g_NamesHits;
unordered_map<string, uint32_t> g_NamesPerProcessHits;
uint32_t g_MappingSeed { 1 };
uint32_t g_MaxTraceSize { 0 };
uint32_t g_AverageTraceSize { 0 };
vector<wstring> g_LabelsMapping;
vector<wstring> g_FileNamesMapping;

void WrteOutputCvsFile( _In_ wstring& path, _In_ uint32_t MaxRecordSize )
{
    FILE* out = NULL;
    _wfopen_s( &out, path.c_str(), L"wt" );
    UINT index = 0;

    if ( out )
    {
        char* numBuffer = new char [10240];
        //
        // Print header
        //
        sprintf( numBuffer, "Label " );
        fwrite( numBuffer, 1, strlen( numBuffer ), out );
        uint32_t k = 0;

        for ( uint32_t i = 1; i <= MaxRecordSize; i++ )
        {
            sprintf( numBuffer, "Call%ld ", i );
            fwrite( numBuffer, 1, strlen( numBuffer ), out );
        }

        sprintf( numBuffer, "\n" );
        fwrite( numBuffer, 1, strlen( numBuffer ), out );

        for ( const auto& var : g_MappedTraces )
        {
            if ( index < g_LabelsMapping.size() )
            {
                auto& label = g_LabelsMapping.at( index++ );

                sprintf( numBuffer, "%S ", label.c_str() );
                fwrite( numBuffer, 1, strlen( numBuffer ), out );
            }
            k = 0;
            for ( const auto& name : var )
            {
                if ( k < MaxRecordSize )
                {
                    sprintf( numBuffer, "%ld ", name );
                    fwrite( numBuffer, 1, strlen( numBuffer ), out );
                }
                else
                {
                    break;
                }
                k++;
            }

            //
            // Perform padding
            //
            if ( MaxRecordSize > var.size() )
            {
                for ( uint32_t i = 0; i < MaxRecordSize - var.size(); i++ )
                {
                    sprintf( numBuffer, "%ld ", 0 );
                    fwrite( numBuffer, 1, strlen( numBuffer ), out );
                }
            }
            sprintf( numBuffer, "\n" );
            fwrite( numBuffer, 1, strlen( numBuffer ), out );
        }

        fflush( out );
        fclose( out );
        delete[] numBuffer;
    }

}

bool AggregateFilteredCsvFile( _In_ const wstring& file )
{
    FILE* in = NULL;;
    char* buffer = NULL;
    _wfopen_s( &in, file.c_str(), L"rt" );

    auto size = fsize( in );

    if ( size > 1 )
    {
        buffer = new char [size*2];

        if ( buffer )
        {
            auto read_size = fread( buffer, 1, size, in );
            UnMappedTrace trace;
            char* ptr = buffer;
            char* head = buffer;
            char* end = buffer + read_size;
            while ( (*ptr != 0 ) && (ptr <= end))
            {
                if ( *ptr == ',' )
                {
                    *ptr = 0;
                    trace.push_back( head );
                    head = ptr + 1;
                    ptr++;
                    continue;
                }
                ptr++;
            }
            g_UnmappedTraces.push_back( trace );

        }
    }

    if ( buffer )
    {
        delete[] buffer;
    }

    if ( in )
    {
        fclose( in );
    }

    RecordFileName( file );

    return true;
}

void MapCollectedTraces( _In_ const wchar_t* output_folder, _In_ vector<wstring>& inputs )
{
    if ( g_UnmappedTraces.empty() )
    {
        return;
    }

    // perform names mapping
    for ( const auto& var : g_UnmappedTraces )
    {
        unordered_map<string, uint32_t> localNamesPerProcessHits;

        for ( const auto& name : var )
        {
            auto it = g_NamesMapping.find( name );
            if ( it == g_NamesMapping.end() ) // no mapping
            {
                g_NamesMapping.insert( make_pair<>( name, g_MappingSeed++ ) );
                g_NamesHits.insert( make_pair<>( name, 1 ) );
            }
            else
            {
                auto hits = g_NamesHits [name];
                g_NamesHits [name] = hits + 1;
            }

            auto itPerProcess = g_NamesPerProcessHits.find( name );
            auto itLocalHit = localNamesPerProcessHits.find( name );


            if ( itPerProcess == g_NamesPerProcessHits.end() ) // no mapping
            {
                g_NamesPerProcessHits.insert( make_pair<>( name, 1 ) );
                localNamesPerProcessHits.insert( make_pair<>( name, 1 ) );
            }
            else
            {
                if ( itLocalHit == localNamesPerProcessHits.end() ) // no mapping
                {
                    auto hits = g_NamesPerProcessHits [name];
                    g_NamesPerProcessHits [name] = hits + 1;
                    localNamesPerProcessHits.insert( make_pair<>( name, 1 ) );
                }
            }

        }
    }

    // perform remapping
    for ( const auto& var : g_UnmappedTraces )
    {
        MappedTrace trace;
        for ( const auto& name : var )
        {
            auto it = g_NamesMapping.find( name );
            if ( it != g_NamesMapping.end() ) // no mapping
            {
                trace.push_back( it->second );
            }
        }

        if ( trace.size() > g_MaxTraceSize )
        {
            g_MaxTraceSize = (uint32_t)trace.size();
        }

        if ( !trace.empty() )
        {
            g_MappedTraces.push_back( trace );
        }
    }

    // calculate average value of trace size
    ULONG64 traceSizeSum = 0;
    for ( const auto& var : g_MappedTraces )
    {
        traceSizeSum += var.size();
    }

    g_AverageTraceSize = (uint32_t) ( traceSizeSum / g_MappedTraces.size() );

    wstring path( output_folder );
    path += L"\\output\\alltraces.csv";

    WrteOutputCvsFile( path, g_MaxTraceSize );

    path = output_folder;
    path += L"\\output\\alltraces_average.csv";
    WrteOutputCvsFile( path, g_AverageTraceSize );

    path = output_folder;
    path += L"\\output\\alltraces_10.csv";
    WrteOutputCvsFile( path, 10 );

    FILE* out = NULL;
    path = output_folder;
    path += L"\\output\\alltraces_summary.csv";

    _wfopen_s( &out, path.c_str(), L"wt" );
    if ( out )
    {
        char numBuffer [256];
        for ( const auto& var : g_NamesMapping )
        {
            auto it = g_NamesHits.find( var.first );
            auto it2 = g_NamesPerProcessHits.find( var.first );

            if ( (it != g_NamesHits.end() ) && ( it2 != g_NamesPerProcessHits.end() ))
            {
                sprintf( numBuffer, "%s %ld %ld %ld\n", var.first.c_str(), var.second, it->second, it2->second );
                fwrite( numBuffer, 1, strlen( numBuffer ), out );
            }
        }
        fflush( out );
        fclose( out );
    }

    path = output_folder;
    path += L"\\output\\alltraces_files.csv";

    _wfopen_s( &out, path.c_str(), L"wt" );
    if ( out )
    {

        char nameBuffer [256];
        for ( const auto& var : g_FileNamesMapping )
        {
            sprintf( nameBuffer, "%S\n", var.c_str());
            fwrite( nameBuffer, 1, strlen( nameBuffer ), out );
        }

        fflush( out );
        fclose( out );
    }

}

void AggregateFilteredTraces( _In_ const wchar_t* output_folder, _In_ vector<wstring>& inputs, _In_ const wchar_t* LabelFile )
{
    if ( output_folder == nullptr )
    {
        return;
    }

    ULONG index = 0;
    for ( auto& var : inputs )
    {
        if ( LabelFile == NULL )
        {
            ParseFolderWithCallback( var.c_str(), NULL, NULL, L"\\*.csv.aggr", AggregateFilteredCsvFile );
        }
        else
        {
            if ( index < 1 )
            {
                ParseFolderWithCallback( var.c_str(), LabelFile, L"Bening", L"\\*.csv.aggr", AggregateFilteredCsvFile );
            }
            else
            {
                ParseFolderWithCallback( var.c_str(), LabelFile, L"Malware", L"\\*.csv.aggr", AggregateFilteredCsvFile );
            }
            index++;
        }
    }
    MapCollectedTraces( output_folder, inputs );
}
