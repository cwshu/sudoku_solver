/**
 * @file sudoku_solver.cpp
 * @brief solve sudoku problem by SAT. mapping sudoku puzzle to SAT problem, serialize to/deserialize from DIMACS CNF form.
 */

#include "sudoku_solver.h"
#include <iostream>

uint32_t SudokuSolver::count_block(uint32_t row, uint32_t col){
    /*
     * block: 
     *   1, 2, 3
     *   4, 5, 6
     *   7, 8, 9
     *
     * row => 1, 4, 7 => 3n
     * col => 3n+1, 3n+2, 3n+3
     * block = 3 * (row-1)/3 + (col-1)/3 + 1;
     */
    return 3 * ((row-1)/3) + (col-1)/3 + 1;
}

SudokuSolver::SudokuSolver(vector_2d<uint32_t> puzzle, uint32_t size) : puzzle(puzzle), size(size), encoder(size*size) {

    row_numbers_use.resize(size_square()+1, std::vector<bool>(size_square()+1, false));
    row_empty_cells.resize(size_square()+1, std::vector<uint32_t>());
    col_numbers_use.resize(size_square()+1, std::vector<bool>(size_square()+1, false));
    col_empty_cells.resize(size_square()+1, std::vector<uint32_t>());
    block_numbers_use.resize(size_square()+1, std::vector<bool>(size_square()+1, false));
    block_empty_cells.resize(size_square()+1, std::vector<std::pair<uint32_t, uint32_t>>());
}

/** @brief preprocess some data into data structure */
void SudokuSolver::prepare(){
    for( uint32_t row = 1; row <= size_square(); row++ ){
        const auto& line = puzzle[row];

        for( uint32_t col = 1; col <= size_square(); col++ ){
            uint32_t block = count_block(row, col);
            uint32_t number = line[col];

            if( number == 0 ){
                // empty cell
                row_empty_cells[row].push_back(col);
                col_empty_cells[col].push_back(row);
                block_empty_cells[block].push_back(std::pair<uint32_t, uint32_t>(row, col));
            }
            else{
                // prefilled cell
                row_numbers_use[row][number] = true;
                col_numbers_use[col][number] = true;
                block_numbers_use[block][number] = true;
            }
        }
    }
}

void SudokuSolver::gen_unuse_numbers(){
    // numbers_use to unuse_numbers for speed
    row_unuse_numbers = vector_2d<uint32_t>(size_square()+1, std::vector<uint32_t>());
    col_unuse_numbers = vector_2d<uint32_t>(size_square()+1, std::vector<uint32_t>());
    block_unuse_numbers = vector_2d<uint32_t>(size_square()+1, std::vector<uint32_t>());

    for( uint32_t row = 1; row <= size_square(); row++ ){
        for( uint32_t number = 1; number <= size_square(); number++ ){
            if( row_numbers_use[row][number] == false ){
                row_unuse_numbers[row].push_back(number);
            }
        }
    }
    for( uint32_t col = 1; col <= size_square(); col++ ){
        for( uint32_t number = 1; number <= size_square(); number++ ){
            if( col_numbers_use[col][number] == false ){
                col_unuse_numbers[col].push_back(number);
            }
        }
    }
    for( uint32_t block = 1; block <= size_square(); block++ ){
        for( uint32_t number = 1; number <= size_square(); number++ ){
            if( block_numbers_use[block][number] == false ){
                block_unuse_numbers[block].push_back(number);
            }
        }
    }
}

void SudokuSolver::gen_clauses(){
    // process cell, row, col, and block constraint

    gen_unuse_numbers();

    // cell => X[row][col][{num}] for row in rows for col in cols
    for( uint32_t row = 1; row <= size_square(); row++ ){
        for( uint32_t col = 1; col <= size_square(); col++ ){
            if( puzzle[row][col] == 0 ){
                std::vector<SudokuVariable> once_list;
                
                for( const auto& unuse_number : row_unuse_numbers[row] ){
                    once_list.emplace_back(row, col, unuse_number);
                }

                gen_define_unique_clause(once_list);
            }
        }    
    }    

    // row => X[row][{col}][num] for row in rows
    for( uint32_t row = 1; row <= size_square(); row++ ){

        for( const auto& unuse_number : row_unuse_numbers[row] ){
            std::vector<SudokuVariable> once_list;
            
            for( const auto& empty_cell_col : row_empty_cells[row] ){
                once_list.emplace_back(row, empty_cell_col, unuse_number);
            }

            gen_define_unique_clause(once_list);
        }
        
    }    

    // col => X[{row}][col][num] for col in cols
    for( uint32_t col = 1; col <= size_square(); col++ ){

        for( const auto& unuse_number : col_unuse_numbers[col] ){
            std::vector<SudokuVariable> once_list;
            
            for( const auto& empty_cell_row : col_empty_cells[col] ){
                once_list.emplace_back(empty_cell_row, col, unuse_number);
            }

            gen_define_unique_clause(once_list);
        }
        
    }    

    // block => X[{row}][{col}][num] for block in blocks
    for( uint32_t block = 1; block <= size_square(); block++ ){

        for( const auto& unuse_number : block_unuse_numbers[block] ){
            std::vector<SudokuVariable> once_list;
            
            for( const auto& empty_cell : block_empty_cells[block] ){
                once_list.emplace_back(empty_cell.first, empty_cell.second, unuse_number);
            }

            gen_define_unique_clause(once_list);
        }
    }    
}

// debug use
void print_once_list(std::vector<SudokuVariable> once_list){
    for( const auto& var : once_list ){
        std::cout << "(" << var.row << ", " << var.col << ", " << var.number << "), ";
    }
    std::cout << std::endl;
}
// debug use

void SudokuSolver::gen_define_unique_clause(std::vector<SudokuVariable> once_list){

    // debug use
    // print_once_list(once_list);

    std::vector<uint32_t> once_list_encode;
    for( const auto& var : once_list ){
        if( !encoder.is_encoded(var) ){
            encoder.add_variable_mapping(var);
        }

        once_list_encode.push_back(encoder.encode_var(var));
    }

    // define
    Clause clause;
    for( const auto& encode_var : once_list_encode ){
        clause.emplace_back(encode_var);
    }
    clause_list.push_back(clause);
    // use
    for( auto it = once_list_encode.cbegin(); it != once_list_encode.cend(); it++ ){
        for( auto it2 = std::next(it, 1); it2 != once_list_encode.cend(); it2++ ){
            Clause clause = { {*it, false}, {*it2, false} };
            clause_list.push_back(clause);
        }
    }
}

std::string SudokuSolver::clause_list_to_DIMACS(){
    uint32_t var_num = encoder.counter - 1;
    uint32_t clause_num = clause_list.size();

    std::string ret;
    ret = "p cnf " + std::to_string(var_num) + " " + std::to_string(clause_num) + "\n";
    for( const auto& clause : clause_list ){
        for( const auto& enc_var : clause ){
            if( !enc_var.is_positive ){
                ret += "-";
            }
            ret += std::to_string(enc_var.encode_num) + " ";
        }
        ret += "0\n";
    }

    return ret;
}

void SudokuSolver::decode(std::vector<int32_t> sat_output_num){
    for( const auto& number : sat_output_num ){
        if( number > 0 ){
            SudokuVariable var = encoder.decode_var(number);
            if( var.is_valid ){
                if( puzzle[var.row][var.col] != 0 ){
                    std::cerr << "decode error: (" << var.row << ", " << var.col << ") = " << var.number << std::endl;
                    continue;
                }
                puzzle[var.row][var.col] = var.number;
            }
        }
    }
}
