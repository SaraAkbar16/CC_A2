#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
using namespace std;

// Utility function to join strings with a separator
string join(const vector<string>& vec, const string& separator) {
    string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i < vec.size() - 1) {
            result += separator;
        }
    }
    return result;
}

void constructLL1ParsingTable(
    unordered_map<string, vector<vector<string>>>& cfg,
    unordered_map<string, unordered_set<string>>& first_set,
    unordered_map<string, unordered_set<string>>& follow_set,
    unordered_map<string, unordered_map<string, vector<vector<string>>>>& parsing_table,
    unordered_set<string>& terminals,
    unordered_set<string>& nonTerminals
) {
    for (const auto& [nonTerminal, productions] : cfg) {
        for (const auto& production : productions) {
            unordered_set<string> first_alpha;
            bool containsEpsilon = true;

            // Compute FIRST(α) for α in A → α
            for (const string& symbol : production) {
                if (terminals.count(symbol)) {
                    if (symbol != "ε") {
                        first_alpha.insert(symbol);
                        containsEpsilon = false;
                        break;
                    }
                } else if (nonTerminals.count(symbol)) {
                    for (const string& s : first_set[symbol]) {
                        if (s != "ε") {
                            first_alpha.insert(s);
                        }
                    }
                    if (first_set[symbol].count("ε") == 0) {
                        containsEpsilon = false;
                        break;
                    }
                }
            }

            // Add A → α for each token in FIRST(α)
            for (const string& terminal : first_alpha) {
                parsing_table[nonTerminal][terminal].push_back(production);
            }

            // If ε is in FIRST(α), add A → α to FOLLOW(A)
            if (containsEpsilon) {
                for (const string& followSymbol : follow_set[nonTerminal]) {
                    parsing_table[nonTerminal][followSymbol].push_back(production);
                }
            }
        }
    }
}

void printLL1ParsingTable(unordered_map<string, unordered_map<string, vector<vector<string>>>>& parsing_table) {
    cout << "\nLL(1) Parsing Table:\n";
    cout << "-----------------------------------------\n";

    for (const auto& [nonTerminal, row] : parsing_table) {
        for (const auto& [terminal, productions] : row) {
            cout << nonTerminal << " at " << terminal << ": ";

            vector<string> formatted_productions;
            for (const auto& production : productions) {
                string prod_str = nonTerminal + " -> ";
                for (const string& symbol : production) {
                    prod_str += symbol;
                }
                formatted_productions.push_back(prod_str);
            }

            // Join productions using ", " for cleaner output
            cout << join(formatted_productions, ", ") << endl;
        }
    }

    cout << "-----------------------------------------\n";
}
