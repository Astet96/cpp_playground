// uncomment as needed to compile
// #include <stdexcept>
// #include <string>
// #include <random>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>


const std::string TEST_FILE = "graph.in";
const int TEST_GRAPH_SIZE = 50;
const float TEST_DENSITY = 0.6;
const float TEST_MIN_EDGE_COST = 1;
const float TEST_MAX_EDGE_COST = 10;
// ^TEST PARAMS^

enum class EdgeColour{
    NoColour,   // 0
    Green,      // 1
    Yellow,     // 2
    Red         // 3
};
// ^Enums^

const std::vector<std::string> COLOUR_NAMES = {
    "No Colour",
    "Green",
    "Yellow",
    "Red",
};

const int ALPHABET_SIZE = 'Z' - 'A' + 1;
const int MISSING_NODE_VAL = -1;
const int MISSING_EDGE_VAL = -1;
// ^CONSTANTS^

class PrintableTree;
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

// Ordered Queue template: small to large
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

        // extend this OrderedQueue with the elements of other_queue
        inline void extend(const OrderedQueue<T>& other_queue){
            for(auto& var : other_queue.queue) insert(var);
        }

        // Provide useful methods of the C++ standard list to the OrderedQueue
        inline void pop_front(){return queue.pop_front();}
        inline T front(){return queue.front();}
        inline int size(){return queue.size();}
        inline typename std::list<T>::iterator begin(){return queue.begin();}
        inline typename std::list<T>::iterator end(){return queue.end();}
};

// Tree Node class
class TreeNode{
    private:
        friend class Tree;
        friend class Graph;
        friend class PrintableTree;

        int idx;
        float cost;
        std::string name;
        std::list<TreeNode> child_nodes;

    public:
        // Uninitialised TreeNode constructor, used as a Falsy in boolean logic
        // MISSING_NODE_VAL used as sentinel
        TreeNode():
            idx(MISSING_NODE_VAL), name("UNINITIALISED NODE"), cost(MISSING_NODE_VAL), child_nodes(std::list<TreeNode>()){}
        
        // TreeNode constructor for actual leafes
        TreeNode(int idx, std::string name, float cost):
            idx(idx), name(name), cost(cost), child_nodes(std::list<TreeNode>()){}
        ~TreeNode(){}

        // boolean check based on MISSING_NODE_VAL
        explicit operator bool() const {return idx != MISSING_NODE_VAL;}
        
        // check equality on node idx
        inline bool operator==(const TreeNode& other_node){return idx == other_node.idx;}

        // check inequality on node idx
        inline bool operator!=(const TreeNode& other_node){return idx != other_node.idx;}

        // used to assign this node the values of other_node
        inline TreeNode operator=(const TreeNode& other_node){
            idx = other_node.idx;
            name = other_node.name;
            cost = other_node.cost;
            child_nodes = other_node.child_nodes;
            return *this;
        }
};
TreeNode UNINITIALISED_NODE = TreeNode();

// Tree class, used for MST representation
class Tree{
    private:
        friend class TreeNode;
        friend class Graph;
        friend class PrintableTree;
        friend std::ostream& operator<<(std::ostream& out_str, const PrintableTree& tree_repr);

        float total_cost = 0;
        int size = 0;
        TreeNode root_node;
        std::vector<EdgeColour> limit_colours;

        // check if two nodes exist inside the tree, depth first search
        bool nodes_connect(const int start_node, int end_node){
            bool seen_start=false, seen_end=false;

            std::list<TreeNode> available_nodes;
            available_nodes.push_front(root_node);
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

        // returns a reference the the node given by node_idx, if it exists in the tree
        // else returns UNINITIALISED_NODE
        TreeNode& get_node(int node_idx){
            std::list<TreeNode*> available_nodes;
            TreeNode* node = &root_node;
            available_nodes.push_front(node);
            do{
                node = available_nodes.front();
                available_nodes.pop_front();

                if(node->idx == node_idx) return *node;

                for(auto& child_node : node->child_nodes)
                    available_nodes.push_back(&child_node);

            }while(available_nodes.size());
            return UNINITIALISED_NODE;
        }

    public:
        Tree(std::vector<EdgeColour> limit_colours):root_node(TreeNode()), limit_colours(limit_colours){}
        ~Tree(){}

        // boolean check for tree existance based on root node
        inline bool operator!()const{return !root_node;}
};

/*
 * PrintableTree class used to construct a visual representation of the MST,
 * Works for any Tree object
 * example output:
 * [0 (A)]
 *    │
 *    ├───[1 (B) 1.00]
 *    │         │
 *    │         ├───[2 (C) 2.00]
 *    │         │
 *    │         └───[3 (D) 3.00]
 *    │
 *    ├───[4 (E) 4.00]
 *    │
 *    └───[5 (F) 5.00]
 */
class PrintableTree{
    private:
        friend std::ostream& operator<<(std::ostream& out_str, const PrintableTree& tree_repr);

        const char SEARCH_CHAR = '\342';        // search for common sub-char
        const std::string LAST_CHILD = "└";     // {'\342', '\224', '\224'} (+'\000')
        const std::string MIDDLE_CHILD = "├";   // {'\342', '\224', '\234'} (+'\000')
        const std::string VERT_SEP = "│";       // {'\342', '\224', '\202'} (+'\000')
        const std::string HORIZ_SEP = "───";    // {'\342', '\224', '\200'} (+'\000')
        const char LABEL_START_CHAR = '[';
        const int EXT_ASCII_LEN = 3;
        const int LABEL_CONNECTOR_SIZE = 4;
        const int OFFSET = 1;

        Tree tree;

        // string representation of a TreeNode, it includes the cost of the edge between the current TreeNode and it's parent if not Nullable
        inline std::string node_label(TreeNode node)const{
            return LABEL_START_CHAR + std::to_string(node.idx) + " (" + node.name + ")" + (node.cost ? " " + prettify_float(node.cost) : "") + "]";
        }

        // creates separator row between rows containing TreeNode labels
        // implementation works by copying the relevant portion of the previous row and adding or removing the relevant label connectors
        inline std::string make_filler_row(std::string prev_row, int node_label_half_len, bool has_child)const{
            int end_idx = prev_row.substr(0, OFFSET + prev_row.find_last_of(LABEL_START_CHAR) - LABEL_CONNECTOR_SIZE*EXT_ASCII_LEN).find_last_of(SEARCH_CHAR);
            std::string rtrn_str = prev_row.substr(0, end_idx);
            bool middle_child_check = prev_row.substr(end_idx, EXT_ASCII_LEN) == MIDDLE_CHILD;

            if(!has_child){
                if(middle_child_check) return rtrn_str + VERT_SEP + '\n';
                return prev_row.substr(0, rtrn_str.find_last_of(SEARCH_CHAR) + EXT_ASCII_LEN) + '\n';
            }
            return rtrn_str + (middle_child_check ? VERT_SEP : " ") + std::string(LABEL_CONNECTOR_SIZE + node_label_half_len, ' ') + VERT_SEP + '\n';
        }

        // creates row for printing TreeNode labels
        // implementation works by copying the previous row up to the last relevant char and adds relevant label connectors
        inline std::string make_tree_row(std::string prev_row, bool last_child)const{
            std::string rtrn_str = prev_row.substr(OFFSET, prev_row.find_last_of(SEARCH_CHAR) - OFFSET);
            if(!last_child) rtrn_str += MIDDLE_CHILD;
            else rtrn_str += LAST_CHILD;
            return rtrn_str + HORIZ_SEP;
        }

        // populates the out_str with the graphic representation of the Tree, each TreeNode is separated by a filler row
        inline void print_row(std::string& out_str, TreeNode node, int level, bool has_child, bool last_child)const{
            int prev_row_start = out_str.substr(0, out_str.find_last_of('\n') - OFFSET).find_last_of('\n');
            std::string node_label = PrintableTree::node_label(node);
            try{
                std::string prev_row = out_str.substr(prev_row_start, out_str.size()-prev_row_start - OFFSET);
                int node_label_half_len = node_label.size()/2 - OFFSET;
    
                prev_row = make_tree_row(prev_row, last_child) + node_label;
                out_str += prev_row + '\n';
                out_str += make_filler_row(prev_row, node_label_half_len, has_child);
            }catch(std::out_of_range){
                out_str += node_label + '\n' + std::string(node_label.size()/2, ' ') + VERT_SEP + '\n';
            }
        }

    public:
        inline PrintableTree(Tree tree):tree(tree){}
        inline ~PrintableTree(){}

        // Entrypoint method for constructing the visual representation of the Tree
        inline std::string to_string() const {
            int level = 0;
            TreeNode node = tree.root_node;
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

// ostream& << overload for printing vector<EdgeColour>
inline std::ostream& operator<<(std::ostream& out_str, const std::vector<EdgeColour>& edge_colours){
    for(auto& colour : edge_colours)
        out_str << '\t' << COLOUR_NAMES[int(colour)] << std::endl;
    return out_str;
}

// ostream& << overload for printing an MST represented as a PrintableTree&
// this overload also prints out relevant metadata pertaining to the MST
inline std::ostream& operator<<(std::ostream& out_str, const PrintableTree& tree_repr){    
    std::stringstream colour_limit;
    if(tree_repr.tree.limit_colours.size())
        colour_limit << "The MST was limited to nodes with the following colours:" << std::endl << tree_repr.tree.limit_colours << std::endl;
    else
        colour_limit << "The MST was not limited to any colours" << std::endl << std::endl;

    if(!tree_repr.tree)
        return out_str << colour_limit.str() << "The graph does not have an MST!" << std::endl << std::endl;

    return out_str <<
        "The graph MST cost is " << std::fixed << std::setprecision(2) << tree_repr.tree.total_cost << std::endl
        << std::endl << colour_limit.str() << "The following is a graphic representation of the MST," << std::endl
        << "each node label includes the cost between itself and its parent:" << std::endl
        << std::endl << tree_repr.to_string() << std::endl;
}

// GraphEdge class used for describing Edges between nodes inside a Graph
class GraphEdge{
    private:
        friend class Graph;

        EdgeColour colour;
        int root_node_idx;
        int target_node_idx;
        float edge_cost;

    public:
        inline GraphEdge():root_node_idx(MISSING_EDGE_VAL), target_node_idx(MISSING_EDGE_VAL), edge_cost(MISSING_EDGE_VAL){}
        inline GraphEdge(
            int root_node_idx,
            int target_node_idx,
            float edge_cost,
            EdgeColour colour = EdgeColour::NoColour
        ):
            root_node_idx(root_node_idx),
            target_node_idx(target_node_idx),
            edge_cost(edge_cost),
            colour(colour)
        {}
        inline ~GraphEdge(){}

        // check if the edge has a valid colour
        inline bool has_valid_colour(std::vector<EdgeColour> colours_to_check = std::vector(1, EdgeColour::NoColour)){
            for(auto& colour_check : colours_to_check)
                if(colour == colour_check) return true;
            return false;
        }

        // Boolean logic operators, these will always return True if one of the edges is uninitialised,
        // otherwise checks the edge_cost
        inline bool operator<=(const GraphEdge& other_edge){return edge_cost == MISSING_EDGE_VAL || other_edge.edge_cost == MISSING_EDGE_VAL || (edge_cost <= other_edge.edge_cost);}
        inline bool operator>=(const GraphEdge& other_edge){return edge_cost == MISSING_EDGE_VAL || other_edge.edge_cost == MISSING_EDGE_VAL || (edge_cost >= other_edge.edge_cost);}
        inline bool operator<(const GraphEdge& other_edge){return edge_cost == MISSING_EDGE_VAL || other_edge.edge_cost == MISSING_EDGE_VAL || (edge_cost < other_edge.edge_cost);}
        inline bool operator>(const GraphEdge& other_edge){return edge_cost == MISSING_EDGE_VAL || other_edge.edge_cost == MISSING_EDGE_VAL || (edge_cost > other_edge.edge_cost);}

        // made to handle uninitialised nodes
        inline bool operator!(){return root_node_idx == MISSING_EDGE_VAL || target_node_idx== MISSING_EDGE_VAL || edge_cost == MISSING_EDGE_VAL;}

        // used to assign this edge the values of other_edge
        inline GraphEdge& operator=(const GraphEdge& other_edge){
            colour = other_edge.colour;
            root_node_idx = other_edge.root_node_idx;
            target_node_idx = other_edge.target_node_idx;
            edge_cost = other_edge.edge_cost;
            return *this;
        }
};

// GraphNode class used to represent a node inside a Graph
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

        // seeds edges between this node and other_node, used when generating a random graph
        inline void seed_edge(GraphNode& other_node, float& min_cost, float& max_cost){
            float edge_cost = float(random() % int(100*(max_cost - min_cost)+1) + int(100*min_cost))/100;
            EdgeColour colour = EdgeColour(1 + random() % 3);
            edges.insert(GraphEdge(node_idx, other_node.node_idx, edge_cost, colour));
            other_node.edges.insert(GraphEdge(other_node.node_idx, node_idx, edge_cost, colour));
            return;
        }

        // inserts an edge between this node and other_node, used when reading the graph data from a file
        inline void insert_edge(GraphNode& other_node, float& edge_cost, EdgeColour colour = EdgeColour::NoColour){
            edges.insert(GraphEdge(node_idx, other_node.node_idx, edge_cost, colour));
            other_node.edges.insert(GraphEdge(other_node.node_idx, node_idx, edge_cost, colour));
            return;
        }

    public:
        inline GraphNode(int node_idx):node_idx(node_idx), node_name(make_str_from_idx(node_idx)){}
        inline ~GraphNode(){}
};

// Graph base class used for storing a graph in Adjacency list form
class Graph{
    protected:
        friend class Tree;
        friend class PrintableTree;
        std::vector<GraphNode> nodes;
        int size;

        // checks if an edge has a valid colour
        bool edge_valid_colour(GraphEdge& edge, std::vector<EdgeColour>& limit_colours){
            if(!limit_colours.size()) return true;
            for(auto& colour : limit_colours)
                if(edge.colour == colour)
                    return true;
            return false;
        }

        // calculates the graph MST via the Prim algorithm
        Tree calc_mst_prim(std::vector<EdgeColour>& limit_colours){
            if(!nodes.size()) return Tree(limit_colours);
            
            Tree mst_subtree = Tree(limit_colours);
            std::vector<bool> seen_idx(nodes.size(), false);

            GraphEdge min_edge;
            for(auto& node : nodes){
                // edge case: node has no edges!!!
                if(!node.edges.size()) return Tree(limit_colours);
                if(
                    GraphEdge curr_edge = node.edges.front();
                    edge_valid_colour(curr_edge, limit_colours) && curr_edge < min_edge
                ) min_edge = node.edges.front();
            }

            if(!min_edge) return Tree(limit_colours);

            OrderedQueue<GraphEdge> available_edges; available_edges.extend(nodes[min_edge.root_node_idx].edges);
            mst_subtree.size++; mst_subtree.root_node = TreeNode(min_edge.root_node_idx, nodes[min_edge.root_node_idx].node_name, 0);
            seen_idx[min_edge.root_node_idx] = true;
            do{
                min_edge = available_edges.front();
                available_edges.pop_front();

                if(!edge_valid_colour(min_edge, limit_colours)) continue;

                if(seen_idx[min_edge.root_node_idx] && seen_idx[min_edge.target_node_idx])
                if(mst_subtree.nodes_connect(min_edge.root_node_idx, min_edge.target_node_idx))
                    continue;

                    mst_subtree.size++; mst_subtree.total_cost += min_edge.edge_cost;
                    
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

            if(mst_subtree.size != size) return Tree(limit_colours);

            return mst_subtree;
        }

    public:
        inline Graph():nodes(std::vector<GraphNode>()){}
        inline ~Graph(){}

        // method used to calculate a Graph's MST and store the resulting Tree into a PrintableTree object
        inline PrintableTree printable_mst(std::vector<EdgeColour> limit_colours = std::vector<EdgeColour>()){
            return PrintableTree(calc_mst_prim(limit_colours));
        }
};

// SeededGraph class used to randomly generate a Graph
class SeededGraph: public Graph{
    private:
        const float density;
        const float min_cost;
        const float max_cost;

    public:
        inline SeededGraph(
            int size, float density, float min_cost, float max_cost
        ):density(density), min_cost(min_cost), max_cost(max_cost){
            this->size = size;
            for(int i=0; i < size; i++){
                nodes.push_back(GraphNode(i));
                for(int j=0; j < i; j++)
                    if(static_cast<float>(random()%100)/100 <= density)
                        nodes[i].seed_edge(nodes[j], min_cost, max_cost);
            }
        }
        inline ~SeededGraph(){}
};

// GraphFromFile class used to read a Graph from a file
class GraphFromFile: public Graph{
    public:
        inline GraphFromFile(std:: string fl_path){
            std::ifstream in_file(fl_path);
            std::string row_data;

            getline(in_file, row_data);
            size = stoi(row_data);
            for(int i=0; i < size; i++) nodes.push_back(GraphNode(i));

            std::vector<std::string> split_row_data;
            std::string final_entry;
            size_t str_idx, end_idx;
            float edge_cost;
            while(getline(in_file, row_data)){
                str_idx = 0; end_idx=row_data.find(' ');
                while(end_idx != std::string::npos){
                    split_row_data.push_back(row_data.substr(str_idx, end_idx - str_idx));
                    str_idx = end_idx + 1;
                    end_idx = row_data.find(' ', str_idx);
                }

                // edge case: row entry may end in either ' ' or '/n'
                final_entry = row_data.substr(str_idx, row_data.find('\n', str_idx) - str_idx);
                if(final_entry != "") split_row_data.push_back(final_entry);

                edge_cost = std::stof(split_row_data[2]);
                if(split_row_data.size() == 3)
                    nodes[stoi(split_row_data[0])].insert_edge(nodes[stoi(split_row_data[1])], edge_cost);
                else // split_row_data.size() == 4
                    nodes[stoi(split_row_data[0])].insert_edge(nodes[stoi(split_row_data[1])], edge_cost, EdgeColour(stoi(split_row_data[4])));
                split_row_data.clear();
            }
            in_file.close();
        }
        inline ~GraphFromFile(){}
};

// entry point
int main(){
    srandom(clock());
    SeededGraph test_graph = SeededGraph(
        TEST_GRAPH_SIZE,
        TEST_DENSITY,
        TEST_MIN_EDGE_COST,
        TEST_MAX_EDGE_COST
    );
    std::cout << "Test 1: Randomly generated graph, without limit on colours" << std::endl << test_graph.printable_mst();
    std::cout << "Test 2: Randomly generated graph, with limit on colours" << std::endl << test_graph.printable_mst(std::vector<EdgeColour>{EdgeColour::Yellow, EdgeColour::Red});

    GraphFromFile test_graph_2 = GraphFromFile(TEST_FILE);
    std::cout << "Test 3: Graph read from file" << std::endl << test_graph_2.printable_mst();

    return 0;
}