// g++ -pthread -o hex -I ./source/ main.cpp source/*cpp

#include "utils.h"
#include "hex_board.h"
#include "player.h"

#include <iostream>
#include <unordered_map>

void game_loop(
    HexPlayerABC* p1,
    HexPlayerABC* p2,
    HexBoardABC* game_board,
    HexBoardABC* virtual_board
)
{
    bool player_switch = true;
    const std::unordered_map<bool, HexPlayerABC*> players =
    {
        {true, p1},
        {false, p2}
    };

    while(!game_board->get_win_state())
    {
        std::cout << game_board;
        players.at(player_switch)->make_move(game_board, virtual_board, player_switch);
        clear_lines(3*game_board->get_size());
    }
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

    u_int board_size;
    query_board_params(board_size);

    HexBoardABC *game_board, *virtual_board;
    HexBoardFactory::init_boards(game_board, virtual_board, board_size, ai_switch);

    game_loop(p1, p2, game_board, virtual_board);

    delete p1;
    delete p2;
    delete game_board;
    delete virtual_board;
    std::cout << RESET;

    return 0;
}
