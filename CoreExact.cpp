#include <bits/stdc++.h>
using namespace std;

const int INF = 1e9;

class Graph {
public:
    int n;
    vector<vector<int>> adj;
    
    Graph(int n) : n(n), adj(n) {}

    void add_edge(int u, int v) {
        if (u < 0 || u >= n || v < 0 || v >= n) {
            cerr << "Invalid edge: (" << u+1 << ", " << v+1 << ")" << endl;
            return;
        }
        // Add edge only if not already present
        if (find(adj[u].begin(), adj[u].end(), v) == adj[u].end()) {
            adj[u].push_back(v);
            if (u != v) {  // Avoid duplicate for self-loops
                adj[v].push_back(u);
            }
        }
    }
    
    // Count triangles (3-cycles) in the graph
    int count_triangles() const {
        int count = 0;
        vector<vector<bool>> neighbor_matrix(n, vector<bool>(n, false));
        
        // Precompute neighbor matrix
        for (int u = 0; u < n; u++) {
            for (int v : adj[u]) {
                neighbor_matrix[u][v] = true;
            }
        }
        
        for (int u = 0; u < n; u++) {
            for (size_t i = 0; i < adj[u].size(); i++) {
                int v = adj[u][i];
                if (v > u) { // Count each triangle only once
                    for (size_t j = 0; j < adj[v].size(); j++) {
                        int w = adj[v][j];
                        if (w > v && neighbor_matrix[u][w]) {
                            count++;
                        }
                    }
                }
            }
        }
        return count;
    }
    
    // Count edges (2-cliques)
    int count_2cliques() const {
        int count = 0;
        for (int u = 0; u < n; u++) {
            for (int v : adj[u]) {
                if (v > u) count++; // Count each edge only once
            }
        }
        return count;
    }
    
    // Count edges within a subgraph
    int count_internal_edges(const vector<bool>& in_subgraph) const {
        int edges = 0;
        for (int u = 0; u < n; u++) {
            if (in_subgraph[u]) {
                for (int v : adj[u]) {
                    if (v > u && in_subgraph[v]) { // Count each edge only once
                        edges++;
                    }
                }
            }
        }
        return edges;
    }
    
    // Calculate average degree density for a given subgraph (edges/vertices)
    double calculate_avg_degree_density(const vector<bool>& in_subgraph) const {
        int edges = count_internal_edges(in_subgraph);
        int vertices = 0;
        
        for (int i = 0; i < n; i++) {
            if (in_subgraph[i]) {
                vertices++;
            }
        }
        
        if (vertices == 0) return 0.0;
        return static_cast<double>(edges) / vertices;
    }
    
    // Count edges both inside and outside a subgraph
    pair<int, int> count_subgraph_edges(const vector<bool>& in_subgraph) const {
        int internal = 0;
        int external = 0;
        
        for (int u = 0; u < n; u++) {
            if (in_subgraph[u]) {
                for (int v : adj[u]) {
                    if (v > u) { // Count each edge only once
                        if (in_subgraph[v]) {
                            internal++;
                        } else {
                            external++;
                        }
                    }
                }
            }
        }
        
        return {internal, external};
    }
    
    // Helper function for core decomposition
    vector<int> core_decomposition() const {
        vector<int> degree(n);
        vector<int> core(n, 0);
        
        // Calculate initial degrees
        for (int u = 0; u < n; u++) {
            degree[u] = adj[u].size();
        }
        
        // Find maximum degree
        int max_deg = *max_element(degree.begin(), degree.end());
        
        // Create bins
        vector<vector<int>> bin(max_deg + 1);
        for (int u = 0; u < n; u++) {
            bin[degree[u]].push_back(u);
        }
        
        // Process vertices in order of increasing degree
        for (int d = 0; d <= max_deg; d++) {
            while (!bin[d].empty()) {
                int u = bin[d].back();
                bin[d].pop_back();
                
                if (core[u] != 0) continue; // Already processed
                
                core[u] = d;
                
                // Update neighbors
                for (int v : adj[u]) {
                    if (core[v] == 0 && degree[v] > d) {
                        // Find and remove v from its current bin
                        auto& v_bin = bin[degree[v]];
                        auto it = find(v_bin.begin(), v_bin.end(), v);
                        if (it != v_bin.end()) {
                            *it = v_bin.back();
                            v_bin.pop_back();
                        }
                        
                        // Decrease degree and add to new bin
                        degree[v]--;
                        bin[degree[v]].push_back(v);
                    }
                }
            }
        }
        
        return core;
    }
    
    // Find densest subgraph using the greedy algorithm
    pair<vector<int>, pair<int, int>> find_densest_subgraph() const {
        if (n == 0) return {{}, {0, 0}};
        
        // Use a greedy algorithm for approximate densest subgraph
        // Store all nodes initially
        vector<bool> in_subgraph(n, true);
        vector<bool> best_subgraph = in_subgraph;
        double best_density = calculate_avg_degree_density(in_subgraph);
        
        // Calculate degree of each vertex in original graph
        vector<int> degree(n);
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        
        for (int u = 0; u < n; u++) {
            degree[u] = adj[u].size();
            pq.push({degree[u], u});
        }
        
        int remaining_vertices = n;
        
        // Iteratively remove the vertex with minimum degree
        while (!pq.empty() && remaining_vertices > 0) {
            int u = pq.top().second;
            pq.pop();
            
            if (!in_subgraph[u]) continue; // Already removed
            
            // Remove this vertex
            in_subgraph[u] = false;
            remaining_vertices--;
            
            if (remaining_vertices == 0) break;
            
            // Calculate density of current subgraph
            double density = calculate_avg_degree_density(in_subgraph);
            
            // Update best subgraph if needed
            if (density > best_density) {
                best_density = density;
                best_subgraph = in_subgraph;
            }
            
            // Update degrees of neighbors
            for (int v : adj[u]) {
                if (in_subgraph[v]) {
                    degree[v]--;
                    pq.push({degree[v], v});
                }
            }
        }
        
        // Convert boolean vector to list of vertices
        vector<int> result;
        for (int u = 0; u < n; u++) {
            if (best_subgraph[u]) {
                result.push_back(u);
            }
        }
        
        // Count edges
        auto [internal, external] = count_subgraph_edges(best_subgraph);
        
        return {result, {internal, external}};
    }
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    ifstream infile("graph.txt");
    if (!infile) {
        cerr << "Error: Cannot open graph.txt" << endl;
        return 1;
    }
    
    int n, m;
    infile >> n >> m;
    
    if (n <= 0 || m < 0) {
        cerr << "Invalid graph size: " << n << " nodes, " << m << " edges" << endl;
        return 1;
    }
    
    Graph g(n);
    
    for (int i = 0; i < m; i++) {
        int u, v;
        if (!(infile >> u >> v)) {
            cerr << "Error reading edge #" << i << endl;
            break;
        }
        
        // Convert to 0-indexed
        u--; v--;
        
        if (u < 0 || u >= n || v < 0 || v >= n) {
            cerr << "Invalid edge: (" << u+1 << ", " << v+1 << ")" << endl;
            continue;
        }
        
        g.add_edge(u, v);
    }
    
    // Count triangles and 2-cliques
    int triangles = g.count_triangles();
    int cliques = g.count_2cliques();
    
    cout << "Found " << cliques << " (2)-cliques" << endl;
    cout << "Found " << triangles << " triangles" << endl;
    
    // Find densest subgraph
    auto [vertices, edges] = g.find_densest_subgraph();
    
    cout << "Densest subgraph found with " << vertices.size() << " vertices. Edges: ";
    cout << "(" << edges.first << ", " << edges.second << ")" << endl;
    
    if (!vertices.empty()) {
        cout << "Densest Subgraph Vertices (0-indexed):" << endl;
        for (int v : vertices) {
            cout << v << " ";
        }
        cout << endl;
    }
    
    return 0;
}
