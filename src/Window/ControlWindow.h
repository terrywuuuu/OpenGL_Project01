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
		void DisplayActionSelector(int actionIndex, bool& isEdit);
		void DisplayEditorItem(bool& isEdit, float curFrame, JsonIO::FrameData& curFD, JsonIO::Action& actionData);
		void DisplayModleControl(bool& isEdit, float curFrame, JsonIO::FrameData& curFD);
		void DisplayEffect();
		void SetActionData();
    
		void HandleInput();

	private:
		bool showDemoWindow;
		bool showMtlWindow;

		bool isKeyboardEnable;
		bool keepMultipleActive;
		bool previousKeepMultipleActive;

		bool showEffectWindow;
		bool isEdit;
		float speed;

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
