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

			//todo get action data from MainScene
			std::vector<std::string> actionNames = targetScene->GetActionNames();
			ImGui::Text("Action: ");
			ImGui::SameLine(100);
			ImGui::SameLine(100);
			ImGui::SetNextItemWidth(150);
			if (ImGui::BeginCombo("##Action", actionNames[_actionIndex].c_str()))
			{
				for (int n = 0; n < actionNames.size(); n++)
				{
					const bool is_selected = (_actionIndex == n);
					if (ImGui::Selectable(actionNames[n].c_str(), is_selected))
					{
						_actionIndex = n;
						std::cout << "Set Action " << _actionIndex << std::endl;
						targetScene->SetAction(n);
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

		actionData = targetScene->GetAction();
		// Show the big demo window or not
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);
		if (showMtlWindow)
			DisplayMtl();
		DisplayEditor(controlPos, controlSize, _actionIndex);
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

	void removeInput() {
		ImGuiIO& io = ImGui::GetIO();

		// 如果有任何 Popup 顯示，就允許接受鍵盤輸入
		if (1) {
			io.WantTextInput = true;  // 允許接受鍵盤輸入
		}
		else {
			io.WantTextInput = false; // 禁用主視窗的鍵盤輸入
		}

		// 禁用滑鼠滾輪輸入，當編輯器被懸停時
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
			io.MouseWheel = 0.0f;
		}
	}

	void ControlWindow::DisplayEditor(ImVec2 postPos, ImVec2 postSize, int actionIndex) {
		ImGui::SetNextWindowPos(ImVec2(postPos.x, postPos.y + postSize.y + 10));
		ImGui::SetNextWindowSize(ImVec2(postSize.x, 400));
		ImGui::Begin("Editor");
		{
			removeInput();
			static float curFrame = 0.0f;
			static bool isSave = false;
			static bool isEdit = false;
			JsonIO::Action actionData;
			JsonIO::FrameData curFD;

			actionData = targetScene->GetAction();
			curFD = targetScene->GetFrameData();
			curFrame = (int)curFD.frame;

			ImGuiInputTextFlags isReadOnly = (isEdit ? 0 : ImGuiInputTextFlags_ReadOnly);// read only when not edit
			//draw editor window
			DisplayEditorItem(isEdit, curFrame, isReadOnly, curFD, actionData);
			DisplayModleControl(isEdit, curFrame, isReadOnly, curFD, actionData);
		}
		ImGui::End();
	}

	void ControlWindow::DisplayEditorItem(bool& isEdit, float curFrame, bool isReadOnly, JsonIO::FrameData& curFD, JsonIO::Action& actionData) {
		if (ImGui::Checkbox("Edit Action", &isEdit)) {// when state change
			targetScene->SetEdit(isEdit, 0);
		}
		float maxFrame = (actionData.FDs.empty()) ? 0.0f : (actionData.FDs.size() - 1.0f);
		ImGui::SliderFloat("Timeline", &curFrame, 0.0f, maxFrame, "Frame: %.1f", isReadOnly);
		if (isEdit)
			targetScene->SetFrame((int)curFrame);
		if (isEdit && ImGui::Button("Add Frame")) { // copy current frame
			targetScene->SetNewFrameData(curFD, (int)curFrame);
		}
		if (isEdit && ImGui::Button("Save")) {
			targetScene->SaveAction(actionData.name);
			ImGui::OpenPopup("SaveSuccessPopup");
		}
		if (isEdit && ImGui::Button("Save as New Action")) {
			ImGui::OpenPopup("InputPopup"); // Open the input popup
		}
		if (ImGui::BeginPopupModal("InputPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			static char input_buf[128] = "";
			ImGui::InputTextWithHint("##input", "input action name", input_buf, IM_ARRAYSIZE(input_buf));
			if (ImGui::Button("ok")) {
				targetScene->SaveAction(input_buf);
				ImGui::CloseCurrentPopup();  // Close the popup
				ImGui::OpenPopup("SaveSuccessPopup");  // Open the "Save success" popup
			}
			ImGui::SameLine();
			if (ImGui::Button("cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("SaveSuccessPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Success!");  // Save success message
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();  // Close the success popup when "OK" is clicked
			}
			ImGui::EndPopup();
		}
	}

	void ControlWindow::DisplayModleControl(bool isEdit, float curFrame, bool isReadOnly, JsonIO::FrameData& curFD, JsonIO::Action& actionData) {
		static float position[3] = { 0 };
		static float alphas[10] = { 0 }, betas[10] = { 0 }, gammas[10] = { 0 };
		const char* bodyParts[10] = { "body", "left_arm", "left_hand", "head", "right_arm",
			"right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };
		const char* axes[3] = { "X", "Y", "Z" };

		//get model position and rotation
		for (int i = 0; i < 3; i++) {
			position[i] = curFD.position[i];
		}
		for (int i = 0; i < PARTSNUM - 1; i++)
		{
			alphas[i] = curFD.partRotations[i].alpha;
			betas[i] = curFD.partRotations[i].beta;
			gammas[i] = curFD.partRotations[i].gamma;
		}

		ImGui::BeginChild("BodyPartsScroll", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		//set modle position
		for (int i = 0; i < 3; i++) {
			if (ImGui::InputFloat(axes[i], &position[i], 0.1f, 2.0f, "%.1f", isReadOnly)) {
				position[i] = (position[i] < -180.0f) ? -180.0f : (position[i] > 180.0f) ? 180.0f : position[i];
				targetScene->SetPosition(i,position[i]);
			}
		}
		//set modle parts rotation
		for (int i = 0; i < 10; i++) if (ImGui::TreeNode(bodyParts[i])) {
			bool isChange = false;
			if (ImGui::InputFloat(("Alpha##" + std::to_string(i)).c_str(), &alphas[i], 1.0f, 10.0f, "%.1f",
				ImGuiInputTextFlags_EnterReturnsTrue)) {
				isChange = true;
				alphas[i] = (alphas[i] < -180.0f) ? -180.0f : (alphas[i] > 180.0f) ? 180.0f : alphas[i];
			}
			if (ImGui::InputFloat(("Beta##" + std::to_string(i)).c_str(), &betas[i], 1.0f, 10.0f, "%.1f",
				ImGuiInputTextFlags_EnterReturnsTrue)) {
				isChange = true;
				betas[i] = (betas[i] < -180.0f) ? -180.0f : (betas[i] > 180.0f) ? 180.0f : betas[i];
			}
			if (ImGui::InputFloat(("Gamma##" + std::to_string(i)).c_str(), &gammas[i], 1.0f, 10.0f, "%.1f",
				ImGuiInputTextFlags_EnterReturnsTrue)) {
				isChange = true;
				gammas[i] = (gammas[i] < -180.0f) ? -180.0f : (gammas[i] > 180.0f) ? 180.0f : gammas[i];
			}

			if (isChange) {
				targetScene->SetRotate(i, alphas[i], betas[i], gammas[i]);
				std::cout << "Set " << bodyParts[i] << " rotation: " << alphas[i] << ", " << betas[i] << ", " << gammas[i] << std::endl;
			}
			ImGui::TreePop();
		}

		ImGui::EndChild();
	}
}