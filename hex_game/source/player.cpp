#include "player.h"

#include <cstdint>

PlayerType HexPlayerHuman::get_player_type()
{
    return PlayerType::Real;
}

PlayerType HexPlayerAI::get_player_type()
{
    return PlayerType::Virtual;
}

void HexPlayerABC::make_move(HexBoardABC*& board)
{
    u_int move_row_id, move_col_id;
    get_player_move(move_row_id, move_col_id, board);

    notify(board->get_board_type(), move_row_id, move_col_id, id);
}

void HexPlayerAI::get_player_move(u_int& move_row_id, u_int& move_col_id, HexBoardABC*& board)
{
    std::pair<u_int, u_int> move = static_cast<HexBoardVirtual*>(board)->generate_move(id);
    move_row_id = move.first;
    move_col_id = move.second;
}

void HexPlayerHuman::get_player_move(u_int& move_row_id, u_int& move_col_id, HexBoardABC*& board)
{
    std::string cell_str_id = sanitise_input_with_context<std::string>(
        colour.ANSII + "\nEnter a move: " + WHITE,
        colour.ANSII + "\nInvalid move, try again (format i.e. a1 or B2): " + WHITE,
        [](std::string& cell_str_id, void*& board)-> bool {return !static_cast<HexBoardReal*>(board)->cell_is_populated(cell_str_id);},
        board
    );
    move_row_id = static_cast<HexBoardReal*>(board)->get_cell_by_str_id(cell_str_id).first;
    move_col_id = static_cast<HexBoardReal*>(board)->get_cell_by_str_id(cell_str_id).second;
}

void HexPlayerABC::attach(HexBoardABC*& target)
{
    observers.emplace(target->get_board_type(), std::list<std::pair<const std::string, void*>>{{}});
    observers.at(target->get_board_type()).remove(std::pair<const std::string, void*>{});

    observers.at(target->get_board_type()).emplace_back(std::pair{std::to_string(reinterpret_cast<uintptr_t>(target)), target});
}

void HexPlayerABC::detach(HexBoardABC*& target)
{
    observers.at(target->get_board_type()).remove(std::pair{std::to_string(reinterpret_cast<uintptr_t>(target)), target});
}

void HexPlayerABC::notify(BoardType board_type, ...)
{
    va_list args;
    va_start(args, OBSERVER_TYPE_CAST_DISPATCHER.at(board_type).first);
    for(auto target : observers.at(board_type))
    {
        OBSERVER_TYPE_CAST_DISPATCHER.at(board_type).second(target.second, args);
    }
}

HexPlayerABC* HexPlayerFactory::make(PLAYER_ID id, bool ai_switch)
{
    if(ai_switch)
        return new HexPlayerAI(id);
    return new HexPlayerHuman(id);
}

void HexPlayerFactory::init_players(HexPlayerABC*& p1, HexPlayerABC*& p2, bool ai_switch, bool colour_switch)
{
    p1 = HexPlayerFactory::make(PLAYER_ID::P1, ai_switch && colour_switch);
    p2 = HexPlayerFactory::make(PLAYER_ID::P2, ai_switch && !colour_switch);
}
