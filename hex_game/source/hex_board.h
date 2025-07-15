#ifndef HEX_BOARD_H
#define HEX_BOARD_H

#include "utils.h"
#include "player.h"

#include <vector>
#include <iostream>

#define VIRTUAL_PIECE ID_ENUM

struct GamePiece
{
    std::string colour;
    std::string piece;
};

class HexBoardReal;
class HexBoardVirtual;

class HexBoardABC
{
    friend class HexBoardReal;
    friend class HexBoardVirtual;

    protected:
        const u_int size;
        std::vector<std::vector<VIRTUAL_PIECE>> game_board;
        virtual std::string serialise()=0;
    
    public:
        HexBoardABC(u_int size):size(size), game_board(generate_board()){}
        HexBoardABC(HexBoardABC* root_board):size(root_board->size), game_board(root_board->game_board){}

        virtual ~HexBoardABC(){}

        virtual std::vector<std::vector<VIRTUAL_PIECE>> generate_board();

        friend std::ostream& operator<<(std::ostream&, HexBoardABC*);
};

class HexBoardReal:public HexBoardABC
{
    protected:
        static std::string get_piece_symbol(VIRTUAL_PIECE);
        std::string serialise();

    public:
        HexBoardReal(u_int size):HexBoardABC(size){}

        ~HexBoardReal(){}
};

class HexBoardVirtual:public HexBoardABC
{
    protected:
        std::vector<std::vector<VIRTUAL_PIECE>>&& root_board;

    public:
        HexBoardVirtual(HexBoardABC* root_board):HexBoardABC(root_board), root_board(std::move(root_board->game_board)){}

        ~HexBoardVirtual(){}

        std::vector<std::vector<VIRTUAL_PIECE>> generate_board() override {throw UNDEFINED_BEHAVIOUR_ERROR;}
        std::string serialise(){throw UNDEFINED_BEHAVIOUR_ERROR;}
};

class HexBoardFactory
{
    public:
        static HexBoardABC* make(HexBoardABC*);
        static HexBoardABC* make(u_int);
        static void init_boards(HexBoardABC*&, HexBoardABC*&, u_int);
};

#endif
