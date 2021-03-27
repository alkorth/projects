#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <strsafe.h>
#include <wmistr.h>
#include <evntrace.h>
#include <string>
#include "TraceCollector.h"

#pragma comment(lib, "advapi32.lib")

static TRACEHANDLE g_SystemTraceHandle = 0; // global singletone for system session
static const GUID g_systemGuid = { 0x9E814AAD, 0x3204, 0x11D2, { 0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39 } };

HRESULT CloseSystemLogSession( _In_ const wchar_t* logPath )
{
    ULONG status = ERROR_SUCCESS;
    //TRACEHANDLE SessionHandle = 0;
    EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
    ULONG BufferSize = 0;
    HRESULT hResult = S_OK;

    if ( !g_SystemTraceHandle )
    {
        return E_ABORT;
    }

    std::wstring systemLogFile = logPath;

    systemLogFile += L".syslog";
    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.

    BufferSize = (ULONG) ( sizeof( EVENT_TRACE_PROPERTIES ) + sizeof( wchar_t ) * ( systemLogFile.size() + 1 ) + sizeof( KERNEL_LOGGER_NAME ) );
    pSessionProperties = (EVENT_TRACE_PROPERTIES*) malloc( BufferSize );
    if ( NULL == pSessionProperties )
    {
        return E_OUTOFMEMORY;
    }

    // Set the session properties. You only append the log file name
    // to the properties structure; the StartTrace function appends
    // the session name for you.

    ZeroMemory( pSessionProperties, BufferSize );
    pSessionProperties->Wnode.BufferSize = BufferSize;
    pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
    pSessionProperties->Wnode.Guid = g_systemGuid;
    pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_PROCESS
        | EVENT_TRACE_FLAG_THREAD
        | EVENT_TRACE_FLAG_IMAGE_LOAD
        | EVENT_TRACE_FLAG_REGISTRY
        | EVENT_TRACE_FLAG_SYSTEMCALL
        | EVENT_TRACE_FLAG_ALPC
        | EVENT_TRACE_FLAG_VIRTUAL_ALLOC
        | EVENT_TRACE_FLAG_VAMAP
        | EVENT_TRACE_FLAG_DISPATCHER
        | EVENT_TRACE_FLAG_DRIVER
        | EVENT_TRACE_FLAG_DPC
        | EVENT_TRACE_FLAG_FILE_IO
        | EVENT_TRACE_FLAG_FILE_IO_INIT;

    pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
    pSessionProperties->MaximumFileSize = 0;  // 100 MB
    pSessionProperties->LoggerNameOffset = sizeof( EVENT_TRACE_PROPERTIES );
    pSessionProperties->LogFileNameOffset = sizeof( EVENT_TRACE_PROPERTIES ) + sizeof( KERNEL_LOGGER_NAME );
    StringCbCopy( (LPWSTR) ( (char*) pSessionProperties + pSessionProperties->LogFileNameOffset ), sizeof( wchar_t ) * ( systemLogFile.size() + 1 ), systemLogFile.c_str() );

    if ( g_SystemTraceHandle )
    {
        status = ControlTrace( g_SystemTraceHandle, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_STOP );

        if ( pSessionProperties )
        {
            free( pSessionProperties );
        }

        if ( ERROR_SUCCESS != status )
        {
            return HRESULT_FROM_WIN32( status );
        }
        else
        {
            g_SystemTraceHandle = 0;
        }
    }

    return S_OK;

}

HRESULT CreateSystemLogSession( _In_ const wchar_t* logPath )
{
    ULONG status = ERROR_SUCCESS;
    EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
    ULONG BufferSize = 0;
    HRESULT hResult = S_OK;
    std::wstring systemLogFile = logPath;

    systemLogFile += L".syslog";
    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.

    BufferSize = (ULONG)(sizeof( EVENT_TRACE_PROPERTIES ) + sizeof(wchar_t)*(systemLogFile.size()+1) + sizeof( KERNEL_LOGGER_NAME ));
    pSessionProperties = (EVENT_TRACE_PROPERTIES*) malloc( BufferSize );
    if ( NULL == pSessionProperties )
    {
        return E_OUTOFMEMORY;
    }

    // Set the session properties. You only append the log file name
    // to the properties structure; the StartTrace function appends
    // the session name for you.

    ZeroMemory( pSessionProperties, BufferSize );
    pSessionProperties->Wnode.BufferSize = BufferSize;
    pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
    pSessionProperties->Wnode.Guid = g_systemGuid;
    pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_PROCESS
        | EVENT_TRACE_FLAG_THREAD
        | EVENT_TRACE_FLAG_IMAGE_LOAD
        | EVENT_TRACE_FLAG_REGISTRY
        | EVENT_TRACE_FLAG_SYSTEMCALL
        | EVENT_TRACE_FLAG_ALPC
        | EVENT_TRACE_FLAG_VIRTUAL_ALLOC
        | EVENT_TRACE_FLAG_VAMAP
        | EVENT_TRACE_FLAG_DISPATCHER
        | EVENT_TRACE_FLAG_DRIVER
        | EVENT_TRACE_FLAG_DPC
        | EVENT_TRACE_FLAG_FILE_IO
        | EVENT_TRACE_FLAG_FILE_IO_INIT;


    pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
    pSessionProperties->MaximumFileSize = 0;  // 100 MB
    pSessionProperties->LoggerNameOffset = sizeof( EVENT_TRACE_PROPERTIES );
    pSessionProperties->LogFileNameOffset = sizeof( EVENT_TRACE_PROPERTIES ) + sizeof( KERNEL_LOGGER_NAME );
    StringCbCopy( (LPWSTR) ( (char*) pSessionProperties + pSessionProperties->LogFileNameOffset ), sizeof( wchar_t ) * ( systemLogFile.size() + 1 ), systemLogFile.c_str() );

    // Create the trace session.

    status = StartTrace( (PTRACEHANDLE) &g_SystemTraceHandle, KERNEL_LOGGER_NAME, pSessionProperties );

    if ( pSessionProperties )
    {
        free( pSessionProperties );
    }

    if ( ERROR_SUCCESS != status )
    {
        hResult = HRESULT_FROM_WIN32( status );
        goto cleanup;
    }

    return hResult;

cleanup:
    CloseSystemLogSession( logPath );

    return hResult;
}


HRESULT DestroyLoggingSession( HANDLE SessionHandle, const wchar_t* SessionName, const wchar_t* OutputFile, const GUID& SessionGuid, const GUID* ProviderGuids, UINT NumOfProviders )
{
    HRESULT hResult = S_OK;
    ULONG status = ERROR_SUCCESS;
    EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;

    if ( !ProviderGuids || (NumOfProviders == 0) || (SessionHandle == INVALID_HANDLE_VALUE) || !SessionName || !OutputFile )
    {
        return E_INVALIDARG;
    }

    CloseSystemLogSession( OutputFile );

    auto NameSize = (wcslen( SessionName ) + 1) * sizeof( wchar_t );
    auto OutputSize = (wcslen( OutputFile ) + 1) * sizeof( wchar_t );

    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.

    ULONG BufferSize = (ULONG)(sizeof( EVENT_TRACE_PROPERTIES ) + NameSize + OutputSize);
    pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc( BufferSize );
    if ( NULL == pSessionProperties )
    {
        return E_OUTOFMEMORY;
    }

    // Set the session properties. You only append the log file name
    // to the properties structure; the StartTrace function appends
    // the session name for you.

    ZeroMemory( pSessionProperties, BufferSize );
    pSessionProperties->Wnode.BufferSize = BufferSize;
    pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
    pSessionProperties->Wnode.Guid = SessionGuid;
    pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
    pSessionProperties->MaximumFileSize = 0;  // 100 MB
    pSessionProperties->LoggerNameOffset = sizeof( EVENT_TRACE_PROPERTIES );
    pSessionProperties->LogFileNameOffset = ( ULONG )(sizeof( EVENT_TRACE_PROPERTIES ) + NameSize);
    StringCbCopy( (LPWSTR)((char*)pSessionProperties + pSessionProperties->LogFileNameOffset), OutputSize, OutputFile );

    if ( SessionHandle )
    {
        for ( UINT i = 0; i < NumOfProviders; i++ )
        {
            status = EnableTraceEx2(
                (TRACEHANDLE)SessionHandle,
                (LPCGUID)&ProviderGuids[i],
                EVENT_CONTROL_CODE_DISABLE_PROVIDER,
                TRACE_LEVEL_INFORMATION,
                0,
                0,
                0,
                NULL
            );

            status = ControlTrace( (TRACEHANDLE)SessionHandle, SessionName, pSessionProperties, EVENT_TRACE_CONTROL_STOP );

            if ( ERROR_SUCCESS != status )
            {
                hResult = HRESULT_FROM_WIN32( status );
            }
        }
    }

    if ( pSessionProperties )
    {
        free( pSessionProperties );
        pSessionProperties = NULL;
    }

    return hResult;
}

HRESULT CreateLoggingSession( const wchar_t* SessionName, const GUID& SessionGuid, const GUID* ProviderGuids, UINT NumOfProviders, const wchar_t* OutputFile, HANDLE& SessionOutHandle )
{
    ULONG status = ERROR_SUCCESS;
    TRACEHANDLE SessionHandle = 0;
    EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
    ULONG BufferSize = 0;
    BOOL TraceOn = TRUE;
    HRESULT hResult = S_OK;

    if ( !SessionName || !ProviderGuids || (NumOfProviders == 0) || !OutputFile )
    {
        return E_INVALIDARG;
    }

    auto NameSize = (wcslen( SessionName ) + 1) * sizeof( wchar_t );
    auto OutputSize = (wcslen( OutputFile ) + 1) * sizeof( wchar_t );

    // Allocate memory for the session properties. The memory must
    // be large enough to include the log file name and session name,
    // which get appended to the end of the session properties structure.

    BufferSize = ( ULONG )(sizeof( EVENT_TRACE_PROPERTIES ) + NameSize + OutputSize);
    pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc( BufferSize );
    if ( NULL == pSessionProperties )
    {
        hResult = E_OUTOFMEMORY;
        goto cleanup;
    }

    // Set the session properties. You only append the log file name
    // to the properties structure; the StartTrace function appends
    // the session name for you.

    ZeroMemory( pSessionProperties, BufferSize );
    pSessionProperties->Wnode.BufferSize = BufferSize;
    pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
    pSessionProperties->Wnode.Guid = SessionGuid;
    pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
    pSessionProperties->MaximumFileSize = 0;  // 100 MB
    pSessionProperties->LoggerNameOffset = sizeof( EVENT_TRACE_PROPERTIES );
    pSessionProperties->LogFileNameOffset = ( ULONG )(sizeof( EVENT_TRACE_PROPERTIES ) + NameSize);
    StringCbCopy( (LPWSTR)((char*)pSessionProperties + pSessionProperties->LogFileNameOffset), OutputSize, OutputFile );

    // Create the trace session.

    status = StartTrace( (PTRACEHANDLE)&SessionHandle, SessionName, pSessionProperties );
    if ( ERROR_SUCCESS != status )
    {
        hResult = HRESULT_FROM_WIN32( status );
        goto cleanup;
    }

    // Enable the providers that you want to log events to your session.
    for ( UINT i = 0; i < NumOfProviders; i++ )
    {
        status = EnableTraceEx2(
            SessionHandle,
            (LPCGUID)&ProviderGuids[i],
            EVENT_CONTROL_CODE_ENABLE_PROVIDER,
            TRACE_LEVEL_INFORMATION,
            0,
            0,
            0,
            NULL
        );

        if ( ERROR_SUCCESS != status )
        {
            hResult = HRESULT_FROM_WIN32( status );
            goto cleanup;
        }
    }

    SessionOutHandle = (HANDLE)SessionHandle;
    hResult = CreateSystemLogSession( OutputFile );
    if ( hResult != S_OK )
    {
        goto cleanup;
    }

    if ( pSessionProperties )
    {
        free( pSessionProperties );
        pSessionProperties = NULL;
    }

    return S_OK;
cleanup:

    DestroyLoggingSession( (HANDLE)SessionHandle, SessionName, OutputFile, SessionGuid, ProviderGuids, NumOfProviders );
    return hResult;
}

