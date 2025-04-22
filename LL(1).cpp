#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <iterator>

using namespace std;

#define EPSILON "ε"

// Function to remove leading and trailing spaces/tabs from a string
string trim2(const string& str) {
    int start = 0, end = str.length() - 1;
    while (start <= end && (str[start] == ' ' || str[start] == '\t')) 
        start++;

    while (end >= start && (str[end] == ' ' || str[end] == '\t')) 
        end--;

    string result = "";
    for (int i = start; i <= end; i++) 
        result += str[i];

    return result;
}

// Function to read CFG from a file and store in vectors
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
                    string productionRule = left + " -> " + production;
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

// Modify the generateLL1Table function to save to a file simultaneously
void generateLL1Table(const map<string, vector<vector<string>>>& productions,
    const map<string, set<string>>& first,
    const map<string, set<string>>& follow) {

    // Open file to save the LL(1) table
    ofstream outputFile("ll1Table.txt");

    map<string, map<string, string>> table;

    // Iterate over each production rule
    for (auto it = productions.begin(); it != productions.end(); ++it) {
        string A = it->first;
        vector<vector<string>> rules = it->second;

        for (const auto& rule : rules) {
            set<string> firstSet;
            bool hasEpsilon = false;

            // Compute FIRST(alpha)
            for (const auto& symbol : rule) {
                auto itFirst = first.find(symbol);
                if (itFirst == first.end()) { // Terminal symbol
                    firstSet.insert(symbol);
                    break;
                } else { // Non-terminal symbol
                    firstSet.insert(itFirst->second.begin(), itFirst->second.end());
                    if (itFirst->second.count(EPSILON) == 0) break;
                }
            }

            if (firstSet.count(EPSILON)) {
                firstSet.erase(EPSILON);
                hasEpsilon = true;
            }

            // Fill table for FIRST(alpha)
            string productionStr = "";
            for (const auto& symbol : rule) {
                productionStr += symbol + " ";
            }
            productionStr = trim2(productionStr);

            for (const auto& terminal : firstSet) {
                table[A][terminal] = productionStr;
            }

            // If epsilon is in FIRST(alpha), fill table for FOLLOW(A)
            if (hasEpsilon) {
                auto itFollow = follow.find(A);
                if (itFollow != follow.end()) {
                    for (const auto& terminal : itFollow->second) {
                        table[A][terminal] = EPSILON;
                    }
                    if (itFollow->second.count("$")) {
                        table[A]["$"] = EPSILON;
                    }
                }
            }
        }
    }

    // Collect all terminals for table headers
    set<string> allTerminals;
    for (const auto& [lhs, row] : table) {
        for (const auto& [terminal, prod] : row) {
            allTerminals.insert(terminal);
        }
    }

    for (const auto& [nonterminal, followSet] : follow) {
        if (followSet.count("$")) {
            allTerminals.insert("$");
        }
    }

    // Print and write the LL(1) Parsing Table to the file
    cout << "\nLL(1) Parsing Table:\n";
    outputFile << "\nLL(1) Parsing Table:\n";

    // Print the header (non-terminals)
    cout << "    ";
    outputFile << "    ";
    for (const auto& term : allTerminals) {
        cout << term << "\t";
        outputFile << term << "\t";
    }
    cout << std::endl;
    outputFile << std::endl;

    // Print the table rows (non-terminals with corresponding productions)
    for (const auto& [lhs, row] : table) {
        cout << lhs << " | ";
        outputFile << lhs << " | ";
        for (const auto& term : allTerminals) {
            if (row.find(term) != row.end()) {
                cout << row.at(term) << "\t";
                outputFile << row.at(term) << "\t";
            } else {
                cout << "-\t"; // Empty cell
                outputFile << "-\t";
            }
        }
        cout << std::endl;
        outputFile << std::endl;
    }

    // Close the output file after writing
    outputFile.close();
}

