#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"
#include "hex_board.h"

#include <list>
#include <unordered_map>
#include <string>
#include <map>
#include <functional>

#define PLAYER_ID ID_ENUM
#define PlayerType REAL_VIRTUAL

// enums

enum class PIECE
{
    NOT_SET = '.',
    P1 = 'X',
    P2 = 'O',
};

// structs

struct Colour
{
    std::string ANSII;
    std::string NAME;
};

// consts

const std::unordered_map<PLAYER_ID, PIECE> PLAYER_PIECE =
    {
        {PLAYER_ID::NOT_SET, PIECE::NOT_SET},
        {PLAYER_ID::P1, PIECE::P1},
        {PLAYER_ID::P2, PIECE::P2},
};

const std::unordered_map<PLAYER_ID, Colour> PLAYER_COLOUR =
    {
        {PLAYER_ID::NOT_SET, Colour{WHITE, "white"}},
        {PLAYER_ID::P1, Colour{BLUE, "blue"}},
        {PLAYER_ID::P2, Colour{RED, "red"}},
};

// player abstract base class
class HexPlayerABC
{
protected:
    const Colour colour;
    const PLAYER_ID id;
    const PIECE piece;
    std::map<const BoardType, std::list<std::pair<const std::string, void *>>> observers;

    const std::map<const BoardType, std::pair<const int, std::function<void(void *&, va_list)>>> OBSERVER_TYPE_CAST_DISPATCHER =
        {
            {BoardType::Real, {3, [](void *&pntr_to_cast, va_list args) -> void
                               { static_cast<HexBoardReal *>(pntr_to_cast)->update(args); }}},
            {BoardType::Virtual, {3, [](void *&pntr_to_cast, va_list args) -> void
                                  { static_cast<HexBoardVirtual *>(pntr_to_cast)->update(args); }}},
    };

public:
    HexPlayerABC() : id(PLAYER_ID::NOT_SET), piece(PIECE::NOT_SET), colour(Colour{"NOT_SET", "NOT_SET"}) {}
    HexPlayerABC(PLAYER_ID id) : id(id), piece(PLAYER_PIECE.at(id)), colour(PLAYER_COLOUR.at(id)) {}

    virtual ~HexPlayerABC() {}

    virtual void get_player_move(u_int &, u_int &, HexBoardABC *&) = 0;
    virtual PlayerType get_player_type() = 0;
    virtual void make_move(HexBoardABC *&);
    virtual void attach(HexBoardABC *&);
    virtual void detach(HexBoardABC *&);
    virtual void notify(BoardType, ...);

    PLAYER_ID get_id() { return id; }
};

// AI player class
class HexPlayerAI : public HexPlayerABC
{
private:
protected:
public:
    HexPlayerAI(PLAYER_ID id) : HexPlayerABC(id) {}
    ~HexPlayerAI() {}

    void get_player_move(u_int &, u_int &, HexBoardABC *&);
    PlayerType get_player_type();
};

// Human player class
class HexPlayerHuman : public HexPlayerABC
{
private:
protected:
public:
    HexPlayerHuman(PLAYER_ID id) : HexPlayerABC(id) {}
    ~HexPlayerHuman() {}

    void get_player_move(u_int &, u_int &, HexBoardABC *&);
    PlayerType get_player_type();
};

class HexPlayerFactory
{
public:
    static HexPlayerABC *make(PLAYER_ID, bool);
    static void init_players(HexPlayerABC *&, HexPlayerABC *&, bool, bool);
};
#endif
