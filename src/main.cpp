#include <iostream>
#include "CampusCompass.h"
using namespace std;

int main() {
    CampusCompass compass;
    // Try a few candidate paths so the program works when run from
    // the repository root or from the build directory used by some test harnesses.
    if (!compass.parse_csv("data/edges.csv", "data/classes.csv")) {
        if (!compass.parse_csv("../data/edges.csv", "../data/classes.csv")) {
            // last attempt: try looking under "build/data" when CMake copied files there
            compass.parse_csv("build/data/edges.csv", "build/data/classes.csv");
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
