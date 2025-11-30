#include <iostream>
#include <filesystem>
#include <unistd.h>
#include "CampusCompass.h"
using namespace std;

int main() {
    CampusCompass compass;
    // Helper: try various locations to locate the data directory reliably
    auto locate_data_dir = [&]() -> std::string {
        namespace fs = std::filesystem;
        std::vector<fs::path> candidates = {"data", "../data", "build/data"};
        for (auto &p : candidates) {
            if (fs::exists(p / "edges.csv") && fs::exists(p / "classes.csv")) {
                return fs::canonical(p).string();
            }
        }
        fs::path cur = fs::current_path();
        for (int i = 0; i < 6; ++i) {
            fs::path cand = cur / "data";
            if (fs::exists(cand / "edges.csv") && fs::exists(cand / "classes.csv")) {
                return fs::canonical(cand).string();
            }
            if (cur.has_parent_path()) cur = cur.parent_path(); else break;
        }
        char exe_path[4096] = {0};
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
        if (len > 0) {
            fs::path execdir = fs::path(std::string(exe_path, (size_t)len)).parent_path();
            std::vector<fs::path> exec_candidates = {execdir / "data", execdir.parent_path() / "data"};
            for (auto &p : exec_candidates) {
                if (fs::exists(p / "edges.csv") && fs::exists(p / "classes.csv")) {
                    return fs::canonical(p).string();
                }
            }
        }
        return std::string();
    };

    std::string data_dir = locate_data_dir();
    if (!data_dir.empty()) {
        compass.parse_csv(data_dir + "/edges.csv", data_dir + "/classes.csv");
    } else {
        if (!compass.parse_csv("data/edges.csv", "data/classes.csv")) {
            if (!compass.parse_csv("../data/edges.csv", "../data/classes.csv")) {
                compass.parse_csv("build/data/edges.csv", "build/data/classes.csv");
            }
        }
    }
    int no_of_lines;
    string command;
    cin >> no_of_lines;
    cin.ignore();
    for (int i = 0; i < no_of_lines; i++) {
        getline(cin, command);
        compass.parse_command(command);
    }
}
