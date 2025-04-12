#pragma once
#include "dircrypt.hpp"
#include <string>
#include <vector>

struct DirTreeRoot
{
    size_t                                              files_count;
    std::wstring                                        directory_path;
    std::wstring                                        directory_name;
    std::vector<DirTreeRoot>                            directories;
    std::vector<std::pair<std::wstring, std::wstring> > files;
};

class DirTree
{
private:
    HWND         _hwnd;
    DirTreeRoot  _DirTreeRoot;
    std::wstring _L;

public:
    DirTree(const std::wstring root, HWND _hwnd);
    ~DirTree();

public:
    const DirTreeRoot&  getTreeRoot() const;
    const std::wstring& getLPath() const;

private:
    DirTree(const DirTree& _);
    DirTree& operator=(const DirTree& _);
    bool     RecurseDirSearch(const std::wstring& root, DirTreeRoot& dirTreeRoot);
};