// TODO: initialise player params from json

#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"

#include <unordered_map>
#include <string>

#define PLAYER_ID ID_ENUM

enum class PIECE
{
    NOT_SET='.',
    P1='X',
    P2='O',
};

struct Colour
{
    std::string ANSII;
    std::string NAME;
};

const std::unordered_map<PLAYER_ID, PIECE> PLAYER_PIECE =
{
    {PLAYER_ID::NOT_SET, PIECE::NOT_SET},
    {PLAYER_ID::P1, PIECE::P1},
    {PLAYER_ID::P2, PIECE::P2},
};

const std::unordered_map<PLAYER_ID, Colour> PLAYER_COLOUR =
{
    {PLAYER_ID::P1, Colour{BLUE, "blue"}},
    {PLAYER_ID::P2, Colour{RED, "red"}},
};

class HexPlayerABC
{
    protected:
        const Colour colour;
        const PLAYER_ID id;
        const PIECE piece;
    
    public:
        HexPlayerABC():id(PLAYER_ID::NOT_SET), piece(PIECE::NOT_SET), colour(Colour{"NOT_SET", "NOT_SET"}){}
        HexPlayerABC(PLAYER_ID id):id(id), piece(PLAYER_PIECE.at(id)), colour(PLAYER_COLOUR.at(id)){}
        
        virtual ~HexPlayerABC(){}
        
        virtual void make_move()=0;
};

class HexPlayerAI:public HexPlayerABC
{
    private:

    protected:

    public:
        HexPlayerAI(PLAYER_ID id):HexPlayerABC(id){}
        ~HexPlayerAI(){}

        void make_move();
};

class HexPlayerHuman:public HexPlayerABC
{
    private:

    protected:

    public:
        HexPlayerHuman(PLAYER_ID id):HexPlayerABC(id){}
        ~HexPlayerHuman(){}

        void make_move();
};

class HexPlayerFactory
{
    public:
        static HexPlayerABC* make(PLAYER_ID, bool);
        static void init_players(HexPlayerABC*&, HexPlayerABC*&, bool, bool);
};


#endif
