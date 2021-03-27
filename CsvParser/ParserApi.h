#pragma once

#include <string>
#include <vector>

typedef bool ( *ParseCallback )( _In_ const std::wstring& file );

bool ParseCVSFileByPattern( _In_ const wchar_t* Input, _In_ const wchar_t* Pattern, _Out_ const wchar_t* Output );
void ParseEtlFolders( _In_ const wchar_t* output_folder, _In_ std::vector<std::wstring>& inputs );
void ParseAggregateFolders( _In_ const wchar_t* output_folder, _In_ std::vector<std::wstring>& inputs );
void ParseFolderWithCallback( _In_ const wchar_t* folder, _In_ const wchar_t* LabelFile, _In_ const wchar_t* Label, _In_ const wchar_t* postFix, _In_ ParseCallback callback );
void FilterAllCSvFiles( _In_ const wchar_t* Input );
void AggregateAllTraces( _In_ const wchar_t* Input );
void AggregateFilteredTraces( _In_ const wchar_t* output_folder, _In_ std::vector<std::wstring>& inputs, _In_ const wchar_t* LabelFile );


bool ConvertToAnsi( _In_ const wchar_t* Pattern, _In_ ULONG size, _Out_ char* Output );
bool ConvertToWide( _In_ const char* val, _Out_ std::wstring& string );
std::string AggregateString( _In_ const char* line, char splitChar, int limit );
void RecordFileName( _In_ const std::wstring& path );
int fsize( FILE* fp );

