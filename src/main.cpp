/**
 * @file main.cpp
 * @brief the entry point of sudoku solver program.
 */

/* 
 * usage: ./solver [Input Puzzle] [Output Puzzle] [MiniSatExe] 
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstdlib>

#include "sudoku_solver.h"
#include "utils.h"

// const char WHITESPACE[] = " \t\r\n\v\f"
const char DIGIT[] = "0123456789";
const char DIGIT_NEG[] = "-0123456789";

/** @brief parse sudoku input, parse single line */
std::vector<uint32_t> parse_line(std::string line);
/** @brief parse SAT solver output, split SAT solution(variable = true/false) */
std::vector<int32_t> split_number(std::string line);

void print_sudoku_puzzle(const vector_2d<uint32_t>& puzzle);
void print_sudoku_solution(std::fstream& output_file, const vector_2d<uint32_t>& puzzle);

void minisat_solver(std::string executable, std::string& input_data, std::string& output_data, bool& is_satisfied){
    const char INPUT_FILE[] = "/tmp/minisat_in";
    const char OUTPUT_FILE[] = "/tmp/minisat_out";

    std::fstream sat_in(INPUT_FILE, std::ios::out);
    if( !sat_in ){
        std::cerr << "open sat_in error" << std::endl;
        std::exit(1);
    }
    sat_in << input_data;
    sat_in.close();

    std::string command = executable + " " + INPUT_FILE + " " + OUTPUT_FILE;
    std::cout << command << std::endl;
    std::system(command.c_str());

    std::fstream sat_out(OUTPUT_FILE, std::ios::in);
    if( !sat_out ){
        std::cerr << "open sat_out error" << std::endl;
        std::exit(1);
    }
    std::string sat_string;
    std::getline(sat_out, sat_string);
    std::getline(sat_out, output_data);

    is_satisfied = false;
    if( sat_string == "SAT" ){
        is_satisfied = true;
    }

    return;
}

int main(int argc, char *argv[]){
    
    if( argc != 4 ){
        std::cerr << "invalid number of arguments" << std::endl;
        std::cerr << "usage: ./sudoku_solver [Input Puzzle] [Output Puzzle] [MiniSatExe]" << std::endl;
        return 1;
    }

    std::string input_name = argv[1];
    std::string output_name = argv[2];
    std::string minisat_exe_name = argv[3];

    std::fstream input_file, output_file;

    input_file.open(input_name, std::ios::in);
    if( !input_file ){
        std::cerr << "input file error" << std::endl;
        return 1;
    }
    output_file.open(output_name, std::ios::out);
    if( !output_file ){
        std::cerr << "output file error" << std::endl;
        return 1;
    }

    // 1. parse sudoku puzzle
    // sudoku puzzle use 1-based array, index 0 is ignored.
    vector_2d<uint32_t> sudoku_puzzle;

    std::string line;
    std::getline(input_file, line);
    std::vector<uint32_t> numbers = parse_line(line);

    uint32_t sudoku_size_square = numbers.size() - 1;
    uint32_t sudoku_size = static_cast<uint32_t>( std::sqrt(static_cast<double>(sudoku_size_square)) );
    
    sudoku_puzzle.resize(sudoku_size_square+1);
    sudoku_puzzle[0] = std::vector<uint32_t>(sudoku_size_square+1, 0); // ignore row 0
    sudoku_puzzle[1] = numbers;

    for( int i = 2; i <= sudoku_size_square; i++ ){
        std::string line;
        std::getline(input_file, line);
        std::vector<uint32_t> numbers = parse_line(line);
        sudoku_puzzle[i] = numbers;
    }

#ifdef DEBUG
    print_sudoku_puzzle(sudoku_puzzle);
#endif

    // 2. to DS
    SudokuSolver solver(sudoku_puzzle, sudoku_size);
    solver.prepare();

    // 3. gen clauses + encode
    solver.gen_clauses();

    // 4. SAT solver
    std::string sat_input = solver.clause_list_to_DIMACS();
    std::string sat_output;
    bool is_satisfied;

    minisat_solver(minisat_exe_name, sat_input, sat_output, is_satisfied);

    if( !is_satisfied ){
        std::cout << "NO";
        return 0;
    }

    // 5. decode and get solution
    std::vector<int32_t> sat_output_num = split_number(sat_output);
    solver.decode(sat_output_num);

#ifdef DEBUG
    print_sudoku_puzzle(solver.puzzle);
#endif

    // 6. output solution
    print_sudoku_solution(output_file, solver.puzzle);

    input_file.close();
    output_file.close();

    return 0;
}

void print_sudoku_solution(std::fstream& output_file, const vector_2d<uint32_t>& puzzle){
    for( auto row_iter = std::next(puzzle.cbegin(), 1); row_iter != puzzle.cend(); row_iter++ ){
        for( auto col_iter = std::next(row_iter->cbegin(), 1); col_iter != row_iter->cend(); col_iter++ ){
            if( std::next(col_iter, 1) == row_iter->cend() ){
                output_file << *col_iter << std::endl;
            }
            else{
                output_file << *col_iter << " ";
            }
        }        
    }
}

std::vector<uint32_t> parse_line(std::string line){
    /* use 1-based array store number */

    std::vector<uint32_t> numbers(1);

    std::size_t offset = 0;
    offset = line.find_first_of(DIGIT, 0);

    while( 1 ){
        /*
         * digit:
         *   [^DIGIT]: count word, => not digit
         *   [EOF]: exit
         *
         * not digit:
         *   [DIGIT]: => digit
         */

        // digit
        std::size_t found = line.find_first_not_of(DIGIT, offset);

        // [EOF]: exit
        if( found == std::string::npos ){
            if( offset != std::string::npos ){
                uint32_t num = std::stoi(line.substr(offset));
                numbers.push_back(num);
            }
            break;
        }

        uint32_t num = std::stoi(line.substr(offset, found-offset));
        numbers.push_back(num);

        // not digit
        offset = line.find_first_of(DIGIT, found);
    }

    return numbers;
}

std::vector<int32_t> split_number(std::string line){
    std::vector<int32_t> numbers;

    std::size_t offset = 0;
    offset = line.find_first_of(DIGIT_NEG, 0);

    while( 1 ){
        // digit
        std::size_t found = line.find_first_not_of(DIGIT_NEG, offset);

        // [EOF]: exit
        if( found == std::string::npos ){
            if( offset != std::string::npos ){
                int32_t num = std::stoi(line.substr(offset));
                numbers.push_back(num);
            }
            break;
        }

        int32_t num = std::stoi(line.substr(offset, found-offset));
        numbers.push_back(num);

        // not digit
        offset = line.find_first_of(DIGIT_NEG, found);
    }

    return numbers;
}

void print_sudoku_puzzle(const vector_2d<uint32_t>& puzzle){

    for( const auto& line : puzzle ){
        for( const auto& number : line ){
            std::cout << number << " ";
        }
        std::cout << std::endl;
    }
}
