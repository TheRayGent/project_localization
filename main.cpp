#include <windows.h>
#include <iostream>
#include <string>
#include <clocale>
#include <fstream>
#include <list>
#include <unordered_map>
#include "parser.hpp"

using namespace std;

// bool ExtractResourceToFile(int resourceID, const std::string& targetPath) {
//     // 1. Поиск ресурса в EXE
//     HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), RT_RCDATA);
//     if (!hRes) {
//         std::cerr << "Ошибка: Ресурс не найден." << std::endl;
//         return false;
//     }

//     // 2. Загрузка данных в память
//     HGLOBAL hData = LoadResource(NULL, hRes);
//     if (!hData) return false;

//     DWORD size = SizeofResource(NULL, hRes);
//     const char* pData = (const char*)LockResource(hData);

//     if (!pData) return false;

//     // 3. Запись данных в физический файл (режим ios::binary важен, чтобы не исказить данные)
//     std::ofstream outFile(targetPath, std::ios::binary | std::ios::trunc);
//     if (!outFile) {
//         std::cerr << "Ошибка: Не удалось открыть файл для записи: " << endl << targetPath << std::endl;
//         return false;
//     }

//     outFile.write(pData, size);
//     outFile.close();

//     return true;
// }

void clear() {
    COORD topLeft  = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

std::string GetSteamPath() {
    HKEY hKey;
    char path[MAX_PATH];
    DWORD size = sizeof(path);
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "SteamPath", NULL, NULL, (LPBYTE)path, &size);
        RegCloseKey(hKey);
        return std::string(path);
    }
    return "";
}

void custom_replace(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty()) return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

void custom_strip(std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    str.erase(str.find_last_not_of(whitespace) + 1);
    str.erase(0, str.find_first_not_of(whitespace));
}

bool filter_attributes(string str) {
    if (str.find("modifier") != string::npos || str.find("Citadel_StatusEffect") != string::npos || str.find("MODIFIER_STATE") != string::npos || str.find("movement_slow") != string::npos){
        return true;
    }
    return false;
}

bool filter_heroes(string str) {
    if (str.find("desc") != string::npos){
        return false;
    }
    return true;
}

bool CreateBackupFile(string& file){
    ifstream in_file(file);
    ofstream out_file(file + "_backup");
    if (in_file && out_file){
        out_file << in_file.rdbuf();
        in_file.close();
        out_file.close();
        return true;
    }
    return false;
}

bool GetDeadlockPath(string& DeadlockPath_){
    string SteamPath = GetSteamPath();
    cout << "Путь к Steam в реестре:" << endl << SteamPath << endl;
    string LibrariesPath = "/steamapps/libraryfolders.vdf";
    string DeadlockFolder = "/steamapps/common/Deadlock";
    string DeadlockExePath = "/game/bin/win64/deadlock.exe";
    string DeadlockPath = "";

    list<string> SteamAppsPaths;

    ifstream file(SteamPath + LibrariesPath);
    if (file.is_open()){
        string line;
        while(getline(file, line)){
            if (line.find("\"path\"") != std::string::npos){
                custom_replace(line, "\"path\"", "");
                custom_replace(line, "\\", "/");
                custom_replace(line, "//", "/");
                custom_replace(line, "\"", "");
                custom_strip(line);
                SteamAppsPaths.push_back(line);
            }
        }
    }
    file.close();
    cout << endl << "Папки с играми Steam:" << endl;
    for (string i: SteamAppsPaths){
        cout << i << endl;
    }

    for (string i: SteamAppsPaths){
        ifstream file(i + DeadlockFolder + DeadlockExePath);
        if (!file.is_open()){
            file.close();
            continue;
        }
        file.close();
        DeadlockPath = i + DeadlockFolder;
        cout << endl;
        // cout << endl << "Deadlock обнаружен в папке:" << endl << DeadlockPath << endl;
        break;
    }
    if (DeadlockPath == ""){
        cout << endl << "ERROR: Не удалось обнаружить Deadlock!" << endl;
        return false;
    }
    DeadlockPath_ = DeadlockPath;
    return true;
}


bool EditLocalizationFile(string DeadlockLocalizationFolder, string category){
    string DeadlockRuFilePath = DeadlockLocalizationFolder + "/citadel_" + category + "/citadel_" + category + "_russian.txt";
    string DeadlockEnFilePath = DeadlockLocalizationFolder + "/citadel_" + category + "/citadel_" + category + "_english.txt";

    VDFParser en_file;
    en_file.parse(DeadlockEnFilePath);
    VDFParser ru_file;
    ru_file.parse(DeadlockRuFilePath);
    vector<string> ru_keys = ru_file.keys();

    if (!en_file.modified)
        CreateBackupFile(DeadlockEnFilePath);

    if (category == "attributes"){
        for (string i: ru_keys){
            if (filter_attributes(i)){
                ru_file.erase(i);
            }
        }
    }
    
    if (category == "heroes"){
        for (string i: ru_keys){
            if (filter_heroes(i)){
                ru_file.erase(i);
            }
        }
    }

    if (en_file.merge_and_write(ru_file, DeadlockEnFilePath)){
        std::cout << endl << "Файл " << DeadlockEnFilePath << endl << "успешно модифицирован!" << std::endl;
        return true;
    }

    // if (ExtractResourceToFile(CitadelModsCustom, DeadlockLocalizationFilePath)) {
    //     std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
    // }
    return false;
}

// void EditDeadlockLocalization_old(string DeadlockPath){
//     string DeadlockLocalizationFolder = "/game/citadel/resource/localization";
//     unordered_map<string, string> DeadlockLocalizationPaths;
//     DeadlockLocalizationPaths["attributes"] = "/citadel_attributes/citadel_attributes_english.txt";
//     DeadlockLocalizationPaths["mods"] = "/citadel_mods/citadel_mods_english.txt";
//     DeadlockLocalizationPaths["heroes"] = "/citadel_heroes/citadel_heroes_english.txt";

//     DeadlockLocalizationFolder = DeadlockPath + DeadlockLocalizationFolder;
//     string DeadlockLocalizationFilePath = DeadlockLocalizationFolder + DeadlockLocalizationPaths["mods"];
//     if (ExtractResourceToFile(CitadelModsCustom, DeadlockLocalizationFilePath)) {
//         std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
//     }
//     DeadlockLocalizationFilePath = DeadlockLocalizationFolder + DeadlockLocalizationPaths["heroes"];
//     if (ExtractResourceToFile(CitadelHeroesCustom, DeadlockLocalizationFilePath)) {
//         std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
//     }
//     DeadlockLocalizationFilePath = DeadlockLocalizationFolder + DeadlockLocalizationPaths["attributes"];
//     if (ExtractResourceToFile(CitadelAttributesCustom, DeadlockLocalizationFilePath)) {
//         std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
//     }
// }

bool BackupLocalizationFile(string DeadlockLocalizationFolder, string category){
    string DeadlockEnFilePath = DeadlockLocalizationFolder + "/citadel_" + category + "/citadel_" + category + "_english.txt";

    VDFParser en_file;
    en_file.parse(DeadlockEnFilePath);

    if (!en_file.modified){
        std::cout << endl << "Файл " << DeadlockEnFilePath << endl << "не был модифицирован. Backup не требуется!" << std::endl;
        return true;
    }

    if (en_file.write(DeadlockEnFilePath)){
        std::cout << endl << "В файл " << DeadlockEnFilePath << endl << "был успешно возвращён стандартный перевод!" << std::endl;
        return true;
    }

    return false;
}

void EditDeadlockLocalization(string DeadlockLocalizationFolder){
    EditLocalizationFile(DeadlockLocalizationFolder, "attributes");
    EditLocalizationFile(DeadlockLocalizationFolder, "heroes");
    EditLocalizationFile(DeadlockLocalizationFolder, "mods");
}

void BackupDeadlockLocalization(string DeadlockLocalizationFolder){
    BackupLocalizationFile(DeadlockLocalizationFolder, "attributes");
    BackupLocalizationFile(DeadlockLocalizationFolder, "heroes");
    BackupLocalizationFile(DeadlockLocalizationFolder, "mods");
}

inline void the_end_program(bool confirm = true){
    cout << endl;
    cout <<  "Программа завершена. Нажмите Enter, чтобы выйти...";
    if (confirm) cin.get();
    return;
}

void main() {
    setlocale(LC_ALL, "Russian");

    string DeadlockPath;

    if (!GetDeadlockPath(DeadlockPath)){
        the_end_program();
        return;
    }

    string DeadlockLocalizationFolder = "/game/citadel/resource/localization";
    DeadlockLocalizationFolder = DeadlockPath + DeadlockLocalizationFolder;

    // Sleep(1500);
    // clear();
    
    while(true){
        cout << "Deadlock обнаружен в папке:" << endl << DeadlockPath << endl;
        cout << endl << "project_localization v0.1" << endl;
        cout << "\t1. Установить кастомный перевод" << endl;
        cout << "\t2. Вернуть стандартный перевод" << endl;
        cout << "\t0. Выйти из программы" << endl;
        cout << "Выберите действие (0-2): ";

        string select;
        getline(cin, select);
        if (select.empty()){
            clear();
            continue;
        }

        switch(stoi(select))
        {
        case 0:
            return;
            break;
        case 1:
            clear();
            EditDeadlockLocalization(DeadlockLocalizationFolder);
            cout << endl << "!Файлы .txt_backup созданы в тех же папках для ручного возрата стандартного перевода!" << endl;
            cout << endl << "Нажмите Enter, чтобы продожить...";
            cin.get();
            clear();
            break;
        case 2:
            clear();
            BackupDeadlockLocalization(DeadlockLocalizationFolder);
            cout << endl << "Нажмите Enter, чтобы продожить...";
            cin.get();
            clear();
            break;
        default:
            clear();
            break;
        }

    }
    return;
}
