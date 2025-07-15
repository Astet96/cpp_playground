#include "utils.h"
#include "hex_board.h"
#include "player.h"

#include <algorithm>
#include <random>
#include <thread>

// consts

const int SIM_ITERATIONS = 3000;

// print the game board
std::ostream &operator<<(std::ostream &out_str, HexBoardABC *board)
{
    out_str << board->serialise();
    return out_str;
}

// returns the board type (real or virtual)
BoardType HexBoardReal::get_board_type()
{
    return BoardType::Real;
}

// returns the board type (real or virtual)
BoardType HexBoardVirtual::get_board_type()
{
    return BoardType::Virtual;
}

// generates an empty board of a given size
std::vector<std::vector<VIRTUAL_PIECE>> HexBoardABC::generate_board()
{
    return std::vector<std::vector<VIRTUAL_PIECE>>(size, std::vector<VIRTUAL_PIECE>(size, VIRTUAL_PIECE::NOT_SET));
}

// generates map of cell string names to matrix positions
std::map<std::string, std::pair<u_int, u_int>> HexBoardReal::generate_str_cell_id_map(u_int size)
{
    std::map<std::string, std::pair<u_int, u_int>> str_cell_id_map;

    for (u_int i = 0; i < size; i++)
        for (u_int j = 0; j < size; j++)
            str_cell_id_map.emplace(make_string_idx_from_int_idx((j)) + std::to_string(i + 1), std::pair<u_int, u_int>{i, j});

    return str_cell_id_map;
}

// generates the sets of nodes each player must connect to win
std::unordered_map<VIRTUAL_PIECE, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>> HexBoardABC::generate_player_targets()
{
    std::unordered_map<VIRTUAL_PIECE, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>> targets =
        {
            {VIRTUAL_PIECE::P1, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>{}},
            {VIRTUAL_PIECE::P2, std::pair<std::set<std::pair<u_int, u_int>>, std::set<std::pair<u_int, u_int>>>{}},
        };

    std::set<std::pair<u_int, u_int>> sub_targets;

    for (u_int i = 0; i < size; i++)
    {
        sub_targets.emplace(std::pair<u_int, u_int>{i, 0});
    }
    targets.at(VIRTUAL_PIECE::P1).first = sub_targets;
    sub_targets = {};

    for (u_int i = 0; i < size; i++)
    {
        sub_targets.emplace(std::pair<u_int, u_int>{i, size - 1});
    }
    targets.at(VIRTUAL_PIECE::P1).second = sub_targets;
    sub_targets = {};

    for (u_int i = 0; i < size; i++)
    {
        sub_targets.emplace(std::pair<u_int, u_int>{0, i});
    }
    targets.at(VIRTUAL_PIECE::P2).first = sub_targets;
    sub_targets = {};

    for (u_int i = 0; i < size; i++)
    {
        sub_targets.emplace(std::pair<u_int, u_int>{size - 1, i});
    }
    targets.at(VIRTUAL_PIECE::P2).second = sub_targets;

    return targets;
}

// returns true if a path exists from the starting node to any of the target nodes
bool HexBoardABC::find_any_path_one_to_many(std::pair<u_int, u_int> start_cell, const std::set<std::pair<u_int, u_int>> &targets, std::vector<std::vector<bool>> seen_ids, std::list<std::pair<u_int, u_int>> traverse_chain)
{
    if (!seen_ids.size())
        seen_ids = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));

    if (targets.find(start_cell) != targets.end())
        return true;
    seen_ids[start_cell.first][start_cell.second] = true;

    traverse_chain.push_back(start_cell);
    while (!traverse_chain.empty())
    {
        std::pair<u_int, u_int> next_cell;
        for (auto next_cell_offsets : DIRECTION_OFFSET)
        {
            next_cell = {start_cell.first + next_cell_offsets.second.first, start_cell.second + next_cell_offsets.second.second};

            if (next_cell.first < 0 || next_cell.first >= size || next_cell.second < 0 || next_cell.second >= size)
                continue;

            if (game_board[start_cell.first][start_cell.second] != game_board[next_cell.first][next_cell.second] || seen_ids[next_cell.first][next_cell.second])
                continue;

            return find_any_path_one_to_many(next_cell, targets, seen_ids, traverse_chain);
        }
        traverse_chain.pop_back();
        start_cell = traverse_chain.back();
    }
    return false;
}

// returns the colour used between two adgacent pieces
std::string HexBoardReal::get_colour_between_pieces(std::pair<int, int> piece_idx, NEIGHBOUR direction)
{
    std::pair<int, int> offset = DIRECTION_OFFSET.at(direction);

    VIRTUAL_PIECE this_piece, next_piece;
    this_piece = game_board[piece_idx.first][piece_idx.second];
    next_piece = game_board[piece_idx.first + offset.first][piece_idx.second + offset.second];

    if (this_piece != next_piece)
        return PLAYER_COLOUR.at(VIRTUAL_PIECE::NOT_SET).ANSII;
    return PLAYER_COLOUR.at(this_piece).ANSII;
}

// returns the type of separator used between two adgacent pieces
std::string HexBoardReal::get_separator_between_pieces(std::pair<int, int> piece_idx, NEIGHBOUR direction)
{
    std::string sep_colour = get_colour_between_pieces(piece_idx, direction);
    return sep_colour + PIECE_SEPARATORS.at(DIRECTION_OFFSET.at(direction)) + WHITE;
}

// returns the piece symbol
std::string HexBoardReal::get_piece_symbol(VIRTUAL_PIECE piece)
{
    return PLAYER_COLOUR.at(piece).ANSII + std::string(1, static_cast<char>(PLAYER_PIECE.at(static_cast<PLAYER_ID>(piece)))) + WHITE;
}

// generates a row of separators which connect the pieces located above and below
std::string HexBoardReal::generate_separator_row(int next_row_lvl)
{
    std::string sep_row(2 * next_row_lvl - 1, ' ');
    for (int i = 0; i < size - 1; i++)
    {
        sep_row += get_separator_between_pieces({next_row_lvl, i}, NEIGHBOUR::UP_LEFT) + " ";
        sep_row += get_separator_between_pieces({next_row_lvl, i}, NEIGHBOUR::UP_RIGHT) + " ";
    }
    sep_row += get_separator_between_pieces({next_row_lvl, size - 1}, NEIGHBOUR::UP_LEFT);

    return sep_row + "\n";
}

// adds coloured padding to the top of the board
void HexBoardReal::add_top_padding(std::string &serialised_board, std::string colour)
{
    std::string padding = "  " + colour + std::string(4 * size - 2, '_') + WHITE + '\n';
    serialised_board = padding + serialised_board;
}

// adds coloured padding to the left of the board
void HexBoardReal::add_left_padding(std::string &serialised_board, std::string colour)
{
    std::string padding, padding_base = colour + "\\" + WHITE;

    serialised_board = padding_base + serialised_board;
    size_t endl_pos = serialised_board.find('\n');

    for (int i = 0; i < 2 * size - 2; i++)
    {
        padding = '\n' + std::string(i + 1, ' ') + padding_base;
        serialised_board.replace(endl_pos, i + 2, padding);
        endl_pos = serialised_board.find('\n', endl_pos + size);
    }
}

// adds coloured padding to the right of the board
void HexBoardReal::add_right_padding(std::string &serialised_board, std::string colour)
{
    std::string padding = colour + '\\' + WHITE + '\n';
    size_t endl_pos = serialised_board.find('\n');
    while (endl_pos != std::string::npos)
    {
        serialised_board.replace(endl_pos, 1, padding);
        endl_pos = serialised_board.find('\n', endl_pos + padding.size());
    }
}

// adds coloured padding to the bottom of the board
void HexBoardReal::add_bottom_padding(std::string &serialised_board, std::string colour)
{
    std::string padding = std::string(2 * size - 1, ' ') + colour;
    for (int i = 0; i < 4 * size - 2; i++)
        padding += "¯";
    padding += WHITE;
    padding += '\n';
    serialised_board += padding;
}

// boxes the visual game board representation with walls coloured according to player targets
void HexBoardReal::box_board(std::string &serialised_board)
{
    add_left_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P1).ANSII);
    add_right_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P1).ANSII);
    add_top_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P2).ANSII);
    add_bottom_padding(serialised_board, PLAYER_COLOUR.at(PLAYER_ID::P2).ANSII);
}

// adds column labels to the board
void HexBoardReal::add_col_labels(std::string &boxed_serialised_board)
{
    std::string col_labels = std::string(2 * size + 1, ' ');
    for (int i = 0; i < size; i++)
        col_labels += make_string_idx_from_int_idx(i) + "   ";
    col_labels += '\n';
    boxed_serialised_board += col_labels;
}

// adds row labels to the board
void HexBoardReal::add_row_labels(std::string &boxed_serialised_board)
{
    size_t endl_pos = boxed_serialised_board.find('\n');
    for (int i = 1; i < size + 1; i++)
    {
        std::string row_idx = std::to_string(i);
        boxed_serialised_board.replace(endl_pos, row_idx.size(), '\n' + row_idx + ' ');
        endl_pos = boxed_serialised_board.find('\n', endl_pos + 2);
        boxed_serialised_board.replace(endl_pos, 1, "\n  ");
        endl_pos = boxed_serialised_board.find('\n', endl_pos + 2);
    }
}

// labels the rows and columns of the board
void HexBoardReal::label_board(std::string &boxed_serialised_board)
{
    add_col_labels(boxed_serialised_board);
    add_row_labels(boxed_serialised_board);
}

// serialises the hex board as a string
std::string HexBoardReal::serialise()
{
    std::string out_str;
    for (int i = 0; i < size; i++)
    {
        out_str += std::string(2 * i, ' ');
        for (int j = 0; j < size; j++)
        {
            out_str += get_piece_symbol(game_board[i][j]) + ((j != size - 1) ? " " + get_separator_between_pieces({i, j}, NEIGHBOUR::ROW_RIGHT) + " " : "\n");
        }
        out_str += (i < size - 1) ? generate_separator_row(i + 1) : "";
    }
    box_board(out_str);
    label_board(out_str);
    return out_str;
}

// returns the cell coordinates at the given string id
std::pair<u_int, u_int> HexBoardReal::get_cell_by_str_id(std::string cell_id)
{
    return str_cell_id_map.at(cell_id);
}

// checks if a given cell is populated
bool HexBoardReal::cell_is_populated(std::string &cell_id)
{
    for (int i = 0; i < cell_id.length(); i++)
        cell_id[i] = toupper(cell_id[i]);

    u_int x_coord, y_coord;
    try
    {
        x_coord = str_cell_id_map.at(cell_id).first;
        y_coord = str_cell_id_map.at(cell_id).second;
    }
    catch (const std::out_of_range)
    {
        return true;
    }
    return game_board[x_coord][y_coord] != VIRTUAL_PIECE::NOT_SET;
}

// checks if a player won the game after their last move
void HexBoardABC::check_win_on_move(u_int x, u_int y, VIRTUAL_PIECE v)
{
    int edge_cntr = 0;
    for (auto cell : player_targets.at(v).first)
        if (game_board[cell.first][cell.second] == v)
        {
            edge_cntr += 1;
            break;
        }
    if (!edge_cntr)
        return;

    for (auto cell : player_targets.at(v).second)
        if (game_board[cell.first][cell.second] == v)
        {
            edge_cntr += 1;
            break;
        }
    if (edge_cntr == 1)
        return;

    if (find_any_path_one_to_many({x, y}, player_targets.at(v).first) && find_any_path_one_to_many({x, y}, player_targets.at(v).second))
        *win_state = true;
};

// updates the board with the last move
void HexBoardReal::update_board(u_int x, u_int y, VIRTUAL_PIECE v)
{
    game_board[x][y] = v;
    check_win_on_move(x, y, v);
}

// updates the board with the last move
void HexBoardVirtual::update_board(u_int x, u_int y, VIRTUAL_PIECE v)
{
    root_board[x][y] = v;
    game_board = root_board;
    check_win_on_move(x, y, v);
}

// update method called when a player notifies this board
void HexBoardABC::update(va_list args)
{
    u_int x = va_arg(args, u_int);
    u_int y = va_arg(args, u_int);
    VIRTUAL_PIECE v = va_arg(args, VIRTUAL_PIECE);
    update_board(x, y, v);
}

// checks if the given player won the game
bool HexBoardABC::player_has_won(VIRTUAL_PIECE p_id)
{
    for (auto piece : player_targets.at(p_id).first)
        if (game_board[piece.first][piece.second] == p_id && find_any_path_one_to_many(piece, player_targets.at(p_id).second))
            return true;
    return false;
}

// generates all possible legal moves for the player
std::vector<std::pair<u_int, u_int>> HexBoardVirtual::get_possible_moves()
{
    std::vector<std::pair<u_int, u_int>> possible_moves;
    for (u_int i = 0; i < size; i++)
        for (u_int j = 0; j < size; j++)
            if (root_board[i][j] == VIRTUAL_PIECE::NOT_SET)
                possible_moves.emplace_back(std::pair<u_int, u_int>{i, j});

    return possible_moves;
}

// thread safe version of the player_has_won function
bool HexBoardVirtual::thread_safe_player_has_won(std::vector<std::vector<VIRTUAL_PIECE>> thread_safe_board, VIRTUAL_PIECE p_id)
{
    for (auto piece : player_targets.at(p_id).first)
        if (thread_safe_board[piece.first][piece.second] == p_id && thread_safe_find_any_path_one_to_many(thread_safe_board, piece, player_targets.at(p_id).second))
            return true;
    return false;
}

// thread safe version of the find_any_path_one_to_many function
bool HexBoardVirtual::thread_safe_find_any_path_one_to_many(std::vector<std::vector<VIRTUAL_PIECE>> thread_safe_board, std::pair<u_int, u_int> start_cell, const std::set<std::pair<u_int, u_int>> &targets, std::vector<std::vector<bool>> seen_ids, std::list<std::pair<u_int, u_int>> traverse_chain)
{
    std::unordered_map<NEIGHBOUR, std::pair<int, int>> local_direction_offset = DIRECTION_OFFSET;

    if (!seen_ids.size())
        seen_ids = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));

    if (targets.find(start_cell) != targets.end())
        return true;
    seen_ids[start_cell.first][start_cell.second] = true;

    traverse_chain.push_back(start_cell);
    while (!traverse_chain.empty())
    {
        std::pair<u_int, u_int> next_cell;
        for (auto next_cell_offsets : local_direction_offset)
        {
            next_cell = {start_cell.first + next_cell_offsets.second.first, start_cell.second + next_cell_offsets.second.second};

            if (next_cell.first < 0 || next_cell.first >= size || next_cell.second < 0 || next_cell.second >= size)
                continue;

            if (thread_safe_board[start_cell.first][start_cell.second] != thread_safe_board[next_cell.first][next_cell.second] || seen_ids[next_cell.first][next_cell.second])
                continue;

            return thread_safe_find_any_path_one_to_many(thread_safe_board, next_cell, targets, seen_ids, traverse_chain);
        }
        traverse_chain.pop_back();
        start_cell = traverse_chain.back();
    }
    return false;
}

// thread safe montecarlo simulation, will generate SIM_ITERATIONS simulations for the move located at possible_moves[start_idx]
void HexBoardVirtual::thread_safe_montecarlo_sim(atomwrapper<int> &accumulator, std::vector<std::pair<u_int, u_int>> possible_moves, int start_idx, VIRTUAL_PIECE p_id, u_int sim_count)
{
    srand(time(nullptr));

    bool p_switch;
    std::map<bool, VIRTUAL_PIECE> players = {{false, p_id}};
    (p_id == VIRTUAL_PIECE::P1) ? players.emplace(true, VIRTUAL_PIECE::P2) : players.emplace(true, VIRTUAL_PIECE::P1);

    std::vector<std::vector<VIRTUAL_PIECE>> thread_safe_game_board = root_board;

    std::pair<u_int, u_int> swap = possible_moves[0];
    possible_moves[0] = possible_moves[start_idx];
    possible_moves[start_idx] = swap;

    std::pair<u_int, u_int> first_move = possible_moves.at(0);
    for (u_int j = 0; j < sim_count; j++)
    {
        p_switch = false;
        std::shuffle(possible_moves.begin() + 1, possible_moves.end(), std::default_random_engine(rand()));

        for (auto piece : possible_moves)
        {
            thread_safe_game_board[piece.first][piece.second] = players[p_switch];
            p_switch = !p_switch;
        }

        if (thread_safe_player_has_won(thread_safe_game_board, p_id))
            accumulator += 1;
        else
            accumulator -= 1;
    }
}

// multithreaded simulation used by the ai player
// spins one thread running thread_safe_montecarlo_sim for each possible move
std::pair<u_int, u_int> HexBoardVirtual::generate_move(VIRTUAL_PIECE p_id)
{
    std::vector<std::thread> sim_threads;
    std::vector<std::vector<atomwrapper<int>>> legal_moves_heatmap(size, std::vector<atomwrapper<int>>(size, atomwrapper<int>(0)));
    std::vector<std::pair<u_int, u_int>> possible_moves = get_possible_moves();

    int i = 0;
    for (auto start_move : possible_moves)
    {
        atomwrapper<int> &accumulator = legal_moves_heatmap.at(start_move.first).at(start_move.second);
        sim_threads.push_back(std::thread([&]()
                                          { thread_safe_montecarlo_sim(accumulator, possible_moves, i++, p_id, SIM_ITERATIONS); }));
    }

    std::for_each(sim_threads.begin(), sim_threads.end(), std::mem_fn(&std::thread::join));

    u_int max_i = 0;
    u_int max_j = 0;
    int max_val = -SIM_ITERATIONS - 1;
    for (auto cell : possible_moves)
        if (legal_moves_heatmap[cell.first][cell.second] > max_val)
        {
            max_i = cell.first;
            max_j = cell.second;
            max_val = legal_moves_heatmap[max_i][max_j];
        }

    return std::pair<u_int, u_int>{max_i, max_j};
}

// Hexboard factory method
HexBoardABC *HexBoardFactory::make(u_int size)
{
    return new HexBoardReal(size);
}

// Hexboard factory method
HexBoardABC *HexBoardFactory::make(HexBoardABC *game_board)
{
    return new HexBoardVirtual(game_board);
}

// factory generating a real board and a possible virtual board if one of the players is ai
void HexBoardFactory::init_boards(HexBoardABC *&game_board, HexBoardABC *&virtual_board, u_int board_size, bool ai_switch)
{
    game_board = HexBoardFactory::make(board_size);
    if (ai_switch)
        virtual_board = HexBoardFactory::make(game_board);
    else
        virtual_board = nullptr;
}
