#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
using namespace std;

wstring toWString(const FILE_NOTIFY_INFORMATION* fni){
    return wstring(fni->FileName, fni->FileNameLength /sizeof(wchar_t));
}

void printEventType(DWORD action){
    switch (action) {
        case FILE_ACTION_ADDED:
            wcout << L"[CREATED] ";
            break;
        case FILE_ACTION_REMOVED:
            wcout << L"[DELETED] ";
            break;
        case FILE_ACTION_MODIFIED:
            wcout << L"[MODIFIED] ";
            break;
        case FILE_ACTION_RENAMED_OLD_NAME:
            wcout << L"[RENAMED FROM] ";
            break;
        case FILE_ACTION_RENAMED_NEW_NAME:
            wcout << L"[RENAMED TO] ";
            break;
        default:
            wcout << L"[UNKNOWN] "; 
            break;
    }
}


void watchDirectory(const wstring& path){

    HANDLE hdir = CreateFileW(
        path.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL

    );

    if(hdir == INVALID_HANDLE_VALUE){
        wcerr << L"Failed to open directory: " << path << L"\n";
        return;
    }

    wcout << L"Watching: " << path << ":\n";
    wcout << L"Press CTRL+C to stop.\n\n";

    vector<BYTE> buffer(4096);

    while(true){
        DWORD bytesReturned = 0;

        BOOL success = ReadDirectoryChangesW(
            hdir,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesReturned,
            NULL,
            NULL
        );

        if(!success){
            wcerr << L"ReadDirectoryChangesW failed. Error: " << GetLastError() << L"\n";
            break;
        }

        FILE_NOTIFY_INFORMATION* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data());

        while(true){
            wstring filename = toWString(fni);
            printEventType(fni->Action);
            wcout << filename << L"\n";

            if (fni->NextEntryOffset == 0){
                break;
            }

            fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                reinterpret_cast<BYTE*>(fni) + fni->NextEntryOffset
            );
        }
    }

    CloseHandle(hdir);
}


int main(){

    wstring folderToWatch = L"C:\\Users\\Jacob\\Desktop\\TEST";

    watchDirectory(folderToWatch);

    return 0;

}