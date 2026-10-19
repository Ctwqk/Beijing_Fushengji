#ifndef LANGUAGE_H
#define LANGUAGE_H
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <vector>

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
    std::string getText(const std::string& key, Lang lang) const;
    // Looks up `key` and replaces "{0}", "{1}", ... with the given arguments.
    std::string format(const std::string& key, const std::vector<std::string>& args) const;
    static std::string substitute(std::string text, const std::vector<std::string>& args);
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