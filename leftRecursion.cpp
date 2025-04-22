#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>

using namespace std;

#define EPSILON "ε"

struct Production {
    string lhs;
    vector<string> rhs;
};

// Read CFG from file and preserve order
vector<pair<string, Production>> readCFG(const string& filename) {
    vector<pair<string, Production>> cfg;
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return cfg;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string lhs, arrow, rest;
        ss >> lhs >> arrow;
        getline(ss, rest);
        rest = rest.substr(rest.find_first_not_of(" \t"));

        Production prod;
        prod.lhs = lhs;

        stringstream rhsStream(rest);
        string production;
        while (getline(rhsStream, production, '|')) {
            production = production.substr(production.find_first_not_of(" \t"));
            prod.rhs.push_back(production);
        }

        cfg.emplace_back(lhs, prod);
    }
    file.close();
    return cfg;
}

// Eliminate left recursion while preserving order
void eliminateLeftRecursion(vector<pair<string, Production>>& cfg) {
    vector<pair<string, Production>> newProds;

    for (auto& [nonTerminal, prod] : cfg) {
        vector<string> alpha, beta;

        for (const string& rhs : prod.rhs) {
            if (rhs.substr(0, nonTerminal.size()) == nonTerminal &&
                (rhs[nonTerminal.size()] == ' ' || rhs.size() == nonTerminal.size())) {
                alpha.push_back(rhs.substr(nonTerminal.size()));
                alpha.back() = alpha.back().substr(alpha.back().find_first_not_of(" \t"));
            } else {
                beta.push_back(rhs);
            }
        }

        if (!alpha.empty()) {
            string newNonTerminal = nonTerminal + "'";
            Production newProd;
            newProd.lhs = newNonTerminal;

            prod.rhs.clear();
            for (string& b : beta) {
                prod.rhs.push_back(b + " " + newNonTerminal);
            }

            for (string& a : alpha) {
                newProd.rhs.push_back(a + " " + newNonTerminal);
            }
            newProd.rhs.push_back(EPSILON);

            newProds.emplace_back(newNonTerminal, newProd);
        }
    }

    // Append new productions (A') at the end
    for (const auto& p : newProds) {
        cfg.push_back(p);
    }
}

// Print final grammar preserving order
void printCFG(const vector<pair<string, Production>>& cfg) {
    ofstream outputFile("finalGrammar.txt");
    if (!outputFile) {
        cerr << "Error: Could not open file for writing final grammar.\n";
        return;
    }

    cout << "\nFinal Grammar after removing left recursion:\n";
    outputFile << "Final Grammar after removing left recursion:\n";

    for (const auto& [nonTerminal, prod] : cfg) {
        cout << prod.lhs << " -> ";
        outputFile << prod.lhs << " -> ";

        for (size_t i = 0; i < prod.rhs.size(); ++i) {
            cout << prod.rhs[i];
            outputFile << prod.rhs[i];

            if (i < prod.rhs.size() - 1) {
                cout << " | ";
                outputFile << " | ";
            }
        }
        cout << endl;
        outputFile << endl;
    }

    outputFile.close();
}
