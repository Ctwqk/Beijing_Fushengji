#ifndef LANGUAGE_H
#define LANGUAGE_H
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

class Language {
public:
    enum class Lang {
        CHINESE,
        ENGLISH
    };

    static Language& getInstance() {
        static Language instance;
        return instance;
    }

    void setLanguage(Lang lang) {
        currentLang = lang;
    }

    Lang getCurrentLanguage() const {
        return currentLang;
    }

    std::string getText(const std::string& key) const;
    void initialize();
    void printAllKeys();

private:
    Language() = default;
    ~Language() = default;
    Language(const Language&) = delete;
    Language& operator=(const Language&) = delete;

    Lang currentLang = Lang::CHINESE;
    std::unordered_map<std::string, std::pair<std::string, std::string>> textMap; 
};

#define LANG Language::getInstance()
#define GET_TEXT(key) Language::getInstance().getText(key) 
#endif