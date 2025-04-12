#pragma once
#include "DirTree.hpp"
#include "dircrypt.hpp"

#define KEYLENGTH          0x01000000 // 256 bits key length
#define DECRYPT_BLOCK_SIZE 16         // 32 bit block size
#define HASHLEN            32

class DirDecryptor;

struct DecryptAsync
{
    DirDecryptor&       decryptor;
    const std::wstring& src;
    const std::wstring& dest;
    const std::wstring& out_dir;
    const std::wstring& fname;
};

class DirDecryptor
{
private:
    bool         _ready     = false;
    HCRYPTPROV   _CryptProv = NULL;
    PBYTE        _Buffer    = NULL;
    HCRYPTKEY    _Key       = NULL;
    WCHAR        _KeyHash[HASHLEN * 2 + 1];
    BYTE*        _IV;
    DWORD        _BlockLen;
    DWORD        _BufferLen;
    HWND         _hwnd;
    bool         _;
    std::wstring _r;
    std::wstring _outdir;
    wchar_t      __[MAX_PATH];

public:
    DirDecryptor(const std::wstring _, BYTE* IV, HWND hwnd);
    ~DirDecryptor();

public:
    bool                       isReady() const;
    bool                       decryptTree(const DirTreeRoot& dirTreeRoot);
    const std::wstring&        Get_r();
    const std::wstring&        GetOutDir();

private:
    bool RecurseDecryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot);
    bool DirDecryptFile(const std::wstring SourceFile, const std::wstring DestinationFile);

private:
    DirDecryptor(const DirDecryptor& _);
    DirDecryptor& operator=(const DirDecryptor& _);
};