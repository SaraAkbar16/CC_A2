#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>
#include <map>
#include <set>
#include <unordered_map>
#include <iomanip>
#include "leftRecursion.cpp"  // Assume this file contains the left recursion elimination code
#include "FirstFollow.cpp"    // Assume this file contains the First and Follow set computation code

#define EPSILON "ε"

using namespace std;

// Helper function to trim leading and trailing spaces
string trim2(const string& str) {
    int start = 0, end = str.length() - 1;
    while (start <= end && (str[start] == ' ' || str[start] == '\t')) 
        start++;
    while (end >= start && (str[end] == ' ' || str[end] == '\t')) 
        end--;
    return str.substr(start, end - start + 1);
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

// Helper function to join vector elements into a string with separator
string join(const vector<string>& vec, const string& separator) {
    stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        if (i != vec.size() - 1) ss << separator;
    }
    return ss.str();
}

// Function to read FOLLOW sets from a file
map<string, set<string>> readFollowSetsFromFile(const string& filename) {
    map<string, set<string>> followSets;
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open file " << filename << endl;
        return followSets; // Return empty map if file can't be opened
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string nonTerminal, arrow, follow;
        ss >> nonTerminal >> arrow;
        while (ss >> follow) {
            followSets[nonTerminal].insert(follow);
        }
    }
    file.close();
    return followSets;
}

// Helper function to print the LL(1) parsing table
void printParsingTable(const unordered_map<string, unordered_map<string, string>>& parsingTable) {
    // Print the header row
    cout << setw(15) << "Non-Terminal";
    unordered_set<string> terminals;
    for (const auto& row : parsingTable) {
        for (const auto& col : row.second) {
            terminals.insert(col.first);
        }
    }
    for (const auto& terminal : terminals) {
        cout << setw(15) << terminal;
    }
    cout << endl;

    // Print the table contents
    for (const auto& row : parsingTable) {
        cout << setw(15) << row.first;
        for (const auto& terminal : terminals) {
            auto terminalEntry = row.second.find(terminal);
            if (terminalEntry != row.second.end()) {
                cout << setw(15) << terminalEntry->second;
            } else {
                cout << setw(15) << "";
            }
        }
        cout << endl;
    }
}

// Function to construct the LL(1) parsing table// Adjusted part where map.at() is used
unordered_map<string, unordered_map<string, string>> generateLL1ParsingTable(
    const map<string, vector<vector<string>>>& formattedCFG,
    const map<string, set<string>>& firstSets,
    const map<string, set<string>>& followSets) {

    unordered_map<string, unordered_map<string, string>> parsingTable;

    // Loop through all non-terminals and their productions
    for (const auto& lhs : formattedCFG) {
        const string& nonTerminal = lhs.first;
        const vector<vector<string>>& productions = lhs.second;

        // For each production rule of the non-terminal
        for (const auto& production : productions) {
            // If the production starts with a terminal, add it directly to the table
            if (firstSets.find(production[0]) != firstSets.end() &&
                firstSets.at(production[0]).find("ε") == firstSets.at(production[0]).end()) {
                for (const auto& terminal : firstSets.at(production[0])) {
                    // Use find to avoid map.at() exception
                    if (parsingTable[nonTerminal].find(terminal) == parsingTable[nonTerminal].end()) {
                        parsingTable[nonTerminal][terminal] = nonTerminal + " → " + join(production, " ");
                    }
                }
            }
            // If the production starts with epsilon, use follow sets
            else {
                auto followIt = followSets.find(nonTerminal);
                if (followIt != followSets.end()) {
                    for (const auto& terminal : followIt->second) {
                        if (parsingTable[nonTerminal].find(terminal) == parsingTable[nonTerminal].end()) {
                            parsingTable[nonTerminal][terminal] = nonTerminal + " → ε";
                        }
                    }
                } else {
                    cout << "Follow set not found for non-terminal: " << nonTerminal << endl;
                }
            }
        }
    }

    return parsingTable;
}


// Function to read CFG from file
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

    // Dynamically fetch the start symbol from the first production rule
    string startSymbol = left_production[0];

    // Read Follow sets from file
    map<string, set<string>> followSetsFromFile = readFollowSetsFromFile("followsets.txt");

    // Generate the LL(1) parsing table
    unordered_map<string, unordered_map<string, string>> parsingTable = generateLL1ParsingTable(formattedCFG, firstSets, followSetsFromFile);

    // Print the LL(1) parsing table
    printParsingTable(parsingTable);

    return 0;
}
