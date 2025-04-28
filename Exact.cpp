// exact.cpp
// Algorithm 1: Exact Method for Densest Subgraph Discovery (Generalized for h-clique, h=2)

#include <bits/stdc++.h>
#include <chrono>
using namespace std;
using namespace chrono;

const int INF = 1e9;

//--------------------------------------
// Dinic's Algorithm for Max Flow
//--------------------------------------
struct Edge {
    int to, rev;
    int cap;
};

class MaxFlow {
public:
    vector<vector<Edge>> G;
    vector<int> level, iter;

    MaxFlow(int n) {
        G.resize(n);
        level.resize(n);
        iter.resize(n);
    }

    void add_edge(int from, int to, int cap) {
        G[from].push_back({to, (int)G[to].size(), cap});
        G[to].push_back({from, (int)G[from].size()-1, 0});
    }

    void bfs(int s) {
        fill(level.begin(), level.end(), -1);
        queue<int> q;
        level[s] = 0;
        q.push(s);
        while(!q.empty()) {
            int v = q.front(); q.pop();
            for (auto &e : G[v]) {
                if (e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[v]+1;
                    q.push(e.to);
                }
            }
        }
    }

    int dfs(int v, int t, int upTo) {
        if (v == t) return upTo;
        for (int &i = iter[v]; i < G[v].size(); ++i) {
            Edge &e = G[v][i];
            if (e.cap > 0 && level[v] < level[e.to]) {
                int d = dfs(e.to, t, min(upTo, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    G[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }

    int max_flow(int s, int t) {
        int flow = 0;
        while(true) {
            bfs(s);
            if (level[t] < 0) break;
            fill(iter.begin(), iter.end(), 0);
            int f;
            while ((f = dfs(s, t, INF)) > 0)
                flow += f;
        }
        return flow;
    }
};

//--------------------------------------
// Densest Subgraph Discovery: Exact for h=2
//--------------------------------------
class DensestSubgraph {
public:
    int n, m;
    vector<pair<int, int>> edges;
    vector<vector<int>> adj;

    DensestSubgraph(int n): n(n), m(0) {
        adj.resize(n);
    }

    void add_edge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
        edges.push_back({u, v});
        m++;
    }

    vector<int> Exact() {
        double maxDeg = 0;
        for (int i = 0; i < n; ++i)
            maxDeg = max(maxDeg, (double)adj[i].size());
        double l = 0, u = maxDeg;

        vector<int> bestSubgraph;
        while (u - l >= 1.0 / (n * (n - 1))) {
            double alpha = (l + u) / 2;
            vector<int> sub = build_and_solve_flow(alpha);
            if (!sub.empty()) {
                l = alpha;
                bestSubgraph = sub;
            } else {
                u = alpha;
            }
        }
        return bestSubgraph;
    }

    vector<int> build_and_solve_flow(double alpha) {
        int S = n, T = n + 1;
        MaxFlow mf(n + 2);

        for (int v = 0; v < n; ++v) {
            mf.add_edge(S, v, adj[v].size());
            mf.add_edge(v, T, (int)(2 * alpha));
        }

        for (auto &[u, v] : edges) {
            mf.add_edge(u, v, 1);
            mf.add_edge(v, u, 1);
        }

        mf.max_flow(S, T);

        vector<int> vis(n + 2, 0);
        queue<int> q;
        q.push(S);
        vis[S] = 1;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (auto &e : mf.G[u]) {
                if (e.cap > 0 && !vis[e.to]) {
                    vis[e.to] = 1;
                    q.push(e.to);
                }
            }
        }

        vector<int> result;
        for (int i = 0; i < n; ++i)
            if (vis[i])
                result.push_back(i);

        if (result.empty() || result.size() == n)
            return {};
        return result;
    }
};

//--------------------------------------
// Main: Read yeast.txt
//--------------------------------------
int main() {
    string filename = "as-caida20071105_clean.edgelist"; // can change this filename
    ifstream fin(filename);
    if (!fin) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    int n, m;
    fin >> n >> m;
    DensestSubgraph G(n);

    for (int i = 0; i < m; ++i) {
        int u, v;
        fin >> u >> v;
        u--, v--;
        G.add_edge(u, v);
    }

    cout << "Graph Loaded: " << m << " edges and " << n << " vertices.\n";
    cout << "Computing the densest subgraph using Exact Algorithm...\n";

    auto start = high_resolution_clock::now();
    auto result = G.Exact();
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);

    int edge_count = 0;
    set<int> nodeSet(result.begin(), result.end());
    for (int u : result) {
        for (int v : G.adj[u]) {
            if (nodeSet.count(v))
                edge_count++;
        }
    }
    edge_count /= 2;

    double density = (result.size() == 0) ? 0 : (2.0 * edge_count) / result.size();

    cout << "Results:\n";
    cout << "Densest subgraph has " << result.size() << " vertices and " << edge_count << " edges\n";
    cout << fixed << setprecision(6);
    cout << "Density: " << density << "\n";
    cout << "Vertices in the densest subgraph: ";
    for (int u : result) cout << u+1 << " ";
    cout << "\n";

    int ms = duration.count();
    int minutes = ms / 60000;
    int seconds = (ms % 60000) / 1000;
    int millis = ms % 1000;

    cout << "Time taken: " << setfill('0') << setw(2) << minutes << ":"
         << setfill('0') << setw(2) << seconds << ":"
         << setfill('0') << setw(3) << millis << endl;

    return 0;
}