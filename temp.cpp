#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <map>
#include <set>
#include "leftRecursion.cpp"
#include "FirstFollow.cpp"
#include <iomanip>
#include "LL(1).cpp"

using namespace std;

#define EPSILON "ε"


// Read CFG from a file and store in vectors
void readCFGFromFile(const string& filename, vector<string>& prodleft, vector<string>& prodright) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open file " << filename << endl;
        return;
    }
    string line;
    set<string> uniqueProductions;

    while (getline(file, line)) {
        stringstream ss(line);
        string left, arrow, right;
        ss >> left >> arrow;
        getline(ss, right);
        right = trim2(right);

        if (!left.empty() && !right.empty()) {
            stringstream rhs(right);
            string production;
            while (getline(rhs, production, '|')) {
                production = trim2(production);
                if (!production.empty()) {
                    string productionRule = left + " → " + production;
                    if (uniqueProductions.find(productionRule) == uniqueProductions.end()) {
                        prodleft.push_back(left);
                        prodright.push_back(production);
                        uniqueProductions.insert(productionRule);
                    }
                }
            }
        }
    }
    file.close();
}

// Left factoring function
void leftFactoring(vector<string>& left_production, vector<string>& right_production) {
    int production = left_production.size();
    int e = 1;

    for (int i = 0; i < production; ++i) {
        for (int j = i + 1; j < production; ++j) {
            if (left_production[j] == left_production[i]) {
                int k = 0;
                string common = "";
                while (k < right_production[i].length() && k < right_production[j].length() &&
                       right_production[i][k] == right_production[j][k]) {
                    common += right_production[i][k];
                    k++;
                }
                if (k == 0) continue;

                string newNonTerminal = left_production[i] + to_string(e);
                string suffix1 = (k < right_production[i].length()) ? trim2(right_production[i].substr(k)) : EPSILON;
                string suffix2 = (k < right_production[j].length()) ? trim2(right_production[j].substr(k)) : EPSILON;

                left_production.push_back(newNonTerminal);
                right_production.push_back(suffix1 + " | " + suffix2);

                right_production[i] = trim2(common) + " " + newNonTerminal;
                right_production[j] = "";

                production++;
                e++;
            }
        }
    }

    set<string> uniqueProductions;
    vector<string> uniqueRightProduction;
    vector<string> uniqueLeftProduction;

    for (int i = 0; i < left_production.size(); ++i) {
        if (right_production[i].empty()) continue;
        string productionRule = left_production[i] + " → " + right_production[i];
        if (uniqueProductions.find(productionRule) == uniqueProductions.end()) {
            uniqueProductions.insert(productionRule);
            uniqueLeftProduction.push_back(left_production[i]);
            uniqueRightProduction.push_back(right_production[i]);
        }
    }

    left_production = uniqueLeftProduction;
    right_production = uniqueRightProduction;
}

// Function to read FOLLOW sets from file
// Generate LL(1) Parsing Table
// Generate LL(1) Parsing Table
// Helper function to join vector elements into a string with separator
string join(const vector<string>& vec, const string& separator) {
    stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        if (i != vec.size() - 1) ss << separator;
    }
    return ss.str();
}


int main() {
    vector<string> left_production, right_production;
    string filename = "cfg.txt";

    // Read CFG from the file
    readCFGFromFile(filename, left_production, right_production);

    // Apply left factoring
    leftFactoring(left_production, right_production);

    // Group the productions by non-terminal
    map<string, vector<string>> groupedProductions;
    vector<string> nonTerminalOrder;

    for (int i = 0; i < left_production.size(); ++i) {
        if (groupedProductions.find(left_production[i]) == groupedProductions.end()) {
            nonTerminalOrder.push_back(left_production[i]);
        }
        groupedProductions[left_production[i]].push_back(right_production[i]);
    }

    // Write left-factored grammar to tempLeftFactored.txt while preserving order
    ofstream tempFile("tempLeftFactored.txt");
    for (const string& lhs : nonTerminalOrder) {
        const vector<string>& rhsList = groupedProductions[lhs];
        tempFile << lhs << " -> ";
        for (size_t i = 0; i < rhsList.size(); ++i) {
            tempFile << rhsList[i];
            if (i < rhsList.size() - 1)
                tempFile << " | ";
        }
        tempFile << endl;
    }
    tempFile.close();

    // Read the left-factored CFG and eliminate left recursion
    vector<pair<string, Production>> cfg = readCFG("tempLeftFactored.txt");
    eliminateLeftRecursion(cfg);
    printCFG(cfg);

    // Convert cfg to expected format
    map<string, vector<vector<string>>> formattedCFG;
    for (const auto& [lhs, prod] : cfg) {
        vector<vector<string>> rules;
        for (const string& rhs : prod.rhs) {
            istringstream iss(rhs);
            vector<string> tokens;
            string token;
            while (iss >> token) tokens.push_back(token);
            rules.push_back(tokens);
        }
        formattedCFG[lhs] = rules;
    }

    // Read Follow sets from file
    map<string, set<string>> followSetsFromFile = readFollowSetsFromFile("FollowSets.txt");

    // Compute First and Follow sets, then generate LL(1) table
    FirstFollowSet ff(formattedCFG, cfg.begin()->first);
    ff.computeAllFirst();
    ff.computeAllFollow();
    ff.printFirstSets();
    ff.printFollowSets();
    ff.saveFirstSetsToFile("FirstSets.txt");
    ff.saveFollowSetsToFile("FollowSets.txt");

    // Generate the LL(1) Parsing Table using the FOLLOW sets from the file
generateLL1Table(formattedCFG, ff.getFirstSets(), followSetsFromFile);


    return 0;
}
