#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <regex>
#include <vector>

class VDFParser {
private:
    std::unordered_map<std::string, std::string> tokens_;
    std::vector<std::string> keys_;
    std::vector<std::string> values_;
    std::string first_line;
    std::string language_line;
    
public:

    bool modified = false;
    bool parse(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        // Регулярка ищет "ключ" и "значение" в кавычках
        //R"(\"(.+?)\"\s*\"(.+?)\")"
        std::regex entry_regex(R"raw("([^"]+)"\s+"(.*)")raw");
        std::smatch match;
        std::string line;

        int it = 0; 
        int original_size = -1;

        std::getline(file, line);
        first_line = line;
        std::getline(file, line);
        std::getline(file, line);
        if (line.find("/") != std::string::npos){
            original_size = std::stoi(line.substr(line.find_last_of("/")+1, line.size()-1));
            modified = true;
            std::getline(file, line);
        }
        language_line = line;

        while (std::getline(file, line)) {
            // Убираем комментарии //
            size_t comment_pos = line.find("//");
            if (comment_pos != std::string::npos) line.erase(comment_pos);

            // Ищем пару ключ-значение
            if (std::regex_search(line, match, entry_regex)) {
                tokens_[match[1]] = match[2];
                keys_.push_back(match[1]);
                values_.push_back(match[2]);
            }
            
            if (original_size>0 && it > original_size){
                break;
            }
            it++;
            
        }
        return true;
    }

    std::string& operator[](const std::string& key) {
        return tokens_[key];
    }

    void erase(const std::string& key) {
        auto it = std::find(keys_.begin(), keys_.end(), key);
        keys_.erase(it);
        auto it1 = std::find(values_.begin(), values_.end(), tokens_[key]);
        values_.erase(it1);
        tokens_.erase(key);
    }

    const std::vector<std::string>& keys() const{
        return keys_;
    }

    const std::vector<std::string>& values() const{
        return values_;
    }

    const std::unordered_map<std::string, std::string>& items() const{
        return tokens_;
    }

    bool merge_and_write(VDFParser& second, const std::string& filename){
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << first_line << "\n";
        file << "{\n";
        file << "//" + std::to_string(tokens_.size()) + "\n";
        file << language_line << "\n";
        file << "\t\"Tokens\"\n";
        file << "\t{\n";
        for (std::string i: keys_){
            file << "\t\t\"";
            file << i << "\"\t\t\"";
            file << tokens_[i] << "\"\n";
        }
        file << std::endl;
        for (std::string i: second.keys_){
            file << "\t\t\"";
            file << i << "\"\t\t\"";
            file << second[i] << "\"\n";
        }
        file << "\t}\n";
        file << "}";
        file.close();
        return true;
    }

    bool write(const std::string& filename){
        std::ofstream file(filename);
        if (!file.is_open()) return false;

        file << first_line << "\n";
        file << "{\n";
        file << "//0\n";
        file << language_line << "\n";
        file << "\t\"Tokens\"\n";
        file << "\t{\n";
        for (std::string i: keys_){
            file << "\t\t\"";
            file << i << "\"\t\t\"";
            file << tokens_[i] << "\"\n";
        }
        file << "\t}\n";
        file << "}";
        file.close();
        return true;
    }
};

// int main() {
    // setlocale(LC_ALL, "Russian");
    // VDFParser a;
    // a.parse("D:/SteamLibrary/steamapps/common/Deadlock/game/citadel/resource/localization/citadel_attributes/citadel_attributes_russian.txt");
    // VDFParser b;
    // b.parse("citadel_attributes_english_old.txt");
    // b.merge_and_write(a, "test.txt");
    // VDFParser c;
    // c.parse("test.txt");
    // c.write("test1.txt");
    
    // // std::cout << c.keys().size() << "\n" << c.keys()[c.keys().size()-1] << "\n" << c[c.keys()[c.keys().size()-1]];

    // return 0;
// }
