#include <iostream>
#include <string>
#include "CampusCompass.h"

int main() {
    CampusCompass compass;
    compass.parse_csv("data/edges.csv", "data/classes.csv");
    int n;
    if (!(std::cin >> n)) return 0;
    std::string line;
    std::getline(std::cin, line); 
    for (int i = 0; i < n; ++i) {
        if (!std::getline(std::cin, line)) break;
        compass.parse_command(line);
    }
    return 0;
}