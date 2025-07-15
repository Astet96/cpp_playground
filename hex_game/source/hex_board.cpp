#include "hex_board.h"
#include "player.h"


std::string HexBoardReal::get_piece_symbol(VIRTUAL_PIECE piece)
{
    return std::string(1, static_cast<char>(PLAYER_PIECE.at(static_cast<PLAYER_ID>(piece))));
}

std::string HexBoardReal::serialise()
{
    std::string out_str;
    for(int i = 0; i < size; i++)
    {
        out_str += std::string(i, ' ');
        for(int j = 0; j < size; j++)
        {
            out_str += get_piece_symbol(game_board[i][j]) + ((j!=size-1) ? " - " : "");
        }
        out_str;
        out_str += '\n';
    }
    return out_str;
}

std::ostream& operator<<(std::ostream& out_str, HexBoardABC* board)
{
    out_str << board->serialise();
    return out_str;
}

std::vector<std::vector<VIRTUAL_PIECE>> HexBoardABC::generate_board()
{
    return std::vector<std::vector<VIRTUAL_PIECE>>(size, std::vector<VIRTUAL_PIECE>(size, VIRTUAL_PIECE::NOT_SET));
}

HexBoardABC* HexBoardFactory::make(u_int size)
{
    return new HexBoardReal(size);
}

HexBoardABC* HexBoardFactory::make(HexBoardABC* game_board)
{
    return new HexBoardVirtual(game_board);
}

void HexBoardFactory::init_boards(HexBoardABC*& game_board, HexBoardABC*& virtual_board, u_int board_size)
{
    game_board = HexBoardFactory::make(board_size);
    virtual_board = HexBoardFactory::make(game_board);
}
