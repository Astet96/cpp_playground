// g++ -pthread -o hex -I ./source/ main.cpp source/*cpp -Wno-varargs

#include "utils.h"
#include "hex_board.h"
#include "player.h"

#include <iostream>
#include <unordered_map>
#include <map>

void game_loop(const std::unordered_map<bool, HexPlayerABC*>& players, std::map<PlayerType, HexBoardABC*&>& boards)
{
    bool player_switch = true;
    while(!boards.at(PlayerType::Real)->get_win_state())
    {
        std::cout << boards.at(PlayerType::Real);
        players.at(player_switch)->make_move(boards.at(players.at(player_switch)->get_player_type()));
        player_switch = !player_switch;
        clear_lines(3 * boards.at(PlayerType::Real)->get_size());
    }
    std::cout << boards.at(PlayerType::Real);
    print_win_state(players.at(!player_switch)->get_id());
}

int main()
{
    clear_lines(100);
    std::cout << WHITE;

    print_game_intro();
    wait_for_enter_press();
    clear_lines(7);

    bool ai_switch, colour_switch;
    query_player_params(ai_switch, colour_switch);

    HexPlayerABC *p1, *p2;
    HexPlayerFactory::init_players(p1, p2, ai_switch, colour_switch);
    const std::unordered_map<bool, HexPlayerABC*> players =
    {
        {true, p1},
        {false, p2}
    };

    u_int board_size;
    query_board_params(board_size);

    HexBoardABC *game_board, *virtual_board;
    HexBoardFactory::init_boards(game_board, virtual_board, board_size, ai_switch);

    p1->attach(game_board);
    p2->attach(virtual_board);

    std::map<PlayerType, HexBoardABC*&> boards =
    {
        {PlayerType::Real, game_board},
        {PlayerType::Virtual, virtual_board},
    };

    game_loop(players, boards);

    p1->detach(game_board);
    p2->detach(virtual_board);

    delete p1;
    delete p2;
    delete game_board;
    delete virtual_board;
    std::cout << RESET;

    return 0;
}
