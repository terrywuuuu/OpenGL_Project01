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
			std::vector<std::string> actions = { "Idle", "Walk", "sit_up", "push_up","multiple", "Hopak Dance" };
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
		ImVec2 controlPos = ImGui::GetWindowPos();
		ImVec2 controlSize = ImGui::GetWindowSize();
		ImGui::End();


		ImGui::SetWindowPos(ImVec2(controlPos.x, controlPos.y + controlSize.y + 10));
		ImGui::SetWindowSize(ImVec2(controlSize.x, 600.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("editor");
		{
			static float alphas[10] = { 0 }, betas[10] = { 0 }, gammas[10] = { 0 }, position[3] = { 0 };
			const char* axes[3] = { "X", "Y", "Z" };
			const char* rotations[3] = { "Alpha", "Beta", "Gamma" };
			const char* bodyParts[10] = { "body", "left_arm", "left_hand", "head", "right_arm",
									   "right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };
			//position
			ImGui::BeginChild("BodyPartsScroll", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			for (int i = 0; i < 3; i++) {
				if (ImGui::InputFloat(axes[i], &position[i], 0.1f, 2.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
					position[i] = (position[i] < -180.0f) ? -180.0f : (position[i] > 180.0f) ? 180.0f : position[i];
					targetScene->SetPosition(i, position[i]);
					std::cout << "Set position " << axes[i] << ": " << position[i] << "\n";
				}
			}
			//rotation
			for (int i = 0; i < 10; i++) if (ImGui::TreeNode(bodyParts[i])) {
				float* values[3] = { &alphas[i], &betas[i], &gammas[i] };
				for (int j = 0; j < 3; j++) {
					if (ImGui::InputFloat(rotations[j], values[j], 1.0f, 10.0f, "%.1f",
						ImGuiInputTextFlags_EnterReturnsTrue)) {
						*values[j] = (*values[j] < -180.0f) ? -180.0f : (*values[j] > 180.0f) ? 180.0f : *values[j];
						targetScene->SetRotate(i, alphas[i], betas[i], gammas[i]);
						std::cout << "Set rotation " << bodyParts[i] << ": Alpha=" << alphas[i]
							<< ", Beta=" << betas[i] << ", Gamma=" << gammas[i] << "\n";
					}
				}
				ImGui::TreePop();
			}
			ImGui::EndChild();
		}
		ImGui::End();

		// Show the big demo window or not
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);
	}
}