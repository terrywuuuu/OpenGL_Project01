#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>

#include "JsonIO.h"

using json = nlohmann::json;

static const char* bodyNames[10] = {
            "body","left_arm","left_hand","head",
            "right_arm","right_hand","left_leg","left_foot",
            "right_leg","right_foot"
};

bool JsonIO::LoadAction(const std::string& filename, Action& out) {
    std::ifstream ifs(filename);
    json j;
    if (!ifs.is_open()) {
        std::cerr << "Cannot open " << filename << " for reading\n";
        return false;
    }
    // Prevent empty file
    if (ifs.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "JsonIO::LoadFrames: empty JSON file\n";
        return false;
    }
    if (!(ifs >> j)) {
        std::cerr << "JsonIO::LoadFrames: JSON parse error\n";
        return false;
    }

    // Clear the existing action data
    out.FDs.clear();

    out.name = j.at("name").get<std::string>();
    out.speed = j.at("speed").get<float>();
    out.musicName = j.at("musicName").get<std::string>();
    if (out.musicName == "")
    {
        out.musicName = "Default.mp3";
    }
    
    for (auto& frameJson : j.at("action")) {
        FrameData ofd;
        ofd.frame = frameJson.at("frame").get<int>();
        ofd.position[0] = frameJson["position"].at("x").get<float>();
        ofd.position[1] = frameJson["position"].at("y").get<float>();
        ofd.position[2] = frameJson["position"].at("z").get<float>();

        for (int i = 0; i < 10; ++i) {
            auto& jn = frameJson[bodyNames[i]];
            ofd.partRotations[i].alpha = jn.at("alpha").get<float>();
            ofd.partRotations[i].beta = jn.at("beta").get<float>();
            ofd.partRotations[i].gamma = jn.at("gamma").get<float>();
        }

        out.FDs.push_back(ofd);  // Push the FrameData to Action's FDs
    }

    return true;
}

bool JsonIO::SaveAction(const std::string& filename, const Action& action) {
    json j;

	j["name"] = action.name;
	j["speed"] = action.speed;
    j["musicName"] = action.musicName;

    for (auto& f : action.FDs) {  // Iterate through Action's FDs
        json frameJson;
        frameJson["frame"] = f.frame;
        frameJson["position"] = {
            {"x", f.position[0]},
            {"y", f.position[1]},
            {"z", f.position[2]}
        };

        for (int i = 0; i < 10; ++i) {
            const auto& pr = f.partRotations[i];
            frameJson[bodyNames[i]] = {
                {"alpha", pr.alpha},
                {"beta",  pr.beta},
                {"gamma", pr.gamma}
            };
        }

        j["action"].push_back(frameJson);
    }

    std::ofstream ofs(filename + ".json");
    if (!ofs.is_open()) {
        std::cerr << "Cannot open " << filename << " for writing\n";
        return false;
    }
    ofs << std::setw(2) << j;
    return true;
}