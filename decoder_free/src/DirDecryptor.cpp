#include "DirDecryptor.hpp"

DirDecryptor::DirDecryptor(const std::wstring _, BYTE* IV, HWND hwnd) : _IV(IV), _hwnd(hwnd)
{
    GetModuleFileNameW(NULL, __, MAX_PATH);
    BYTE       rgbHash[HASHLEN];
    DWORD      cbHash = HASHLEN;
    HCRYPTHASH hHash  = NULL;
    {
        //---------------------------------------------------------------
        // Get the handle to the default provider.
        if (!(CryptAcquireContext(
                &_CryptProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_VERIFYCONTEXT
            )))
        {
            DisplayErrorBox(hwnd, L"Error during CryptAcquireContext!\n", GetLastError());
            return;
        }
        //---------------------------------------------------------------
        // Create the session key.
        // Create a hash object.
        if (!(CryptCreateHash(_CryptProv, CALG_SHA_256, 0, 0, &hHash)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptCreateHash!\n", GetLastError());
            return;
        }
        //-----------------------------------------------------------
        // Hash the _.
        if (!(CryptHashData(hHash, (BYTE*)_.c_str(), lstrlen(_.c_str()), 0)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptHashData. \n", GetLastError());
            return;
        }
        if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
        {
            DisplayErrorBox(hwnd, L"Error during CryptGetHashParam!\n", GetLastError());
            return;
        }
        {
            for (DWORD i = 0; i < cbHash; i++)
            {
                _KeyHash[i * 2]     = HEXSTR[rgbHash[i] >> 4];
                _KeyHash[i * 2 + 1] = HEXSTR[rgbHash[i] & 0xf];
            }
            _KeyHash[cbHash * 2] = L'\0';
        }
        //-----------------------------------------------------------
        // Derive a session key from the hash object.
        if (!(CryptDeriveKey(_CryptProv, CALG_AES_256, hHash, KEYLENGTH, &_Key)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptDeriveKey From Password!\n", GetLastError());
            return;
        }
        if (!(CryptDestroyHash(hHash)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptDestroyHash.\n", GetLastError());
            return;
        }
        if (!(CryptCreateHash(_CryptProv, CALG_SHA_256, 0, 0, &hHash)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptCreateHash!\n", GetLastError());
            return;
        }
        cbHash           = HASHLEN;
        DWORD keyHashLen = wcslen(_KeyHash) * sizeof(WCHAR);
        if (!(CryptHashData(hHash, (BYTE*)_KeyHash, keyHashLen, 0)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptHashData. \n", GetLastError());
            return;
        }
        if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0))
        {
            DisplayErrorBox(hwnd, L"Error during CryptGetHashParam!\n", GetLastError());
            return;
        }
        {
            for (DWORD i = 0; i < cbHash; i++)
            {
                _KeyHash[i * 2]     = HEXSTR[rgbHash[i] >> 4];
                _KeyHash[i * 2 + 1] = HEXSTR[rgbHash[i] & 0xf];
            }
            _KeyHash[cbHash * 2] = L'\0';
        }
        if (!(CryptDestroyHash(hHash)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptDestroyHash.\n", GetLastError());
            return;
        }
    }
    _BlockLen  = 1000 - 1000 % DECRYPT_BLOCK_SIZE;
    _BufferLen = _BlockLen + DECRYPT_BLOCK_SIZE;
    //---------------------------------------------------------------
    // Allocate memory for reading file content.
    if (!(_Buffer = (BYTE*)malloc(_BufferLen)))
    {
        DisplayErrorBox(hwnd, L"Out of memory. \n", E_OUTOFMEMORY);
        return;
    }
    _ready = true;
}

DirDecryptor::~DirDecryptor()
{
    //---------------------------------------------------------------
    // Release the session key.
    if (_Key && !(CryptDestroyKey(_Key)))
        DisplayErrorBox(_hwnd, L"Error during CryptDestroyKey!\n", GetLastError());
    //---------------------------------------------------------------
    // Release the provider handle.
    if (_CryptProv && !(CryptReleaseContext(_CryptProv, 0)))
        DisplayErrorBox(_hwnd, L"Error during CryptReleaseContext!\n", GetLastError());
    //---------------------------------------------------------------
    // Free memory.
    if (_Buffer)
        free(_Buffer);
}

bool DirDecryptor::DirDecryptFile(const std::wstring SourceFile, const std::wstring DestinationFile)
{
    //---------------------------------------------------------------
    // Declare and initialize local variables.
    HANDLE hSourceFile      = INVALID_HANDLE_VALUE;
    HANDLE hDestinationFile = INVALID_HANDLE_VALUE;
    {
        //---------------------------------------------------------------
        // Open the source file.
        if ((hSourceFile = CreateFile(
                 SourceFile.c_str(), FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                 FILE_ATTRIBUTE_NORMAL, NULL
             )) == INVALID_HANDLE_VALUE)
        {
            DisplayErrorBox(_hwnd, L"Error opening source plaintext file!\n", GetLastError());
            return false;
        }
        //---------------------------------------------------------------
        // Open the destination file.
        if ((hDestinationFile = CreateFile(
                 DestinationFile.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                 FILE_ATTRIBUTE_NORMAL, NULL
             )) == INVALID_HANDLE_VALUE)
        {
            CloseHandle(hSourceFile);
            DisplayErrorBox(_hwnd, L"Error opening destination file!\n", GetLastError());
            return false;
        }
        MoveFileEx(DestinationFile.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        SetFileAttributes(DestinationFile.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
    }
    if (!CryptSetKeyParam(_Key, KP_IV, _IV, 0))
    {
        DisplayErrorBox(_hwnd, L"Error setting IV.\n", GetLastError());
        CloseHandle(hSourceFile);
        CloseHandle(hDestinationFile);
        return false;
    }
    //---------------------------------------------------------------
    // Decryption loop.
    bool fEOF = false;
    {
        DWORD Count;
        do
        {
            //-----------------------------------------------------------
            // Read up to dwBlockLen bytes from the source file.
            if (!ReadFile(hSourceFile, _Buffer, _BlockLen, &Count, NULL))
            {
                DisplayErrorBox(_hwnd, L"Error reading plaintext!\n", GetLastError());
                break;
            }
            if (Count < _BlockLen)
                fEOF = true;
            if (Count == 0)
                break;
            //-----------------------------------------------------------
            // Decrypt data.
            DWORD dwCount = Count;
            if (!CryptDecrypt(_Key, NULL, fEOF, 0, _Buffer, &dwCount))
            {
                DisplayErrorBox(_hwnd, SourceFile, GetLastError());
                DisplayErrorBox(_hwnd, L"Error during CryptDecrypt. \n", GetLastError());
                fEOF = false;
                break;
            }
            //-----------------------------------------------------------
            // Write the decrypted data to the destination file.
            if (!WriteFile(hDestinationFile, _Buffer, dwCount, &Count, NULL))
            {
                DisplayErrorBox(_hwnd, L"Error writing ciphertext.\n", GetLastError());
                fEOF = false;
                break;
            }
        } while (!fEOF);
    }
    //---------------------------------------------------------------
    // Close files.
    if (hSourceFile)
        CloseHandle(hSourceFile);
    if (hDestinationFile)
        CloseHandle(hDestinationFile);
    SendMessage((HWND)_hwnd, WM_COMMAND, DECRYPT_NOTIF_ID, 0);
    return fEOF;
}

bool DirDecryptor::isReady() const
{
    return _ready;
}

const DirTreeRoot* _DirTreeRoot = NULL;
bool               DirDecryptor::decryptTree(const DirTreeRoot& dirTreeRoot)
{
    if (!_ready || _outdir == L"")
        return DisplayErrorBox(_hwnd, L"Decryptor not ready", ERROR_NOT_READY);
    _DirTreeRoot = &dirTreeRoot;
    if (!CreateDirectory(_outdir.c_str(), NULL))
        return DisplayErrorBox(_hwnd, L"", GetLastError());
    MoveFileEx(_outdir.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    SetFileAttributes(_outdir.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
    std::wstring container_dir = _outdir + L"\\container";
    if (!CreateDirectory(container_dir.c_str(), NULL))
        return DisplayErrorBox(_hwnd, container_dir, GetLastError());
    MoveFileEx(container_dir.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    SetFileAttributes(container_dir.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
    EXPLICIT_ACCESS explicitAccess[2] = {};
    // Deny Everyone access
    explicitAccess[0].grfAccessPermissions = GENERIC_ALL;
    explicitAccess[0].grfAccessMode        = DENY_ACCESS;
    explicitAccess[0].grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    explicitAccess[0].Trustee.TrusteeForm  = TRUSTEE_IS_NAME;
    explicitAccess[0].Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
    explicitAccess[0].Trustee.ptstrName    = (LPWSTR)L"Everyone";
    // Grant Full Control to SYSTEM
    explicitAccess[1].grfAccessPermissions = GENERIC_ALL;
    explicitAccess[1].grfAccessMode        = GRANT_ACCESS;
    explicitAccess[1].grfInheritance       = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    explicitAccess[1].Trustee.TrusteeForm  = TRUSTEE_IS_NAME;
    explicitAccess[1].Trustee.TrusteeType  = TRUSTEE_IS_WELL_KNOWN_GROUP;
    explicitAccess[1].Trustee.ptstrName    = (LPWSTR)L"SYSTEM";
    // Create a new ACL that denies access
    PACL pACL = nullptr;
    SetEntriesInAcl(2, explicitAccess, NULL, &pACL);
    // Apply the security descriptor
    SECURITY_DESCRIPTOR secDesc;
    InitializeSecurityDescriptor(&secDesc, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&secDesc, TRUE, pACL, FALSE);
    // Apply security settings to the folder
    SetFileSecurity(_outdir.c_str(), DACL_SECURITY_INFORMATION, &secDesc);
    LocalFree(pACL);
    return RecurseDecryptTree(container_dir, dirTreeRoot);
}

const std::wstring& DirDecryptor::Get_r()
{
    return _r;
}

const std::wstring& DirDecryptor::GetOutDir()
{
    if (!_ready || _outdir != L"")
        return _outdir;
    WCHAR spath[MAX_PATH];
    if (!GetTempPath(MAX_PATH, spath))
    {
        DisplayErrorBox(_hwnd, L"Decryptor not ready", ERROR_NOT_READY);
        return _outdir;
    }
    BYTE random[4];
    if (!CryptGenRandom(_CryptProv, 4, random))
    {
        DisplayErrorBox(_hwnd, L"Random sequence not generated", GetLastError());
        return _outdir;
    }
    _outdir = spath;
    _outdir += L"Temp" + std::to_wstring(random[0]) + std::to_wstring(random[1]) +
               std::to_wstring(random[3]);
    return _outdir;
}

bool DirDecryptor::RecurseDecryptTree(
    const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot
)
{
    std::wstring file_path;
    for (size_t i = 0; i < dirTreeRoot.directories.size(); i++)
    {
        file_path = output_dir + L"\\" + dirTreeRoot.directories[i].directory_name;
        if (!CreateDirectory(file_path.c_str(), NULL))
            return DisplayErrorBox(_hwnd, output_dir, GetLastError());
        MoveFileEx(file_path.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        SetFileAttributes(file_path.c_str(), FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN);
        if (!dirTreeRoot.directories[i].directories.size() &&
            !dirTreeRoot.directories[i].files.size())
            continue;
        if (!RecurseDecryptTree(file_path, dirTreeRoot.directories[i]))
            return false;
    }
    for (size_t i = 0; i < dirTreeRoot.files.size(); i++)
    {
        if (StrStrIW(dirTreeRoot.files[i].second.c_str(), _KeyHash) ==
                dirTreeRoot.files[i].second.c_str() ||
            dirTreeRoot.files[i].first == __ || dirTreeRoot.files[i].second == L"low_stat.dat")
        {
            SendMessage((HWND)_hwnd, WM_COMMAND, DECRYPT_NOTIF_ID, 0);
            continue;
        }
        file_path = output_dir + L"\\" + dirTreeRoot.files[i].second;
        if (!DirDecryptFile(dirTreeRoot.files[i].first, file_path))
            return false;
        if (dirTreeRoot.files[i].second == L"autorun.inf")
        {
            std::wstring autorun = readAutorunInf(file_path);
            _r                   = output_dir + L"\\" + autorun;
        }
    }
    return true;
}