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

		ImGuiIO& io = ImGui::GetIO();

		static float lastPressTime = 0.0f;  // last keydown time
		float triggerInterval = 0.05f;

		//Key event A, D control eyes Angley
		if (ImGui::IsKeyDown(ImGuiKey_A)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(0);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_D)) {

			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(1);
				lastPressTime = 0.0f;
			}
		}

		//Key event W, S control angle
		if (ImGui::IsKeyDown(ImGuiKey_W)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(2);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_S)){
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(3);
				lastPressTime = 0.0f;
			}
		}

		//Mouse wheel control eyes distance
		if (io.MouseWheel != 0.0f)
		{
			if (io.MouseWheel > 0)
				//Mouse wheel up
				targetScene->OnKeyboard(4);
			else
				//Mouse wheel down
				targetScene->OnKeyboard(5);
		}

		ImGui::Begin("Control");
		{
			ImGui::Checkbox("Demo Window", &showDemoWindow);
			ImGui::Checkbox("Material Setting Window", &showMtlWindow);

            static int actionIndex = 0;
            std::vector<std::string> actions = { "Idle", "Walk", "sit_up", "push_up","multiple", "Hopak Dance", "APT", "T-pose"};
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

			if (ImGui::SliderFloat("Speed", &speed, 0.1f, 10.0f, "%.3f"))
			{
				targetScene->SetSpeed(speed);
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
		ImVec2 controlPos = ImGui::GetWindowPos();      // ��� Control ���f��m
		ImVec2 controlSize = ImGui::GetWindowSize();    // ��� Control ���f�j�p
		ImGui::End();


		ImGui::SetNextWindowPos(ImVec2(controlPos.x, controlPos.y + controlSize.y + 10));
		ImGui::SetNextWindowSize(ImVec2(controlSize.x, 600.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin("Editor");
		{
			ImGui::BeginChild("BodyPartsScroll", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

			static float alphas[10] = { 0 }, betas[10] = { 0 }, gammas[10] = { 0 }, position[3] = { 0 };
			const char* bodyParts[10] = { "body", "left_arm", "left_hand", "head", "right_arm",
									   "right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };
			const char* axes[3] = { "X", "Y", "Z" };

			for (int i = 0; i < 3; i++) {
				if (ImGui::InputFloat(axes[i], &position[i], 0.1f, 2.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue)) {
					position[i] = (position[i] < -180.0f) ? -180.0f : (position[i] > 180.0f) ? 180.0f : position[i];
					targetScene->SetPosition(i, position[i]);
					std::cout << "Set position " << axes[i] << ": " << position[i] << "\n";
				}
			}

			for (int i = 0; i < 10; i++) if (ImGui::TreeNode(bodyParts[i])) {
				bool changed = false;

				if (ImGui::InputFloat(("Alpha##" + std::to_string(i)).c_str(), &alphas[i], 1.0f, 10.0f, "%.1f",
					ImGuiInputTextFlags_EnterReturnsTrue)) {
					alphas[i] = (alphas[i] < -180.0f) ? -180.0f : (alphas[i] > 180.0f) ? 180.0f : alphas[i];
					changed = true;
				}
				if (ImGui::InputFloat(("Beta##" + std::to_string(i)).c_str(), &betas[i], 1.0f, 10.0f, "%.1f",
					ImGuiInputTextFlags_EnterReturnsTrue)) {
					betas[i] = (betas[i] < -180.0f) ? -180.0f : (betas[i] > 180.0f) ? 180.0f : betas[i];
					changed = true;
				}
				if (ImGui::InputFloat(("Gamma##" + std::to_string(i)).c_str(), &gammas[i], 1.0f, 10.0f, "%.1f",
					ImGuiInputTextFlags_EnterReturnsTrue)) {
					gammas[i] = (gammas[i] < -180.0f) ? -180.0f : (gammas[i] > 180.0f) ? 180.0f : gammas[i];
					changed = true;
				}

				if (changed) {
					targetScene->SetRotate(i, alphas[i], betas[i], gammas[i]);
					std::cout << "Set rotation " << bodyParts[i] << ": Alpha=" << alphas[i]
						<< ", Beta=" << betas[i] << ", Gamma=" << gammas[i] << "\n";
				}
				ImGui::TreePop();
			}
			ImGui::EndChild();
		}
		ImGui::End();

		// Show the big demo window or not
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);

		if (showMtlWindow)
			DisplayMtl();
	}

	void ControlWindow::DisplayMtl() {
		ImGui::SetNextWindowPos(ImVec2(1000, 100));              // 設定位置 (x=1000, y=100)
		ImGui::SetNextWindowSize(ImVec2(300, 200));             // 設定寬度 300、高度 200
		ImGui::Begin("My Custom Window");
		{
			static std::vector<int> partsIndex;
			partsIndex.resize(10, 0);
			std::vector<std::string> Parts = { "body", "left_arm", "left_hand", "head","right_arm", "right_hand", "left_leg", "left_foot", "right_leg", "right_foot"};
			std::vector<std::string> material = { "Matte", "Metal", "Dark" };

			for (int i = 0; i < Parts.size(); ++i)
			{
				ImGui::Text("%s", Parts[i].c_str());
				ImGui::SameLine(100);
				ImGui::SetNextItemWidth(150);
				std::string comboID = "##Material_" + Parts[i];

				if (ImGui::BeginCombo(comboID.c_str(), material[partsIndex[i]].c_str()))
				{
					for (int n = 0; n < material.size(); n++)
					{
						const bool is_selected = (partsIndex[i] == n);
						if (ImGui::Selectable(material[n].c_str(), is_selected))
						{
							partsIndex[i] = n;
							std::cout << "Set Mtl " << partsIndex[i] << std::endl;
							targetScene->SetMtl(i, material[n]);
						}

						if (is_selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			ImGui::End();
		}
	}
}