#pragma once

#include <vector>

class JsonIO {
public:
    struct partRotation { float alpha, beta, gamma; };
    struct FrameData {
        int frame;
        float position[3];            // x, y, z
        partRotation partRotations[10];
    };

    // Load frames from JSON file
    static bool LoadFrames(const std::string& filename, std::vector<FrameData>& out);

    // Save frames to JSON file
    static bool SaveFrames(const std::string& filename, const std::vector<FrameData>& frames);
};