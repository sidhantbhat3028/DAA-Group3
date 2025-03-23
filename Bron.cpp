#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
using namespace std;

// Using unordered_set for faster lookups
struct Graph {
    int V;
    vector<unordered_set<int>> adjSet;
    
    Graph(int v) : V(v), adjSet(v) {}
    
    void addEdge(int u, int v) {
        if (u >= V || v >= V) {
            cerr << "Error: Invalid edge (" << u << ", " << v << ") for a graph with " << V << " vertices." << endl;
            return;
        }
        adjSet[u].insert(v);
        adjSet[v].insert(u);
    }
};

// Find the pivot vertex for the Bron-Kerbosch algorithm
int findPivot(const unordered_set<int>& P, const unordered_set<int>& X, const Graph& G) {
    int pivot = -1;
    int max_connections = -1;
    
    // Consider vertices from both P and X for pivot selection
    unordered_set<int> P_union_X;
    P_union_X.insert(P.begin(), P.end());
    P_union_X.insert(X.begin(), X.end());
    
    for (int u : P_union_X) {
        int connections = 0;
        
        // Count connections to vertices in P
        for (int v : P) {
            if (G.adjSet[u].count(v) > 0) {
                connections++;
            }
        }
        
        if (connections > max_connections) {
            max_connections = connections;
            pivot = u;
        }
    }
    
    return pivot;
}

// Optimized Bron-Kerbosch algorithm with pivoting
void BronKerboschWithPivot(unordered_set<int>& R, unordered_set<int>& P, unordered_set<int>& X, 
                          const Graph& G, map<int, int>& cliqueCounts, int& totalCliques) {
    if (P.empty() && X.empty()) {
        // Found a maximal clique
        int size = R.size();
        cliqueCounts[size]++;
        totalCliques++;
        return;
    }
    
    if (P.empty()) return;
    
    // Select a pivot vertex
    int pivot = findPivot(P, X, G);
    
    // Get neighbors of pivot for faster checking
    const unordered_set<int>& pivot_neighbors = G.adjSet[pivot];
    
    // Create a copy of P to iterate through (vertices that we will try to add to R)
    unordered_set<int> P_copy = P;
    
    for (int v : P_copy) {
        // Skip vertices that are neighbors of the pivot, as they will form cliques with the pivot
        if (pivot != v && pivot_neighbors.count(v) > 0) continue;
        
        // Add v to the current clique
        R.insert(v);
        
        // Create new candidate and excluded sets
        unordered_set<int> P_new, X_new;
        
        // P_new = P ∩ N(v)
        for (int u : P) {
            if (G.adjSet[v].count(u) > 0) {
                P_new.insert(u);
            }
        }
        
        // X_new = X ∩ N(v)
        for (int u : X) {
            if (G.adjSet[v].count(u) > 0) {
                X_new.insert(u);
            }
        }
        
        // Recursive call
        BronKerboschWithPivot(R, P_new, X_new, G, cliqueCounts, totalCliques);
        
        // Remove v from R for backtracking
        R.erase(v);
        
        // Move v from P to X
        P.erase(v);
        X.insert(v);
    }
}

// Optimized degeneracy ordering using bucket sort technique
vector<int> degeneracyOrdering(const Graph& G) {
    vector<int> ordering;
    vector<int> degree(G.V);
    vector<bool> removed(G.V, false);
    int max_degree = 0;
    
    // Calculate initial degrees and find maximum degree
    for (int v = 0; v < G.V; v++) {
        degree[v] = G.adjSet[v].size();
        max_degree = max(max_degree, degree[v]);
    }
    
    // Create buckets for vertices with each degree
    vector<vector<int>> buckets(max_degree + 1);
    for (int v = 0; v < G.V; v++) {
        buckets[degree[v]].push_back(v);
    }
    
    // Process vertices in order of increasing degree
    for (int d = 0; d <= max_degree; d++) {
        while (!buckets[d].empty()) {
            // Get a vertex with current minimum degree
            int v = buckets[d].back();
            buckets[d].pop_back();
            
            if (removed[v]) continue;  // Skip if already processed
            if (degree[v] > d) {       // If degree has changed, reinsert
                buckets[degree[v]].push_back(v);
                continue;
            }
            
            // Add to ordering and mark as removed
            ordering.push_back(v);
            removed[v] = true;
            
            // Update neighbors' degrees
            for (int u : G.adjSet[v]) {
                if (!removed[u] && degree[u] > 0) {
                    degree[u]--;
                }
            }
        }
    }
    
    return ordering;
}

// Bron-Kerbosch with degeneracy ordering
void BronKerboschDegeneracy(const Graph& G, map<int, int>& cliqueCounts, int& totalCliques) {
    vector<int> ordering = degeneracyOrdering(G);
    
    for (int i = 0; i < ordering.size(); i++) {
        int v = ordering[i];
        
        // R = {v}
        unordered_set<int> R = {v};
        
        // P = neighbors of v that come later in the ordering
        unordered_set<int> P;
        for (int u : G.adjSet[v]) {
            // Check if u comes after v in the ordering
            auto it = find(ordering.begin() + i + 1, ordering.end(), u);
            if (it != ordering.end()) {
                P.insert(u);
            }
        }
        
        // X = neighbors of v that come before in the ordering
        unordered_set<int> X;
        for (int u : G.adjSet[v]) {
            // Check if u comes before v in the ordering
            auto it = find(ordering.begin(), ordering.begin() + i, u);
            if (it != ordering.end()) {
                X.insert(u);
            }
        }
        
        // Run Bron-Kerbosch with pivot on this subgraph
        BronKerboschWithPivot(R, P, X, G, cliqueCounts, totalCliques);
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
    
    map<int, int> cliqueCounts;  // Map to store counts of cliques by size
    int totalCliques = 0;
    
    cout << "Running Optimized Bron-Kerbosch with Degeneracy Ordering..." << endl;
    
    // Start timing
    auto start = chrono::high_resolution_clock::now();
    
    // Run the algorithm
    BronKerboschDegeneracy(G, cliqueCounts, totalCliques);
    
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