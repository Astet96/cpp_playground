/*
HW 4 expectations:
    1. Be able to draw the board using ASCII symbols and a given size, such as 7 by 7 or 11 by 11. [x]
    2. Input a move and determine if a move is legal. [ ]
    3. Determine who won. [ ]
*/

#include<iostream>
#include<vector>
#include<list>

// prototypes
class HexBoard;


/*
TODO:
hex board
    [x] seed board
    [x] representation
    [ ] win state
player
    [ ] make move
        - brainstorm possible mechanisms
        - read up on docs etc.
        - figure out reasonable graphical implementation
        - refresh board (preety look)
        - empty terminal before game start (preety look)
    [ ] player switching
AI - no functionality (random moves)
*/

class HexCell{
    private:
        friend class HexBoard;
        friend std::ostream& operator<<(std::ostream& out_str, const HexBoard& board);

        const int id;
        std::list<int> neighbour_ids;
        char value = '.';

        HexCell(int id):id(id){}

    public:
        ~HexCell(){}
};

class HexBoard{
    private:
        friend std::ostream& operator<<(std::ostream& out_str, const HexBoard& board);

        const int size;
        std::vector<HexCell> cells;

        // returns leftward & upward neighbours (if they exist)
        // strategy: evaluate all possible (3) neighbours & only add valid ones to return vector
        inline std::list<int> get_previous_neighbours(int cell_id){
            std::list<int> prev_neighbours;

            // left neighbour (if it exists)
            if(cell_id % size) prev_neighbours.push_back(cell_id - 1);

            // previous row - up left neighbour (if it exists)
            if(cell_id >= size) prev_neighbours.push_back(cell_id - size);
            
            // previous row - up right neighbour (if it exists)
            if(cell_id >= size - 1 && cell_id % size != size - 1) prev_neighbours.push_back(cell_id - size + 1);

            return prev_neighbours;
        }

        inline void seed_board(){
            for(int i=0; i<size*size; i++){
                cells.push_back(HexCell(i));

                for(int j : get_previous_neighbours(i)){
                    // add previous neighbours
                    cells[j].neighbour_ids.push_back(cells[i].id);

                    // add forward neighbours
                    cells[i].neighbour_ids.push_back(cells[j].id);
                }
            }
        }

        inline std::string print_filler_row()const{
            std::string rtrn_string;
            rtrn_string += "\\ ";
            for(int i =0; i < size-1; i++)
                rtrn_string += "/ \\ ";
            rtrn_string += '\n';
            return rtrn_string;
        }

    public:
        HexBoard(int size):size(size){seed_board();}
        ~HexBoard(){}
};

// print HexBoard
inline std::ostream& operator<<(std::ostream& out_str, const HexBoard& board){
    int lvl = 0;
    for(auto& cell : board.cells){
        // print cell value
        out_str << cell.value;

        // break on last row
        if(cell.id == board.size*board.size - 1){out_str << '\n'; break;};

        !((cell.id+1) % board.size) ?
            // start new row
            // 1. add offset
            lvl++, out_str << '\n'

            // 2. print filler row with paths between levels
            << std::string(2*lvl - 1, ' ')
            << board.print_filler_row()

            // 3. start new row
            << std::string(2*lvl, ' ')

            // ...or continue row
            : out_str << " - ";
    }
    return out_str;
}

int main(){

    HexBoard game_board(11);

    std::cout << game_board;

    return 0;
}
