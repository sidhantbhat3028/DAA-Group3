#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>  // Required for setw and setfill
using namespace std;

struct Graph {
    int V;
    vector<vector<int>> adjList;
    
    Graph(int v) : V(v), adjList(v) {}
    
    void addEdge(int u, int v) {
        if (u >= V || v >= V) {
            cerr << "Error: Invalid edge (" << u << ", " << v << ") for a graph with " << V << " vertices." << endl;
            return;
        }
        adjList[u].push_back(v);
        adjList[v].push_back(u);
    }
};

// Find the pivot vertex for the Tomita algorithm
int findPivot(const set<int>& P, const set<int>& X, const Graph& G) {
    int pivot = -1;
    int max_intersect_size = -1;
    
    // Combine P and X
    set<int> P_union_X;
    P_union_X.insert(P.begin(), P.end());
    P_union_X.insert(X.begin(), X.end());
    
    // Find the pivot vertex with maximum intersection with P
    for (int u : P_union_X) {
        int intersect_size = 0;
        for (int v : G.adjList[u]) {
            if (P.count(v) > 0) {
                intersect_size++;
            }
        }
        if (intersect_size > max_intersect_size) {
            max_intersect_size = intersect_size;
            pivot = u;
        }
    }
    
    return pivot;
}

// Expand function for the Tomita et al. algorithm with pivoting
void Expand(set<int>& R, set<int>& P, set<int>& X, const Graph& G, map<int, int>& cliqueCounts, int& totalCliques) {
    if (P.empty() && X.empty()) {
        // Found a maximal clique
        int size = R.size();
        cliqueCounts[size]++;
        totalCliques++;
        return;
    }
    
    if (P.empty()) {
        return;
    }
    
    // Select a pivot vertex
    int pivot = findPivot(P, X, G);
    
    // Find vertices not adjacent to the pivot
    set<int> P_minus_NP;
    for (int v : P) {
        bool is_adj_to_pivot = false;
        for (int u : G.adjList[pivot]) {
            if (u == v) {
                is_adj_to_pivot = true;
                break;
            }
        }
        if (!is_adj_to_pivot) {
            P_minus_NP.insert(v);
        }
    }
    
    if (P_minus_NP.empty()) {
        // If all vertices in P are adjacent to pivot, choose any vertex from P
        P_minus_NP.insert(*P.begin());
    }
    
    // For each vertex not adjacent to the pivot
    for (int v : P_minus_NP) {
        // Add v to the current clique
        set<int> R_new = R;
        R_new.insert(v);
        
        // Create new candidates
        set<int> P_new;
        for (int u : G.adjList[v]) {
            if (P.count(u) > 0) {
                P_new.insert(u);
            }
        }
        
        // Create new excluded
        set<int> X_new;
        for (int u : G.adjList[v]) {
            if (X.count(u) > 0) {
                X_new.insert(u);
            }
        }
        
        // Recursive call
        Expand(R_new, P_new, X_new, G, cliqueCounts, totalCliques);
        
        // Move v from P to X
        P.erase(v);
        X.insert(v);
    }
}

int main() {
    ifstream file("filename.txt");
    string line;
    vector<pair<int, int>> edges;
    int numVertices = 0;
    
    // Check if file is open
    if (!file.is_open()) {
        cerr << "Failed to open the file!" << endl;
        return 1;
    }
    
    // Reading the graph from the file
    while (getline(file, line)) {
        // Skip empty lines or lines starting with comment
        if (line.empty() || line[0] == '#') continue;
        
        stringstream ss(line);
        int u, v;
        ss >> u >> v;
        
        // Check if the edge was successfully read
        if (ss.fail()) {
            cerr << "Failed to read edge: " << line << endl;
            continue;  // Skip invalid lines
        }
        
        edges.push_back({u, v});
        numVertices = max(numVertices, max(u, v) + 1);  // Update the number of vertices
    }
    
    // Print the number of edges and vertices
    cout << "Graph Loaded: " << edges.size() << " edges and " << numVertices << " vertices." << endl;
    
    if (edges.empty()) {
        cerr << "No valid edges found in the file!" << endl;
        return 1;
    }
    
    // Ensure valid number of vertices
    if (numVertices <= 0) {
        cerr << "Invalid number of vertices!" << endl;
        return 1;
    }
    
    Graph G(numVertices);
    for (auto& edge : edges) {
        G.addEdge(edge.first, edge.second);
    }
    
    // Run the Tomita et al. algorithm
    set<int> R;  // Current clique (initially empty)
    set<int> P;  // Candidate vertices
    set<int> X;  // Excluded vertices
    
    // Initialize P with all vertices
    for (int i = 0; i < numVertices; i++) {
        P.insert(i);
    }
    
    map<int, int> cliqueCounts;  // Map to store counts of cliques by size
    int totalCliques = 0;
    
    cout << "Running Tomita et al.'s Maximal CLIQUE Algorithm..." << endl;
    
    // Start timing
    auto start = chrono::high_resolution_clock::now();
    
    // Run the algorithm
    Expand(R, P, X, G, cliqueCounts, totalCliques);
    
    // End timing
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    
    // Convert to hours, minutes, seconds, milliseconds
    int hours = static_cast<int>(elapsed.count()) / 3600;
    int minutes = (static_cast<int>(elapsed.count()) % 3600) / 60;
    int seconds = static_cast<int>(elapsed.count()) % 60;
    int milliseconds = static_cast<int>(elapsed.count() * 1000) % 1000;
    
    // Print clique size counts
    cout << "Clique size counts:" << endl;
    for (const auto& pair : cliqueCounts) {
        cout << "Size " << pair.first << ": " << pair.second << " cliques" << endl;
    }
    
    cout << "Total number of maximal cliques: " << totalCliques << endl;
    
    // Format time as HH:MM:SS:MMM
    cout << "Time taken: ";
    cout << setfill('0') << setw(2) << hours << ":";
    cout << setfill('0') << setw(2) << minutes << ":";
    cout << setfill('0') << setw(2) << seconds << ":";
    cout << setfill('0') << setw(3) << milliseconds << endl;
    
    return 0;
}