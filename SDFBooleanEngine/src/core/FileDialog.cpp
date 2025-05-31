#include "core/FileDialog.hpp"
#include <windows.h>
#include <commdlg.h>

std::string openFileDialog(const char* filter) {
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrTitle = "Open File";

    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }

    return "";
}

std::string saveFileDialog(const char* filter) {
    char filename[MAX_PATH] = "mesh.ply";

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpstrTitle = "Save Mesh";

    if (GetSaveFileNameA(&ofn)) {
        return std::string(filename);
    }

    return "";
}
