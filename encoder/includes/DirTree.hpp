#pragma once
#include "dircrypt.hpp"
#include <string>
#include <vector>

struct DirTreeRoot
{
    std::wstring              directory_path;
    std::wstring              directory_name;
    std::vector<DirTreeRoot>  directories;
    std::vector<std::wstring> files;
};

class DirTree
{
private:
    HWND        _TreeViewUIHandler;
    DirTreeRoot _DirTreeRoot;

public:
    DirTree(const std::wstring root, HWND TreeViewUIHandler);
    DirTree(DirTreeRoot _DirTreeRoot, HWND TreeViewUIHandler);
    ~DirTree();

public:
    const DirTreeRoot& getTreeRoot() const;
private:
    DirTree(const DirTree& _);
    DirTree& operator=(const DirTree& _);
    bool      RecurseDirSearch(const std::wstring& root, DirTreeRoot& dirTreeRoot);
    void      PopulateTreeView(const DirTreeRoot& dirTreeRoot, HTREEITEM parent);
    HTREEITEM InsertToTreeView(const std::wstring& text, HTREEITEM parent, HTREEITEM prev);
};