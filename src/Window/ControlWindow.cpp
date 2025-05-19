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
		showWaterWindow = false;
		keepMultipleActive = false;
		previousKeepMultipleActive = keepMultipleActive;
		showEffectWindow = false;
	}

	auto ControlWindow::Initialize() -> bool
	{
		return true;
	}

	void ControlWindow::Display()
	{
		static JsonIO::Action actionData;

		ImGui::SetNextWindowSize(ImVec2(300, 200));
		ImGui::Begin("Control");
		{
			ImGui::Checkbox("Demo Window", &showDemoWindow);
			ImGui::Checkbox("Material Setting Window", &showMtlWindow);
			ImGui::Checkbox("Special Effects Setting Window", &showEffectWindow);
			ImGui::Checkbox("Show Water", &showWaterWindow);
			targetScene->SetWater(showWaterWindow);

			ImGui::SetNextItemWidth(150);

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

			static int multipleModesIndex = 0, multipleNumber = 100;
			std::vector<std::string> multipleModes = { "Triangle Edge Only" ,"Triangular Grid", "Circular Spread" };
			ImGui::Text("Multiple Number: ");
			if (ImGui::InputInt("multipleNumber##", &multipleNumber, 10.0f)) {
				multipleNumber = multipleNumber < 1 ? 1 : multipleNumber;
				targetScene->SetMultipleNumber(multipleNumber);
			}
			ImGui::Text("Multiple Setting: ");
			ImGui::Checkbox("Keep Multiple Active", &keepMultipleActive);
			ImGui::Text("Multiple Mode: ");
			if (ImGui::BeginCombo("##MultipleMode", multipleModes[multipleModesIndex].c_str()))
			{
				for (int n = 0; n < multipleModes.size(); n++)
				{
					const bool is_selected = (multipleModesIndex == n);
					if (ImGui::Selectable(multipleModes[n].c_str(), is_selected))
					{
						multipleModesIndex = n;
						std::cout << "Set Multiple Mode " << multipleModesIndex << std::endl;
						targetScene->SetMultipleMode(n);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}
		if (keepMultipleActive != previousKeepMultipleActive)
		{
			targetScene->SetkeepMultipleActive(keepMultipleActive);
			previousKeepMultipleActive = keepMultipleActive;
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
		if (showEffectWindow)
			DisplayEffect();
		if (showWaterWindow)
			DisplayWater();
		DisplayEditor(controlPos, controlSize);
		//ToggleInput(0, 0);
		HandleInput();
	}

	void ControlWindow::DisplayMtl() {
		ImGui::Begin("Material");
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

	int lastEffect = -1;
	int effectNum = 4;
	std::vector<std::string> effectName = { "Vague", "Quantization", "Mosaic", "Motion Blur" };
	static std::map<std::string, bool> isActive;
	std::vector<float> num = { 0, 2.0f, 1.0f, 1.0f};
	std::vector<std::pair<float, float >> Range = { {0,3.0f},{2.0f,8.0f}, {1.0f,16.0f}, {1.0f,16.0f} };

	void ControlWindow::DisplayEffect() {
		ImGui::Begin("Special Effect");
		{
			int effectNum = 6;
			std::vector<std::string> effectName = { "Vague", "Quantization", "Mosaic", "MotionBlur", "EnvironmentMap", "ToonShader"};
			static std::map<std::string, bool> isActive;
			static std::map<std::string, float> num;
			std::vector<std::pair<float, float >> Range = { {0,3.0f},{2.0f,8.0f}, {1.0f,16.0f}, {1.0,8.0} };

			for (int i = 0; i < effectNum; i++) {
				ImGui::Checkbox(effectName[i].c_str(), &isActive[effectName[i]]);

				ImGui::SetNextItemWidth(100);
				if (isActive[effectName[i]])
				{
					if (i == 4 || i == 5) {
						targetScene->SetEffect(0, i, true);
					}
					else {
						ImGui::SliderFloat("Strength", &num[effectName[i]], Range[i].first, Range[i].second, "%.3f");
						targetScene->SetEffect(num[effectName[i]], i, true);
					}
				}
				else
				{
					targetScene->SetEffect(0, i, false);
				}
			}

			ImGui::End();
		}
	}


	void ControlWindow::DisplayWater() {
		ImGui::Begin("Water");
		{
			int effectNum = 2;
			std::vector<std::string> effectName = { "Wave", "Light Reflection"};
			static std::map<std::string, bool> isActive;

			for (int i = 0; i < effectNum; i++) {
				ImGui::Checkbox(effectName[i].c_str(), &isActive[effectName[i]]);

				ImGui::SetNextItemWidth(100);
				if (isActive[effectName[i]])
				{
					targetScene->SetWaterEffect(i, true);
				}
				else
				{
					targetScene->SetWaterEffect(i, false);
				}
			}

			ImGui::End();
		}
	}

	void ControlWindow::DisplayEditor(ImVec2 postPos, ImVec2 postSize) {
		ImGui::SetWindowPos(ImVec2(postPos.x, postPos.y + postSize.y + 10));
		ImGui::SetWindowSize(ImVec2(postSize.x, 400));
		ImGui::Begin("Editor");
		{
			//ToggleInput(0, 0);
			static float curFrame = 0.0f;
			static bool isSave = false;
			static bool isEdit = false;
			JsonIO::Action actionData;
			JsonIO::FrameData curFD;

			actionData = targetScene->GetAction();
			curFD = targetScene->GetFrameData();
			curFrame = (int)curFD.frame;

			//draw editor window
			DisplayActionSelector(-1,isEdit);
			DisplayEditorItem(isEdit, curFrame, curFD, actionData);
			DisplayModleControl(isEdit, curFrame, curFD);
		}
		ImGui::End();
	}

	void ControlWindow::DisplayActionSelector(int actionIndex, bool& isEdit) {
		static int _actionIndex = 0;
		_actionIndex = actionIndex < 0 ? _actionIndex : actionIndex;

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
					targetScene->SetEdit(0);
					isEdit = false;
				}

				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

	}

	void ControlWindow::DisplayEditorItem(bool& isEdit, float curFrame, JsonIO::FrameData& curFD, JsonIO::Action& actionData) {
		ImGuiInputTextFlags isReadOnly = (isEdit ? 0 : ImGuiInputTextFlags_ReadOnly);
		float maxFrame = (actionData.FDs.empty()) ? 0.0f : (actionData.FDs.size() - 1.0f);
		float speed = actionData.speed;

		// display current frame
		ImGui::SliderFloat("Timeline", &curFrame, 0.0f, maxFrame, "Frame: %.1f", isReadOnly);
		// set action speed
		if (ImGui::SliderFloat("Speed", &speed, 0.1f, 10.0f, "%.3f")) {
			targetScene->SetSpeed(speed);
		}
		if (ImGui::Checkbox("Edit Action", &isEdit)) {// set edit mode
			targetScene->SetEdit(isEdit);
		}
		if (isEdit) {
			targetScene->SetFrame((int)curFrame);
			if (ImGui::Button("Add Frame")) { // copy current frame
				targetScene->SetNewFrameData(curFD, (int)curFrame,1);
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Frame")) { // copy current frame
				targetScene->SetNewFrameData(curFD, (int)curFrame,0);
			}
			if (ImGui::Button("Save")) {
				targetScene->SaveAction(actionData.name);
				ImGui::OpenPopup("SaveSuccessPopup");
			}
			ImGui::SameLine();
			if (ImGui::Button("Save as New Action")) {
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
				DisplayActionSelector(0, isEdit);
			}
		}
	}

	void ControlWindow::DisplayModleControl(bool& isEdit, float curFrame, JsonIO::FrameData& curFD) {
		const char* bodyParts[10] = { "body", "left_arm", "left_hand", "head", "right_arm",
			"right_hand", "left_leg", "left_foot", "right_leg", "right_foot" };
		const char* axes[3] = { "X", "Y", "Z" };
		bool isChange = false;
		ImGuiInputTextFlags isReadOnly = (isEdit ? 0 : ImGuiInputTextFlags_ReadOnly);

		ImGui::BeginChild("BodyPartsScroll", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		//set modle position
		for (int i = 0; i < 3; i++) {
			if (ImGui::InputFloat(axes[i], &curFD.position[i], 0.1f, 2.0f, "%.1f", isReadOnly)) {
				curFD.position[i] = (curFD.position[i] < -180.0f) ? -180.0f : (curFD.position[i] > 180.0f) ? 180.0f : curFD.position[i];
				isChange = true;
			}
		}
		//set modle parts rotation
		for (int i = 0; i < 10; i++) if (ImGui::TreeNode(bodyParts[i])) {
			float* alpha = &curFD.partRotations[i].alpha, * beta = &curFD.partRotations[i].beta, * gamma = &curFD.partRotations[i].gamma;
			if (ImGui::InputFloat(("Alpha##" + std::to_string(i)).c_str(), alpha, 1.0f, 10.0f, "%.1f", isReadOnly)) {
				*alpha = (*alpha < -180.0f) ? -180.0f : (*alpha > 180.0f) ? 180.0f : *alpha;
				isChange = true;
			}
			if (ImGui::InputFloat(("Beta##" + std::to_string(i)).c_str(), beta, 1.0f, 10.0f, "%.1f", isReadOnly)) {
				*beta = (*beta < -180.0f) ? -180.0f : (*beta > 180.0f) ? 180.0f : *beta;
				isChange = true;
			}
			if (ImGui::InputFloat(("Gamma##" + std::to_string(i)).c_str(), gamma, 1.0f, 10.0f, "%.1f", isReadOnly)) {
				*gamma = (*gamma < -180.0f) ? -180.0f : (*gamma > 180.0f) ? 180.0f : *gamma;
				isChange = true;
			}
			ImGui::TreePop();
		}
		//set display modle position, alpha, beta, gamma
		if (isChange) {
			targetScene->SetCurFrameData(curFD, (int)curFrame);
		}
		ImGui::EndChild();
	}

	void ControlWindow::HandleInput()
	{
		ImGuiIO& io = ImGui::GetIO();

		static float lastPressTime = 0.0f;  // last keydown time
		float triggerInterval = 0.05f;

		if (io.WantCaptureKeyboard)
			return;

		if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			targetScene->OnMouseDownAndMove(io.MouseDelta.x, io.MouseDelta.y);
		}

		if (ImGui::IsKeyDown(ImGuiKey_Q)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(10);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_E)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(11);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(6);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(7);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(8);
				lastPressTime = 0.0f;
			}
		}

		if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
			lastPressTime += io.DeltaTime;

			if (lastPressTime > triggerInterval)
			{
				targetScene->OnKeyboard(9);
				lastPressTime = 0.0f;
			}
		}

		//Mouse wheel control eyes distance
		if (io.MouseWheel != 0.0f && !io.WantCaptureMouse) {
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
}