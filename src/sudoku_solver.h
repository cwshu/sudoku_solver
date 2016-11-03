/**
 * @file sudoku_solver.h
 * @brief solve sudoku problem by SAT. mapping sudoku puzzle to SAT problem, serialize to/deserialize from DIMACS CNF form.
 */

#ifndef __SUDOKU_SOLVER_H__
#define __SUDOKU_SOLVER_H__

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "utils.h"

struct SudokuVariable {
    bool is_valid;
    uint32_t row;
    uint32_t col;
    uint32_t number;

    SudokuVariable() : is_valid(false), row(-1), col(-1), number(-1) {}
    SudokuVariable(uint32_t row, uint32_t col, uint32_t number) : is_valid(true), row(row), col(col), number(number) {}
};

struct Encoder {
    // counter start from 1, 0 for no mapping
    // row, col, number use 1-based array

    // [row][col][number] => variable_number
    vector_3d<uint32_t> to_number;
    // [row][col][number] <= variable_number
    std::unordered_map<uint32_t, SudokuVariable> to_variable;
    uint32_t counter;

    Encoder(uint32_t size_square) {
        // std::cout << "size: " << size_square << std::endl;
        to_number = vector_3d<uint32_t>(size_square+1, vector_2d<uint32_t>(size_square+1, std::vector<uint32_t>(size_square+1, 0)));
        counter = 1;
    }

    bool is_encoded(uint32_t row, uint32_t col, uint32_t number){
        return to_number[row][col][number] != 0;
    }
    bool is_encoded(SudokuVariable var){
        return to_number[var.row][var.col][var.number] != 0;
    }
    uint32_t encode_var(uint32_t row, uint32_t col, uint32_t number){
        return to_number[row][col][number];
    }
    uint32_t encode_var(SudokuVariable var){
        return to_number[var.row][var.col][var.number];
    }
    SudokuVariable decode_var(uint32_t var_num){
        if( to_variable.count(var_num) == 0 ){
            return SudokuVariable();
        }
        return to_variable[var_num];
    }

    void add_variable_mapping(uint32_t row, uint32_t col, uint32_t number){
        SudokuVariable var(row, col, number);
        add_variable_mapping(var);
    }

    void add_variable_mapping(SudokuVariable var){
        to_number[var.row][var.col][var.number] = counter;
        to_variable[counter] = var;
        counter++;
    }
};

struct EncodeVariable {
    bool is_positive;

    uint32_t encode_num;

    EncodeVariable(uint32_t encode_num, bool is_positive=true) : is_positive(is_positive), encode_num(encode_num) {}
};

using Clause = std::vector<EncodeVariable>;

class SudokuSolver {
public:
    vector_2d<uint32_t> puzzle;

    vector_2d<bool> row_numbers_use;
    vector_2d<uint32_t> row_empty_cells;
    vector_2d<bool> col_numbers_use;
    vector_2d<uint32_t> col_empty_cells;
    vector_2d<bool> block_numbers_use;
    vector_2d<std::pair<uint32_t, uint32_t>> block_empty_cells;

    vector_2d<uint32_t> row_unuse_numbers;
    vector_2d<uint32_t> col_unuse_numbers;
    vector_2d<uint32_t> block_unuse_numbers;

    Encoder encoder;
    std::vector<Clause> clause_list;

    uint32_t size;
    uint32_t size_square() const { return size*size; }
    uint32_t count_block(uint32_t row, uint32_t col) const;

    SudokuSolver(vector_2d<uint32_t> puzzle, uint32_t size);

    /** @brief preprocess some data into data structure */
    void prepare();
    void gen_unuse_numbers();
    void gen_clauses();
    void gen_define_unique_clause(std::vector<SudokuVariable> once_list);

    std::string clause_list_to_DIMACS();

    void decode(std::vector<int32_t> sat_output_num);

private:
    /* data */
};
#endif /* end of include guard: __SUDOKU_SOLVER_H__ */
