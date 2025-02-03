#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "inputbuf.h"
#include "lexer.h"  
#include <algorithm>
#include <set>
#include <map>


using namespace std;

map<string, unordered_set<string>> followSets;
map<string, vector<vector<string>>> grammarRules;
// map<string, unordered_set<string>> firstSets;
map<string, set<string>> firstSets;
set<string> NTSet;
vector<string> allSymbols;  //has all grammar
vector<string> TOrder;
vector<string> NTOrder; 

void ReadGrammar() {
    LexicalAnalyzer lexer;
    Token tok;
    string currLHS;
    vector<string> currentRHS;
    
    while ((tok = lexer.GetToken()).token_type != HASH) {
        if (tok.token_type == ID) {
            //tok.Print();
            //if its the rhs is empty that means we havent hit arrow yet and gone to rhs
            if (currentRHS.empty()) {
                //cout << tok.lexeme<< " curr RHS is empty"<<endl;
                if (currLHS == ""){
                currLHS = tok.lexeme;
                }
                else {
                    currentRHS.push_back(tok.lexeme);

                }

                //add the lhs to both the nonterminal set and all symbol vector
                //tok.lexemewas currLHS
                if (find(allSymbols.begin(), allSymbols.end(), tok.lexeme) == allSymbols.end()) {
                    allSymbols.push_back(tok.lexeme);
                }

                NTSet.insert(currLHS);
                // cout<<"NT set:";
                // for (const auto& t : NTSet) {
                // cout << t << " ";
                //  }
                //  cout<<endl;
                //testing
                //if (currLHS != tok.lexeme){
                    // currentRHS.push_back(tok.lexeme);
                //}

                //currentRHS.push_back(tok.lexeme);
            } else {
            //    cout <<tok.lexeme<< " curr RHS isnt empty "<< currentRHS.size()<<endl;
                currentRHS.push_back(tok.lexeme);
                
                // if its the first time running into add to all symbols vector
                if (find(allSymbols.begin(), allSymbols.end(), tok.lexeme) == allSymbols.end()) {
                    allSymbols.push_back(tok.lexeme);
                }
            }
        } else if (tok.token_type == ARROW) {
            //skip arrow now we know we are in rhs
            continue;
        } else if (tok.token_type == STAR) {
            // cout<<currLHS<<" "<<endl;
            // cout<<currentRHS.size()<< " "<<currentRHS.front()<<endl;
            // push the rhs to the grammar rules map and have the key be the current lhs
            if (currentRHS.empty()){
                // cout<<"in empty"<<endl;
                currentRHS.push_back("#");
            }
            grammarRules[currLHS].push_back(currentRHS);
            currentRHS.clear();

            //testing
            currLHS = "";
        }
    }

    // get NT order from all symbols
    for (const auto& symbol : allSymbols) {
        // cout<<symbol<<" ";
        if (NTSet.find(symbol) != NTSet.end()) {
            NTOrder.push_back(symbol);
        } else {
            TOrder.push_back(symbol);
        }
    }
    // cout<<endl;
}


// Task 1: Print Terminals and Non-Terminals
void printTerminalsAndNonTerminals() {
    bool first = true;
    for (const auto& t : TOrder) {
        cout << t<< " ";
        first = false;
    }

    cout<< " ";


   first = true;
    for (const auto& nt : NTOrder) {
        cout << nt << " ";
        first = false;
    }

 
    cout<<" \n";

}

// Task 2
void RemoveUselessSymbols() {
    // Step 1: Identify generating non-terminals
    unordered_set<string> generatingSymbols;
    bool changed = true;

    while (changed) {
        changed = false;
        for (const auto& [lhs, rhsList] : grammarRules) {
            // Check if LHS is non-terminal and RHS is generating
            if (generatingSymbols.find(lhs) == generatingSymbols.end()) {
                for (const auto& rhs : rhsList) {
                    bool allGenerating = true;
                    for (const string& symbol : rhs) {
                        if (NTSet.find(symbol) != NTSet.end() && generatingSymbols.find(symbol) == generatingSymbols.end()) {
                            allGenerating = false;
                            break;
                        }
                    }
                    if (allGenerating) {
                        generatingSymbols.insert(lhs);
                        changed = true;
                        break;
                    }
                }
            }
        }
    }

    // Remove non-generating rules from grammarRules
    unordered_map<string, vector<vector<string>>> filteredRules;
    for (const auto& [lhs, rhsList] : grammarRules) {
        if (generatingSymbols.find(lhs) != generatingSymbols.end()) {
            vector<vector<string>> validRHS;
            for (const auto& rhs : rhsList) {
                bool isValid = true;
                for (const string& symbol : rhs) {
                    if (NTSet.find(symbol) != NTSet.end() && generatingSymbols.find(symbol) == generatingSymbols.end()) {
                        isValid = false;
                        break;
                    }
                }
                if (isValid) {
                    validRHS.push_back(rhs);
                }
            }
            if (!validRHS.empty()) {
                filteredRules[lhs] = validRHS;
            }
        }
    }

    // Step 2: Identify reachable non-terminals from the start symbol
    unordered_set<string> reachableSymbols;
    string startSymbol = allSymbols[0]; 
    reachableSymbols.insert(startSymbol);  // Start from the start symbol
    changed = true;

    while (changed) {
        changed = false;
        for (const auto& [lhs, rhsList] : filteredRules) {
            if (reachableSymbols.find(lhs) != reachableSymbols.end()) {
                for (const auto& rhs : rhsList) {
                    for (const string& symbol : rhs) {
                        if (NTSet.find(symbol) != NTSet.end() && reachableSymbols.find(symbol) == reachableSymbols.end()) {
                            reachableSymbols.insert(symbol);
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    // Update grammarRules with only reachable and generating symbols
    grammarRules.clear();
    for (const auto& [lhs, rhsList] : filteredRules) {
        if (reachableSymbols.find(lhs) != reachableSymbols.end()) {
            grammarRules[lhs] = rhsList;
        }
    }
}

// Print function to display only useful rules based on current grammarRules and allSymbols
void printUsefulRules() {
    for (const auto& symbol : allSymbols) {
        if (grammarRules.find(symbol) != grammarRules.end()) {
            for (const auto& rhs : grammarRules[symbol]) {
                cout << symbol << " -> ";
                for (const string& sym : rhs) {
                    cout << sym << " ";
                }
                cout << endl;
            }
        }
    }
}

void CalculateFirstSets() {
    //initialize first for terminals and epsilon
    firstSets["#"].insert("#"); 
    for (const auto& terminal : TOrder) {
        firstSets[terminal].insert(terminal); 
    }

    // initialize the firsts of nonterminals as empty
    for (const auto& nonTerminal : NTSet) {
        firstSets[nonTerminal] = {};
    }

    //go through rules 3 4 5
    bool changed;
    do {
        changed = false;
        // go through all rules
        for (const auto& [lhs, rhsList] : grammarRules) {
            for (const auto& rhs : rhsList) {
                size_t originalSize = firstSets[lhs].size(); 

                // if there is an epsilon the first of the next symbol is added to the current one
                bool allHaveEpsilon = true;
                for (const string& symbol : rhs) {
                    for (const string& firstElem : firstSets[symbol]) {
                        if (firstElem != "#") {
                            firstSets[lhs].insert(firstElem);
                        }
                    }

                    //if a epsilon is not found then set to false
                    if (firstSets[symbol].find("#") == firstSets[symbol].end()) {
                        allHaveEpsilon = false;
                        // cout<<"recognized epsilon"<<endl;
                        break;
                    }
                }

                // if all symbols find epsilon
                if (allHaveEpsilon) {
                    firstSets[lhs].insert("#");
                }

                // if any changes made then keep going if not then stop 
                if (firstSets[lhs].size() > originalSize) {
                    changed = true;
                }
            }
        }
    } while (changed);

    // // Step 4: Print the FIRST sets for non-terminals in the order they appeared
    // for (const auto& nonTerminal : NTOrder) {
    //     cout << "FIRST(" << nonTerminal << ") = { ";
    //     const auto& firstSet = firstSets[nonTerminal];
    //     for (auto it = firstSet.begin(); it != firstSet.end(); ++it) {
    //         if (it != firstSet.begin()) {
    //             cout << ", ";
    //         }
    //         cout << *it;
    //     }
    //     cout << " }" << endl;
    // }
}

void printFirstSets() {


    // print in NTOrder
    for (const auto& nonTerminal : NTOrder) {
        cout << "FIRST(" << nonTerminal << ") = { ";
        const auto& firstSet = firstSets[nonTerminal];
        for (auto it = firstSet.begin(); it != firstSet.end(); ++it) {
            if (it != firstSet.begin()) {
                cout << ", ";
            }
            cout << *it;
        }
        cout << " }" << endl;
    }
}

// Task 4
void CalculateFollowSets() {
    // all followsets must have $
    followSets[NTOrder[0]].insert("$"); // Rule I: Start symbol gets "$"

    // steps 4 and 5 first
    for (const auto& [lhs, rhsList] : grammarRules) {
        for (const auto& rhs : rhsList) {
            for (size_t i = 0; i < rhs.size(); ++i) {
                if (NTSet.find(rhs[i]) != NTSet.end()) { // If it's a non-terminal
                    // Rule IV: If B is followed by A1, A2, ..., Ak, add FIRST(A1) - {Ɛ} to FOLLOW(B)
                    if (i + 1 < rhs.size()) {
                        for (const auto& firstElem : firstSets[rhs[i + 1]]) {
                            if (firstElem != "#") { // Exclude epsilon
                                followSets[rhs[i]].insert(firstElem);
                            }
                        }
                    }

                    // Rule V: If all A1, A2, ..., Ak can derive epsilon, add FOLLOW(A) to FOLLOW(B)
                    bool allEpsilons = true;
                    for (size_t j = i + 1; j < rhs.size(); ++j) {
                        if (firstSets[rhs[j]].find("#") == firstSets[rhs[j]].end()) {
                            allEpsilons = false;
                            break;
                        }
                    }
                    if (allEpsilons || i + 1 == rhs.size()) {
                        for (const auto& followElem : followSets[lhs]) {
                            followSets[rhs[i]].insert(followElem);
                        }
                    }
                }
            }
        }
    }

    // do steps 2 and 3 until followsets dont change
    bool changed;
    do {
        changed = false;
        for (const auto& [lhs, rhsList] : grammarRules) {
            for (const auto& rhs : rhsList) {
                for (size_t i = 0; i < rhs.size(); ++i) {
                    if (NTSet.find(rhs[i]) != NTSet.end()) { 
                        size_t originalSize = followSets[rhs[i]].size();

                        // Rule II: If B is at the end, add FOLLOW(A) to FOLLOW(B)
                        if (i + 1 == rhs.size()) {
                            for (const auto& followElem : followSets[lhs]) {
                                followSets[rhs[i]].insert(followElem);
                            }
                        }

                        // Rule III: If B is followed by A1, A2, ..., Ak and they derive epsilon, add FOLLOW(A) to FOLLOW(B)
                        bool allEpsilons = true;
                        for (size_t j = i + 1; j < rhs.size(); ++j) {
                            if (firstSets[rhs[j]].find("#") == firstSets[rhs[j]].end()) {
                                allEpsilons = false;
                                break;
                            }
                        }
                        if (allEpsilons) {
                            for (const auto& followElem : followSets[lhs]) {
                                followSets[rhs[i]].insert(followElem);
                            }
                        }

                        // if followset grew set changed to true
                        if (followSets[rhs[i]].size() > originalSize) {
                            changed = true;
                        }
                    }
                }
            }
        }
    } while (changed);
}
    //print follow sets in NTOrder
    void printFollowSets() {
    for (const auto& nonTerminal : NTOrder) {
        cout << "FOLLOW(" << nonTerminal << ") = { ";
        const auto& followSet = followSets[nonTerminal];
        bool firstPrinted = false;

        if (followSet.find("$") != followSet.end()) {
            cout << "$";
            firstPrinted = true;
        }

        for (const auto& symbol : TOrder) {
            if (followSet.find(symbol) != followSet.end()) {
                if (firstPrinted) {
                    cout << ", ";
                }
                cout << symbol;
                firstPrinted = true;
            }
        }
        cout<<" }"<<endl;
    }
}

// Task 5
void CheckIfGrammarHasPredictiveParser() {
    bool isPredictive = true;

    for (const auto& [lhs, rhsList] : grammarRules) {
        // us sets to find combined first sets
        unordered_set<string> combinedFirsts;

        for (size_t i = 0; i < rhsList.size(); ++i) {
            const auto& rhs1 = rhsList[i];
            unordered_set<string> firstRHS1;

            //find first rhs1
            for (const string& symbol : rhs1) {
                firstRHS1.insert(firstSets[symbol].begin(), firstSets[symbol].end());

                if (firstSets[symbol].find("#") == firstSets[symbol].end()) {
                    break; 
                }
            }

            // make sure the firsts are a disjoint
            for (const string& symbol : firstRHS1) {
                if (combinedFirsts.count(symbol) > 0) {
                    isPredictive = false;
                    break;
                }
            }

            //combvine thge firsts
            combinedFirsts.insert(firstRHS1.begin(), firstRHS1.end());

            // check2: If FIRST(rhs1) contains epsilon, FIRST(rhs2) ∩ FOLLOW(lhs) = ∅
            if (firstRHS1.find("#") != firstRHS1.end()) {
                for (size_t j = i + 1; j < rhsList.size(); ++j) {
                    const auto& rhs2 = rhsList[j];
                    unordered_set<string> firstRHS2;

                    // find first rhs2
                    for (const string& symbol : rhs2) {
                        firstRHS2.insert(firstSets[symbol].begin(), firstSets[symbol].end());

                        if (firstSets[symbol].find("#") == firstSets[symbol].end()) {
                            break;  // Stop if epsilon is not in FIRST(symbol)
                        }
                    }

                    // see if FIRST(rhs2) ∩ FOLLOW(lhs) = ∅
                    for (const string& symbol : firstRHS2) {
                        if (symbol != "#" && followSets[lhs].count(symbol) > 0) {
                            isPredictive = false;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (isPredictive) {
        cout << "NO\n";
    } else {
        cout << "YES\n";
    }
}

    
int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);
    
    ReadGrammar();  // Reads the input grammar from standard input
                    // and represent it internally in data structures
                    // ad described in project 2 presentation file

    switch (task) {
        case 1: printTerminalsAndNonTerminals();
            break;

        case 2: RemoveUselessSymbols();
                printUsefulRules();
            break;

        case 3: 
            CalculateFirstSets();
             printFirstSets();
            break;

        case 4: 
            CalculateFirstSets();
            CalculateFollowSets();
            printFollowSets();
            break;

        case 5: 
            CheckIfGrammarHasPredictiveParser();
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}