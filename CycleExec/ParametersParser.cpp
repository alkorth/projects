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
#include <tuple>
#include <algorithm>

using namespace std;

void ToLower(_In_ wstring& result)
{
    //
    // Convert all to lower
    //
    transform(
        result.begin(), result.end(),
        result.begin(),
        towlower);
}

//
//The function removes extra spaces from beginning and end of the given input
//
wstring NormalizeString(_In_ const wchar_t* input)
{
    wstring result;
    if (input != NULL)
    {
        auto size = wcslen(input);

        if (size == 0)
        {
            return result;
        }

        wchar_t* head = (wchar_t*)input;
        while (*head != 0)
        {
            if (*head != ' ')
            {
                break;
            }
            head++;
        }
        wchar_t* tail = (wchar_t*)input;
        tail += size - 1;
        while (tail != head)
        {
            if ((*tail != ' ') && ( ( *tail != '\n' ) ))
            {
                break;
            }
            tail--;
        }
        result = head;
        result.resize(tail - head);
        ToLower(result);
    }
    return result;
}

unordered_map<wstring, wstring> ParseParameters(_In_ const wchar_t* configFile, _In_ ULONG MaxParameterSize)
{
    unordered_map<wstring, wstring> parametersMap;
    if ((configFile != NULL) && ( MaxParameterSize >0))
    {
        FILE* inputFile;
        if (_wfopen_s(&inputFile, configFile, L"rt") == 0)
        {
            wchar_t* buffer =new wchar_t[MaxParameterSize+1];
            if (buffer != NULL)
            {
                while (fgetws(buffer, MaxParameterSize, inputFile) != NULL)
                {
                    wchar_t* pos = wcschr(buffer+1, '='); // buffer + 1 used to exclude first symbol of the string, it should not be '=' anyway
                    if (pos == NULL)
                    {
                        continue; // wrong format, bypass the string
                    }
                    *pos = 0;
                    auto name = NormalizeString(buffer);
                    auto data = NormalizeString(pos+1);

                    if (( name.size() > 0 ) && ( data.size() > 0 ))
                    {
                        parametersMap.insert(make_pair<>(name, data));
                    }
                }
                delete[] buffer;
            }
            fclose(inputFile);
        }
    }
    return parametersMap;
}