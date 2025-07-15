// g++ -pthread -o hex -I ./source/ main.cpp source/*cpp

#include "utils.h"
#include "hex_board.h"
#include "player.h"

#include <iostream>

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

    u_int board_size;
    query_board_params(board_size);

    HexBoardABC *game_board, *virtual_board;
    HexBoardFactory::init_boards(game_board, virtual_board, board_size);

    delete p1;
    delete p2;
    delete game_board;
    delete virtual_board;
    std::cout << RESET;

    return 0;
}
