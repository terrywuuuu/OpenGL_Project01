#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class JsonIO {
public:
    struct Joint { float alpha, beta, gamma; };
    struct FrameData {
        int frame;
        float posX;
        float posY;
        float posZ;
        Joint body;
        Joint head;
        Joint left_arm;
        Joint left_hand;
        Joint right_arm;
        Joint right_hand;
        Joint left_leg;
        Joint left_foot;
        Joint right_leg;
        Joint right_foot;
    };

    // Load frames from JSON file
    static bool LoadFrames(const std::string& filename, std::vector<FrameData>& out) {
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            std::cerr << "Cannot open " << filename << " for reading\n";
            return false;
        }
        json j;
        ifs >> j;
        out.clear();
        for (auto& frameJson : j) {
            FrameData f;
            f.frame = frameJson.at("frame").get<int>();
            f.posX = frameJson["position"].at("x").get<float>();
            f.posY = frameJson["position"].at("y").get<float>();
            f.posZ = frameJson["position"].at("z").get<float>();
            auto readJoint = [&](Joint& joint, const std::string& name) {
                joint.alpha = frameJson[name].at("alpha").get<float>();
                joint.beta = frameJson[name].at("beta").get<float>();
                joint.gamma = frameJson[name].at("gamma").get<float>();
                };
            readJoint(f.body, "body");
            readJoint(f.head, "head");
            readJoint(f.left_arm, "left_arm");
            readJoint(f.left_hand, "left_hand");
            readJoint(f.right_arm, "right_arm");
            readJoint(f.right_hand, "right_hand");
            readJoint(f.left_leg, "left_leg");
            readJoint(f.left_foot, "left_foot");
            readJoint(f.right_leg, "right_leg");
            readJoint(f.right_foot, "right_foot");
            out.push_back(f);
        }
        return true;
    }

    // Save frames to JSON file
    static bool SaveFrames(const std::string& filename, const std::vector<FrameData>& frames) {
        json j = json::array();
        for (auto& f : frames) {
            json frameJson;
            frameJson["frame"] = f.frame;
            frameJson["position"] = { {"x", f.posX}, {"y", f.posY}, {"z", f.posZ} };
            auto writeJoint = [&](const Joint& joint, const std::string& name) {
                frameJson[name] = {
                    {"alpha", joint.alpha},
                    {"beta",  joint.beta},
                    {"gamma", joint.gamma}
                };
                };
            writeJoint(f.body, "body");
            writeJoint(f.head, "head");
            writeJoint(f.left_arm, "left_arm");
            writeJoint(f.left_hand, "left_hand");
            writeJoint(f.right_arm, "right_arm");
            writeJoint(f.right_hand, "right_hand");
            writeJoint(f.left_leg, "left_leg");
            writeJoint(f.left_foot, "left_foot");
            writeJoint(f.right_leg, "right_leg");
            writeJoint(f.right_foot, "right_foot");
            j.push_back(frameJson);
        }
        std::ofstream ofs(filename);
        if (!ofs.is_open()) {
            std::cerr << "Cannot open " << filename << " for writing\n";
            return false;
        }
        ofs << std::setw(2) << j;
        return true;
    }
};
