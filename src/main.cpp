#include <iostream>
#include <string>
#include "CampusCompass.h"
using namespace std;

int main() {
    CampusCompass compass;
    compass.parse_csv("data/edges.csv", "data/classes.csv");
    int n;
    if (!(cin >> n)) return 0;
    string line;
    getline(cin, line); 
    for (int i = 0; i < n; ++i) {
        if (!getline(cin, line)) break;
        compass.parse_command(line);
    }
    return 0;
}