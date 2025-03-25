// TODO: MAKE Tree.nodes a TreeNode value instead of vector<TreeNode>

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <vector>
#include <list>

const int ALPHABET_SIZE = 'Z' - 'A' + 1;
const int MISSING_NODE_IDX = -1;

// TEST PARAMS
const std::string TEST_FILE = "graph.in";

const int TEST_GRAPH_SIZE = 50;
const float TEST_DENSITY = 0.6;
const float TEST_MIN_EDGE_COST = 1;
const float TEST_MAX_EDGE_COST = 10;
const int TEST_NODE_IDX = 0;
// ^CONSTANTS^

class Graph;
class SeededGraph;
// ^Prototypes^

// Round float to 2 significant digits
inline float round_float(float val){
    return (float)((int)(val * 100 + 0.5))/100;
}

// prepare float for ostream
std::string prettify_float(float val){
    std::string str_val = std::to_string(round_float(val));
    return str_val.substr(0, str_val.find(".")+3);
}

// Ordered small to large
template <class T>
class OrderedQueue{
    private:
        std::list<T> queue;
    public:
        inline OrderedQueue():queue(std::list<T>()){}
        inline ~OrderedQueue(){queue.clear();}

        // binary search insert: O(logN)
        inline void insert(T elem){
            if(!queue.size() || elem <= queue.front()){queue.emplace_front(elem); return;}
            else if(elem >= queue.back()){queue.emplace_back(elem); return;}

            int step_size = queue.size()/2;
            auto cursor = queue.begin(); advance(cursor, step_size);
            while(true){
                step_size /= 2;
                if(!step_size) step_size=1;
                auto prev_val = *(--cursor);
                auto next_val = *(++cursor);
                if((prev_val <= elem) && (elem <= next_val)) break;
                else if(elem > next_val) advance(cursor, step_size);
                else if(elem < next_val) advance(cursor, -step_size);
            }
            queue.emplace(cursor, elem); return;
        }

        inline void extend(const OrderedQueue<T>& other_queue){
            for(auto& var : other_queue.queue) insert(var);
        }

        inline void pop_front(){return queue.pop_front();}
        inline T front(){return queue.front();}
        inline int size(){return queue.size();}
        inline typename std::list<T>::iterator begin(){return queue.begin();}
        inline typename std::list<T>::iterator end(){return queue.end();}

};

class TreeNode{
    private:
        int idx;
        std::string name;
        float cost;
        std::list<TreeNode> child_nodes;
        friend class Tree;
        friend class Graph;
        friend class PrintableMSTQuery;

    public:
        TreeNode(int idx, std::string name, float cost):
            idx(idx), name(name), cost(cost), child_nodes(std::list<TreeNode>()){}
        ~TreeNode(){}

        explicit operator bool() const {return idx != MISSING_NODE_IDX;}
        inline TreeNode operator=(const TreeNode& other_node){
            idx = other_node.idx;
            name = other_node.name;
            cost = other_node.cost;
            child_nodes = other_node.child_nodes;
            return *this;
        }
        inline bool operator==(const TreeNode& other_node){return idx == other_node.idx;}
        inline bool operator!=(const TreeNode& other_node){return idx != other_node.idx;}
};
TreeNode MISSING_NODE = TreeNode(MISSING_NODE_IDX, "MISSING NODE", -1);

class Tree{
    private:
        std::list<TreeNode> nodes;
        float total_cost = 0;
        friend class TreeNode;
        friend class Graph;
        friend class PrintableMSTQuery;

        // TODO: nodes_connect & get_node use similar code, maybe this can be refactored to avoid repetition???

        // returns true if two nodes connect inside a Tree
        // test this works as intended (it should)
        bool nodes_connect(const int start_node, int end_node){
            bool seen_start=false, seen_end=false;
            
            std::list<TreeNode> available_nodes;
            available_nodes.push_front(nodes.front());
            do{
                TreeNode node = available_nodes.front();
                available_nodes.pop_front();
                
                if(node.idx == start_node) seen_start = true;
                if(node.idx == end_node) seen_end = true;
                if(seen_start && seen_end) return true;

                available_nodes.insert(available_nodes.end(), node.child_nodes.begin(), node.child_nodes.end());
            }while(available_nodes.size());
            return false;
        }

        TreeNode& get_node(int node_idx){
            std::list<TreeNode*> available_nodes;
            TreeNode* node = &nodes.front();
            available_nodes.push_front(node);
            do{
                node = available_nodes.front();
                available_nodes.pop_front();

                if(node->idx == node_idx) return *node;

                for(auto& child_node : node->child_nodes)
                    available_nodes.push_back(&child_node);

            }while(available_nodes.size());
            return MISSING_NODE;
        }

    public:
        Tree():nodes(std::list<TreeNode>()){}
        ~Tree(){}

        inline bool operator!()const{return !nodes.size();}
};

// TODO:
class PrintableMSTQuery{
    private:
        const char SEARCH_CHAR = '\342';        // search for common sub-char
        const std::string LAST_CHILD = "└";     // '\342' + '\224' + '\224' (+'\000')
        const std::string MIDDLE_CHILD = "├";   // '\342' + '\224' + '\234' (+'\000')
        const std::string VERT_SEP = "│";       // '\342' + '\224' + '\202' (+'\000')
        const std::string HORIZ_SEP = "───";    // '\342' + '\224' + '\200' (+'\000')

        const char LABEL_START_CHAR = '[';
        const int EXT_ASCII_LEN = 3;
        const int LABEL_CONNECTOR_SIZE = 4;

        friend std::ostream& operator<<(std::ostream& out_str, const PrintableMSTQuery& tree_repr);
        Tree tree;

        const float mst_cost = 0;

        inline std::string node_label(TreeNode node)const{
            return LABEL_START_CHAR + std::to_string(node.idx) + " (" + node.name + ")" + (node.cost ? " " + prettify_float(node.cost) : "") + "]";
        }

        // TODO: CLEAN THIS UP!!!
        // revise use of substr!!!
        // write comments
        inline std::string make_filler_row(std::string prev_row, int node_label_half_len, bool has_child)const{
            int end_idx = prev_row.substr(0, 1 + prev_row.find_last_of(LABEL_START_CHAR) - LABEL_CONNECTOR_SIZE*EXT_ASCII_LEN).find_last_of(SEARCH_CHAR);
            std::string rtrn_str = prev_row.substr(0, end_idx);
            bool middle_child_check = prev_row.substr(end_idx, EXT_ASCII_LEN) == MIDDLE_CHILD;

            if(!has_child){
                if(middle_child_check) return rtrn_str + VERT_SEP + '\n';
                return prev_row.substr(0, rtrn_str.find_last_of(SEARCH_CHAR) + EXT_ASCII_LEN) + '\n';
            }
            return rtrn_str + (middle_child_check ? VERT_SEP : " ") + std::string(LABEL_CONNECTOR_SIZE + node_label_half_len, ' ') + VERT_SEP + '\n';
        }

        inline std::string make_tree_row(std::string prev_row, int node_label_half_len, bool last_child)const{
            std::string rtrn_str = prev_row.substr(1, prev_row.find_last_of(SEARCH_CHAR) - 1);
            if(!last_child) rtrn_str += MIDDLE_CHILD;
            else rtrn_str += LAST_CHILD;
            return rtrn_str + HORIZ_SEP;
        }

        inline void print_row(std::string& out_str, TreeNode node, int level, bool has_child, bool last_child)const{
            int prev_row_start = out_str.substr(0, out_str.find_last_of('\n') - 1).find_last_of('\n');
            std::string node_label = PrintableMSTQuery::node_label(node);
            try{
                std::string prev_row = out_str.substr(prev_row_start, out_str.size()-prev_row_start - 1);
                int node_label_half_len = node_label.size()/2 - 1;
    
                prev_row = make_tree_row(prev_row, node_label_half_len, last_child) + node_label;
                out_str += prev_row + '\n';
                out_str += make_filler_row(prev_row, node_label_half_len, has_child);
            }catch(std::out_of_range){
                out_str += node_label + '\n' + std::string(node_label.size()/2, ' ') + VERT_SEP + '\n';
            }
        }

    public:
        inline PrintableMSTQuery(Tree tree, float mst_cost):tree(tree), mst_cost(mst_cost){}
        inline ~PrintableMSTQuery(){}

        /*
         * [0 (A)]
         *    │
         *    ├───[1 (B)]
         *    │      │
         *    │      ├───[2 (C)]
         *    │      │
         *    │      └───[3 (D)]
         *    │
         *    ├───[4 (E)]
         *    │
         *    └───[5 (F)]
         */
        inline std::string to_string() const {
            int level = 0;
            TreeNode node = tree.nodes.front();
            std::list<std::pair<TreeNode, int>> print_queue(1, std::pair(node, level));
            
            std::string out_str = "";
            do{
                node = print_queue.front().first;
                level = print_queue.front().second;
                print_queue.pop_front();

                print_row(out_str, node, level, node.child_nodes.size(), level != print_queue.front().second);

                for(auto chld_iter = node.child_nodes.end(); chld_iter-- != node.child_nodes.begin();)
                    print_queue.push_front(std::pair(*chld_iter, level+1));

            }while(print_queue.size());

            return out_str;
        }
};

// TODO:
inline std::ostream& operator<<(std::ostream& out_str, const PrintableMSTQuery& tree_repr){
    if(!tree_repr.tree)
        return out_str << "The graph does not have an MST!";

    return out_str <<
        "The graph MST cost is " << std::fixed << std::setprecision(2) << tree_repr.mst_cost << std::endl
        << "The following is a graphic representation of the MST," << std::endl
        << "each node label includes the cost between itself and its parent:" << std::endl
        << std::endl << tree_repr.to_string() << std::endl;
}

class GraphEdge{
    private:
        int root_node_idx;
        int target_node_idx;
        float edge_cost;
        bool seen = false;
        friend class Graph;
    public:
        inline GraphEdge():root_node_idx(-1), target_node_idx(-1), edge_cost(-1){}
        inline GraphEdge(
            int root_node_idx,
            int target_node_idx,
            float edge_cost
        ):
            root_node_idx(root_node_idx),
            target_node_idx(target_node_idx),
            edge_cost(edge_cost)
        {}
        inline ~GraphEdge(){}

        inline bool operator<=(const GraphEdge& other_edge){return edge_cost <= other_edge.edge_cost;}
        inline bool operator>=(const GraphEdge& other_edge){return edge_cost >= other_edge.edge_cost;}
        inline bool operator<(const GraphEdge& other_edge){return edge_cost < other_edge.edge_cost;}
        inline bool operator>(const GraphEdge& other_edge){return edge_cost > other_edge.edge_cost;}
        inline GraphEdge& operator=(const GraphEdge& other_edge){
            root_node_idx = other_edge.root_node_idx;
            target_node_idx = other_edge.target_node_idx;
            edge_cost = other_edge.edge_cost;
            return *this;
        }

        GraphEdge& find_reciprocal(OrderedQueue<GraphEdge>& other_edges){
            for(auto& edge: other_edges)
                if(root_node_idx == edge.target_node_idx && target_node_idx == edge.root_node_idx)
                    return edge;
            return *this;
        }

        void mark_seen(){seen = true;}
        void mark_unseen(){seen = false;}

};

class GraphNode{
    private:
        friend class Graph;
        friend class SeededGraph;
        friend class GraphFromFile;

        const int node_idx;
        const std::string node_name;
        OrderedQueue<GraphEdge> edges;

        /*
        * Converts an int index to a string index with the following rule:
        * A  = 0 | Z  = 25 | AA = 26 | AB = 27 etc...
        */ 
        inline std::string make_str_from_idx(int x, std::string rtrn_str=""){
            if(x / ALPHABET_SIZE)
                return make_str_from_idx(
                    (x / ALPHABET_SIZE) - 1,
                    char(x % ALPHABET_SIZE + 'A') + rtrn_str
                );
            return char(x % ALPHABET_SIZE + 'A') + rtrn_str;
        }

        inline void seed_edge(GraphNode& other_node, float& min_cost, float& max_cost){
            float edge_cost = gen_edge_cost(min_cost, max_cost);
            edges.insert(GraphEdge(node_idx, other_node.node_idx, edge_cost));
            other_node.edges.insert(GraphEdge(other_node.node_idx, node_idx, edge_cost));
            return;
        }

        inline float gen_edge_cost(float& min_cost, float& max_cost){
            return float(random() % int(100*(max_cost - min_cost)+1) + int(100*min_cost))/100;
        }

        inline void insert_edge(GraphNode& other_node, float& edge_cost){
            edges.insert(GraphEdge(node_idx, other_node.node_idx, edge_cost));
            other_node.edges.insert(GraphEdge(other_node.node_idx, node_idx, edge_cost));
            return;
        }

    public:
        inline GraphNode(int node_idx):node_idx(node_idx), node_name(make_str_from_idx(node_idx)){}
        inline ~GraphNode(){}
};

class Graph{
    protected:
        std::vector<GraphNode> nodes;
        friend class Tree;
        friend class PrintableMSTQuery;

        Tree calc_mst_prim(){
            if(!nodes.size()) return Tree();
            
            Tree mst_subtree = Tree();
            std::vector<bool> seen_idx(nodes.size(), false);
            
            GraphEdge min_edge = nodes[0].edges.front();
            for(auto& node : nodes){
                // edge case: node has no edges!!!
                if(!node.edges.size()) return Tree();
                if(node.edges.front() < min_edge) min_edge = node.edges.front();
            }
            
            OrderedQueue<GraphEdge> available_edges; available_edges.extend(nodes[min_edge.root_node_idx].edges);
            mst_subtree.nodes.push_back(TreeNode(min_edge.root_node_idx, nodes[min_edge.root_node_idx].node_name, 0));
            seen_idx[min_edge.root_node_idx] = true;
            do{
                min_edge = available_edges.front();
                available_edges.pop_front();

                if(seen_idx[min_edge.root_node_idx] && seen_idx[min_edge.target_node_idx])
                if(mst_subtree.nodes_connect(min_edge.root_node_idx, min_edge.target_node_idx))
                    continue;

                mst_subtree.total_cost += min_edge.edge_cost;
                    
                TreeNode& parent_node = mst_subtree.get_node(min_edge.root_node_idx);
                if(parent_node){
                    seen_idx[min_edge.target_node_idx] = true;
                    parent_node.child_nodes.push_back(TreeNode(min_edge.target_node_idx, nodes[min_edge.target_node_idx].node_name, min_edge.edge_cost));
                    available_edges.extend(nodes[min_edge.target_node_idx].edges);
                }
                else{
                    parent_node = mst_subtree.get_node(min_edge.target_node_idx);
                    seen_idx[min_edge.root_node_idx] = true;
                    if(parent_node) parent_node.child_nodes.push_back(TreeNode(min_edge.root_node_idx, nodes[min_edge.root_node_idx].node_name, min_edge.edge_cost));
                    else throw std::logic_error("ERROR: Malformed Graph!");
                }
            }while(available_edges.size());

            return mst_subtree;
        }

    public:
        inline Graph():nodes(std::vector<GraphNode>()){}
        inline ~Graph(){}

        inline PrintableMSTQuery get_mst(){
            Tree mst_subtree = calc_mst_prim();
            return PrintableMSTQuery(mst_subtree, mst_subtree.total_cost);
        }
};

class SeededGraph: public Graph{
    private:
        const int size;
        const float density;
        const float min_cost;
        const float max_cost;

    public:
        inline SeededGraph(
            int size, float density, float min_cost, float max_cost
        ):size(size), density(density), min_cost(min_cost), max_cost(max_cost){
            for(int i=0; i < size; i++){
                nodes.push_back(GraphNode(i));
                for(int j=0; j < i; j++)
                    if(static_cast<float>(random()%100)/100 <= density)
                        nodes[i].seed_edge(nodes[j], min_cost, max_cost);
            }
        }
        inline ~SeededGraph(){}
};

class GraphFromFile: public Graph{
    private:
        int size;

    public:
        inline GraphFromFile(std:: string fl_path){
            std::ifstream in_file(fl_path);
            std::string row_data;

            getline(in_file, row_data);
            size = stoi(row_data);
            for(int i=0; i < size; i++) nodes.push_back(GraphNode(i));

            std::vector<std::string> split_row_data;
            size_t str_idx, end_idx;
            float edge_cost;
            while(getline(in_file, row_data)){
                str_idx = 0; end_idx=row_data.find(' ');
                while(end_idx != std::string::npos){
                    split_row_data.push_back(row_data.substr(str_idx, end_idx - str_idx));
                    str_idx = end_idx + 1;
                    end_idx = row_data.find(' ', str_idx);
                }

                edge_cost = std::stof(split_row_data[2]);
                nodes[stoi(split_row_data[0])].insert_edge(nodes[stoi(split_row_data[1])], edge_cost);
                split_row_data.clear();
            }
            in_file.close();
        }
        inline ~GraphFromFile(){}
};

// TODO: add edge type discriminator + implement prim on type discrimination
// TODO: read graph from file
int main(){
    srandom(clock());
    SeededGraph test_graph = SeededGraph(
        TEST_GRAPH_SIZE,
        TEST_DENSITY,
        TEST_MIN_EDGE_COST,
        TEST_MAX_EDGE_COST
    );
    std::cout << "Test 1: Randomly generated graph" << std::endl << test_graph.get_mst();

    GraphFromFile test_graph_2 = GraphFromFile(TEST_FILE);
    std::cout << "Test 2: Graph read from file" << std::endl << test_graph_2.get_mst();

    return 0;
}