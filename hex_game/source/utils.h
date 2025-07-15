#ifndef UTILS_H
#define UTILS_H

#include <functional>
#include <stdexcept>
#include <string>
#include <iostream>
#include <atomic>

// Errors
#define UNDEFINED_BEHAVIOUR_ERROR std::runtime_error("Undefined Behaviour!")

// ANSI macros

// colour codes from: https://gist.github.com/Kielx/2917687bc30f567d45e15a4577772b02
#define RESET "\033[0m"  /* Reset to normal */
#define RED "\033[31m"   /* Red */
#define BLUE "\033[34m"  /* Blue */
#define WHITE "\033[37m" /* White */

// escape codes from: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
#define ERASE_LINE "\x1b[2K"  /* Erases entire line */
#define MOVE_UP_ONE "\x1b[1A" /* Moves cursor up 1 line */

// consts

const int ALPHABET_SIZE = 26;
const int ASCII_ALPHABET_START = 65;

// enums

enum class ID_ENUM
{
    NOT_SET,
    P1,
    P2,
};

enum class REAL_VIRTUAL
{
    Real,
    Virtual,
};

// Function definitions

std::string make_string_idx_from_int_idx(int, std::string = "");
void cin_clear_buffer();
void print_game_intro();
void print_win_state(ID_ENUM);
void wait_for_enter_press();
void clear_lines(u_int);
void query_player_params(bool &, bool &);
void query_board_params(u_int &);

// Template implementations
// (Note: templates cannot have their definition and implementation separated: https://isocpp.org/wiki/faq/templates#templates-defn-vs-decl)

template <class T>
T sanitise_input(
    std::string initial_prompt,
    std::string secondary_prompt,
    std::function<bool(T &)> &&validator = [](T &) -> bool
    { return true; })
{
    T in_val;
    std::cout << initial_prompt;
    std::cin >> in_val;
    while (!validator(in_val) || std::cin.fail())
    {
        clear_lines(2);
        cin_clear_buffer();
        std::cout << secondary_prompt;
        std::cin >> in_val;
    }
    clear_lines(2);
    return in_val;
}

template <class T>
T sanitise_input_with_context(
    std::string initial_prompt,
    std::string secondary_prompt,
    std::function<bool(T &, void *&)> &&validator = [](T &, void *) -> bool
    { return true; },
    void *context = nullptr)
{
    T in_val;
    std::cout << initial_prompt;
    std::cin >> in_val;
    while (!validator(in_val, context) || std::cin.fail())
    {
        clear_lines(2);
        cin_clear_buffer();
        std::cout << secondary_prompt;
        std::cin >> in_val;
    }
    clear_lines(2);
    return in_val;
}

template <class T1, class T2>
T1 sanitise_input_with_cast(
    std::string initial_prompt,
    std::string secondary_prompt,
    std::function<bool(T2 &)> &&validator = [](T2 &) -> bool
    { return true; })
{
    T2 in_val;
    std::cout << initial_prompt;
    std::cin >> in_val;
    while (!validator(in_val) || std::cin.fail())
    {
        clear_lines(2);
        cin_clear_buffer();
        std::cout << secondary_prompt;
        std::cin >> in_val;
    }
    clear_lines(2);
    return static_cast<T1>(in_val);
}

// solution for vector of atomic ints from:
// https://stackoverflow.com/questions/13193484/how-to-declare-a-vector-of-atomic-in-c
template <typename T>
struct atomwrapper
{
    std::atomic<T> _a;

    atomwrapper() : _a() {}
    atomwrapper(const std::atomic<T> &a) : _a(a.load()) {}
    atomwrapper(const atomwrapper &other) : _a(other._a.load()) {}

    atomwrapper &operator=(const atomwrapper &other)
    {
        _a.store(other._a.load());
        return *this;
    }
    atomwrapper &operator+=(T other)
    {
        _a.store(_a.load() + other);
        return *this;
    }
    atomwrapper &operator-=(T other)
    {
        _a.store(_a.load() - other);
        return *this;
    }
    operator T()
    {
        return _a.load();
    }
};

#endif
