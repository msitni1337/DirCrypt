#pragma once
#include "DirTree.hpp"
#include "dircrypt.hpp"

#define KEYLENGTH          0x01000000 // 256 bits key length
#define ENCRYPT_BLOCK_SIZE 16         // 32 bit block size
#define HASHLEN            32
class DirEncryptor
{
private:
    bool         _ready     = false;
    HCRYPTPROV   _CryptProv = NULL;
    PBYTE        _Buffer    = NULL;
    HCRYPTKEY    _Key       = NULL;
    WCHAR        _KeyHash[HASHLEN * 2 + 1];
    DWORD        _BlockLen;
    DWORD        _BufferLen;
    HWND         _hwnd;
    std::wstring __;
    UINT         _credit;

public:
    DirEncryptor(const std::wstring _, const std::wstring __, UINT credit, HWND hwnd);
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