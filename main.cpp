#include <windows.h>
#include <iostream>
#include <string>
#include <clocale>
#include <fstream>
#include <list>
#include <unordered_map>
#include "resources.h"

using namespace std;

bool ExtractResourceToFile(int resourceID, const std::string& targetPath) {
    // 1. Поиск ресурса в EXE
    HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!hRes) {
        std::cerr << "Ошибка: Ресурс не найден." << std::endl;
        return false;
    }

    // 2. Загрузка данных в память
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return false;

    DWORD size = SizeofResource(NULL, hRes);
    const char* pData = (const char*)LockResource(hData);

    if (!pData) return false;

    // 3. Запись данных в физический файл (режим ios::binary важен, чтобы не исказить данные)
    std::ofstream outFile(targetPath, std::ios::binary | std::ios::trunc);
    if (!outFile) {
        std::cerr << "Ошибка: Не удалось открыть файл для записи: " << endl << targetPath << std::endl;
        return false;
    }

    outFile.write(pData, size);
    outFile.close();

    return true;
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

int main() {
    setlocale(LC_ALL, "Russian");

    string SteamPath = GetSteamPath();
    cout << "Путь к Steam в реестре:" << endl << SteamPath << endl;
    string LibrariesPath = "/steamapps/libraryfolders.vdf";
    string DeadlockFolder = "/steamapps/common/Deadlock";
    string DeadlockExePath = "/game/bin/win64/deadlock.exe";
    string DeadlockPath = "";
    string DeadlockLocalizationFolder = "/game/citadel/resource/localization";

    unordered_map<string, string> DeadlockLocalizationPaths;
    DeadlockLocalizationPaths["citadel_attributes"] = "/citadel_attributes/citadel_attributes_english.txt";
    DeadlockLocalizationPaths["citadel_mods"] = "/citadel_mods/citadel_mods_english.txt";
    DeadlockLocalizationPaths["citadel_heroes"] = "/citadel_heroes/citadel_heroes_english.txt";

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
        cout << endl << "Deadlock обнаружен в папке:" << endl << DeadlockPath << endl;
        break;
    }
    if (DeadlockPath == ""){
        cout << endl << "ERROR: Не удалось обнаружить Deadlock!" << endl;
        cout << endl;
        cout << "Программа завершена. Нажмите Enter, чтобы выйти...";
        cin.get();
        return 0;
    }

    DeadlockLocalizationFolder = DeadlockPath + DeadlockLocalizationFolder;
    string DeadlockLocalizationFilePath = DeadlockLocalizationFolder + DeadlockLocalizationPaths["citadel_mods"];
    if (ExtractResourceToFile(CitadelModsCustom, DeadlockLocalizationFilePath)) {
        std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
    }
    DeadlockLocalizationFilePath = DeadlockLocalizationFolder + DeadlockLocalizationPaths["citadel_heroes"];
    if (ExtractResourceToFile(CitadelHeroesCustom, DeadlockLocalizationFilePath)) {
        std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
    }
    DeadlockLocalizationFilePath = DeadlockLocalizationFolder + DeadlockLocalizationPaths["citadel_attributes"];
    if (ExtractResourceToFile(CitadelAttributesCustom, DeadlockLocalizationFilePath)) {
        std::cout << endl << "Файл " << DeadlockLocalizationFilePath << endl << "успешно модифицирован!" << std::endl;
    }
    cout << endl;
    cout << "Программа завершена. Нажмите Enter, чтобы выйти...";
    cin.get();
    return 0;
}
