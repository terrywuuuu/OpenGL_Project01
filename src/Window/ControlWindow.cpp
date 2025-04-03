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
			std::vector<std::string> actions = { "Idle", "Walk" };
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

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
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

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

		}
		ImGui::End();

		// Show the big demo window or not (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);
	}
}
