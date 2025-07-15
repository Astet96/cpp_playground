#include "player.h"

void HexPlayerAI::make_move()
{

}

void HexPlayerHuman::make_move()
{

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
