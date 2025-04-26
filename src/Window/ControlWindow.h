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
		void DisplayEditor(ImVec2 postPos,ImVec2 postSize,int actionIndex, bool isEdit, JsonIO::Action actionData);
		void SetActionData();
		void HandleInput();

	private:
		bool showDemoWindow;
		bool showMtlWindow;
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
