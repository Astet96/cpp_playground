#ifndef HEX_BOARD_H
#define HEX_BOARD_H

#include "utils.h"

#include <vector>
#include <iostream>
#include <unordered_map>
#include <map>

#define VIRTUAL_PIECE ID_ENUM

struct GamePiece
{
    std::string colour;
    std::string piece;
};

enum class NEIGHBOUR
{
    UP_LEFT,
    UP_RIGHT,
    ROW_RIGHT,
};

const std::unordered_map<NEIGHBOUR, std::pair<int, int>>DIRECTION_OFFSET =
{
    {NEIGHBOUR::UP_LEFT, std::pair<int, int>{-1, 0}},
    {NEIGHBOUR::UP_RIGHT, std::pair<int, int>{-1, 1}},
    {NEIGHBOUR::ROW_RIGHT, std::pair<int, int>{0, 1}},
};

// cannot hash pairs out of the box so use map instead
const std::map<std::pair<int, int>, std::string>PIECE_SEPARATORS =
{
    {std::pair<int, int>{-1, 0}, "\\"},
    {std::pair<int, int>{-1, 1}, "/"},
    {std::pair<int, int>{0, 1}, "-"},
};

class HexBoardReal;
class HexBoardVirtual;

class HexBoardABC
{
    friend class HexBoardReal;
    friend class HexBoardVirtual;

    protected:
        bool win_state=false;
        const u_int size;
        std::vector<std::vector<VIRTUAL_PIECE>> game_board;
        virtual std::string serialise()=0;
    
    public:
        HexBoardABC(u_int size):size(size), game_board(generate_board()){}
        HexBoardABC(HexBoardABC* root_board):size(root_board->size), game_board(root_board->game_board){}

        virtual ~HexBoardABC(){}

        virtual std::vector<std::vector<VIRTUAL_PIECE>> generate_board();

        friend std::ostream& operator<<(std::ostream&, HexBoardABC*);

        bool get_win_state(){return win_state;}
        std::vector<std::vector<VIRTUAL_PIECE>> get_game_board(){return game_board;}
        
        virtual bool cell_is_populated(std::string&)=0;

        u_int get_size(){return size;}
        void update_board(u_int, u_int, VIRTUAL_PIECE);
};

class HexBoardReal:public HexBoardABC
{
    private:
        const std::map<std::string, std::pair<u_int, u_int>> str_cell_id_map;

        static std::map<std::string, std::pair<u_int, u_int>> seed_str_cell_id_map(u_int);

        std::string get_colour_between_pieces(std::pair<int, int>, NEIGHBOUR);
        std::string get_separator_between_pieces(std::pair<int, int>, NEIGHBOUR);
        std::string generate_separator_row(int);
        void box_board(std::string&);
        void add_top_padding(std::string&, std::string);
        void add_left_padding(std::string&, std::string);
        void add_right_padding(std::string&, std::string);
        void add_bottom_padding(std::string&, std::string);
        void label_board(std::string&);
        void add_col_labels(std::string&);
        void add_row_labels(std::string&);

    protected:
        static std::string get_piece_symbol(VIRTUAL_PIECE);
        std::string serialise();

    public:
        HexBoardReal(u_int size):HexBoardABC(size), str_cell_id_map(seed_str_cell_id_map(size)){}

        ~HexBoardReal(){}

        std::pair<u_int, u_int> get_cell_by_str_id(std::string);
        bool cell_is_populated(std::string&);
};

class HexBoardVirtual:public HexBoardABC
{
    protected:
        std::vector<std::vector<VIRTUAL_PIECE>>&& root_board;
        std::string serialise(){throw UNDEFINED_BEHAVIOUR_ERROR;}

    public:
        HexBoardVirtual(HexBoardABC* root_board):HexBoardABC(root_board), root_board(std::move(root_board->game_board)){}

        ~HexBoardVirtual(){}

        std::vector<std::vector<VIRTUAL_PIECE>> generate_board() override {throw UNDEFINED_BEHAVIOUR_ERROR;}
        bool cell_is_populated(std::string&){throw UNDEFINED_BEHAVIOUR_ERROR;}
};

class HexBoardFactory
{
    public:
        static HexBoardABC* make(HexBoardABC*);
        static HexBoardABC* make(u_int);
        static void init_boards(HexBoardABC*&, HexBoardABC*&, u_int, bool);
};

#endif
