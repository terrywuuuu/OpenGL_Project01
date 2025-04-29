#pragma once

#include <Scene/MainScene.h>
#include <imgui.h>

namespace CG
{
	class ControlWindow
	{
	public:
		ControlWindow();

		auto Initialize() -> bool;
		void Display();
		void DisplayMtl();
		void DisplayEditor(ImVec2 postPos,ImVec2 postSize);
		void DisplayActionSelector(int actionIndex = -1);
		void DisplayEditorItem(bool& isEdit, float curFrame, JsonIO::FrameData& curFD, JsonIO::Action& actionData);
		void DisplayModleControl(bool& isEdit, float curFrame, JsonIO::FrameData& curFD);
		void HandleInput();
		void ToggleInput(bool isKeyboardEnable, bool isMouseEnable);

	private:
		bool showDemoWindow;
		bool showMtlWindow;
		bool isKeyboardEnable = 1;
		//JsonIO::Action actionData;
		JsonIO::FrameData frameData;
		

	private:
		MainScene* targetScene;

	public:
		void SetTargetScene(MainScene* scene)
		{
			targetScene = scene;
		}
	};
}
