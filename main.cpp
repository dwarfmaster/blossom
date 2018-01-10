
#include <iostream>
#include <vector>
#include <list>
#include <cstring>
#include <cassert>
#include <unordered_set>
#include <algorithm>

using namespace std;

//  _   _       _                   _____ _           _ 
// | | | |_ __ (_) ___  _ __       |  ___(_)_ __   __| |
// | | | | '_ \| |/ _ \| '_ \ _____| |_  | | '_ \ / _` |
// | |_| | | | | | (_) | | | |_____|  _| | | | | | (_| |
//  \___/|_| |_|_|\___/|_| |_|     |_|   |_|_| |_|\__,_|
//                                                      

template <typename T>
struct uf_item {
    T value;
    size_t rk;
    size_t parent;
    // The next item in the current subset
    size_t fchild;
    // The last item in the current subset, valid only for the parent
    // Used to keep join constant time
    size_t lchild;
};

// a(n) is the inverse Ackermann function
template <typename T>
class uf_data {
    uf_item<T>* m_data;
    size_t m_size;

    public:
        uf_data() = delete;

        // O(n)
        uf_data(size_t n)
        : m_data(nullptr), m_size(n)
        {
            assert(m_size > 0);
            m_data = new uf_item<T>[m_size];
            for(size_t i = 0; i < m_size; ++i) {
                m_data[i].rk   = 0;
                m_data[i].parent = i;
                m_data[i].fchild = -1;
                m_data[i].lchild = -1;
            }
        }

        // O(cp.m_size)
        uf_data(const uf_data& cp)
        : uf_data(cp.m_size)
        {
            memcpy(m_data, cp.m_data, m_size * sizeof(uf_item<T>));
        }

        // O(1)
        uf_data(uf_data&& cp)
        : m_data(cp.m_data), m_size(cp.m_size)
        {
            cp.m_data = nullptr;
            cp.m_size = 0;
        }

        // O(m_size)
        ~uf_data() {
            if(m_size > 0) {
                delete[] m_data;
            }
        }

        // O(a(m_size))
        size_t parent(size_t i) {
            assert(m_size > 0);
            if(m_data[i].parent == i) return i;

            m_data[i].parent = parent(m_data[i].parent);
            return m_data[i].parent;
        }

        // O(1)
        void attach(size_t pr, size_t ch) {
            m_data[m_data[pr].lchild].fchild = m_data[ch].fchild;
            m_data[pr].lchild = m_data[ch].lchild;
        }

        // O(a(m_size))
        void join(size_t i, size_t j) {
            size_t pi = parent(i);
            size_t pj = parent(j);
            if(pi == pj) return;

            if(m_data[pi].rk < m_data[pj].rk) {
                m_data[pi].parent = pj;
                attach(pj, pi);
            } else if(m_data[pi].rk > m_data[pj].rk) {
                m_data[pj].parent = pi;
                attach(pi, pj);
            } else {
                m_data[pi].parent = pj;
                attach(pj, pi);
                ++m_data[pj].rk;
            }
        }

        // O(1)
        T& get(size_t i) {
            return m_data[i].value;
        }

        // Returns -1 if i was the last element from the set
        // O(1)
        size_t next(size_t i) {
            return m_data[i].fchild;
        }
};

//  ____  _                               
// | __ )| | ___  ___ ___  ___  _ __ ____
// |  _ \| |/ _ \/ __/ __|/ _ \| '_ ` _  | 
// | |_) | | (_) \__ \__ \ (_) | | | | | |
// |____/|_|\___/|___/___/\___/|_| |_| |_|
//                                        

struct Edge {
    size_t u, v;
    bool matched;

    size_t other(size_t n) {
        return (n == u) ? v : u;
    }

    bool has(size_t n) {
        return (n == u) || (n == v);
    }
};

struct Node {
    bool erased;
    vector<size_t> edges;
    size_t matcher;
    bool matched;

    Node()
    : erased(false), edges(), matched(false)
    {}
};

struct Graph {
    vector<Edge> edges;
    vector<Node> nodes;
    unordered_set<size_t> unmatched;

    Graph() = delete;

    Graph(size_t n)
    : edges(), nodes(n)
    {}
};

void print_matching(const Graph& g) {
    for(auto e = g.edges.begin(); e != g.edges.end(); ++e) {
        if(e->matched) cout << e->u << " -- " << e->v << endl;
    }
}

struct Cycle {
    list<size_t> edges;
    size_t in_edge;
};
struct TreeNode {
    size_t prec; // Edge
    size_t dist_to_root;
    bool in;
    bool A, B;
};

// The function is recursively called O(n) times, because gr.unmatched decrease
// in cardinality every time it is called
// Initialisation is in O(n) = O(m), computation in O(n) = O(m) are done before
// every recursive call
// Otherwise, the main loop is O(m) since every edge is added at most twice to
// L during an execution. The main loop body is O(a(n)) amortised where a(n)
// is the inversed Ackermann function.
// Thus the function is O(nm * a(n))
void blossom(Graph& gr) {
    if(gr.unmatched.empty()) return;
    bool recurse_after = false;

    // Initialisation is O(n) = O(m)
    size_t n = gr.nodes.size();
    list<Cycle> contractions;
    vector<TreeNode> T(n);
    list<size_t> L;
    // The data is the matched node of the compressed one if it is matched
    uf_data<char> compress(n);

    for(size_t i = 0; i < n; ++i) {
        T[i].A  = false;
        T[i].B  = false;
        T[i].in = false;
    }

    size_t root = *gr.unmatched.begin();
    T[root].A = false;
    T[root].B = true;
    T[root].in = true;
    T[root].dist_to_root = 0;
    T[root].prec = -1;
    for(auto it = gr.nodes[root].edges.begin();
            it != gr.nodes[root].edges.end(); ++it) {
        L.push_back(*it);
    }

    // We loop at most O(m) times
    while(!L.empty()) {
        size_t edge = L.front(); L.pop_front(); // O(1)
        size_t uorig = gr.edges[edge].u;
        size_t vorig = gr.edges[edge].v;
        size_t u = compress.parent(uorig); // O(a(n))
        size_t v = compress.parent(vorig); // O(a(n))

        // Nodes in the same compressed cycle
        if(u == v)                                        continue; // O(1)
        // Impossible because when an edge is added to T, it has one node in B
        else if(!T[u].B && !T[v].B)                       continue; // O(1)
        // One of the nodes has been deleted
        else if(gr.nodes[u].erased || gr.nodes[v].erased) continue; // O(1)


        // We found an augmenting path
        // Test O(1)
        else if((T[u].B && !T[v].in && !gr.nodes[v].matched)
                || (T[v].B && !T[u].in && !gr.nodes[u].matched)) {
            // Swap O(1)
            if(T[u].B) {
                size_t tmp = u;
                u = v;
                v = tmp;
                tmp = uorig;
                uorig = vorig;
                vorig = tmp;
            }

            // Init O(1)
            list<size_t> edges;
            edges.push_back(edge);
            size_t node = v;

            cout << "Found augmenting path : " << u << " -> " << v;

            // Discover path O(n)
            while(gr.nodes[node].matched) {
                size_t e = T[node].prec;
                cout << "(" << e << ")";
                edges.push_back(e);
                node = gr.edges[e].other(node);
                cout << " -> " << node << flush;
                node = compress.parent(node);
                cout << "[" << node << "]";
            }
            cout << endl;

            // Augment along path on compressed graph O(n)
            gr.nodes[u].matched = true;
            gr.nodes[u].matcher = edge;
            for(auto edg = edges.begin(); edg != edges.end(); ++edg) {
                gr.edges[*edg].matched = !gr.edges[*edg].matched;
                if(gr.edges[*edg].matched) {
                    size_t node = gr.edges[*edg].u;
                    gr.nodes[compress.parent(node)].matched = true;
                    gr.nodes[compress.parent(node)].matcher = *edg;
                    node = gr.edges[*edg].v;
                    gr.nodes[compress.parent(node)].matched = true;
                    gr.nodes[compress.parent(node)].matcher = *edg;
                }
            }
            gr.unmatched.erase(root);
            gr.unmatched.erase(u);

            // Expand cycles first and recurse afterward
            recurse_after = true;
            break;
        }


        // We can increase one of our alternating path
        // Test O(1)
        else if((T[u].B && !T[v].in && gr.nodes[v].matched)
                || (T[v].B && !T[u].in && gr.nodes[u].matched)) {
            // Swap O(1)
            if(T[v].B) {
                size_t tmp = u;
                u = v;
                v = tmp;
            }

            T[v].in = true;
            T[v].A  = true;
            T[v].B  = false;
            T[v].prec = edge;
            T[v].dist_to_root = T[u].dist_to_root + 1;

            Edge& nedge = gr.edges[gr.nodes[v].matcher];
            size_t w = nedge.other(v);
            T[w].in = true;
            T[w].A  = false;
            T[w].B  = true;
            T[w].prec = gr.nodes[v].matcher;
            T[w].dist_to_root = T[v].dist_to_root + 1;

            // O(d_u), executed at most one for every u, so O(n+m) = O(m) for
            // the whole execution loop
            for(auto it = gr.nodes[w].edges.begin();
                    it != gr.nodes[w].edges.end(); ++it) {
                L.push_back(*it);
            }
        }


        // Found an odd cycle, let's compress it
        // Test O(1)
        else if(T[u].B && T[v].B) {
            // Init O(1)
            size_t u2 = u, v2 = v;
            Cycle c;
            c.edges.push_back(edge);
            list<size_t> nodes;
            nodes.push_back(u); nodes.push_back(v);

            // Discover cycle
            // Complete complexity O(size of cycle), so the total complexity
            // over all iterations is O(sum of size of all cycles), which is 
            // O(n) = O(m)
            while(T[u2].dist_to_root > T[v2].dist_to_root) {
                c.edges.push_back(T[u2].prec);
                u2 = gr.edges[T[u2].prec].other(u2);
                nodes.push_front(u2);
            }
            while(T[v2].dist_to_root > T[u2].dist_to_root) {
                c.edges.push_front(T[v2].prec);
                v2 = gr.edges[T[v2].prec].other(v2);
                nodes.push_back(v2);
            }
            while(u2 != v2) {
                c.edges.push_back(T[u2].prec);
                c.edges.push_front(T[v2].prec);
                u2 = gr.edges[T[u2].prec].other(u2);
                v2 = gr.edges[T[v2].prec].other(v2);
                nodes.push_front(u2);
                nodes.push_back(v2);
            }
            nodes.pop_front();
            c.in_edge = T[u2].prec; // == T[v2].prec
            contractions.push_back(c);

            // Add outgoing edges to L
            // Complexity over all execution O(m * a(n))
            cout << "Found odd cycle ";
            for(auto nd = nodes.begin(); nd != nodes.end(); ++nd) {
                cout << *nd << " -- ";
                compress.join(u, *nd);
                for(auto ed = gr.nodes[*nd].edges.begin();
                        ed != gr.nodes[*nd].edges.end(); ++ed) {
                    L.push_back(*ed);
                }
            }
            cout << endl;

            // Unmatch all edges and nodes of the cycle, except the parent
            // O(size of cycle), same remark as above
            for(auto edg = c.edges.begin(); edg != c.edges.end(); ++edg) {
                gr.edges[*edg].matched = false;
                gr.nodes[gr.edges[*edg].u].matched = false;
                gr.nodes[gr.edges[*edg].v].matched = false;
            }

            // Compress it
            // O(a(n))
            size_t prt = compress.parent(u);
            gr.nodes[prt].matched = (u2 != root);
            gr.nodes[prt].matcher = c.in_edge;
            T[prt].prec = T[u2].prec;
            T[prt].dist_to_root = T[u2].dist_to_root;
            T[prt].A = false;
            T[prt].B = true;
        }

        // The last case is when the edge is between B(T) and A(T) : we ignore
        // it in this case because it doesn't add anything to the search of an
        // augmenting path
    }

    if(!recurse_after) {
        // Remove nodes in V(T) from G
        // O(n)
        for(size_t nd = 0; nd < n; ++nd) {
            gr.nodes[nd].erased = gr.nodes[nd].erased || T[nd].in;
        }
        gr.unmatched.erase(root);
        blossom(gr);
    }


    // Reset node matching anotations O(m)
    for(size_t nd = 0; nd < n; ++nd) gr.nodes[nd].matched = false;
    for(size_t edg = 0; edg < gr.edges.size(); ++edg) {
        if(gr.edges[edg].matched) {
            gr.nodes[gr.edges[edg].u].matched = true;
            gr.nodes[gr.edges[edg].u].matcher = edg;
            gr.nodes[gr.edges[edg].v].matched = true;
            gr.nodes[gr.edges[edg].v].matcher = edg;
        }
    }

    // Expand compressed odd cycles O(\sum |c|) = O(n)
    for(auto c = contractions.rbegin(); c != contractions.rend(); ++c) {
        // Find matched node in cycle O(|c|)
        vector<size_t> mnodes; // mnodes.size() <= 2 over all execution
        for(auto edg = c->edges.begin(); edg != c->edges.end(); ++edg) {
            size_t u = gr.edges[*edg].u;
            size_t v = gr.edges[*edg].v;

            if(gr.nodes[u].matched) mnodes.push_back(u);
            if(gr.nodes[v].matched) mnodes.push_back(v);
        }
        if(mnodes.empty()) mnodes.push_back(gr.edges[*c->edges.begin()].u);

        // Set the matching on the cycle O(|c|)
        long long dist = -1;
        for(auto edg = c->edges.begin(); edg != c->edges.end(); ++edg) {
            if(any_of(mnodes.begin(), mnodes.end(), 
                        [&] (size_t v) { return gr.edges[*edg].has(v); })) {
                dist = 0;
                gr.edges[*edg].matched = false;
            } else if(dist >= 0) {
                ++dist;
                if(dist % 2 == 1) {
                    gr.edges[*edg].matched = true;
                    gr.nodes[gr.edges[*edg].u].matched = true;
                    gr.nodes[gr.edges[*edg].u].matcher = *edg;
                    gr.nodes[gr.edges[*edg].v].matched = true;
                    gr.nodes[gr.edges[*edg].v].matcher = *edg;
                } else {
                    gr.edges[*edg].matched = false;
                }
            }
        }
        dist = -1;
        for(auto edg = c->edges.rbegin(); edg != c->edges.rend(); ++edg) {
            if(any_of(mnodes.begin(), mnodes.end(), 
                        [&] (size_t v) { return gr.edges[*edg].has(v); })) {
                dist = 0;
                gr.edges[*edg].matched = false;
            } else if(dist >= 0) {
                ++dist;
                if(dist % 2 == 1) {
                    gr.edges[*edg].matched = true;
                    gr.nodes[gr.edges[*edg].u].matched = true;
                    gr.nodes[gr.edges[*edg].u].matcher = *edg;
                    gr.nodes[gr.edges[*edg].v].matched = true;
                    gr.nodes[gr.edges[*edg].v].matcher = *edg;
                } else {
                    gr.edges[*edg].matched = false;
                }
            }
        }
    }

    if(recurse_after) blossom(gr);
}

//  __  __       _       
// |  \/  | __ _(_)_ ___  
// | |\/| |/ _` | | '_  |
// | |  | | (_| | | | | |
// |_|  |_|\__,_|_|_| |_|
//                       

int main(int argc, char *argv[]) {
    if(argc && argv) {}

    size_t n;
    cin >> n;
    Graph g(n);
    for(size_t i = 0; i < n; ++i) g.unmatched.insert(i);

    size_t e;
    cin >> e;
    for(size_t i = 0; i < e; ++i) {
        Edge edg;
        cin >> edg.u >> edg.v;
        edg.matched = false;

        size_t nedg = g.edges.size();
        g.edges.push_back(edg);
        g.nodes[edg.u].edges.push_back(nedg);
        g.nodes[edg.v].edges.push_back(nedg);
    }

    blossom(g);
    for(auto e = g.edges.begin(); e != g.edges.end(); ++e) {
        if(e->matched) {
            cout << e->u << " -- " << e->v << " M" << endl;
        } else {
            cout << e->u << " -- " << e->v << endl;
        }
    }

    return 0;
}

