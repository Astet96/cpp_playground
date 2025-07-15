#ifndef HEX_BOARD_H
#define HEX_BOARD_H

#include "utils.h"

#include <vector>
#include <iostream>
#include <unordered_map>
#include <map>
#include <set>
#include <list>
#include <cstdarg>

#define VIRTUAL_PIECE ID_ENUM
#define BoardType REAL_VIRTUAL

// structs

struct GamePiece
{
    std::string colour;
    std::string piece;
};

// enums

enum class NEIGHBOUR
{
    UP_LEFT,
    UP_RIGHT,
    ROW_RIGHT,
    ROW_LEFT,
    DOWN_RIGHT,
    DOWN_LEFT,
};

// consts

const std::unordered_map<NEIGHBOUR, std::pair<int, int>> DIRECTION_OFFSET =
    {
        {NEIGHBOUR::UP_LEFT, std::pair<int, int>{-1, 0}},
        {NEIGHBOUR::UP_RIGHT, std::pair<int, int>{-1, 1}},
        {NEIGHBOUR::ROW_RIGHT, std::pair<int, int>{0, 1}},
        {NEIGHBOUR::ROW_LEFT, std::pair<int, int>{0, -1}},
        {NEIGHBOUR::DOWN_RIGHT, std::pair<int, int>{1, 0}},
        {NEIGHBOUR::DOWN_LEFT, std::pair<int, int>{1, -1}},
};

// cannot hash pairs out of the box so use map instead
const std::map<std::pair<int, int>, std::string> PIECE_SEPARATORS =
    {
        {std::pair<int, int>{-1, 0}, "\\"},
        {std::pair<int, int>{-1, 1}, "/"},
        {std::pair<int, int>{0, 1}, "-"},
};

// class prototypes

class HexBoardReal;
class HexBoardVirtual;

// HexBoard abstract base class
class HexBoardABC
{
    friend class HexBoardReal;
    friend class HexBoardVirtual;

protected:
    bool *win_state = new bool(false);
    const u_int size;
    const std::unordered_map<VIRTUAL_PIECE, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>> player_targets;
    std::vector<std::vector<VIRTUAL_PIECE>> game_board;

    virtual std::string serialise() = 0;
    virtual void update_board(u_int, u_int, VIRTUAL_PIECE) = 0;

public:
    HexBoardABC(u_int size) : size(size), game_board(generate_board()), player_targets(generate_player_targets()) {}
    HexBoardABC(HexBoardABC *root_board) : size(root_board->size), win_state(root_board->win_state), game_board(root_board->game_board), player_targets(root_board->player_targets) {}
    virtual ~HexBoardABC() {}

    std::vector<std::vector<VIRTUAL_PIECE>> get_game_board() { return game_board; }
    bool get_win_state() { return *win_state; }
    u_int get_size() { return size; }

    virtual BoardType get_board_type() = 0;

    virtual std::vector<std::vector<VIRTUAL_PIECE>> generate_board();
    virtual std::unordered_map<VIRTUAL_PIECE, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>> generate_player_targets();
    bool player_has_won(VIRTUAL_PIECE);
    virtual void update(va_list args);
    void check_win_on_move(u_int, u_int, VIRTUAL_PIECE);
    bool find_any_path_one_to_many(std::pair<u_int, u_int>, const std::set<std::pair<u_int, u_int>> &, std::vector<std::vector<bool>> = std::vector<std::vector<bool>>(), std::list<std::pair<u_int, u_int>> = std::list<std::pair<u_int, u_int>>());

    friend std::ostream &operator<<(std::ostream &, HexBoardABC *);
};

// Actual Hex game board
class HexBoardReal : public HexBoardABC
{
private:
    const std::map<std::string, std::pair<u_int, u_int>> str_cell_id_map;

    static std::map<std::string, std::pair<u_int, u_int>> generate_str_cell_id_map(u_int);

    std::string get_colour_between_pieces(std::pair<int, int>, NEIGHBOUR);
    std::string get_separator_between_pieces(std::pair<int, int>, NEIGHBOUR);
    std::string generate_separator_row(int);

    void box_board(std::string &);
    void add_top_padding(std::string &, std::string);
    void add_left_padding(std::string &, std::string);
    void add_right_padding(std::string &, std::string);
    void add_bottom_padding(std::string &, std::string);

    void label_board(std::string &);
    void add_col_labels(std::string &);
    void add_row_labels(std::string &);

protected:
    static std::string get_piece_symbol(VIRTUAL_PIECE);
    std::string serialise();
    void update_board(u_int, u_int, VIRTUAL_PIECE);

public:
    HexBoardReal(u_int size) : HexBoardABC(size), str_cell_id_map(generate_str_cell_id_map(size)) {}

    ~HexBoardReal() { delete win_state; }

    BoardType get_board_type();
    std::pair<u_int, u_int> get_cell_by_str_id(std::string);
    bool cell_is_populated(std::string &);
};

// Virtual Hex game board used for montecarlo simulations
class HexBoardVirtual : public HexBoardABC
{
protected:
    std::vector<std::vector<VIRTUAL_PIECE>> &&root_board;
    std::string serialise() { throw UNDEFINED_BEHAVIOUR_ERROR; }
    void update_board(u_int, u_int, VIRTUAL_PIECE);
    bool thread_safe_player_has_won(std::vector<std::vector<VIRTUAL_PIECE>>, VIRTUAL_PIECE);
    bool thread_safe_find_any_path_one_to_many(std::vector<std::vector<VIRTUAL_PIECE>>, std::pair<u_int, u_int>, const std::set<std::pair<u_int, u_int>> &, std::vector<std::vector<bool>> = std::vector<std::vector<bool>>(), std::list<std::pair<u_int, u_int>> = std::list<std::pair<u_int, u_int>>());
    void thread_safe_montecarlo_sim(atomwrapper<int> &, std::vector<std::pair<u_int, u_int>>, int, VIRTUAL_PIECE, u_int);
    std::vector<std::pair<u_int, u_int>> get_possible_moves();

public:
    HexBoardVirtual(HexBoardABC *root_board) : HexBoardABC(root_board), root_board(std::move(root_board->game_board)) {}

    ~HexBoardVirtual() {}

    BoardType get_board_type();
    std::pair<u_int, u_int> generate_move(VIRTUAL_PIECE);
    std::vector<std::vector<VIRTUAL_PIECE>> generate_board() override { throw UNDEFINED_BEHAVIOUR_ERROR; }
    virtual std::unordered_map<VIRTUAL_PIECE, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>> generate_player_targets() override { throw UNDEFINED_BEHAVIOUR_ERROR; }
};

// Hex board factory
class HexBoardFactory
{
public:
    static HexBoardABC *make(HexBoardABC *);
    static HexBoardABC *make(u_int);
    static void init_boards(HexBoardABC *&, HexBoardABC *&, u_int, bool);
};

#endif
