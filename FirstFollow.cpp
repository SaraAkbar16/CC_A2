// Class to compute FIRST and FOLLOW sets

#include <string>
#include <vector>
#include <set>
#include <sstream>
#define EPSILON "Îµ"
#include <iostream>
#include <map>
using namespace std;

class FirstFollowSet {
private:
    map<string, vector<vector<string>>> productions;
    string startSymbol;
    
    // Helper function to compute FIRST set for a symbol
    void computeFirstHelper(const string& symbol) {
        if (!first[symbol].empty()) return;
    
        for (const auto& production : productions[symbol]) {
            bool epsilonInAll = true;
            for (const auto& sym : production) {
                if (productions.find(sym) == productions.end()) {
                    first[symbol].insert(sym);
                    epsilonInAll = false;
                    break;
                } else {
                    computeFirstHelper(sym);
                    first[symbol].insert(first[sym].begin(), first[sym].end());
                    if (first[sym].find("ε") == first[sym].end()) {
                        epsilonInAll = false;
                        break;
                    }
                }
            }
            if (epsilonInAll) {
                first[symbol].insert("ε");
            }
        }
    }
    
    // Helper function to compute FIRST of a string (sequence of symbols)
    set<string> firstOfString(const vector<string>& symbols, int index) {
        set<string> result;
        bool epsilonInAll = true;
        for (int i = index; i < symbols.size(); ++i) {
            const string& sym = symbols[i];
            if (productions.find(sym) == productions.end()) {
                result.insert(sym);
                epsilonInAll = false;
                break;
            } else {
                result.insert(first[sym].begin(), first[sym].end());
                if (first[sym].find("ε") == first[sym].end()) {
                    epsilonInAll = false;
                    break;
                }
            }
        }
        if (epsilonInAll) {
            result.insert("ε");
        }
        return result;
    }
    
    // Helper function to add sets and return if the set changed
    bool addSet(set<string>& dest, const set<string>& src) {
        bool changed = false;
        for (const string& sym : src) {
            if (dest.insert(sym).second) {
                changed = true;
            }
        }
        return changed;
    }
    map<string, set<string>> firstSets;
    map<string, set<string>> followSets;
public:
    map<string, set<string>> first; // Stores the FIRST sets
    map<string, set<string>> follow; // Stores the FOLLOW sets

    // Constructor
    FirstFollowSet(const map<string, vector<vector<string>>>& prods, const string& start)
        : productions(prods), startSymbol(start) {}

    // Function to compute all FIRST sets
    void computeAllFirst() {
        for (auto it = productions.begin(); it != productions.end(); ++it) {
            const string& symbol = it->first;
            computeFirstHelper(symbol);
        }
    }

    // Function to compute all FOLLOW sets
    void computeAllFollow() {
        // Initialize FOLLOW sets
        for (const auto& symbol : productions) {
            follow[symbol.first] = set<string>();
        }
        follow[startSymbol].insert("$"); // Rule 1: Add $ to the start symbol's FOLLOW set
    
        bool changed;
        do {
            changed = false;
            for (const auto& production : productions) {
                const string& A = production.first;
                const auto& rules = production.second;
                for (const auto& rule : rules) {
                    for (size_t i = 0; i < rule.size(); ++i) {
                        const string& B = rule[i];
                        if (productions.count(B)) { // B is a non-terminal
                            set<string> firstBeta;
                            bool epsilonInBeta = true;
                            if (i + 1 < rule.size()) {
                                const string& beta = rule[i + 1];
                                if (productions.count(beta)) {
                                    firstBeta = first[beta];
                                } else {
                                    firstBeta.insert(beta);
                                    epsilonInBeta = false;
                                }
                            }
                            // Remove ε from firstBeta if it exists
                            firstBeta.erase("ε");

                            // Add firstBeta to FOLLOW(B)
                            if (addSet(follow[B], firstBeta)) changed = true;
                            
                            // If ε is in firstBeta or B is the last symbol in the production
                            if (epsilonInBeta || i + 1 >= rule.size()) {
                                // Add FOLLOW(A) to FOLLOW(B)
                                if (addSet(follow[B], follow[A])) changed = true;
                            }
                        }
                    }
                }
            }
        } while (changed);
    }
    
    // Function to print FIRST sets
    void printFirstSets() {
        cout << "\nFIRST sets:\n";
        for (auto it = first.begin(); it != first.end(); ++it) {
            const string& symbol = it->first;
            const set<string>& firstSet = it->second;
            cout << "FIRST(" << symbol << ") = { ";
            for (const auto& val : firstSet) {
                cout << val << " ";
            }
            cout << "}" << endl;
        }
    }

    // Function to print FOLLOW sets
    void printFollowSets() {
        cout << "\nFOLLOW sets:\n";
        for (auto it = follow.begin(); it != follow.end(); ++it) {
            const string& symbol = it->first;
            const set<string>& followSet = it->second;
            cout << "FOLLOW(" << symbol << ") = { ";
            for (const auto& val : followSet) {
                cout << val << " ";
            }
            cout << "}" << endl;
        }
    }
    // Function to save FIRST sets to a file
void saveFirstSetsToFile(const string& filename) {
    ofstream outputFile(filename);
    outputFile << "FIRST sets:\n";
    for (const auto& entry : first) {
        outputFile << "FIRST(" << entry.first << ") = { ";
        for (const auto& val : entry.second) {
            outputFile << val << " ";
        }
        outputFile << "}\n";
    }
    outputFile.close();
}

// Function to save FOLLOW sets to a file
void saveFollowSetsToFile(const string& filename) {
        ofstream outputFile(filename);
        outputFile << "FOLLOW sets:\n";
        for (const auto& entry : follow) {
            // Only print sets that are not empty
            if (!entry.second.empty()) {
                outputFile << "FOLLOW(" << entry.first << ") = { ";
                for (const auto& val : entry.second) {
                    outputFile << val << " ";
                }
                outputFile << "}\n";
            } else {
                outputFile << "FOLLOW(" << entry.first << ") = { }\n";
            }
        }
        outputFile.close();
    }
    

// Add these inside FirstFollowSet class
const map<string, set<string>>& getFirstSets() const {
    return firstSets;
}

const map<string, set<string>>& getFollowSets() const {
    return followSets;
}

};
