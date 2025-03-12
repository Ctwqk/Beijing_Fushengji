#include "TextPopUp.h"
#include "Events.h"
#include "Item.h"
#include "Language.h"


TextPopUp::TextPopUp(){
    
}

void TextPopUp::Render(){
    
    if (isOpen){
        ImGui::OpenPopup(label.c_str());
        label = events.front()->label;
        text = events.front()->text;
        if(events.front()->buttons.size()) buttons = events.front()->buttons;
    }
    if(ImGui::BeginPopupModal(label.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::TextWrapped("%s", text.c_str());
        ImGui::Spacing();
        if(!buttons.size()){
            buttons.push_back(std::make_shared<ButtonWithAction>(GET_TEXT("CONFIRM"), [](){}));
        }
        for (auto& button:buttons){
            if(ImGui::Button(button ? button->label.c_str() : GET_TEXT("CONFIRM").c_str(), ImVec2(200, 40))) {
                if(button)button->action();
                events.pop_front();
                if(events.size() ==0) {
                    Close();
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::EndPopup();
    }
    // std::cout<<"hhh"<<std::endl;
}

void TextPopUp::setEvents(std::vector<std::shared_ptr<Event>> &e){
    // std::move(e.begin(),e.end(),std::back_inserter(events));
    events.insert(events.end(),e.begin(),e.end());
    if(events.size()){  
        isOpen = true;
    }
}

void TextPopUp::Open(std::string t, std::string l, std::shared_ptr<ButtonWithAction>b){
    events.push_back(std::make_shared<Event>(l, t, b));
    isOpen = true;
}