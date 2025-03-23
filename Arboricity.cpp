#include <iostream>
#include <vector>
#include <bitset>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <thread>
#include <memory>
using namespace std;

// Define maximum number of vertices the graph can have
const int MAX_V = 10000000;

struct Graph {
    int V;
    vector<vector<int>> adjList;
    vector<bitset<MAX_V>> adjBits; // Adjacency as bitsets for faster intersection
    
    Graph(int v) : V(v), adjList(v), adjBits(v) {}
    
    void addEdge(int u, int v) {
        if (u >= V || v >= V) {
            cerr << "Error: Invalid edge (" << u << ", " << v << ") for a graph with " << V << " vertices." << endl;
            return;
        }
        adjList[u].push_back(v);
        adjList[v].push_back(u);
        adjBits[u].set(v);
        adjBits[v].set(u);
    }
};

// Better pivot selection strategy
int selectPivot(const bitset<MAX_V>& P, const bitset<MAX_V>& X, const Graph& G) {
    int maxCount = -1;
    int pivot = -1;
    
    // Consider vertices from both P and X for pivot selection
    bitset<MAX_V> candidates = P | X;
    
    for (int i = 0; i < G.V; i++) {
        if (candidates[i]) {
            // Count how many vertices in P are not neighbors of this vertex
            int count = (P & ~G.adjBits[i]).count();
            if (count > maxCount) {
                maxCount = count;
                pivot = i;
            }
        }
    }
    
    return pivot;
}

// Arboricity-based Clique Algorithm using bitsets
void ArboricityClique(bitset<MAX_V>& P, bitset<MAX_V>& R, bitset<MAX_V>& X, const Graph& G, 
                      map<int, int>& cliqueCounts, int& totalCliques) {
    if (P.none() && X.none()) {
        // Found a maximal clique
        int size = R.count();
        cliqueCounts[size]++;
        totalCliques++;
        return;
    }
    
    if (P.none()) return;
    
    // Choose pivot vertex with better strategy
    int pivot = selectPivot(P, X, G);
    
    // Get non-neighbors of pivot
    bitset<MAX_V> nonNeighbors = P & ~G.adjBits[pivot];
    
    // Iterate over non-neighbors of pivot
    for (int v = 0; v < G.V; v++) {
        if (nonNeighbors[v]) {
            // Add v to the current clique
            R.set(v);
            
            // Create new candidate and excluded sets by intersecting with neighbors of v
            bitset<MAX_V> P_new = P & G.adjBits[v];
            bitset<MAX_V> X_new = X & G.adjBits[v];
            
            // Recursive call
            ArboricityClique(P_new, R, X_new, G, cliqueCounts, totalCliques);
            
            // Remove v from current clique
            R.reset(v);
            
            // Move v from P to X
            P.reset(v);
            X.set(v);
        }
    }
}

// Find vertices with degeneracy ordering for better performance
vector<int> degeneracyOrdering(const Graph& G) {
    vector<int> order;
    vector<int> degree(G.V);
    vector<vector<int>> bins(G.V + 1);
    vector<bool> removed(G.V, false);
    
    // Initialize degrees
    for (int i = 0; i < G.V; i++) {
        degree[i] = G.adjList[i].size();
        bins[degree[i]].push_back(i);
    }
    
    // Process vertices in order of increasing degree
    for (int d = 0; d <= G.V; d++) {
        while (!bins[d].empty()) {
            // Get vertex with minimum degree
            int v = bins[d].back();
            bins[d].pop_back();
            
            if (removed[v]) continue;
            removed[v] = true;
            
            // Add to ordering
            order.push_back(v);
            
            // Update neighbors' degrees
            for (int neighbor : G.adjList[v]) {
                if (!removed[neighbor]) {
                    bins[degree[neighbor]].erase(
                        remove(bins[degree[neighbor]].begin(), bins[degree[neighbor]].end(), neighbor),
                        bins[degree[neighbor]].end()
                    );
                    degree[neighbor]--;
                    bins[degree[neighbor]].push_back(neighbor);
                }
            }
        }
    }
    
    // Reverse to get degeneracy ordering (highest core number first)
    reverse(order.begin(), order.end());
    return order;
}

int main() {
    // Use faster I/O
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    
    ifstream file("filename.txt");
    string line;
    vector<pair<int, int>> edges;
    int numVertices = 0;
    
    // Check if file is open
    if (!file.is_open()) {
        cerr << "Failed to open the file!" << endl;
        return 1;
    }
    
    // Reading the graph from the file - buffer for faster reading
    const int BUFFER_SIZE = 16384;
    unique_ptr<char[]> buffer(new char[BUFFER_SIZE]);
    file.rdbuf()->pubsetbuf(buffer.get(), BUFFER_SIZE);
    
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
    
    if (numVertices > MAX_V) {
        cerr << "Graph too large! Maximum supported vertices: " << MAX_V << endl;
        return 1;
    }
    
    Graph G(numVertices);
    for (auto& edge : edges) {
        G.addEdge(edge.first, edge.second);
    }
    
    map<int, int> cliqueCounts;  // Map to store counts of cliques by size
    int totalCliques = 0;
    
    cout << "Running Optimized Arboricity-based Clique Algorithm..." << endl;
    
    // Start timing
    auto start = chrono::high_resolution_clock::now();
    
    // Initialize bitsets
    bitset<MAX_V> R; // Current clique (initially empty)
    bitset<MAX_V> P; // Candidate vertices
    bitset<MAX_V> X; // Excluded vertices
    
    // Initialize P with all vertices using degeneracy ordering for better performance
    vector<int> ordering = degeneracyOrdering(G);
    for (int v : ordering) {
        P.set(v);
    }
    
    // Run the algorithm
    ArboricityClique(P, R, X, G, cliqueCounts, totalCliques);
    
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
