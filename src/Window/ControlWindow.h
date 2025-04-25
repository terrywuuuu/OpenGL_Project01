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

	private:
		bool showDemoWindow;
		bool showMtlWindow;
		bool showEditor;
		float speed;

	private:
		MainScene* targetScene;

	public:
		void SetTargetScene(MainScene* scene)
		{
			targetScene = scene;
		}
	};
}
