#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>

#include "CampusCompass.h"

using namespace std;

static void capture_output(function<void()> fn, string &out) {
  ostringstream oss;
  streambuf *old = cout.rdbuf(oss.rdbuf());
  fn();
  cout.rdbuf(old);
  out = oss.str();
}

TEST_CASE("Invalid commands", "[invalid]") {
  CampusCompass c;
  ofstream("edges_tmp.csv") << "LocationID_1,LocationID_2,Name_1,Name_2,Time\n";
  ofstream("classes_tmp.csv") << "ClassCode,LocationID,Start Time (HH:MM),End Time (HH:MM)\n";
  REQUIRE(c.parse_csv("edges_tmp.csv", "classes_tmp.csv") == true);

  vector<string> bad_cmds = {
    "insertt \"Name\" 100 1 1 COP3502",
    "insert Name 100 1 1 COP3502",
    "insert \"A11y\" abc 1 1 COP3530",
    "replaceClass 99999 COP3502 COP3503",
    "dropClass 10000000 NONEXIST"
  };

  for (auto &cmd : bad_cmds) {
    string out;
    bool ret = false;
    capture_output([&]{ ret = c.parse_command(cmd); }, out);
    REQUIRE(ret == false);
  }
}

TEST_CASE("Edge cases: remove non-existent student, dropClass non-existent, removeClass missing", "[edgecases]") {
  CampusCompass c;
  ofstream("edges_tmp2.csv") << "LocationID_1,LocationID_2,Name_1,Name_2,Time\n";
  ofstream("classes_tmp2.csv") << "ClassCode,LocationID,Start Time (HH:MM),End Time (HH:MM)\n";
  REQUIRE(c.parse_csv("edges_tmp2.csv", "classes_tmp2.csv") == true);

  REQUIRE(c.remove_student(55555) == false);
  REQUIRE(c.drop_class(55555, "COP3502") == false);
  int removed = c.remove_class("NON_EXISTENT_CLASS");
  REQUIRE(removed == 0);
}

TEST_CASE("Test dropClass, removeClass, remove, replaceClass behaviors", "[commands]") {
  CampusCompass c;
  ofstream ofs1("edges_cmd.csv");
  ofs1 << "LocationID_1,LocationID_2,Name_1,Name_2,Time\n";
  ofstream ofs2("classes_cmd.csv");
  ofs2 << "ClassCode,LocationID,Start Time (HH:MM),End Time (HH:MM)\nCOP100,10,09:00,10:00\nCOP200,11,10:00,11:00\n";
  ofs1.close();
  ofs2.close();
  REQUIRE(c.parse_csv("edges_cmd.csv", "classes_cmd.csv") == true);

  REQUIRE(c.insert_student("Stu", 11111, 5, vector<string>{"COP100"}) == true);
  REQUIRE(c.replace_class(11111, "COP100", "COP200") == true);
  REQUIRE(c.remove_student(11111) == true);

  REQUIRE(c.insert_student("A", 33333, 5, vector<string>{"COP100"}) == true);
  REQUIRE(c.insert_student("B", 44444, 6, vector<string>{"COP100"}) == true);
  int removed = c.remove_class("COP100");
  REQUIRE(removed == 2);
}

TEST_CASE("printShortestEdges reflects right edges", "[path][toggle]") {
  CampusCompass c;
  ofstream es("edges_path.csv");
  es << "LocationID_1,LocationID_2,Name_1,Name_2,Time\n";
  es << "1,2,A,B,5\n";
  es << "2,3,B,C,5\n";
  es.close();
  ofstream cs("classes_path.csv");
  cs << "ClassCode,LocationID,Start Time (HH:MM),End Time (HH:MM)\n";
  cs << "COPX,3,09:00,10:00\n";
  cs.close();

  REQUIRE(c.parse_csv("edges_path.csv", "classes_path.csv") == true);

  REQUIRE(c.insert_student("P", 77777, 1, vector<string>{"COPX"}) == true);
  string out2;
  capture_output([&]{ REQUIRE(c.parse_command("printShortestEdges 77777") == true); }, out2);
  REQUIRE(out2.find("Total Time: 10") != string::npos);

  string out3;
  capture_output([&]{ REQUIRE(c.parse_command("toggleEdgesClosure 1 2 3") == true); }, out3);
  string out4;
  capture_output([&]{ REQUIRE(c.parse_command("printShortestEdges 77777") == true); }, out4);
  REQUIRE(out4.find("Total Time: -1") != string::npos);
}

TEST_CASE("checkEdgeStatus and isConnected reflect open/closed state", "[connectivity]") {
  CampusCompass c;
  ofstream es("edges_conn.csv");
  es << "LocationID_1,LocationID_2,Name_1,Name_2,Time\n";
  es << "1,2,A,B,3\n";
  es << "2,3,B,C,3\n";
  es.close();
  ofstream cs("classes_conn.csv");
  cs << "ClassCode,LocationID,Start Time (HH:MM),End Time (HH:MM)\n";
  cs.close();
  REQUIRE(c.parse_csv("edges_conn.csv", "classes_conn.csv") == true);

  string out;
  capture_output([&]{ REQUIRE(c.parse_command("checkEdgeStatus 2 3") == true); }, out);
  REQUIRE(out.find("open") != string::npos);
  capture_output([&]{ REQUIRE(c.parse_command("isConnected 1 3") == true); }, out);
  REQUIRE(out.find("successful") != string::npos);

  capture_output([&]{ REQUIRE(c.parse_command("toggleEdgesClosure 1 2 3") == true); }, out);
  string status = c.check_edge_status(2,3);
  REQUIRE(status == "closed");

  REQUIRE(c.is_connected(1,3) == false);
}
