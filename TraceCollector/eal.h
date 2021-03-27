#pragma once

HRESULT DestroyLoggingSession( HANDLE SessionHandle, const wchar_t* SessionName, const wchar_t* OutputFile, const GUID& SessionGuid, const GUID* ProviderGuids, UINT NumOfProviders );
HRESULT CreateLoggingSession( const wchar_t* SessionName, const GUID& SessionGuid, const GUID* ProviderGuids, UINT NumOfProviders, const wchar_t* OutputFile, HANDLE& SessionOutHandle );
