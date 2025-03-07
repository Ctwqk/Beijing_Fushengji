#include "SelectPopUp.h"
#include "Button.h"

void SelectPopUp::Render(){
    if(isOpen){
        ImGui::OpenPopup("选择");
    }
    if(ImGui::BeginPopupModal("选择", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::Text("你要干什么");
        
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