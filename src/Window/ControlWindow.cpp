#include "ControlWindow.h"

#include <imgui.h>

#include <iostream>
#include <vector>
#include <string>

namespace CG
{
	ControlWindow::ControlWindow()
	{
		showDemoWindow = false;
	}

	auto ControlWindow::Initialize() -> bool
	{
		return true;
	}

    void ControlWindow::Display()
    {
        ImGui::Begin("Control");
        {
            ImGui::Checkbox("Demo Window", &showDemoWindow);

            static int actionIndex = 0;
            std::vector<std::string> actions = { "Idle", "Walk", "Sit Up", "Push Up" };
            ImGui::Text("Action: ");
            ImGui::SameLine(100);
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Action", actions[actionIndex].c_str()))
            {
                for (int n = 0; n < actions.size(); n++)
                {
                    const bool is_selected = (actionIndex == n);
                    if (ImGui::Selectable(actions[n].c_str(), is_selected))
                    {
                        actionIndex = n;
                        std::cout << "Set Action " << actionIndex << std::endl;
                        targetScene->SetAction(n);
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            static int modeIndex = 0;
            std::vector<std::string> modes = { "Fill", "Line" };
            ImGui::Text("Mode: ");
            ImGui::SameLine(100);
            ImGui::SetNextItemWidth(150);
            if (ImGui::BeginCombo("##Mode", modes[modeIndex].c_str()))
            {
                for (int n = 0; n < modes.size(); n++)
                {
                    const bool is_selected = (modeIndex == n);
                    if (ImGui::Selectable(modes[n].c_str(), is_selected))
                    {
                        modeIndex = n;
                        std::cout << "Set Mode " << modeIndex << std::endl;
                        targetScene->SetMode(n);
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImVec2 controlPos = ImGui::GetWindowPos();      // 獲取 Control 窗口位置
        ImVec2 controlSize = ImGui::GetWindowSize();    // 獲取 Control 窗口大小
        ImGui::End();


        ImGui::SetNextWindowPos(ImVec2(controlPos.x, controlPos.y + controlSize.y + 10));
        ImGui::SetNextWindowSize(ImVec2(controlSize.x, 600.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Editor");
        {
            ImGui::BeginChild("BodyPartsScroll", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

            static float alphas[10] = { 0.0f };
            static float betas[10] = { 0.0f };
            static float gammas[10] = { 0.0f };
            const char* bodyParts[10] = { "body", "left_arm", "left_hand", "head", "right_arm", "right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };

            for (int i = 0; i < 10; i++)
            {
                // 使用 TreeNode 創建可摺疊的節點
                if (ImGui::TreeNode(bodyParts[i]))
                {
                    bool valueChanged = false;

                    //x, y, z方向角度
                    if (ImGui::InputFloat((std::string("Alpha##") + std::to_string(i)).c_str(), &alphas[i], 1.0f, 10.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        alphas[i] = (alphas[i] < -180.0f) ? -180.0f : (alphas[i] > 180.0f) ? 180.0f : alphas[i];
                        valueChanged = true;
                    }
                    if (ImGui::InputFloat((std::string("Beta##") + std::to_string(i)).c_str(), &betas[i], 1.0f, 10.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        betas[i] = (betas[i] < -180.0f) ? -180.0f : (betas[i] > 180.0f) ? 180.0f : betas[i];
                        valueChanged = true;
                    }
                    if (ImGui::InputFloat((std::string("Gamma##") + std::to_string(i)).c_str(), &gammas[i], 1.0f, 10.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        gammas[i] = (gammas[i] < -180.0f) ? -180.0f : (gammas[i] > 180.0f) ? 180.0f : gammas[i];
                        valueChanged = true;
                    }

                    if (valueChanged)
                    {
                        targetScene->SetRotate(i, alphas[i], betas[i], gammas[i]);
                        std::cout << "Set rotation for " << bodyParts[i] << ": Alpha=" << alphas[i]
                            << ", Beta=" << betas[i] << ", Gamma=" << gammas[i] << std::endl;
                    }
                    ImGui::TreePop(); // 關閉 TreeNode
                }
            }
            ImGui::EndChild(); // 結束滾動區域
        }
        ImGui::End();

        // Show the big demo window or not
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);
    }
}
