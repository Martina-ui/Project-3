#include "CampusCompass.h"

#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>

using namespace std;

CampusCompass::CampusCompass() {
}

bool CampusCompass::parse_csv(const string &edges_filepath, const string &classes_filepath) {
    ifstream edges_file(edges_filepath);
    if (!edges_file.is_open()) {
        return false;
    }
    string line;
    getline(edges_file, line);
    while (getline(edges_file, line)) {
        if (line.empty()) {
            continue;
        }
        for (char &c : line) {
            if (c == '\t') {
                c = ',';
            }
        }
        vector<string> toks;
        string tok;
        stringstream ss(line);
        while (getline(ss, tok, ',')) {
            size_t a = tok.find_first_not_of(" \r\n\t");
            size_t b = tok.find_last_not_of(" \r\n\t");
            if (a==string::npos) {
                toks.push_back("");
            } else {
                toks.push_back(tok.substr(a, b-a+1));
            }
        }
        if (toks.size() < 5) {
            continue;
        }
        int a = stoi(toks[0]);
        int b = stoi(toks[1]);
        int time = stoi(toks[4]);
        Edge e1{b, time, false};
        Edge e2{a, time, false};
        adj_list[a].push_back(e1);
        adj_list[b].push_back(e2);
    }
    edges_file.close();

    ifstream classes_file(classes_filepath);
    if (!classes_file.is_open()) {
        return false;
    }
    getline(classes_file, line);
    while (getline(classes_file, line)) {
        if (line.empty()) {
            continue;
        }
        for (char &c : line) {
            if (c == '\t') {
                c = ',';
            }
        }
        vector<string> toks;
        string tok;
        stringstream ss(line);
        while (getline(ss, tok, ',')) {
            size_t a = tok.find_first_not_of(" \r\n\t");
            size_t b = tok.find_last_not_of(" \r\n\t");
            if (a==string::npos) {
                toks.push_back("");
            } else {
                toks.push_back(tok.substr(a, b-a+1));
            }
        }
        if (toks.size() < 2) {
            continue;
        }
        string code = toks[0];
        int loc = stoi(toks[1]);
        string start = (toks.size() > 2) ? toks[2] : string();
        string end = (toks.size() > 3) ? toks[3] : string();
        ClassInfo ci{code, loc, start, end};
        classes_map[code] = ci;
    }
    classes_file.close();
    return true;
}


bool CampusCompass::parse_command(const string &command) {
    string cmd;
    stringstream ss(command);
    if (!(ss >> cmd)) {
        return false;
    }
    if (cmd == "insert") {
        size_t p = command.find('"');
        if (p == string::npos) {
            cout << "unsuccessful\n";
            return false;
        }
        size_t q = command.find('"', p+1);
        if (q == string::npos) {
            cout << "unsuccessful\n";
            return false;
        }
        string name = command.substr(p+1, q-p-1);
        string rest = command.substr(q+1);
        stringstream rs(rest);
        int ufid, residence, n;
        if (!(rs >> ufid >> residence >> n)) {
            cout << "unsuccessful\n";
            return false;
        }
        vector<string> class_codes;
        string code;
        for (int i = 0; i < n; ++i) {
            if (!(rs >> code)) {
                cout << "unsuccessful\n";
                return false;
            }
            class_codes.push_back(code);
        }
        bool ok = insert_student(name, ufid, residence, class_codes);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    } else if (cmd == "remove") {
        int ufid;
        if (!(ss >> ufid)) {
            cout << "unsuccessful\n";
            return false;
        }
        bool ok = remove_student(ufid);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    } else if (cmd == "dropClass") {
        int ufid;
        string class_code;
        if (!(ss >> ufid >> class_code)) {
            cout << "unsuccessful\n";
            return false;
        }
        bool ok = drop_class(ufid, class_code);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    } else if (cmd == "replaceClass") {
        int ufid;
        string c1, c2;
        if (!(ss >> ufid >> c1 >> c2)) {
            cout << "unsuccessful\n";
            return false;
        }
        bool ok = replace_class(ufid, c1, c2);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    } else if (cmd == "removeClass") {
        string class_code;
        if (!(ss >> class_code)) {
            cout << "0\n";
            return false;
        }
        int removed = remove_class(class_code);
        cout << removed << '\n';
        return true;
    } else if (cmd == "toggleEdgesClosure") {
        int n;
        if (!(ss >> n)) {
            cout << "unsuccessful\n";
            return false;
        }
        vector<pair<int,int>> edges;
        for (int i = 0; i < n; ++i) {
            int a, b;
            if (!(ss >> a >> b)) {
                cout << "unsuccessful\n";
                return false;
            }
            edges.emplace_back(a,b);
        }
        bool ok = toggle_edges_closure(edges);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    } else if (cmd == "checkEdgeStatus") {
        int a, b;
        if (!(ss >> a >> b)) {
            cout << "DNE\n";
            return false;
        }
        cout << check_edge_status(a,b) << '\n';
        return true;
    } else if (cmd == "isConnected") {
        int a, b;
        if (!(ss >> a >> b)) {
            cout << "unsuccessful\n";
            return false;
        }
        bool ok = is_connected(a,b);
        cout << (ok ? "successful" : "unsuccessful") << '\n';
        return ok;
    } else if (cmd == "printShortestEdges") {
        int ufid;
        if (!(ss >> ufid)) {
            cout << "unsuccessful\n";
            return false;
        }
        auto vec = get_shortest_edges_for_student(ufid);
        auto it = students_map.find(ufid);
        if (it == students_map.end()) {
            cout << "unsuccessful\n";
            return false;
        }
        cout << "Name: " << it->second.name << '\n';
        for (auto &p : vec) {
            cout << p.first << " | Total Time: " << p.second << '\n';
        }
        return true;
    } else if (cmd == "printStudentZone") {
        int ufid;
        if (!(ss >> ufid)) {
            cout << "unsuccessful\n";
            return false;
        }
        int cost = get_student_zone_cost(ufid);
        auto it = students_map.find(ufid);
        if (it == students_map.end() || cost < 0) {
            cout << "unsuccessful\n";
            return false;
        }
        cout << "Student Zone Cost For " << it->second.name << ": " << cost << '\n';
        return true;
    } else if (cmd == "verifySchedule") {
        int ufid;
        if (!(ss >> ufid)) {
            cout << "unsuccessful\n";
            return false;
        }
        bool ok = verify_schedule(ufid);
        return ok;
    }
    return false;
}

int CampusCompass::find_edge_index(int from, int to) const {
    auto it = adj_list.find(from);
    if (it == adj_list.end()) {
        return -1;
    }
    const auto &vec = it->second;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i].to == to) {
            return (int)i;
        }
    }
    return -1;
}

unordered_map<int,int> CampusCompass::dijkstra_distances(int source) const {
    unordered_map<int,int> dist;
    using PI = pair<int,int>;
    priority_queue<PI, vector<PI>, greater<PI>> pq;
    dist[source] = 0;
    pq.push({0, source});
    while (!pq.empty()) {
        auto [d,u] = pq.top();
        pq.pop();
        if (d != dist.at(u)) {
            continue;
        }
        auto it = adj_list.find(u);
        if (it == adj_list.end()) {
            continue;
        }
        for (const Edge &e : it->second) {
            if (e.closed) {
                continue;
            }
            int v = e.to;
            int nd = d + e.time;
            if (!dist.count(v) || nd < dist.at(v)) {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}

vector<int> CampusCompass::dijkstra_path(int source, int target) const {
    unordered_map<int,int> dist;
    unordered_map<int,int> parent;
    using PI = pair<int,int>;
    priority_queue<PI, vector<PI>, greater<PI>> pq;
    dist[source] = 0;
    parent[source] = -1;
    pq.push({0, source});
    while (!pq.empty()) {
        auto [d,u] = pq.top();
        pq.pop();
        if (u == target) {
            break;
        }
        if (d != dist.at(u)) {
            continue;
        }
        auto it = adj_list.find(u);
        if (it == adj_list.end()) {
            continue;
        }
        for (const Edge &e : it->second) {
            if (e.closed) {
                continue;
            }
            int v = e.to;
            int nd = d + e.time;
            if (!dist.count(v) || nd < dist.at(v)) {
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }
    if (!dist.count(target)) {
        return {};
    }
    vector<int> path;
    int cur = target;
    while (cur != -1) {
        path.push_back(cur);
        cur = parent[cur];
    }
    reverse(path.begin(), path.end());
    return path;
}

int CampusCompass::compute_mst_cost(const unordered_set<int> &vertices) const {
    if (vertices.empty()) {
        return 0;
    }
    unordered_set<int> visited;
    using PI = pair<int,pair<int,int>>;
    priority_queue<PI, vector<PI>, greater<PI>> pq;
    int start = *vertices.begin();
    visited.insert(start);
    for (const Edge &e : adj_list.at(start)) {
        if (e.closed) {
            continue;
        }
        if (!vertices.count(e.to)) {
            continue;
        }
        pq.push({e.time, {start, e.to}});
    }
    int total = 0;
    while (!pq.empty() && visited.size() < vertices.size()) {
        auto top = pq.top();
        pq.pop();
        int w = top.first;
        int v = top.second.second;
        if (visited.count(v)) {
            continue;
        }
        visited.insert(v);
        total += w;
        for (const Edge &e : adj_list.at(v)) {
            if (e.closed) {
                continue;
            }
            if (!vertices.count(e.to)) {
                continue;
            }
            if (!visited.count(e.to)) {
                pq.push({e.time, {v, e.to}});
            }
        }
    }
    if (visited.size() != vertices.size()) {
        return -1;
    }
    return total;
}

bool CampusCompass::parse_quoted_name(const string &input, size_t &pos, string &name_out) {
    size_t start = input.find('"', pos);
    if (start == string::npos) {
        return false;
    }
    size_t end = input.find('"', start+1);
    if (end == string::npos) {
        return false;
    }
    name_out = input.substr(start+1, end-start-1);
    pos = end+1;
    return true;
}

bool CampusCompass::insert_student(const string &name, int ufid, int residence, const vector<string> &class_codes) {
    if (students_map.count(ufid)) {
        return false;
    }
    for (const string &c : class_codes) {
        if (!classes_map.count(c)) {
            return false;
        }
    }
    Student s; s.name = name; s.id = ufid; s.residence = residence;
    for (const string &c : class_codes) s.class_codes.insert(c);
    students_map[ufid] = move(s);
    return true;
}

bool CampusCompass::remove_student(int ufid) {
    if (!students_map.count(ufid)) {
        return false;
    }
    students_map.erase(ufid);
    return true;
}

bool CampusCompass::drop_class(int ufid, const string &class_code) {
    auto it = students_map.find(ufid);
    if (it == students_map.end()) {
        return false;
    }
    if (!it->second.class_codes.count(class_code)) {
        return false;
    }
    it->second.class_codes.erase(class_code);
    if (it->second.class_codes.empty()) {
        students_map.erase(it);
    }
    return true;
}

bool CampusCompass::replace_class(int ufid, const string &old_class, const string &new_class) {
    auto it = students_map.find(ufid);
    if (it == students_map.end()) {
        return false;
    }
    if (!it->second.class_codes.count(old_class)) {
        return false;
    }
    if (!classes_map.count(new_class)) {
        return false;
    }
    if (it->second.class_codes.count(new_class)) {
        return false;
    }
    it->second.class_codes.erase(old_class);
    it->second.class_codes.insert(new_class);
    return true;
}

int CampusCompass::remove_class(const string &class_code) {
    int count = 0;
    for (auto it = students_map.begin(); it != students_map.end();) {
        if (it->second.class_codes.count(class_code)) {
            it->second.class_codes.erase(class_code);
            ++count;
            if (it->second.class_codes.empty()) {
                it = students_map.erase(it);
                continue;
            }
        }
        ++it;
    }
    classes_map.erase(class_code);
    return count;
}

bool CampusCompass::toggle_edges_closure(const vector<pair<int,int>> &edges) {
    for (auto &pr : edges) {
        int a = pr.first;
        int b = pr.second;
        int i = find_edge_index(a,b);
        int j = find_edge_index(b,a);
        if (i >= 0) {
            adj_list[a][i].closed = !adj_list[a][i].closed;
        }
        if (j >= 0) {
            adj_list[b][j].closed = !adj_list[b][j].closed;
        }
    }
    return true;
}

string CampusCompass::check_edge_status(int a, int b) const {
    int i = find_edge_index(a,b);
    if (i < 0) {
        return string("DNE");
    }
    if (adj_list.at(a)[i].closed) {
        return string("closed");
    } else {
        return string("open");
    }
}

bool CampusCompass::is_connected(int a, int b) const {
    if (a == b) {
        return true;
    }
    unordered_set<int> vis;
    queue<int> q;
    vis.insert(a);
    q.push(a);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        auto it = adj_list.find(u);
        if (it == adj_list.end()) {
            continue;
        }
        for (const Edge &e : it->second) {
            if (e.closed) {
                continue;
            }
            if (e.to == b) {
                return true;
            }
            if (!vis.count(e.to)) {
                vis.insert(e.to);
                q.push(e.to);
            }
        }
    }
    return false;
}

vector<pair<string,int>> CampusCompass::get_shortest_edges_for_student(int ufid) const {
    vector<pair<string,int>> out;
    auto it = students_map.find(ufid);
    if (it == students_map.end()) {
        return out;
    }
    int residence = it->second.residence;
    auto dist = dijkstra_distances(residence);
    for (const string &c : it->second.class_codes) {
        auto cit = classes_map.find(c);
        int time = -1;
        if (cit != classes_map.end()) {
            int loc = cit->second.location_id;
            if (dist.count(loc)) {
                time = dist.at(loc);
            }
        }
        out.emplace_back(c, time);
    }
    sort(out.begin(), out.end(), [](const pair<string,int>&a,const pair<string,int>&b){ return a.first < b.first; });
    return out;
}

int CampusCompass::get_student_zone_cost(int ufid) const {
    auto it = students_map.find(ufid);
    if (it == students_map.end()) {
        return -1;
    }
    unordered_set<int> vertices;
    int residence = it->second.residence;
    vertices.insert(residence);
    for (const string &c : it->second.class_codes) {
        auto cit = classes_map.find(c);
        if (cit == classes_map.end()) {
            continue;
        }
        int loc = cit->second.location_id;
        auto path = dijkstra_path(residence, loc);
        if (path.empty()) {
            continue;
        }
        for (int v : path) {
            vertices.insert(v);
        }
    }
    if (vertices.empty()) {
        return -1;
    }
    return compute_mst_cost(vertices);
}

static int time_to_minutes(const string &t) {
    if (t.empty()) {
        return -1;
    }
    int hh = stoi(t.substr(0,2));
    int mm = stoi(t.substr(3,2));
    return hh*60 + mm;
}

bool CampusCompass::verify_schedule(int ufid) const {
    auto it = students_map.find(ufid);
    if (it == students_map.end()) {
        return false;
    }
    if (it->second.class_codes.size() <= 1) {
        cout << "unsuccessful" << '\n';
        return false;
    }
    struct Item { int start; int end; string code; int loc; };
    vector<Item> items;
    for (const string &c : it->second.class_codes) {
        auto cit = classes_map.find(c);
        if (cit == classes_map.end()) continue;
        int s = time_to_minutes(cit->second.start_time);
        int e = time_to_minutes(cit->second.end_time);
        items.push_back({s,e,c,cit->second.location_id});
    }
    sort(items.begin(), items.end(), [](const Item&a,const Item&b){ return a.start < b.start; });
    cout << "Schedule Check for " << it->second.name << ':' << '\n';
    for (size_t i = 0; i+1 < items.size(); ++i) {
        int from_loc = items[i].loc;
        int to_loc = items[i+1].loc;
        auto distmap = dijkstra_distances(from_loc);
        int shortest = distmap.count(to_loc) ? distmap.at(to_loc) : numeric_limits<int>::max();
        int gap = items[i+1].start - items[i].end;
        bool can = (shortest <= gap);
        cout << items[i].code << " - " << items[i+1].code << " \"" << (can ? "Can make it!" : "Cannot make it!") << "\"" << '\n';
    }
    return true;
}
