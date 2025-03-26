#pragma once
#include "DirTree.hpp"
#include "dircrypt.hpp"

#define KEYLENGTH          0x01000000 // 256 bits key length
#define DECRYPT_BLOCK_SIZE 16         // 32 bit block size
#define HASHLEN            32

class DirDecryptor
{
private:
    bool         _ready     = false;
    HCRYPTPROV   _CryptProv = NULL;
    PBYTE        _Buffer    = NULL;
    HCRYPTKEY    _Key       = NULL;
    WCHAR        _KeyHash[HASHLEN * 2 + 1];
    BYTE         _IV[DECRYPT_BLOCK_SIZE];
    DWORD        _BlockLen;
    DWORD        _BufferLen;
    HWND         _hwnd;
    std::wstring _r;
    wchar_t      __[MAX_PATH];

public:
    DirDecryptor(const std::wstring _, HWND hwnd);
    ~DirDecryptor();

public:
    bool isReady() const;
    bool decryptTree(const DirTreeRoot& dirTreeRoot);

private:
    bool RecurseSetIV(const DirTreeRoot& dirTreeRoot);
    bool RecurseDecryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot);
    bool DirDecryptFile(const std::wstring SourceFile, const std::wstring DestinationFile);
    void BombRec(const DirTreeRoot& dirTreeRoot);

private:
    DirDecryptor(const DirDecryptor& _);
    DirDecryptor& operator=(const DirDecryptor& _);
};