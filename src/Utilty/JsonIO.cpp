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

bool JsonIO::LoadFrames(const std::string& filename, std::vector<FrameData>& out) {
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
    out.clear();
    for (auto& frameJson : j) {
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
        out.push_back(ofd);
    }
    return true;
}

bool JsonIO::SaveFrames(const std::string& filename, const std::vector<FrameData>& fd) {
    json j = json::array();
    for (auto& f : fd) {
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
