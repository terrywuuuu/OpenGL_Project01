#include "ControlWindow.h"

#include <iostream>
#include <vector>
#include <string>

namespace CG
{
	ControlWindow::ControlWindow()
	{
		showDemoWindow = false;
		showMtlWindow = false;
	}

	auto ControlWindow::Initialize() -> bool
	{
		return true;
	}

	void ControlWindow::HandleInput()
	{
		ImGuiIO& io = ImGui::GetIO();

		static float lastPressTime = 0.0f;  // last keydown time
		float triggerInterval = 0.05f;

		//run other window first to avoid input conflict
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

		if (ImGui::IsKeyDown(ImGuiKey_S)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(3);
				lastPressTime = 0.0f;
			}
		}

		//Mouse wheel control eyes distance
		if (io.MouseWheel != 0.0f) {
			{
				if (io.MouseWheel > 0)
					//Mouse wheel up
					targetScene->OnKeyboard(4);
				else
					//Mouse wheel down
					targetScene->OnKeyboard(5);
			}
		}
	}

	void ControlWindow::Display()
	{
		static int _actionIndex = 0;
		static JsonIO::Action actionData;
		ImGui::Begin("Control");
		{
			ImGui::Checkbox("Demo Window", &showDemoWindow);
			ImGui::Checkbox("Material Setting Window", &showMtlWindow);
			ImGui::Checkbox("Edit Action", &isEdit);

			//todo get action data from MainScene
			std::vector<std::string> actions = { "Idle", "Walk", "sit_up", "push_up","multiple", "Hopak Dance", "APT" };
			ImGui::Text("Action: ");
			ImGui::SameLine(100);
			ImGui::SameLine(100);
			ImGui::SetNextItemWidth(150);
			if (ImGui::BeginCombo("##Action", actions[_actionIndex].c_str()))
			{
				for (int n = 0; n < actions.size(); n++)
				{
					const bool is_selected = (_actionIndex == n);
					if (ImGui::Selectable(actions[n].c_str(), is_selected))
					{
						_actionIndex = n;
						std::cout << "Set Action " << _actionIndex << std::endl;
						targetScene->SetAction(n);
						actionData = targetScene->GetAction();
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SetNextItemWidth(150);
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
		ImVec2 controlPos = ImGui::GetWindowPos();
		ImVec2 controlSize = ImGui::GetWindowSize();
		ImGui::End();

		// Show the big demo window or not
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);
		if (showMtlWindow)
			DisplayMtl();
		//if (isEdit)
		DisplayEditor(controlPos, controlSize, _actionIndex, isEdit, actionData);
		targetScene->SetEdit(isEdit, 0);
		HandleInput();
	}

	void ControlWindow::DisplayMtl() {
		ImGui::Begin("My Custom Window");
		{
			static std::vector<int> partsIndex;
			partsIndex.resize(10, 0);
			std::vector<std::string> Parts = { "body", "left_arm", "left_hand", "head","right_arm", "right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };
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

	void ControlWindow::DisplayEditor(ImVec2 postPos, ImVec2 postSize, int actionIndex, bool isEdit,JsonIO::Action actionData) {
		ImGui::SetNextWindowPos(ImVec2(postPos.x, postPos.y + postSize.y + 10));
		ImGui::SetNextWindowSize(ImVec2(postSize.x, 400));
		ImGui::Begin("Editor");
		{
			ImGuiIO& io = ImGui::GetIO();
			bool editorHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
			// Disable the mouse wheel input when the editor is hovered
			if (editorHovered) {
				io.MouseWheel = 0.0f;
			}

			static float curFrame = 0.0f;
			static bool isChangeFD = false;
			static bool isSave = false;
			static float alphas[10] = { 0 }, betas[10] = { 0 }, gammas[10] = { 0 }, position[3] = { 0 };
			const char* bodyParts[10] = { "body", "left_arm", "left_hand", "head", "right_arm",
									   "right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };
			const char* axes[3] = { "X", "Y", "Z" };
			JsonIO::FrameData curFD, nextFD;

			curFD = targetScene->GetFrameData();
			curFrame = (int)curFD.frame;

			//get model position and rotation
			for (int i = 0; i < 3; i++) {
				position[i] = curFD.position[i];
			}
			for (int i = 0; i < PARTSNUM-1; i++)
			{
				alphas[i] = curFD.partRotations[i].alpha;
				betas[i] = curFD.partRotations[i].beta;
				gammas[i] = curFD.partRotations[i].gamma;
			}

			ImGuiInputTextFlags flag = ImGuiInputTextFlags_EnterReturnsTrue
				| (isEdit ? 0 : ImGuiInputTextFlags_ReadOnly);

			//draw editor window
			//total frames todo add a button to add keyframe
			ImGui::SliderFloat("Timeline", &curFrame, 0.0f, actionData.FDs.size() - 1, "Frame: %.1f");
			if (ImGui::Button("Add Keyframe")) {
				// copy current frame data to a new frame
				actionData.FDs.push_back(curFD);
				actionData.FDs.back().frame = actionData.FDs.size() - 1;
			}
			ImGui::Checkbox("Save Current Frame", &isSave);

			//set action data after change curFrame
			actionData = targetScene->GetAction();
			

			ImGui::BeginChild("BodyPartsScroll", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			//set modle position
			for (int i = 0; i < 3; i++) {
				if (ImGui::InputFloat(axes[i], &position[i], 0.1f, 2.0f, "%.1f", flag)) {
					position[i] = (position[i] < -180.0f) ? -180.0f : (position[i] > 180.0f) ? 180.0f : position[i];
					targetScene->SetPosition(i, position[i]);
					std::cout << "Set position " << axes[i] << ": " << position[i] << "\n";
				}
			}
			//set modle parts rotation
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
					curFD.partRotations[i].alpha = alphas[i];
					std::cout << "Set rotation " << bodyParts[i] << ": Alpha=" << alphas[i]
						<< ", Beta=" << betas[i] << ", Gamma=" << gammas[i] << "\n";
				}
				ImGui::TreePop();
			}
			if(isEdit)
				//targetScene->SetFrameData(curFD, (int)curFrame);
			if (isSave) {
				isSave = false;
				targetScene->SetFrameData(curFD, (int)curFrame);
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}