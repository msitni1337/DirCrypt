#pragma once
#include "DirTree.hpp"
#include "dircrypt.hpp"

class DirDecryptor
{
private:
    bool       _ready     = false;
    HCRYPTPROV _CryptProv = NULL;
    HCRYPTKEY  _Key       = NULL;
    PBYTE      _Buffer    = NULL;
    DWORD      _BlockLen;
    DWORD      _BufferLen;
    HWND       _hwnd;

public:
    DirDecryptor(const std::wstring _, HWND hwnd);
    ~DirDecryptor();

public:
    bool isReady() const;
    bool decryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot);

private:
    bool RecurseDecryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot);
    bool DirDecryptFile(const std::wstring SourceFile, const std::wstring DestinationFile);

private:
    DirDecryptor(const DirDecryptor& _);
    DirDecryptor& operator=(const DirDecryptor& _);
};