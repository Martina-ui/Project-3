#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <limits.h>
#include <cstring>
#include <sys/stat.h>
#include "CampusCompass.h"

int main() {
    CampusCompass compass;
    auto file_exists = [&](const std::string &path)->bool{
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    };
    auto canonical = [&](const std::string &p)->std::string{
        char buf[PATH_MAX];
        if (realpath(p.c_str(), buf)) return std::string(buf);
        return p;
    };
    auto locate_data_dir = [&]() -> std::string {
        std::vector<std::string> candidates = {"data", "../data", "build/data"};
        for (auto &p : candidates) {
            if (file_exists(p + "/edges.csv") && file_exists(p + "/classes.csv")) {
                return canonical(p);
            }
        }
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::string cur(cwd);
            for (int i = 0; i < 6; ++i) {
                std::string cand = cur + "/data";
                if (file_exists(cand + "/edges.csv") && file_exists(cand + "/classes.csv")) {
                    return canonical(cand);
                }
                size_t pos = cur.find_last_of('/');
                if (pos == std::string::npos) break;
                cur = cur.substr(0, pos);
            }
        }
        char exe_path[4096] = {0};
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
        if (len > 0) {
            std::string exec(std::string(exe_path, (size_t)len));
            size_t pos = exec.find_last_of('/');
            if (pos != std::string::npos) {
                std::string execdir = exec.substr(0, pos);
                std::vector<std::string> exec_candidates = {execdir + "/data", execdir + "/../data"};
                for (auto &p : exec_candidates) {
                    if (file_exists(p + "/edges.csv") && file_exists(p + "/classes.csv")) {
                        return canonical(p);
                    }
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
    std::string command;
    std::cin >> no_of_lines;
    std::cin.ignore();
    for (int i = 0; i < no_of_lines; i++) {
        std::getline(std::cin, command);
        compass.parse_command(command);
    }
}
