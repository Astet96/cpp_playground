#include "player.h"

void HexPlayerAI::make_move(HexBoardABC* game_board, HexBoardABC* virtual_board, bool& player_switch)
{

}

void HexPlayerHuman::query_player_move(u_int& move_row_id, u_int& move_col_id, std::string p_colour, HexBoardABC*& game_board)
{
    std::string cell_str_id = sanitise_input_with_context<std::string>(
        p_colour + "\nEnter a move: " + WHITE,
        p_colour + "\nInvalid move, try again (format i.e. a1 or B2): " + WHITE,
        [](std::string& cell_str_id, void*& game_board)-> bool {return static_cast<HexBoardABC*>(game_board)->cell_is_populated(cell_str_id);},
        game_board
    );
    move_row_id = static_cast<HexBoardReal*>(game_board)->get_cell_by_str_id(cell_str_id).first;
    move_col_id = static_cast<HexBoardReal*>(game_board)->get_cell_by_str_id(cell_str_id).second;
}

void HexPlayerHuman::make_move(HexBoardABC* game_board, HexBoardABC* virtual_board, bool& player_switch)
{
    u_int move_row_id, move_col_id;
    query_player_move(move_row_id, move_col_id, colour.ANSII, game_board);
    game_board->update_board(move_row_id, move_col_id, id);
    player_switch = !player_switch;
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
