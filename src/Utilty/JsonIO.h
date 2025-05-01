#pragma once

#include <vector>

class JsonIO {
public:
	struct partRotation { float alpha, beta, gamma; };
	struct FrameData {
		float frame;
		bool isKeyFrame;
		float position[3];            // x, y, z
		partRotation partRotations[10];

		FrameData() : frame(0), position{ 0, 0, 0 } {
			for (int i = 0; i < 10; ++i) {
				partRotations[i] = { 0, 0, 0 };
			}
		}
	};
	struct Action
	{
		std::string name;
		float speed;
		std::string musicName;
		std::vector<FrameData> FDs;

		Action():FDs(),speed(0.0),name(""), musicName("Default.mp3") {}
		Action(std::string name, float speed, std::string musicName, std::vector<FrameData> fd) :name(name),speed(speed), FDs(fd), musicName(musicName) {
			
		}
	};

	// Load frames from JSON file
	static bool LoadAction(const std::string& filename, Action& out);

	// Save frames to JSON file
	static bool SaveAction(const std::string& filename, const Action& ad);
};