/*
Name: Hex Game implementation
Author: Alex Stet
Date: 05-08-2025 (dd-mm-yyyy)

Note:
    ANSI support is required!
    Windows 10   -> version 1607 or above
    macOS, linux -> any version

gcc compile instructions:
    g++ -pthread -o hex -I ./source/ main.cpp source/*cpp -Wno-varargs
*/

#include "utils.h"
#include "hex_board.h"
#include "player.h"

#include <iostream>
#include <unordered_map>
#include <map>

// main game loop
void game_loop(const std::unordered_map<bool, HexPlayerABC *> &players, std::map<PlayerType, HexBoardABC *&> &boards)
{
    bool player_switch = true;
    while (!boards.at(PlayerType::Real)->get_win_state())
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
    clear_lines(10);

    bool ai_switch, colour_switch;
    query_player_params(ai_switch, colour_switch);

    HexPlayerABC *p1, *p2;
    HexPlayerFactory::init_players(p1, p2, ai_switch, colour_switch);
    const std::unordered_map<bool, HexPlayerABC *> players =
        {
            {true, p1},
            {false, p2},
        };

    u_int board_size;
    query_board_params(board_size);

    HexBoardABC *game_board, *virtual_board;
    HexBoardFactory::init_boards(game_board, virtual_board, board_size, ai_switch);

    std::map<PlayerType, HexBoardABC *&> boards =
        {
            {PlayerType::Real, game_board},
            {PlayerType::Virtual, virtual_board},
        };

    p1->attach(boards.at(p1->get_player_type()));
    p2->attach(boards.at(p2->get_player_type()));

    game_loop(players, boards);

    p1->detach(boards.at(p1->get_player_type()));
    p2->detach(boards.at(p2->get_player_type()));

    delete p1;
    delete p2;
    delete game_board;
    delete virtual_board;
    std::cout << RESET;

    return 0;
}
