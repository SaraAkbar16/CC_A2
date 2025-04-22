#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

#define EPSILON "ε"

// Function to trim leading/trailing spaces and tabs
string trim2(const string& str) {
    int start = 0, end = str.length() - 1;
    while (start <= end && (str[start] == ' ' || str[start] == '\t')) start++;
    while (end >= start && (str[end] == ' ' || str[end] == '\t')) end--;
    return str.substr(start, end - start + 1);
}

// Function to read FOLLOW sets from a file
map<string, set<string>> readFollowSetsFromFile(const string& filename) {
    map<string, set<string>> followSets;
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return followSets;
    }

    string line;
    while (getline(inputFile, line)) {
        line = trim2(line);
        size_t colonPos = line.find(":");
        if (colonPos != string::npos) {
            string nonTerminal = trim2(line.substr(0, colonPos));
            string followString = trim2(line.substr(colonPos + 1));
            set<string> followSet;

            size_t pos = 0;
            while ((pos = followString.find(" ")) != string::npos) {
                string terminal = trim2(followString.substr(0, pos));
                if (!terminal.empty())
                    followSet.insert(terminal == "id" ? "$" : terminal);
                followString.erase(0, pos + 1);
            }
            if (!followString.empty())
                followSet.insert(followString == "id" ? "$" : followString);

            followSets[nonTerminal] = followSet;
        }
    }

    inputFile.close();
    return followSets;
}

void generateLL1Table(const map<string, vector<vector<string>>>& productions,
    const map<string, set<string>>& first,
    map<string, set<string>>& follow) {

    ofstream outputFile("ll1Table.txt");
    map<string, map<string, string>> table;

    // Step 1: Build the LL(1) parsing table
    for (const auto& [A, rules] : productions) {
        for (const auto& rule : rules) {
            set<string> firstSet;
            bool hasEpsilon = false;

            // Calculate First set for the production rule
            for (const auto& symbol : rule) {
                if (first.find(symbol) == first.end()) {
                    firstSet.insert(symbol); // terminal
                    break;
                } else {
                    firstSet.insert(first.at(symbol).begin(), first.at(symbol).end());
                    if (first.at(symbol).count(EPSILON) == 0)
                        break;
                }
            }

            if (firstSet.count(EPSILON)) {
                firstSet.erase(EPSILON);
                hasEpsilon = true;
            }

            string productionStr;
            for (const auto& symbol : rule)
                productionStr += symbol + " ";
            productionStr = trim2(productionStr);

            // Add the rule to the table for the corresponding terminal
            for (const string& terminal : firstSet)
                table[A][terminal] = productionStr;

            // Handle epsilon case for FOLLOW set
            if (hasEpsilon) {
                if (follow.find(A) == follow.end())
                    follow = readFollowSetsFromFile("followSets.txt");

                for (const string& terminal : follow[A])
                    table[A][terminal] = EPSILON;
            }
        }
    }

    // Step 2: Extract all non-terminals
    set<string> allNonTerminals;
    for (const auto& [nt, _] : productions)
        allNonTerminals.insert(nt);

    // Step 3: Output table header (X-axis: Terminals)
    cout << "\nTerminals ->\nNT\\T\t";
    outputFile << "\nTerminals ->\nNT\\T\t";
    for (const auto& terminal : terminalOrder) {
        cout << terminal << "\t";
        outputFile << terminal << "\t";
    }
    cout << endl;
    outputFile << endl;

    // Step 4: Output each row (Y-axis: Non-terminals)
    for (const auto& nonterminal : allNonTerminals) {
        cout << nonterminal << " |\t";
        outputFile << nonterminal << " |\t";

        for (const auto& terminal : terminalOrder) {
            if (table[nonterminal].find(terminal) != table[nonterminal].end()) {
                cout << table[nonterminal][terminal] << "\t";
                outputFile << table[nonterminal][terminal] << "\t";
            } else {
                cout << "-\t";
                outputFile << "-\t";
            }
        }

        cout << endl;
        outputFile << endl;
    }

    outputFile.close();
}
