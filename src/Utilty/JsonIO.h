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
		float speed;
		std::vector<FrameData> FDs;
		Action():FDs(),speed(){}
		Action(std::vector<FrameData> fd) : FDs(fd) {}
	};

	// Load frames from JSON file
	static bool LoadFrames(const std::string& filename, std::vector<FrameData>& out);

	// Save frames to JSON file
	static bool SaveFrames(const std::string& filename, const std::vector<FrameData>& frames);
};