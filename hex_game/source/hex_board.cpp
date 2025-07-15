#include "utils.h"
#include "hex_board.h"
#include "player.h"

std::string HexBoardReal::get_colour_between_pieces(std::pair<int, int> piece_idx, NEIGHBOUR direction)
{
    std::pair<int, int> offset = DIRECTION_OFFSET.at(direction);

    VIRTUAL_PIECE this_piece, next_piece;
    this_piece = game_board[piece_idx.first][piece_idx.second];
    next_piece = game_board[piece_idx.first + offset.first][piece_idx.second + offset.second];

    if(this_piece != next_piece)
        return PLAYER_COLOUR.at(VIRTUAL_PIECE::NOT_SET).ANSII;
    return PLAYER_COLOUR.at(this_piece).ANSII;
}

std::string HexBoardReal::get_separator_between_pieces(std::pair<int, int> piece_idx, NEIGHBOUR direction)
{
    std::string sep_colour = get_colour_between_pieces(piece_idx, direction);
    return sep_colour + PIECE_SEPARATORS.at(DIRECTION_OFFSET.at(direction)) + WHITE;
}

std::string HexBoardReal::get_piece_symbol(VIRTUAL_PIECE piece)
{
    return PLAYER_COLOUR.at(piece).ANSII + std::string(1, static_cast<char>(PLAYER_PIECE.at(static_cast<PLAYER_ID>(piece)))) + WHITE;
}

std::string HexBoardReal::generate_separator_row(int next_row_lvl)
{
    std::string sep_row(2 * next_row_lvl - 1, ' ');
    for(int i = 0; i < size - 1; i++)
    {
        sep_row += get_separator_between_pieces({next_row_lvl, i}, NEIGHBOUR::UP_LEFT) + " ";
        sep_row += get_separator_between_pieces({next_row_lvl, i}, NEIGHBOUR::UP_RIGHT) + " ";
    }
    sep_row += get_separator_between_pieces({next_row_lvl, size - 1}, NEIGHBOUR::UP_LEFT);

    return sep_row + "\n";
}

void HexBoardReal::add_top_padding(std::string& serialised_board, std::string colour)
{
    std::string padding = "  " + colour + std::string(4*size-2, '_') + WHITE + '\n';
    serialised_board = padding + serialised_board;
}

void HexBoardReal::add_left_padding(std::string& serialised_board, std::string colour)
{
    std::string padding, padding_base = colour + "\\" + WHITE;

    serialised_board = padding_base + serialised_board;
    size_t endl_pos = serialised_board.find('\n');

    for(int i=0; i<2*size-2; i++)
    {
        padding = '\n' + std::string(i+1 ,' ') + padding_base;
        serialised_board.replace(endl_pos, i+2, padding);
        endl_pos = serialised_board.find('\n', endl_pos + size);
    }
}

void HexBoardReal::add_right_padding(std::string& serialised_board, std::string colour)
{
    std::string padding = colour + '\\' + WHITE + '\n';
    size_t endl_pos = serialised_board.find('\n');
    while(endl_pos != std::string::npos)
    {
        serialised_board.replace(endl_pos, 1, padding);
        endl_pos = serialised_board.find('\n', endl_pos + padding.size());
    }
}

void HexBoardReal::add_bottom_padding(std::string& serialised_board, std::string colour)
{
    std::string padding = std::string(2*size-1, ' ') + colour;
    for(int i=0; i< 4*size-2; i++) padding += "¯";
    padding += WHITE;
    padding += '\n';
    serialised_board += padding;
}

void HexBoardReal::box_board(std::string& serialised_board)
{
    add_left_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P1).ANSII);
    add_right_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P1).ANSII);
    add_top_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P2).ANSII);
    add_bottom_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P2).ANSII);
}

void HexBoardReal::add_col_labels(std::string& boxed_serialised_board)
{
    std::string col_labels = std::string(2*size+1, ' ');
    for(int i=0; i<size; i++) col_labels += make_string_idx_from_int_idx(i) + "   ";
    col_labels += '\n';
    boxed_serialised_board += col_labels;
}

void HexBoardReal::add_row_labels(std::string& boxed_serialised_board)
{
    size_t endl_pos = boxed_serialised_board.find('\n');
    for(int i=1; i<size+1; i++)
    {
        std::string row_idx = std::to_string(i);
        boxed_serialised_board.replace(endl_pos, row_idx.size(), '\n' + row_idx + ' ');
        endl_pos = boxed_serialised_board.find('\n', endl_pos + 2);
        boxed_serialised_board.replace(endl_pos, 1, "\n  ");
        endl_pos = boxed_serialised_board.find('\n', endl_pos + 2);
    }
}

void HexBoardReal::label_board(std::string& boxed_serialised_board)
{
    add_col_labels(boxed_serialised_board);
    add_row_labels(boxed_serialised_board);
}

std::string HexBoardReal::serialise()
{
    std::string out_str;
    for(int i = 0; i < size; i++)
    {
        out_str += std::string(2 * i, ' ');
        for(int j = 0; j < size; j++)
        {
            out_str += get_piece_symbol(game_board[i][j]) + ((j!=size-1) ? " " + get_separator_between_pieces({i, j}, NEIGHBOUR::ROW_RIGHT) + " " : "\n");
        }
        out_str += (i < size-1) ? generate_separator_row(i+1) : "";
    }
    box_board(out_str);
    label_board(out_str);
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

std::map<std::string, std::pair<u_int, u_int>> HexBoardReal::seed_str_cell_id_map(u_int size)
{
    std::map<std::string, std::pair<u_int, u_int>> str_cell_id_map;

    for(u_int i=0; i < size; i++)
        for(u_int j=0; j < size; j++)
            str_cell_id_map.emplace(make_string_idx_from_int_idx((j)) + std::to_string(i+1), std::pair<u_int, u_int>{i, j});

    return str_cell_id_map;
}

std::pair<u_int, u_int> HexBoardReal::get_cell_by_str_id(std::string cell_id)
{
    return str_cell_id_map.at(cell_id);
}

bool HexBoardReal::cell_is_populated(std::string& cell_id)
{
    for(int i = 0; i < cell_id.length(); i++) cell_id[i] = toupper(cell_id[i]);

    u_int x_coord, y_coord;
    try
    {
        x_coord = str_cell_id_map.at(cell_id).first;
        y_coord = str_cell_id_map.at(cell_id).second;
    }
    catch(const std::out_of_range)
    {
        return false;
    }
    return game_board[x_coord][y_coord] == VIRTUAL_PIECE::NOT_SET;
}

void HexBoardABC::update_board(u_int x_coord, u_int y_coord, VIRTUAL_PIECE piece_id)
{
    game_board[x_coord][y_coord] = piece_id;
}

HexBoardABC* HexBoardFactory::make(u_int size)
{
    return new HexBoardReal(size);
}

HexBoardABC* HexBoardFactory::make(HexBoardABC* game_board)
{
    return new HexBoardVirtual(game_board);
}

void HexBoardFactory::init_boards(HexBoardABC*& game_board, HexBoardABC*& virtual_board, u_int board_size, bool ai_switch)
{
    game_board = HexBoardFactory::make(board_size);
    if(ai_switch)
        virtual_board = HexBoardFactory::make(game_board);
    else
        virtual_board = nullptr;
}
