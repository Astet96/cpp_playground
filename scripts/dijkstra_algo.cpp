// extra libraries you might require for compiling, uncomment as needed:
// #include <algorithm>
// #include <random>
// #include <string>
#include <iostream>
#include <functional>
#include <vector>
#include <list>
using namespace::std;

const int ALPHABET_SIZE = (static_cast<int>('Z') - static_cast<int>('A')) + 1;
const int GRAPH_SIZE = 50;
const float TEST_DENSITY = 0.2;
const float TEST_MIN_EDGE_COST = 1;
const float TEST_MAX_EDGE_COST = 10;
const int TEST_NODE_IDX = 0;
// ^CONSTANTS^

// Round float to 2 significant digits
inline float round_float(float val){
    return (float)((int)(val * 100 + 0.5))/100;
}

// prepare float for ostream
string prettify_float(float val){
    string str_val = to_string(round_float(val));
    return str_val.substr(0, str_val.find(".")+3);
}

/*
 * Converts an int index to a string index with the following rule:
 * A  = 0
 * Z  = 25
 * AA = 26
 * AB = 27
 * etc.
 */ 
inline static string make_string_idx_from_int_idx(int x, string rtrn_str=""){
    if(x / (ALPHABET_SIZE)){
        return make_string_idx_from_int_idx(
            (x / ALPHABET_SIZE) - 1,
            static_cast<char>(x % ALPHABET_SIZE + static_cast<int>('A')) + rtrn_str
        );
    }
    else{
        return char(x % ALPHABET_SIZE + static_cast<int>('A')) + rtrn_str;
    }
}

// Utility class for pretty printing paragraphs stored as vectors of strings
class PrettyPrintParagraph{
    private:
        string title;
        vector<string> row_data;
        int* idx = new int(0);

    public:
        PrettyPrintParagraph(string title):title(title){}
        ~PrettyPrintParagraph(){delete idx;}
        // Constructor & Destructor

        // adds row to the Printable Paragraph
        inline void add_row(string data){
            row_data.push_back(data);
        }

        // reads the next row from the Printable Paragraph
        inline string read_row()const{
            if(*idx < row_data.size())
                return row_data[(*idx)++];
            return string();
        }

        // reads the title of the Printable Paragraph
        inline string read_title()const{return title;}

        // set the cursor index inside the paragraph
        void set_idx(int& pos){
            delete idx;
            idx = new (int)(pos);
        }
};

// overload ostream << operator to handle PrettyPrintParagraph object
ostream& operator<<(ostream& out_stream, const PrettyPrintParagraph& out_data){
    out_stream << out_data.read_title() << endl;
    string out_row;
    while((out_row = out_data.read_row()) != string()){
        out_stream << out_row << endl;
    }
    return out_stream;
}

/*
 *OrderedQueue class template:
 * This is a template that enhances the functionality of
 * the C++ standard list by providing a method to insert
 * new elements in (ascending) order.
 * To change the ordering, or to handle non-primitive
 * data types provide a comparison function in the object
 * constructor. This may also be a lambda.
 */
template <class T>
class OrderedQueue{
    private:
        list<T> ordered_list;
        function<bool(T&, T&)> comparison_fn = [](T& left, T& right) -> bool {return left <= right;};
    
    public:
        OrderedQueue(){}
        OrderedQueue(function<bool(T&, T&)> comparison_fn):comparison_fn(comparison_fn){}
        ~OrderedQueue(){}
        // Constructors & Destructor

        inline T front(){return ordered_list.front();}
        inline void pop_front(){return ordered_list.pop_front();}
        inline int size(){return ordered_list.size();}
        // Provide useful methods of the C++ standard list to the Ordered Queue

        /*
         * Method that inserts new values in the order specified by the comparison_fn attribute.
         * Use of binary search ensures O(logN) time complexity. 
         */
        inline void insert(T value){
            if(ordered_list.size() == 0 || comparison_fn(value, ordered_list.front())) ordered_list.push_front(value);
            else if(!comparison_fn(value, ordered_list.back())) ordered_list.push_back(value);
            else{
                int move_dist = ordered_list.size()/2;
                auto cursor = ordered_list.begin(); advance(cursor, move_dist);
                while(true){
                    move_dist/=2;
                    if(!move_dist) move_dist=1;
                    T prev_val = *(--cursor);
                    T next_val = *(++cursor);
                    if(comparison_fn(prev_val, value) && comparison_fn(value, next_val)) break;
                    else if(comparison_fn(value, next_val)) advance(cursor, -move_dist);
                    else if(comparison_fn(next_val, value)) advance(cursor, move_dist);
                }
                ordered_list.emplace(cursor, value);
            }
        }
};

// Class that stores a Path from Node x to Node y
class Path{
    private:
        float path_length;
        vector<int> path_nodes;
        int start_idx;
        int end_idx;

        // updates the path length
        inline void update_path_length(float distance){
            path_length = distance;
        }

        // updates the nodes making up the path
        inline void update_path_nodes(Path& intermediary_nodes, int final_node){
            path_nodes = intermediary_nodes.get_path_nodes();
            path_nodes.resize(intermediary_nodes.get_path_nodes().size());
            path_nodes.push_back(final_node);
        }

        // updates the last node inside the path
        inline void update_end_idx(int end_node){
            end_idx = end_node;
        }

    public:
        Path():path_length(0), path_nodes(vector<int>(0, 0)), start_idx(-1), end_idx(-1){}
        Path(int start_idx):path_length(0), path_nodes(vector<int>(1, start_idx)), start_idx(start_idx), end_idx(-1){}
        ~Path(){path_nodes.clear(); path_nodes.shrink_to_fit();}
        // Constructors & Destructor

        // returns the path length
        inline float get_path_length(void){
            return path_length;
        }

        // returns the nodes making up the path
        inline vector<int> get_path_nodes(void){
            return path_nodes;
        }

        /*
         * Updates the path between Node x and Node y.
         *
         * Note: A slight innefficiency happens here by always
         *       updating the end node index, which in practice
         *       is constant. This has been done out of convenience
         *       to allow for easy initialisation of Path vectors.
         */
        inline void update_path(float distance, Path& intermediary_nodes, int final_node){
            update_path_length(distance);
            update_path_nodes(intermediary_nodes, final_node);
            update_end_idx(final_node);
        }
};

/*
 * Class that stores the Edge cost between Node x and Node y
 *
 * Note: The start node is implied as all Edge instances are
 *       stored in a vector inside a Node object.
 */
class Edge{
    private:
        int target_node_idx;
        float cost;

    public:
        Edge(int target_node_idx, float cost):target_node_idx(target_node_idx), cost(cost){}
        ~Edge(){}
        // Constructor & Destructor

        // returns the end node index
        inline int get_target_node_idx(){return target_node_idx;}

        // returns the edge cost
        inline float get_edge_cost(){return cost;}
};

// Class that represents a Node
class Node{
    private:
        const int node_idx;
        const string node_value;
        vector<Edge*> edges;
    
    public:
        inline Node(int node_idx, string node_value):node_idx(node_idx), node_value(node_value), edges(vector<Edge*>()){}
        inline Node(int node_idx):node_idx(node_idx), node_value(make_string_idx_from_int_idx(node_idx)), edges(vector<Edge*>()){}
        inline ~Node(){
            for(auto& edge : edges){
                delete edge;
            }
            edges.clear();
            edges.shrink_to_fit();
        }
        // Constructors & Destructor
        
        // overwrite == operator between ‘Node’ and ‘const Node’
        inline bool operator==(const Node other_node){return node_idx == other_node.get_node_idx();}
        
        // overwrite = operator between ‘Node& Node::operator=(const Node&)’
        inline const Node& operator=(const Node& other_node){return other_node;}
        
        // returns the node id
        inline int get_node_idx() const{return node_idx;}
        
        // returns the node value
        inline string get_node_value() const{return node_value;}
        
        // add edge between this node and node with node_idx == y
        inline void add_edge(int y, float v){edges.push_back(new Edge(y, v)); return;}
        
        // get the edge value
        inline float get_edge_value(int y) const{
            for(auto& edge : edges)
                if(edge->get_target_node_idx() == y)
                    return edge->get_edge_cost();
            return 0;
        }
        
        // remove the edge value
        inline void delete_edge_value(int y){
            for(auto& edge : edges)
                if(edge->get_target_node_idx() == y){
                    delete edge;
                    return;
                }
        }

        // returns all the edges
        inline vector<Edge*> get_all_edges(){
            return edges;
        }

        // returns the number of edges
        inline int get_nr_of_edges(){
            return edges.size();
        }
};

/*
 * NodeGraph Class that uses an adjacency list approach - hybrid of edge list and adjacency matrix.
 * Shortest paths between nodes are cached for speed.
 * 
 * Note: The cache is slightly memory inneficient because it uses a square matrix.
 *       This can be made more efficient in the future by by halfing the memory usage.
 *       This is possible because the first diagonal of the matrix is a symetry line.
 */
class NodeGraph{
    private:
        int size;
        vector<Node*> adj_list;
        vector<vector<Path>> path_cache;
        bool valid_cache = false;

        const bool is_seeded = false;
        const float density = 0;
        const float min_dist = 0;
        const float max_dist = 0;


        // Clear result cache
        inline void clear_cache(){
            for(int i = 0; i < path_cache.size(); i++){
                path_cache[i].clear();
                path_cache[i].shrink_to_fit();
            }
        }

        // Clear NodeGraph
        inline void clear_graph(){
            for(int i = 0; i < adj_list.size(); i++){
                delete adj_list[i];
            }
            return;
        }
        
        // Adds the edge from x to y, if it is not there. Seeded with a random distance
        inline void create_seeded_edge(Node* x, Node* y, float min_dist, float max_dist){
            if(!adjacent(x->get_node_idx(), y->get_node_idx())){
                float edge_value = static_cast<float>(random()%static_cast<int>(100*(max_dist - min_dist)+1) + static_cast<int>(100*min_dist))/100;
                adj_list[x->get_node_idx()]->add_edge(y->get_node_idx(), edge_value);
                adj_list[y->get_node_idx()]->add_edge(x->get_node_idx(), edge_value);
            }
        }

        // Dijkstra's algorithm for shortest path. Will return 0 if no path exists
        inline vector<Path> calc_dijkstra(int start_node_idx){
            path_cache[start_node_idx] = vector<Path>(size, Path(start_node_idx));
            vector<bool> seen_node_idx = vector<bool>(size, false);
            OrderedQueue<pair<int, float>> available_nodes = OrderedQueue<pair<int, float>>(
                [](pair<int, float>& left, pair<int, float>& right) -> bool {return left.second <= right.second;}
            );
            available_nodes.insert(pair<int, float>(start_node_idx, 0));

            do{
                pair<int, float> node = available_nodes.front();
                int x = node.first;
                seen_node_idx[x] = true;
                available_nodes.pop_front();
                for(int i = 0; i < size; i++){
                    float edge_distance = path_cache[start_node_idx][x].get_path_length() + adj_list[x]->get_edge_value(i);
                    if(adj_list[x]->get_edge_value(i) && !seen_node_idx[i]){
                        available_nodes.insert(pair<int, float>(i, edge_distance));
                        if(path_cache[start_node_idx][i].get_path_length() == 0 || path_cache[start_node_idx][i].get_path_length() > edge_distance){
                            path_cache[start_node_idx][i].update_path(edge_distance, path_cache[start_node_idx][x], adj_list[i]->get_node_idx());
                        }
                    }
                }
            }while(available_nodes.size());

            valid_cache = true;
            return path_cache[start_node_idx];
        }

    public:
        // Empty NodeGraph constructor
        inline NodeGraph(int size):size(size), adj_list(vector<Node*>(size)), path_cache(vector<vector<Path>>(size, vector<Path>())){}

        // Seeded NodeGraph constructor based on given density
        inline NodeGraph(
            int size,
            float density,
            float min_dist,
            float max_dist
        ):
            size(size),
            adj_list(vector<Node*>(size)),
            path_cache(vector<vector<Path>>(size, vector<Path>())),
            is_seeded(true),
            density(density),
            min_dist(min_dist),
            max_dist(max_dist)
        {
            for(int i=0; i < size; i++){
                add_node(new Node(i));
                for(int j=0; j < i; j++){
                    if(static_cast<float>(random() % 100) / 100 <= density){
                        create_seeded_edge(adj_list[j], adj_list[i], min_dist, max_dist);
                    }
                }
            }
            return;
        }

        // NodeGraph destructor
        inline ~NodeGraph(){clear_graph(); clear_cache(); path_cache.clear(); adj_list.clear(); adj_list.shrink_to_fit();}

        // returns true if the graph was randomly generated from a seed
        inline bool is_seeded_graph(){
            return is_seeded;
        }
        
        // returns the number of vertices in the graph
        inline int Vrt(){return adj_list.size();}

        // returns the number of edges in the graph
        inline int Edg(){
            int sum = 0;
            for(auto &node : adj_list){
                sum += node->get_nr_of_edges();
            }
            return sum/2;
        }

        // tests whether there is an edge from node x to node y.
        inline bool adjacent(int x, int y){
            for(auto &neighbour_node_idx : neighbors(x)){
                if(neighbour_node_idx == y){
                    return true;
                }
            }
            return false;
        }
        
        // lists all nodes y such that there is an edge from x to y.
        inline vector<int> neighbors(int x){
            vector<int> rtrn;
            for(auto& edge : adj_list[x]->get_all_edges()){
                rtrn.push_back(edge->get_target_node_idx());
            }
            return rtrn;
        }
        
        // removes the edge from x to y, if it is there.
        inline void remove(const Node* x, const Node* y){
            if(adjacent(x->get_node_idx(), y->get_node_idx())){
                valid_cache = false;
                adj_list[x->get_node_idx()]->delete_edge_value(y->get_node_idx());
                adj_list[y->get_node_idx()]->delete_edge_value(x->get_node_idx());
            }
        }
        
        // returns the value associated with the node x.
        inline string get_node_value(int x){return adj_list[x]->get_node_value();}
        
        // returns the value associated to the edge (x,y).
        inline float get_edge_value(int x, int y){return adj_list[x]->get_edge_value(y);}

        // adds Node to NodeGraph ONLY if it doesn't exist
        inline void add_node(Node* x){
            if(adj_list[x->get_node_idx()] == nullptr){
                valid_cache = false;
                adj_list[x->get_node_idx()] = x;
            }
            return;
        }

        // adds to G the edge from x to y, if it is not there.
        inline void create_edge(Node* x, Node* y, float v){
            if(!adjacent(x->get_node_idx(), y->get_node_idx())){
                valid_cache = false;
                adj_list[x->get_node_idx()]->add_edge(y->get_node_idx(), v);
                adj_list[y->get_node_idx()]->add_edge(x->get_node_idx(), v);
            }
        }

        // get min value between node x and all other nodes in the graph
        // this uses Dijkstra's algorithm
        inline vector<Path> calc_shortest_paths(int x){
            if(!valid_cache){clear_cache(); return calc_dijkstra(x);}
            if(path_cache[x].size() == size) return path_cache[x];
            else return calc_dijkstra(x);
        }

        // returns the average path from node x to every other node in the graph
        inline float calc_average_path(int x){
            float sum = 0;
            int count = 0;
            if(!valid_cache || path_cache[x].size() != size) calc_shortest_paths(x);
            for(auto& path : path_cache[x]){
                if(path.get_path_length()){
                    sum += path.get_path_length();
                    count++;
                }
            }
            return static_cast<float>(sum/count);
        }

        // returns PrettyPrint-able computation of average paths for Node x
        inline PrettyPrintParagraph pprint_avg_path(int x){
            vector<Path> paths = calc_shortest_paths(x);

            string header = "";
            if(is_seeded_graph())
                header +=
                    string("This Graph was seeded with the following parameters:\n")
                    + "\t\tsize:\t\t" + to_string(size) + "\n"
                    + "\t\tdensity:\t" + prettify_float(density) + "\n"
                    + "\t\tmin_dist:\t" + prettify_float(min_dist) + "\n"
                    + "\t\tmax_dist:\t" + prettify_float(max_dist) + "\n";
            header +=
                string("The Average Path from Node ")
                + to_string(x) + " ("
                + make_string_idx_from_int_idx(x)
                + ") to the rest of the Graph is: "
                + prettify_float(calc_average_path(x))
                + "\nThe distances and paths in the Graph are:\n";
            PrettyPrintParagraph printable_results = PrettyPrintParagraph(header);

            for(int i=0; i < size; i++){
                if (float path_length = paths[i].get_path_length()){
                    string row_distance_data =
                        "The distance to Node "
                        + to_string(i) + " ("
                        + make_string_idx_from_int_idx(i)
                        + ") is: " + prettify_float(path_length) + " = ";
                    string row_path_data = "The path taken is: ";

                    int prev_node;
                    for(auto& path_node : paths[i].get_path_nodes()){
                        if(path_node != x)
                            row_distance_data += prettify_float(adj_list[prev_node]->get_edge_value(path_node)) + " + ";
                        row_path_data += to_string(path_node) + " (" + make_string_idx_from_int_idx(path_node) + ") -> ";
                        prev_node = path_node;
                    }

                    if(paths[i].get_path_nodes().size() == 2) row_distance_data.erase(row_distance_data.find('='));
                    else row_distance_data.erase(row_distance_data.size()-3);
                    row_distance_data += "\n";
                    row_path_data.erase(row_path_data.size()-4);
                    row_path_data += "\n";

                    printable_results.add_row(row_distance_data + row_path_data);
                }
            }
            return printable_results;
        }
};

// Entry point
int main(){
    srandom(time(nullptr));
    NodeGraph graph1(GRAPH_SIZE, TEST_DENSITY, TEST_MIN_EDGE_COST, TEST_MAX_EDGE_COST);
    cout << graph1.pprint_avg_path(TEST_NODE_IDX);
    return 0;
}
