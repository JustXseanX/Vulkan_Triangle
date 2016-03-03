//-------------------------------------------------------------------------------------------------
// File : asvkMisc.cpp
// Desc : Utility Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <asvkMisc.h>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <shlwapi.h>
#include <locale>
#include <codecvt>
#include <sstream>


//-------------------------------------------------------------------------------------------------
// Linker
//-------------------------------------------------------------------------------------------------
#pragma comment( lib, "shlwapi.lib" )


namespace asvk {

//-------------------------------------------------------------------------------------------------
//      ファイルパスを検索します.
//-------------------------------------------------------------------------------------------------
bool SearchFilePath( const wchar_t* filePath, std::wstring& result )
{
    if ( filePath == nullptr )
    { return false; }

    if ( wcscmp( filePath, L" " ) == 0 || wcscmp( filePath, L"" ) == 0 )
    { return false; }

    wchar_t exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.
    PathRemoveFileSpecW( exePath );

    wchar_t dstPath[ 520 ] = { 0 };

    wcscpy_s( dstPath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"..\\..\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"\\res\\%s", filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\..\\..\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    swprintf_s( dstPath, L"%s\\res\\%s", exePath, filePath );
    if ( PathFileExistsW( dstPath ) == TRUE )
    {
        result = dstPath;
        return true;
    }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスからディレクトリ名を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetDirectoryPath( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"/" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    idx = path.find_last_of( L"\\" );
    if ( idx != std::wstring::npos )
    {
        auto result = path.substr( 0, idx + 1 );
        return result;
    }

    return std::wstring();
}

//-------------------------------------------------------------------------------------------------
//      実行ファイルのファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExePath()
{
    wchar_t exePath[ 520 ] = { 0 };
    GetModuleFileNameW( nullptr, exePath, 520  );
    exePath[ 519 ] = 0; // null終端化.

    return asvk::GetDirectoryPath( exePath );
}

//-------------------------------------------------------------------------------------------------
//      ファイルパスから拡張子を取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetExt( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"." );
    if ( idx != std::wstring::npos )
    {
        std::wstring result = path.substr( idx + 1 );

        // 小文字化.
        std::transform( result.begin(), result.end(), result.begin(), tolower );

        return result;
    }

    return std::wstring();
}

//-------------------------------------------------------------------------------------------------
//      拡張子を取り除いたファイルパスを取得します.
//-------------------------------------------------------------------------------------------------
std::wstring GetPathWithoutExt( const wchar_t* filePath )
{
    std::wstring path = filePath;
    auto idx = path.find_last_of( L"." );
    if ( idx != std::wstring::npos )
    {
        return path.substr( 0, idx );
    }

    return path;
}

//-------------------------------------------------------------------------------------------------
//      指定されたファイルパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFilePath( const wchar_t* filePath )
{
    if ( PathFileExistsW( filePath ) == TRUE )
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      指定されたフォルダパスが存在するかチェックします.
//-------------------------------------------------------------------------------------------------
bool IsExistFolderPath( const wchar_t* folderPath )
{
    if ( PathFileExistsW ( folderPath ) == TRUE
      && PathIsDirectoryW( folderPath ) != FALSE ) // PathIsDirectoryW() は TRUE を返却しないので注意!!
    { return true; }

    return false;
}

//-------------------------------------------------------------------------------------------------
//      ワイド文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::wstring ToStringW( const std::string& value )
{
    auto length = MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0 );
    auto buffer = new wchar_t[length];

    MultiByteToWideChar(CP_ACP, 0, value.c_str(), int(value.size() + 1),  buffer, length );

    std::wstring result( buffer );
    delete[] buffer;

    return result;
}

//-------------------------------------------------------------------------------------------------
//      マルチバイト文字列に変換します.
//-------------------------------------------------------------------------------------------------
std::string ToStringA( const std::wstring& value )
{
    auto length = WideCharToMultiByte(CP_ACP, 0, value.c_str(), int(value.size() + 1), nullptr, 0, nullptr, nullptr); 
    auto buffer = new char[length];
 
    WideCharToMultiByte(CP_ACP, 0, value.c_str(), int(value.size() + 1), buffer, length, nullptr, nullptr);

    std::string result(buffer);
    delete[] buffer;

    return result;
}

//-------------------------------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-------------------------------------------------------------------------------------------------
std::vector<std::string> Split(const std::string& input, char delimiter)
{
    std::istringstream stream(input);

    std::string field;
    std::vector<std::string> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}

//-------------------------------------------------------------------------------------------------
//      指定文字で文字列を分割します.
//-------------------------------------------------------------------------------------------------
std::vector<std::wstring> Split(const std::wstring& input, wchar_t delimiter)
{
    std::wistringstream stream(input);

    std::wstring field;
    std::vector<std::wstring> result;
    while (std::getline(stream, field, delimiter))
    { result.push_back(field); }
    return result;
}

} // namespace asvk
