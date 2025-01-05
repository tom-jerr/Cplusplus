#pragma once

#include <string>
#include <set>
#include <queue>
#include <unordered_set>

extern std::string kYourName;

std::unordered_set<std::string> get_applicants(std::string filename);
std::queue<const std::string*> find_matches(std::string name, std::unordered_set<std::string>& students);
std::string get_match(std::queue<const std::string*>& matches);

int run_autograder();
