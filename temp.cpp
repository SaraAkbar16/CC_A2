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
    ifstream inputFile(filename);
    string line;

    while (getline(inputFile, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        size_t posStart = line.find("FOLLOW(");
        size_t posEnd = line.find(')', posStart);
        if (posStart == string::npos || posEnd == string::npos) continue;

        string nonTerminal = line.substr(posStart + 7, posEnd - posStart - 7);

        size_t braceStart = line.find('{', posEnd);
        size_t braceEnd = line.find('}', braceStart);
        if (braceStart == string::npos || braceEnd == string::npos) continue;

        string elements = line.substr(braceStart + 1, braceEnd - braceStart - 1);
        stringstream ss(elements);
        string token;
        set<string> followSet;

        while (ss >> token) {
            if (token != "{" && token != "}") {
                followSet.insert(token);
            }
        }

        followSets[nonTerminal] = followSet;
    }

    inputFile.close();
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

    for (const auto& [nonTerminal, productions] : formattedCFG) {
        for (const auto& production : productions) {
            set<string> firstOfProduction;
            bool epsilonInAll = true;

            for (const string& symbol : production) {
                if (firstSets.count(symbol)) {
                    for (const string& terminal : firstSets.at(symbol)) {
                        if (terminal != "ε") {
                            firstOfProduction.insert(terminal);
                        }
                    }
                    if (firstSets.at(symbol).find("ε") == firstSets.at(symbol).end()) {
                        epsilonInAll = false;
                        break;
                    }
                } else {
                    // Terminal symbol (not in FIRST sets)
                    firstOfProduction.insert(symbol);
                    epsilonInAll = false;
                    break;
                }
            }

            // Add production to parsing table for terminals in FIRST
            for (const string& terminal : firstOfProduction) {
                if (terminal != "ε") {
                    parsingTable[nonTerminal][terminal] = nonTerminal + " -> " + join(production, " ");
                }
            }

            // If production derives ε, use FOLLOW set (including $ if present)
            if (epsilonInAll || (production.size() == 1 && production[0] == "ε")) {
                for (const string& terminal : followSets.at(nonTerminal)) {
                    parsingTable[nonTerminal][terminal] = nonTerminal + " -> ε";
                }
            }
        }

        // Ensure $ is explicitly added to the parsing table if it exists in FOLLOW
        if (followSets.at(nonTerminal).count("$")) {
            if (parsingTable[nonTerminal].count("$") == 0) {
                parsingTable[nonTerminal]["$"] = ""; // Can initialize with an empty string or specific rule
            }
        }
    }

    return parsingTable;
}

void saveParsingTableToFile(const unordered_map<string, unordered_map<string, string>>& parsingTable, const string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "Error: Could not open file " << filename << " for writing." << endl;
        return;
    }

    // Collect all terminals
    unordered_set<string> terminals;
    for (const auto& row : parsingTable) {
        for (const auto& col : row.second) {
            terminals.insert(col.first);
        }
    }

    // Write the header row
    outFile << setw(15) << "Non-Terminal";
    for (const auto& terminal : terminals) {
        outFile << setw(15) << terminal;
    }
    outFile << endl;

    // Write the table contents
    for (const auto& row : parsingTable) {
        outFile << setw(15) << row.first;
        for (const auto& terminal : terminals) {
            auto it = row.second.find(terminal);
            if (it != row.second.end()) {
                outFile << setw(15) << it->second;
            } else {
                outFile << setw(15) << "";
            }
        }
        outFile << endl;
    }

    outFile.close();
    cout << "Parsing table saved to: " << filename << endl;
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

map<string, set<string>> readFirstSetsFromFile(const string& filename) {
    map<string, set<string>> firstSets;
    ifstream inputFile(filename);
    string line;

    while (getline(inputFile, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        size_t posStart = line.find("FIRST(");
        size_t posEnd = line.find(')', posStart);
        if (posStart == string::npos || posEnd == string::npos) continue;

        string nonTerminal = line.substr(posStart + 6, posEnd - posStart - 6);

        size_t braceStart = line.find('{', posEnd);
        size_t braceEnd = line.find('}', braceStart);
        if (braceStart == string::npos || braceEnd == string::npos) continue;

        string elements = line.substr(braceStart + 1, braceEnd - braceStart - 1);
        stringstream ss(elements);
        string token;
        set<string> firstSet;

        while (ss >> token) {
            if (token != "{" && token != "}") {
                firstSet.insert(token);
            }
        }
        

        firstSets[nonTerminal] = firstSet;
    }

    inputFile.close();
    return firstSets;
}
// Function to print FOLLOW sets
void printFollowSets(const map<string, set<string>>& followSets) {
    cout << "\nFOLLOW sets:\n";
    for (const auto& entry : followSets) {
        cout << "FOLLOW(" << entry.first << ") = { ";
        for (const auto& val : entry.second) {
            cout << val << " ";
        }
        cout << "}\n";
    }
}

// Function to print FIRST sets
void printFirstSets(const map<string, set<string>>& firstSets) {
        cout << "\nFIRST sets:\n";
        for (const auto& pair : firstSets) {
            cout << "FIRST(" << pair.first << ") = { ";
            for (const string& s : pair.second) {
                cout << s << " ";
            }
            cout << "}" << endl;
        }
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

    // // Write left-factored grammar to tempLeftFactored.txt while preserving order
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

    // // Read the left-factored CFG and eliminate left recursion
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


    // Compute First and Follow sets, then generate LL(1) table
    FirstFollowSet ff(formattedCFG, cfg.begin()->first);
     ff.computeAllFirst();
     ff.computeAllFollow();
     ff.printFirstSets();
     ff.printFollowSets();
     ff.saveFirstSetsToFile("FirstSets.txt");
     ff.saveFollowSetsToFile("FollowSets.txt");
     map<string, set<string>> followSetsFromFile = readFollowSetsFromFile("FollowSets.txt");

    // Read First sets from file (create a function similar to readFollowSetsFromFile)
    map<string, set<string>> firstSetsFromFile = readFirstSetsFromFile("FirstSets.txt");

// Print FOLLOW sets
printFollowSets(followSetsFromFile);
    
printFirstSets(firstSetsFromFile);

    // Generate LL(1) Parsing Table
   auto parsingTable = generateLL1ParsingTable(formattedCFG, firstSetsFromFile, followSetsFromFile);

    // Print the LL(1) parsing table
    printParsingTable(parsingTable);
    
// Save it to a file
saveParsingTableToFile(parsingTable, "parsing_table.txt");
    return 0;
}