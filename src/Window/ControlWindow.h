#pragma once

#include <Scene/MainScene.h>

namespace CG
{
	class ControlWindow
	{
	public:
		ControlWindow();

		auto Initialize() -> bool;
		void Display();
		void DisplayMtl();

	private:
		bool showDemoWindow;
		bool showMtlWindow;
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
