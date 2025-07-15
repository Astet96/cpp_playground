#include "utils.h"
#include "player.h"

#include <string>
#include <iostream>
#include <limits>


void print_game_intro()
{
    Colour p1_colour = PLAYER_COLOUR.at(PLAYER_ID::P1);
    Colour p2_colour = PLAYER_COLOUR.at(PLAYER_ID::P2);

    std::cout <<
    p1_colour.ANSII + "Player 1" + WHITE + " must make a connected set of " + p1_colour.ANSII + p1_colour.NAME + WHITE + " hexagons from east to west.\n"
    + p2_colour.ANSII + "Player 2" + WHITE + " must make a connected set of " + p2_colour.ANSII + p2_colour.NAME + WHITE + " hexagons from north to south.\n"
    + "At each turn a player chooses an unoccupied hexagon and gives it their colour.\n"
    + "Unlike tic-tac-toe the game cannot end in a draw.\n";
}

void wait_for_enter_press()
{
    std::cout << "\nPress ENTER to continue...";
    cin_clear_buffer();
}

void clear_lines(u_int nr)
{
    for(int i = 0; i < nr; i++)
    std::cout << ERASE_LINE << MOVE_UP_ONE;
    std::cout << '\r';
}

void cin_clear_buffer()
{
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string gen_player_option(PLAYER_ID p_id)
{
    Colour p_colour = PLAYER_COLOUR.at(p_id);
    return p_colour.ANSII + p_colour.NAME + "[" + std::to_string(static_cast<int>(p_id)) + "]" + WHITE;
}

void query_player_params(bool &ai_switch, bool &colour_switch)
{
    std::string p1_option, p2_option;
    p1_option = gen_player_option(PLAYER_ID::P1);
    p2_option = gen_player_option(PLAYER_ID::P2);

    ai_switch = sanitise_input<bool, bool>
    (
        "Play against Human[0] or AI[1]? ",
        "Invalid option, please choose Human[0] or AI[1]: "
    );

    if(ai_switch)
        colour_switch = sanitise_input<bool, int>
        (
            "Choose your colour " + p1_option + " or " + p2_option + ": ",
            "Invalid option, please choose " + p1_option + " or " + p2_option + ": ",
            [](int& val) -> bool {val-=1; return val == 0 || val == 1;}
        );
}

void query_board_params(u_int& board_size)
{
    board_size = sanitise_input<u_int, u_int>(
        "Enter board size between [5 - 11]: ",
        "Invalid board size given, please choose a value between [5 - 11]: ",
        [](u_int& val) -> bool {return val > 4 && val < 12;}
    );
}
