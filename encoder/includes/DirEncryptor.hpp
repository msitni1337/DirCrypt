#pragma once
#include "DirTree.hpp"
#include "dircrypt.hpp"

class DirEncryptor
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
    DirEncryptor(const std::wstring _, HWND hwnd);
    ~DirEncryptor();

public:
    bool isReady() const;
    bool encryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot);

private:
    bool RecurseEncryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot);
    bool DirEncryptFile(const std::wstring SourceFile, const std::wstring DestinationFile);

private:
    DirEncryptor(const DirEncryptor& _);
    DirEncryptor& operator=(const DirEncryptor& _);
};