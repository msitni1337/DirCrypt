#include "DirEncryptor.hpp"

DirEncryptor::DirEncryptor(const std::wstring _, const std::wstring __, UINT credit, HWND hwnd)
    : _hwnd(hwnd), __(__), _credit(credit)
{
    HCRYPTHASH hHash = NULL;
    BYTE       rgbHash[HASHLEN];
    DWORD      cbHash = HASHLEN;
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
            _KeyHash[cbHash * 2] = 0;
        }
        //-----------------------------------------------------------
        // Derive a session key from the hash object.
        if (!(CryptDeriveKey(_CryptProv, CALG_AES_256, hHash, KEYLENGTH, &_Key)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptDeriveKey From Password!\n", GetLastError());
            return;
        }
        if (!CryptGenRandom(_CryptProv, ENCRYPT_BLOCK_SIZE, _IV))
        {
            DisplayErrorBox(hwnd, L"Error during CryptGenRandom\n", GetLastError());
            return;
        }
        if (!CryptSetKeyParam(_Key, KP_IV, _IV, 0))
        {
            DisplayErrorBox(_hwnd, L"Error resetting IV.\n", GetLastError());
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
            _KeyHash[cbHash * 2] = 0;
        }
        if (!(CryptDestroyHash(hHash)))
        {
            DisplayErrorBox(hwnd, L"Error during CryptDestroyHash.\n", GetLastError());
            return;
        }
    }
    _BlockLen  = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;
    _BufferLen = _BlockLen + ENCRYPT_BLOCK_SIZE;
    //---------------------------------------------------------------
    // Allocate memory for reading file content.
    if (!(_Buffer = (BYTE*)malloc(_BufferLen)))
    {
        DisplayErrorBox(hwnd, L"Out of memory. \n", E_OUTOFMEMORY);
        return;
    }
    _ready = true;
}

DirEncryptor::~DirEncryptor()
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

bool DirEncryptor::DirEncryptFile(const std::wstring SourceFile, const std::wstring DestinationFile)
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
    }
    //---------------------------------------------------------------
    // Encryption loop.
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
            // Encrypt data.
            DWORD dwCount = Count;
            if (!CryptEncrypt(_Key, NULL, fEOF, 0, _Buffer, &dwCount, _BufferLen))
            {
                DisplayErrorBox(_hwnd, L"Error during CryptEncrypt. \n", GetLastError());
                fEOF = false;
                break;
            }
            //-----------------------------------------------------------
            // Write the encrypted data to the destination file.
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
    SendMessage((HWND)_hwnd, WM_COMMAND, ENCRYPT_NOTIF_ID, 0);
    return fEOF;
}

bool DirEncryptor::isReady() const
{
    return _ready;
}

bool DirEncryptor::encryptTree(const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot)
{
    if (!_ready)
        return DisplayErrorBox(_hwnd, L"Encryptor not ready", ERROR_NOT_READY);
    DWORD dwAttrib = GetFileAttributes(output_dir.c_str());
    if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
        return DisplayErrorBox(_hwnd, L"Invalid Directory Path", GetLastError());
    if (!PathIsDirectoryEmpty(output_dir.c_str()))
        return DisplayErrorBox(_hwnd, L"Directory Not Empty", GetLastError());
    return RecurseEncryptTree(output_dir, dirTreeRoot);
}

bool DirEncryptor::RecurseEncryptTree(
    const std::wstring& output_dir, const DirTreeRoot& dirTreeRoot
)
{
    std::wstring file_path;
    if (dirTreeRoot.directory_path == __)
    {
        for (size_t i = 0; i < _credit; i++)
        {
            BYTE random[4];
            if (!CryptGenRandom(_CryptProv, 4, random))
                return DisplayErrorBox(_hwnd, L"Random sequence not generated", GetLastError());
            file_path = output_dir + L"\\" + _KeyHash + std::to_wstring(random[1]) +
                        std::to_wstring(random[2]);
            HANDLE hFile;
            if ((hFile = CreateFile(
                     file_path.c_str(), FILE_WRITE_DATA, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL, NULL
                 )) == INVALID_HANDLE_VALUE)
            {
                CloseHandle(hFile);
                DisplayErrorBox(_hwnd, L"Error opening destination file!\n", GetLastError());
                return false;
            }
            DWORD Count;
            if (!WriteFile(hFile, _IV, ENCRYPT_BLOCK_SIZE, &Count, NULL))
            {
                DisplayErrorBox(_hwnd, L"Error writing ciphertext.\n", GetLastError());
                CloseHandle(hFile);
                return false;
            }
            CloseHandle(hFile);
        }
    }
    for (size_t i = 0; i < dirTreeRoot.directories.size(); i++)
    {
        file_path = output_dir + L"\\" + dirTreeRoot.directories[i].directory_name;
        if (!CreateDirectory(file_path.c_str(), NULL))
            return DisplayErrorBox(_hwnd, output_dir, GetLastError());
        if (!RecurseEncryptTree(file_path, dirTreeRoot.directories[i]))
            return false;
    }
    for (size_t i = 0; i < dirTreeRoot.files.size(); i++)
    {
        file_path = dirTreeRoot.directory_path + L"\\" + dirTreeRoot.files[i];
        if (!DirEncryptFile(file_path, output_dir + L"\\" + dirTreeRoot.files[i]))
            return false;
    }
    return true;
}
