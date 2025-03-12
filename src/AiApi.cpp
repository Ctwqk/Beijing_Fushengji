#include "AiApi.h"
#include "Events.h"
#include "util.h"
#include "Button.h"

AiApi::AiApi(fs::path &ap, std::string ru, std::string mn, std::string sp):api_path(ap), requestUrl(ru), modelName(mn), systemPrompt(sp){
    requestBody["model"] = mn;
    requestBody["messages"] = json::array();
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string api_key = ReadApiKey(api_path);
    std::string authorization = std::string("Authorization: Bearer ") + api_key;
    // std::cout<<authorization<<std::endl;
    headers = curl_slist_append(headers, authorization.c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  //for debug
}
AiApi::~AiApi(){
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
void AiApi::clearHistory(){
    requestBody["messages"] = json::array();
}
std::string AiApi::run(std::string userInput){
    json message;
    responseBuffer.clear();
    message["role"] = "user";
    message["content"] = userInput;
    requestBody["messages"].push_back(message);
    message["role"] = "system";
    message["content"] = systemPrompt;
    requestBody["messages"].push_back(message);
    jsonData = requestBody.dump();
    // std::cout<<jsonData<<std::endl;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.size());
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK){
        std::cerr<< "failed in curl_easy_perform" << " " << curl_easy_strerror(res) << std::endl;
    }
    else{
        try{
            auto jsonResponse = json::parse(responseBuffer);
            if(jsonResponse.contains("choices") && !jsonResponse["choices"].empty() && jsonResponse["choices"][0]["message"].contains("content")){
                std::string answer = jsonResponse["choices"][0]["message"]["content"];
                return answer;
            }
            else{
                std::cerr << "json response don't have expected structure \n" << jsonResponse.dump() << std::endl;
            }
        }
        catch(std::exception &e){
            std::cerr << "failed in parsing json" << std::endl;
        }
    }
    return "";
}

size_t AiApi::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp){
    size_t totalSize = size * nmemb;
    std::string* mem = (std::string*) userp;
    mem->append((char*) contents, totalSize);
    return totalSize;
}

std::string AiApi::ReadApiKey(fs::path &api_path){
    std::ifstream file(api_path);
    std::cout<<api_path<<std::endl;
    if(!file){
        std::cerr<<"api_key not exists" << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();
    // get rid off wrong syntex
    auto trimPos = fileContent.find_last_not_of(" \t\r\n");
    if (trimPos != std::string::npos) {
        fileContent.erase(trimPos + 1);
    }
    return fileContent;
}

void AiApi::setSystemPrompt(std::string sp){
    systemPrompt = sp;
}

// std::vector<std::vector<std::string>> AiApi::generateRandomEvent( int cash, int health, int debt, const std::string &lifeStory){
//     systemPrompt = 
//         "作为21世纪初北京生存事件生成器，请结合人物故事和当前状态（金钱，健康，债务）严格按以下规则生成：\n"
//         "1. 生成格式：事件标题[5字]|事件描述[30字]#选项1描述[10字]|金钱影响|健康影响|声誉影响#选项2...#选项3\n"
//         "2. 数值范围：金钱±(200-800)，健康±(5-20)，声誉±(10-30)\n"
//         "3. 示例：\n"
//         "黑中介陷阱|承诺高薪后扣押身份证逼交押金#反抗被打|-300|-15|+5#妥协交钱|+0|-5|-20#报警求助|+500|+0|+30\n"
//         "必须用#分隔事件和选项，用|分隔影响数值，不要换行或其它符号";
//     std::string response = run(lifeStory + "\n当前状态：现金：" + std::to_string(cash) + "，健康" + std::to_string(health) + "债务" + std::to_string(debt));
//     // std::cout<<response<<std::endl;
//     std::vector<std::vector<std::string>> params;
//     std::vector<std::string> parts;
//     split(response, '#', parts);
//     for(auto & p : parts){
//         params.push_back({});
//         split(p, '|', params.back());
//     }
//     if(params.size() != 4){
//         throw std::runtime_error("response size or form error: " + std::to_string(params.size()));
//     }
//     clearHistory();
//     return params;
// }

std::vector<std::vector<std::string>> AiApi::generateRandomEvent(int cash, int health, int reputation, const std::string &lifeStory) {
    // 第一步：生成事件/问题
    std::string systemPrompt = "";
    std::string eventPrompt =
        "作为21世纪初北京生存事件生成器，请结合人物故事和当前状态（现金，健康，声誉）生成一个事件或问题，格式为："
        "事件标题[5字]|事件描述[30字]，不要包含其他符号。示例：\n"
        "黑中介陷阱|承诺高薪后扣押身份证逼交押金";
    std::string eventInput = lifeStory + "\n当前状态：现金：" + std::to_string(cash) +
                             "，健康：" + std::to_string(health) + "，声誉：" + std::to_string(reputation) +
                             "\n" + eventPrompt;
    std::string eventResponse = run(eventInput);
    std::vector<std::string> eventParts;
    split(eventResponse, '|', eventParts);
    if(eventParts.size() != 2){
        throw std::runtime_error("事件格式错误：预期2部分，但实际获得 " + std::to_string(eventParts.size()) + " 部分");
    }
    
    std::string optionsPrompt =
        "作为21世纪初北京生存事件生成器，请根据上面的事件和当前状态及人物故事生成三个选项，每个选项格式为："
        "选项描述[10字]|金钱影响[±200-±800]|健康影响[±5-±20]|声誉影响[±10-±30]，"
        "各选项用#分隔，不要包含换行或其它符号。示例：\n"
        "反抗被打|-300|-15|+5#妥协交钱|+0|-5|-20#报警求助|+500|+0|+30";
    std::string optionsInput = lifeStory + "\n当前状态：现金：" + std::to_string(cash) +
                               "，健康：" + std::to_string(health) + "，声誉：" + std::to_string(reputation) +
                               "\n事件：" + eventResponse + "\n" + optionsPrompt;
    std::string optionsResponse = run(optionsInput);
    
    std::vector<std::vector<std::string>> params;
    params.push_back(eventParts);
    
    std::vector<std::string> optionParts;
    split(optionsResponse, '#', optionParts);
    if(optionParts.size() != 3){
        throw std::runtime_error("选项数量错误：预期3个选项，但实际获得 " + std::to_string(optionParts.size()));
    }
    for(auto & opt : optionParts){
        std::vector<std::string> optDetails;
        split(opt, '|', optDetails);
        if(optDetails.size() != 4){
            throw std::runtime_error("选项格式错误：预期4部分，但实际获得 " + std::to_string(optDetails.size()));
        }
        params.push_back(optDetails);
    }
    
    clearHistory();
    return params;
}
