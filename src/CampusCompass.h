#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <utility>
#include <limits>

using namespace std;

class CampusCompass {
private:
    struct Edge {
        int to;
        int time;
        bool closed;
    };

    struct Student {
        string name;
        int id;
        int residence;
        set<string> class_codes;
    };

    struct ClassInfo {
        string code;
        int location_id;
        string start_time;
        string end_time;
    };

    unordered_map<int, vector<Edge>> adj_list;
    unordered_map<int, Student> students_map;
    unordered_map<string, ClassInfo> classes_map;

    int find_edge_index(int from, int to) const;
    unordered_map<int, int> dijkstra_distances(int source) const;
    vector<int> dijkstra_path(int source, int target) const;
    int compute_mst_cost(const unordered_set<int> &vertices) const;
    static bool parse_quoted_name(const string &input, size_t &pos, string &name_out);

public:
    CampusCompass();
    bool parse_csv(const string &edges_filepath, const string &classes_filepath);
    bool parse_command(const string &command);

    bool insert_student(const string &name, int ufid, int residence, const vector<string> &class_codes);
    bool remove_student(int ufid);
    bool drop_class(int ufid, const string &class_code);
    bool replace_class(int ufid, const string &old_class, const string &new_class);
    int remove_class(const string &class_code);
    bool toggle_edges_closure(const vector<pair<int,int>> &edges);
    string check_edge_status(int a, int b) const;
    bool is_connected(int a, int b) const;
    vector<pair<string,int>> get_shortest_edges_for_student(int ufid) const;
    int get_student_zone_cost(int ufid) const;
    bool verify_schedule(int ufid) const;
};
