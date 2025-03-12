#ifndef AIAPI_H
#define AIAPI_H

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <fstream>
#include "json.hpp"
#include <filesystem>

using json = nlohmann::json;

namespace fs = std::filesystem;

class AiApi{
    private:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
        static std::string ReadApiKey(fs::path &api_path);
        // std::string api_key;
        fs::path api_path;
        json requestBody;
        std::string jsonData;
        CURL* curl;
        std::string responseBuffer;
        std::string modelName;
        std::string systemPrompt;
        std::string requestUrl;
        struct curl_slist* headers;
        // std::string generateQuestion();
        // std::vector<std::string> generateSelections();
        
    public:
        AiApi(fs::path &ap, std::string requestUrl, std::string modelName, std::string systemPrompt = "");
        ~AiApi();
        std::string run(std::string userInput);
        void clearHistory();
        void setSystemPrompt(std::string sp);
        std::vector<std::vector<std::string>> generateRandomEvent( int cash, int health,int reputation, const std::string &lifeStory); // [question],[selection1, dCash, dHealth, dDebt], [selection1, dCash, dHealth, dDebt],[selection1, dCash, dHealth, dDebt]
        
};




#endif