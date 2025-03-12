#include "SelectPopUp.h"
#include "Button.h"
#include "Language.h"

void SelectPopUp::Render(){
    if(isOpen){
        ImGui::OpenPopup(GET_TEXT("SELECT_TITLE").c_str());
    }
    if(ImGui::BeginPopupModal(GET_TEXT("SELECT_TITLE").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::Text("%s", GET_TEXT("WHAT_TO_DO").c_str());
        
        int count = 0;
        for(const auto& selection: selections){
            if(ImGui::Button(selection.label.c_str(), ImVec2(80, 30))){
                selection.action();
            }
            count++;
            if(count % buttonPerRow != 0){
                ImGui::SameLine();
            }
        }
        ImGui::EndPopup();
    }
}
// void SelectPopUp::Open(){
//     isOpen = true;
//     std::cout<<"inner open"<<std::endl;
// }

SelectPopUp::SelectPopUp(std::vector<ButtonWithAction> buttons, int bpr):selections(buttons), buttonPerRow(bpr){

}