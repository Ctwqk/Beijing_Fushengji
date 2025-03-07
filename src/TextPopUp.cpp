#include "TextPopUp.h"



TextPopUp::TextPopUp(){
    button = new ButtonWithAction("OK", [&](){
        events.pop_back();
        if(events.size() == 0) Close();
        ImGui::CloseCurrentPopup();
    });
}

void TextPopUp::Render(){
    
    if (isOpen){
        ImGui::OpenPopup(label.c_str());
        label = events.back()->label;
        text = events.back()->text;
        if(events.back()->button) button = events.back()->button;
    }
    if(ImGui::BeginPopupModal(label.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::TextWrapped("%s", text.c_str());
        ImGui::Spacing();
        if (ImGui::Button(button?button->label.c_str():"OK", ImVec2(100, 40))) {
            if(button)button->action();
            events.pop_back();
            if(events.size() ==0) Close();
        }
        ImGui::EndPopup();
    }
    // std::cout<<"hhh"<<std::endl;
}

void TextPopUp::setEvents(std::vector<Event*> &e){
    if(events.size()) e.push_back(events.back());
    events = e;
    if(events.size()){  
        isOpen = true;
    }
}

void TextPopUp::Open(std::string t, std::string l, ButtonWithAction *b){
    events.push_back(new Event(l, t, b));
    button = nullptr;
    isOpen = true;
}