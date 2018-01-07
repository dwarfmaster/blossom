
#include <iostream>
#include <vector>
#include <list>
#include <cstring>
#include <cassert>
#include <unordered_set>

using namespace std;

// _   _       _                   _____ _           _ 
//| | | |_ __ (_) ___  _ __       |  ___(_)_ __   __| |
//| | | | '_ \| |/ _ \| '_ \ _____| |_  | | '_ \ / _` |
//| |_| | | | | | (_) | | | |_____|  _| | | | | | (_| |
// \___/|_| |_|_|\___/|_| |_|     |_|   |_|_| |_|\__,_|
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

        // O(1) mean time
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

        // O(1) mean time
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

//   ___              _   _            _    ____                 _     
//  / _ \ _   _  ___ | |_(_) ___ _ __ | |_ / ___|_ __ __ _ _ __ | |___
// | | | | | | |/ _ \| __| |/ _ \ '_ \| __| |  _| '__/ _` | '_ \| '_  |
// | |_| | |_| | (_) | |_| |  __/ | | | |_| |_| | | | (_| | |_) | | | |
//  \__\_\\__,_|\___/ \__|_|\___|_| |_|\__|\____|_|  \__,_| .__/|_| |_|
//                                                        |_|          
// 

struct Edge {
    size_t src;
    size_t dest;
    bool   matched;
};

using Graph = vector<list<Edge>>;

// Iterates on the edges of a compressed node
// A complete iteration on compressed node n is O(\sum_{i\in n} d(i))
class QuotientGraphNodeIterator {
    Graph* m_graph;
    uf_data<size_t>* m_partition;
    size_t m_node;
    list<Edge>::iterator m_it;
    bool m_ended;

    public:
        QuotientGraphNodeIterator()
        : m_graph(nullptr), m_node(0), m_it(), m_ended(true)
        {}

        QuotientGraphNodeIterator(Graph* gr, uf_data<size_t>* partition, size_t node)
        : m_graph(gr), m_partition(partition), m_node(node),
          m_it((*m_graph)[node].begin()),
          m_ended(false)
        { next(); }

        QuotientGraphNodeIterator(const QuotientGraphNodeIterator& it)
        : m_graph(it.m_graph), m_partition(it.m_partition), m_node(it.m_node),
          m_it(it.m_it), m_ended(it.m_ended)
        { }

        ~QuotientGraphNodeIterator() = default;

        QuotientGraphNodeIterator& operator=(const QuotientGraphNodeIterator& it) {
            m_graph     = it.m_graph;
            m_partition = it.m_partition;
            m_node      = it.m_node;
            m_it        = it.m_it;
            m_ended     = it.m_ended;
            return *this;
        }

        void next() {
            while(m_it == (*m_graph)[m_node].end()) {
                m_node = m_partition->next(m_node);
                if(m_node == (size_t)-1) {
                    m_ended = true;
                    return;
                }
                m_it = (*m_graph)[m_node].begin();
            }

            if(m_partition->parent(m_it->dest) != m_partition->parent(m_node))
                return;

            ++m_it;
            next();
        }

        // Comparison operations
        bool operator==(const QuotientGraphNodeIterator& it) {
            return (m_ended && it.m_ended)
                || (!m_ended && !it.m_ended
                        && (m_node == it.m_node) && (m_it == it.m_it));
        }

        bool operator!=(const QuotientGraphNodeIterator& it) {
            return !(*this == it);
        }

        // Access operations
        Edge& operator*() {
            return *m_it;
        }

        Edge& operator->() {
            return *m_it;
        }

        // Iteration
        QuotientGraphNodeIterator& operator++() {
            ++m_it;
            next();
            return *this;
        }
};

class QuotientGraph {
    Graph* m_graph;
    // The value stored here is useless
    uf_data<size_t> m_partition;

    public:
        QuotientGraph() = delete;

        // O(|gr|)
        QuotientGraph(Graph* gr)
        : m_graph(gr), m_partition(gr->size())
        { }

        ~QuotientGraph() = default;

        QuotientGraph& operator=(const QuotientGraph&) = delete;

        using iterator = QuotientGraphNodeIterator;

        iterator end() {
            iterator end;
            return end;
        }

        iterator operator[](size_t i) {
            iterator it(m_graph, &m_partition, m_partition.parent(i));
            return it;
        }
};

//  ____  _                               
// | __ )| | ___  ___ ___  ___  _ __ ____
// |  _ \| |/ _ \/ __/ __|/ _ \| '_ ` _  | 
// | |_) | | (_) \__ \__ \ (_) | | | | | |
// |____/|_|\___/|___/___/\___/|_| |_| |_|
//                                        

using EdgePtr = list<Edge>::iterator;
using Path = list<EdgePtr>;

// Assumes path is an augmenting path
// O(|path|) worst time
void augment(Path& path) {
    for(auto edg = path.begin(); edg != path.end(); ++edg) {
        (*edg)->matched = !(*edg)->matched;
    }
}

Path find_augmenting_path(Graph& graph, const unordered_set<size_t>& unmatched) {
    QuotientGraph quotient(&graph);
    // TODO
}

// The matching is stored in graph as annotations on edges
// The maximum matching is returned by modifying graph by side-effect
void find_maximum_matching(Graph& graph) {
    // Initialisation of unmatched is O(n)
    unordered_set<size_t> unmatched;
    for(size_t i = 0; i < graph.size(); ++i) {
        unmatched.insert(i);
    }

    Path augmenting;
    augmenting = find_augmenting_path(graph, unmatched);
    while(!augmenting.empty()) {
        // augment is O(|augmenting|) = O(min(n,m))
        augment(augmenting);

        // Updating unmatched is O(1) mean time
        unmatched.erase(augmenting.front()->src);
        unmatched.erase(augmenting.back()->dest);

        // TODO complexity of looking for augmenting path
        augmenting = find_augmenting_path(graph, unmatched);
    }
}

//  __  __       _       
// |  \/  | __ _(_)_ ___  
// | |\/| |/ _` | | '_  |
// | |  | | (_| | | | | |
// |_|  |_|\__,_|_|_| |_|
//                       

int main(int argc, char *argv[]) {
    if(argc && argv) {}

    cout << "Hello world" << endl;
    return 0;
}

