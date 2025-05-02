/*
Name: Hex Game implementation
Author: Alex Stet
Date: 12-07-2025 (dd-mm-yyyy)

Note:
    ANSI support is required!
    Windows 10   -> version 1607 or above
    macOS, linux -> any version
*/

// imports

#include<functional>
#include<iostream>
#include<limits>
#include<vector>
#include<list>
#include<set>
#include<map>

// ANSI macros

// colour codes from: https://gist.github.com/Kielx/2917687bc30f567d45e15a4577772b02
#define RESET       "\033[0m"       /* Reset to normal */
#define RED         "\033[31m"      /* Red */
#define BLUE        "\033[34m"      /* Blue */
#define WHITE       "\033[37m"      /* White */

// escape codes from: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
#define ERASE_LINE  "\x1b[2K"       /* Erases entire line */
#define MOVE_UP_ONE "\x1b[1A"       /* Moves cursor up 1 line */

// enums

enum class PLAYER_ID{
    NOT_SET,
    P1,
    P2
};

enum class PIECE_SYMBOL{
    EMPTY = '.',
    P1 = 'X',
    P2 = 'O'
};

// const map -> functionally used as an enum
// this is a workaround as enum values can't be strings
const std::map<std::string, std::string> PIECE_COLOUR = {
    {"EMPTY", WHITE},
    {"P1", BLUE},
    {"P1_NAME", "blue"},
    {"P2", RED},
    {"P2_NAME", "red"},
};

// constants

const int ALPHABET_SIZE = (static_cast<int>('Z') - static_cast<int>('A')) + 1;
const int ASCII_ALPHABET_START = static_cast<int>('A');
const int ASCII_ZERO = static_cast<int>('0');

// structs

struct gamepiece{
    PLAYER_ID p_id;
    PIECE_SYMBOL value;
    std::string colour;
};

// utility functions

// function that returns the number of digits inside an integer
inline int count_digits(int nr, int cnt=0){
    // edge case, the initial number is 0
    if(!(nr || cnt)) return 1;

    // base case
    if(nr) return count_digits(nr/10, ++cnt);
    return cnt;
}

// function that clears n lines in the terminal
inline void clear_lines(int n_lines){
    for(int i=0; i < n_lines; i++) std::cout << ERASE_LINE << MOVE_UP_ONE;
    std::cout << '\r';
    return;
}

/*
 * Converts an int index to a string index with the following rule:
 * A  = 0
 * Z  = 25
 * AA = 26
 * AB = 27
 * etc.
 */ 
inline static std::string make_string_idx_from_int_idx(int x, std::string rtrn_str=""){
    if(x / (ALPHABET_SIZE))
        return make_string_idx_from_int_idx(
            (x / ALPHABET_SIZE) - 1,
            static_cast<char>(x % ALPHABET_SIZE + ASCII_ALPHABET_START) + rtrn_str
        );
    else return char(x % ALPHABET_SIZE + ASCII_ALPHABET_START) + rtrn_str;
}

// generic truthy validator
template<class T>
inline bool empty_validator(void*, T& val){return true;}

/*
 * generic function that sanitises std::cin inputs
 * takes in an initial prompt string and a secondary
 * prompt string for use if the initial input fails
 * validation.
 *
 * accepts an optional validator function for the input
 * accepts optional context to pass the validator function
 */
template<class T>
inline T sanitise_input(
    std::string initial_prompt,
    std::string fail_prompt,
    const std::function<bool(void*, T&)>& validator_fn = &empty_validator<T>,
    void* _context = NULL
){
    T in_val;
    std::cout << initial_prompt; std::cin >> in_val;
    while(std::cin.fail() || !validator_fn(_context, in_val)){
        clear_lines(2);
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << fail_prompt; std::cin >> in_val;
    }
    return in_val;
}

// class prototypes

class HexBoard;

// classes

// class representing a cell inside a hex board
class HexCell{
    private:
        friend class HexBoard;
        friend class Player;
        friend std::ostream& operator<<(std::ostream& out_str, const HexBoard& board);

        const int id;
        std::list<int> neighbour_ids;
        gamepiece piece{PLAYER_ID::NOT_SET, PIECE_SYMBOL::EMPTY, PIECE_COLOUR.at("EMPTY")};

        HexCell(int id):id(id){}

    public:
        ~HexCell(){}
};

// class representing a hex board
class HexBoard{
    private:
        friend std::ostream& operator<<(std::ostream& out_str, const HexBoard& board);
        friend class Player;

        const std::string colour = WHITE;
        const int size;
        bool game_over = false;
        std::map<std::string, int> cell_idx_to_vector_pos;
        std::vector<HexCell> cells;
        std::map<PLAYER_ID, std::pair<std::set<int>, std::set<int>>> player_targets;

        void re_print_board(){
            clear_lines(size * 2);
            std::cout << *this;
        }

        HexCell& get_cell_by_id(std::string cell_id){
            return cells[cell_idx_to_vector_pos[cell_id]];
        }

        // depth first search
        // returns true if a path exists between the start cell
        // and at least one of the target cells
        bool find_permisive_path(int start_cell_id, const std::set<int>& targets, std::vector<bool>& seen_ids, std::list<int>& traverse_chain){
            // check target is found
            if(targets.find(start_cell_id) != targets.end()) return true;
            seen_ids[start_cell_id] = true;

            // traverse neighbour cells
            traverse_chain.push_back(start_cell_id);
            while(!traverse_chain.empty()){
                for(int next_cell_id : cells[start_cell_id].neighbour_ids){
                    if(cells[next_cell_id].piece.p_id != cells[start_cell_id].piece.p_id || seen_ids[next_cell_id]) continue;
                    return find_permisive_path(next_cell_id, targets, seen_ids, traverse_chain);
                }
                traverse_chain.pop_back();
                // *slightly* undefined behaviour as we eventually call back() on an empty list,
                // however we imediately break the while loop afterwards anyways.
                // Added benefit is this doesn't throw a read error, however on a vector it throws one!
                start_cell_id = traverse_chain.back();
            }
            return false;
        }

        // wrapper for find_permisive_path required because non-static references can't be used as default arguments
        bool find_permisive_path(int start_cell, const std::set<int>& targets){
            std::vector<bool> seen_ids(size*size, false);
            std::list<int> traverse_chain;
            return find_permisive_path(start_cell, targets, seen_ids, traverse_chain);
        }

        /*
         * method that checks if the game was won:
         * first check:
         *     1. red player has at least a piece on top row and bottom row each
         *     2. or blue player has at least a piece on the left side and right side each
         *
         * path finding algorithm:
         *     * optimisation * start search from the last populated cell (last_cell) <- if  a path exists, it MUST include this cell
         *     if a path is found for BOTH opposite board walls -> end the game
         *
         * * property * the hex game CANNOT end in a draw
         */
        bool check_win_state(const HexCell& last_cell){
            const PLAYER_ID &target_val = last_cell.piece.p_id;

            int populated_edges = 0;
            for(int idx : player_targets[target_val].first) if(cells[idx].piece.p_id == target_val){populated_edges++; break;}
            if(!populated_edges) return false;

            for(int idx : player_targets[target_val].second) if(cells[idx].piece.p_id == target_val){populated_edges++; break;}
            if(populated_edges == 1) return false;

            // two opposite populated edges for this player, time to path find
            if(
                find_permisive_path(last_cell.id, player_targets[target_val].first)
                && find_permisive_path(last_cell.id, player_targets[target_val].second)
            ){game_over = true; return true;}

            return false;
        }

        // method that emplaces a piece on the board, uses the grid path
        // to determine the cell position
        bool update_board_grid_path(std::string cell_id, const gamepiece &new_piece){
            HexCell& cell = get_cell_by_id(cell_id);
            cell.piece = new_piece;
            re_print_board();
            return check_win_state(cell);
        }

        // method that emplaces a piece on the board, uses the cell id
        // to determine the cell position
        bool update_board_cell_id(int cell_id, const gamepiece &new_piece){
            HexCell& cell = cells[cell_id];
            cell.piece = new_piece;
            re_print_board();
            return check_win_state(cell);
        }

        // method that returns leftward & upward neighbours (if they exist)
        // strategy: evaluate all possible (3) neighbours & only add valid ones to return vector
        std::list<int> get_previous_neighbours(int cell_id){
            std::list<int> prev_neighbours;

            // left neighbour (if it exists)
            if(cell_id % size) prev_neighbours.push_back(cell_id - 1);

            // previous row - up left neighbour (if it exists)
            if(cell_id >= size) prev_neighbours.push_back(cell_id - size);
            
            // previous row - up right neighbour (if it exists)
            if(cell_id >= size - 1 && cell_id % size != size - 1) prev_neighbours.push_back(cell_id - size + 1);

            return prev_neighbours;
        }

        // method that initialises a game board
        void seed_board(){
            for(int i=0; i<size*size; i++){
                cell_idx_to_vector_pos.emplace(make_string_idx_from_int_idx(i % size) + std::to_string(i / size + 1), i);
                cells.push_back(HexCell(i));

                for(int j : get_previous_neighbours(i)){
                    // add previous neighbours
                    cells[j].neighbour_ids.push_back(cells[i].id);

                    // add forward neighbours
                    cells[i].neighbour_ids.push_back(cells[j].id);
                }
            }
        }

        // method that generates the map of cells the players should connect to win the game,
        // the cells comprising the opposite walls are stored in two distinct sets, mapped to their
        // player id:
        //      for P1: the east and west walls
        //      for P2: the north and south walls
        void generate_player_targets(){
            std::set<int> p1_targets_1, p1_targets_2, p2_targets_1, p2_targets_2;
            for(int i = 0; i <= size*(size-1); i+=size) p1_targets_1.insert(i);
            for(int i = size-1; i < size*size; i+=size) p1_targets_2.insert(i);
            for(int i = 0; i < size; i++) p2_targets_1.insert(i);
            for(int i = size*(size-1); i < size*size; i++) p2_targets_2.insert(i);

            player_targets[PLAYER_ID::P1] = std::pair<std::set<int>, std::set<int>>(p1_targets_1, p1_targets_2);
            player_targets[PLAYER_ID::P2] = std::pair<std::set<int>, std::set<int>>(p2_targets_1, p2_targets_2);
        }

        // method that returns the matching cell colour if two given cells have the same colour,
        // otherwise returns the "EMPTY" PIECE_COLOUR
        std::string get_matching_colour(const HexCell& cell_1, const HexCell& cell_2)const{
            if(cell_1.piece.colour == cell_2.piece.colour)
                return cell_1.piece.colour;
            return PIECE_COLOUR.at("EMPTY");
        }

        // prints filler row connecting two game board rows,
        // used in the std::ostream& operator<< implementation
        std::string print_filler_row(int upper_row_lvl)const{
            std::string rtrn_string;

            std::string colour;
            const HexCell *current_cell, *top_left_cell, *top_right_cell;

            for(int i = 0; i < size-1; i++){
                current_cell = &cells[i + size * upper_row_lvl];
                top_left_cell = &cells[i + size * upper_row_lvl - size];
                top_right_cell = &cells[i + size * upper_row_lvl - size + 1];

                colour = get_matching_colour(*current_cell, *top_left_cell);
                rtrn_string += colour + "\\ ";

                colour = get_matching_colour(*current_cell, *top_right_cell);
                rtrn_string += colour + "/ ";
            }
            current_cell = &cells[size * (upper_row_lvl + 1) - 1];
            top_left_cell = &cells[size * upper_row_lvl - 1];
            colour = get_matching_colour(*current_cell, *top_right_cell);
            rtrn_string += colour + "\\\n";

            return rtrn_string;
        }

    public:
        HexBoard(int size):size(size){seed_board(); generate_player_targets();}
        ~HexBoard(){}

        bool game_is_running(){
            return !game_over;
        }
};

// class representing a hex game player
class Player{
    private:
        const PLAYER_ID id;
        HexBoard* game_board;
        gamepiece piece;
        const bool is_ai;

        // validator that checks a given move is valid
        bool move_validator(std::string& coordinates){
            for(int i = 0; i < coordinates.length(); i++) coordinates[i] = toupper(coordinates[i]);
            if(game_board->cell_idx_to_vector_pos.find(coordinates) != game_board->cell_idx_to_vector_pos.end())
                return game_board->get_cell_by_id(coordinates).piece.value == PIECE_SYMBOL::EMPTY;
            return false;
        }

        // wrapper function used to pass the instantiated object's context to the move_validator
        // used as a workaround to pass to the sanitise_input generic utility function, as it
        // can only accept static member functions
        static bool _enter_move_validator(void* _context, std::string& coordinates){
            return static_cast<Player*>(_context)->move_validator(coordinates);
        }

        // print win message
        void congratulate(){
            std::cout << "\nCongratulations " << piece.colour << "player " << static_cast<int>(id) <<  game_board->colour << " you won!\n";
        }

        // method used to prompt a player for a move
        void make_move_manual(){
            std::string cell_id = sanitise_input<std::string>(
                piece.colour + "\nEnter a move: " + WHITE,
                piece.colour + "\nInvalid move, try again (format i.e. a1 or B2): " + WHITE,
                &_enter_move_validator,
                this
            );
            clear_lines(2);
            bool has_won = game_board->update_board_grid_path(cell_id, piece);
            if(has_won){
                congratulate();
            }
        }

        // method that enables the ai to make it's own moves
        // currently only makes random moves
        void make_move_ai(){
            int cell_id = rand() % (game_board->size * game_board->size);
            while(game_board->cells[cell_id].piece.p_id != PLAYER_ID::NOT_SET){
                cell_id = rand() % (game_board->size * game_board->size);
            }

            bool has_won = game_board->update_board_cell_id(cell_id, piece);
            if(has_won){
                congratulate();
            }
        }

    public:
        Player(HexBoard* game_board, gamepiece piece, bool is_ai=false): id(piece.p_id), game_board(game_board), piece(piece), is_ai(is_ai){}
        ~Player(){}

        // wraper method that enables manual or ai moves to be made
        void make_move(){
            if(is_ai) return make_move_ai();
            return make_move_manual();
        }

        // returns a string like: <colour>[<player_id>]
        static std::string format_colour_option(PLAYER_ID p_id){
            if(p_id == PLAYER_ID::NOT_SET) throw std::invalid_argument("Invalid player id!");
            return PIECE_COLOUR.at(std::string("P") + std::to_string(static_cast<int>(p_id)))
            + static_cast<std::string>(PIECE_COLOUR.at(std::string("P") + std::to_string(static_cast<int>(p_id)) + "_NAME"))
            + "[" + std::to_string(static_cast<int>(p_id))
            + "]" + WHITE;
        }
};

// ostream operator<< implementations

// print HexBoard
inline std::ostream& operator<<(std::ostream& out_str, const HexBoard& board){
    const int LEFT_PADDING = 4;
    int lvl = 0;

    // print column ids
    out_str << PIECE_COLOUR.at("P2") << std::string(LEFT_PADDING, ' ');
    for(int i=0; i < board.size; i++) out_str << make_string_idx_from_int_idx(i) << std::string(LEFT_PADDING - 1, ' '); out_str << board.colour << '\n';

    // initialise first row padding maually
    out_str << PIECE_COLOUR.at("P1") << lvl+1 << board.colour << std::string(LEFT_PADDING-1, ' ');
    for(auto& cell : board.cells){
        // print cell value
        out_str << cell.piece.colour << static_cast<char>(cell.piece.value) << board.colour;

        // break on last row
        if(cell.id == board.size*board.size - 1){out_str << '\n'; break;};

        // print new row
        !((cell.id+1) % board.size) ?
            // 1. increment offset
            lvl++, out_str << '\n'

            // 2. print filler row with paths between levels
            << std::string(LEFT_PADDING + 2*lvl - 1, ' ')
            << board.print_filler_row(lvl)

            // 3. print row ids
            << PIECE_COLOUR.at("P1") << lvl+1 << board.colour

            // 4. start new row
            << std::string(LEFT_PADDING + 2*lvl - count_digits(lvl+1), ' ')

                // ...or continue row
                : out_str << board.get_matching_colour(cell, board.cells[cell.id+1]) << " - " << board.colour;
    }
    return out_str;
}

// entry point
int main(){
    srand(time(NULL));
    std::cout << WHITE; clear_lines(100);

    // print game intro message & await confirmation
    std::cout
    << PIECE_COLOUR.at("P1") << "Player 1" << WHITE << " must make a connected set of " << PIECE_COLOUR.at("P1") << PIECE_COLOUR.at("P1_NAME") << WHITE << " hexagons from east to west.\n"
    << PIECE_COLOUR.at("P2") << "Player 2" << WHITE << " must make a connected set of " << PIECE_COLOUR.at("P2") << PIECE_COLOUR.at("P2_NAME") << WHITE << " hexagons from north to south.\n"
    << "At each turn a player chooses an unoccupied hexagon and gives it their colour.\n"
    << "Unlike tic-tac-toe the game cannot end in a draw.\n\n"
    << "Press ENTER to continue..."; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cin.clear();
    clear_lines(7);

    // query game parameters
    bool ai_switch = sanitise_input<bool>(
        "Play against Human[0] or AI[1]? ",
        "Invalid option, please choose Human[0] or AI[1]: "
    );
    clear_lines(2);

    int colour_switch = 0;
    if(ai_switch){
        colour_switch = sanitise_input<int>(
            std::string("Choose your colour ") + Player::format_colour_option(PLAYER_ID::P1)
            + " or " + Player::format_colour_option(PLAYER_ID::P2) + ": ",
            "Invalid option, please choose " + Player::format_colour_option(PLAYER_ID::P1)
            + " or " + Player::format_colour_option(PLAYER_ID::P2) + ": ",
            [](void*, int& val) -> bool {return val == 1 || val == 2;}
        );
        clear_lines(2);
    }

    // initialise the game board
    int b_size = sanitise_input<int>(
        "Enter board size between [5 - 11]: ",
        "Invalid board size given, please choose a value between [5 - 11]: ",
        [](void*, int& val) -> bool {return val > 4 && val < 12;}
    ); HexBoard game_board(b_size);
    clear_lines(2);

    // initialise players
    Player  p1(&game_board, gamepiece{PLAYER_ID::P1, PIECE_SYMBOL::P1, PIECE_COLOUR.at("P1")}, ai_switch && PLAYER_ID(colour_switch) == PLAYER_ID::P2),
            p2(&game_board, gamepiece{PLAYER_ID::P2, PIECE_SYMBOL::P2, PIECE_COLOUR.at("P2")}, ai_switch && PLAYER_ID(colour_switch) == PLAYER_ID::P1);

    // game loop
    std::map<bool, Player*> game_players = {
        {true, &p1},
        {false, &p2}
    }; bool cur_player = true;

    std::cout << game_board;
    while(game_board.game_is_running()){
        game_players[cur_player]->make_move();
        cur_player = !cur_player;
    }

    std::cout << RESET; return 0;
}
