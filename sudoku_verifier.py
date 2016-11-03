#!/usr/bin/env python3

import sys
import math
import functools

def verify_puzzle(size_square: int, sudoku_puzzle: list):
    size = math.sqrt(size_square)
    answer_set = set([ num for num in range(1, 17) ])

    row_fault = []
    col_fault = []
    block_fault = []

    # row
    for row in sudoku_puzzle:
        current_row_set = set(row)
        row_fault.append(answer_set - current_row_set)

    # col
    for col_idx in range(0, size_square):
        current_col_set = set([ row[col_idx] for row in sudoku_puzzle ])
        col_fault.append(answer_set - current_col_set)

    # block
    for block_idx in range(0, size_square):
        row_level = block_idx // size
        col_level = block_idx % size
        row_first, row_last = row_level*size, row_level*size + size-1
        col_first, col_last = col_level*size, col_level*size + size-1

        block_row = sudoku_puzzle[row_first:row_last]
        block = [ row[col_first:col_last] for row in block_row ]

        block_row_set = [ set(row) for row in block ]
        current_block_set = functools.reduce(block_row_set, set.union)
        block_fault.append(answer_set - current_block_set)

    ## 

    is_fault = False
    for row in row_fault:
        if row:
            is_fault = True
    for col in col_fault:
        if col:
            is_fault = True
    for block in block_fault:
        if block:
            is_fault = True

    print('sudoku is {}'.format(is_fault))
    print(row_fault)
    print(col_fault)
    print(block_fault)
        

f = open(sys.argv[1]).readlines()
size_square = len(f[0].split(' '))

sudoku_puzzle = []
for row in f:
    sudoku_puzzle.append([ int(i) for i in row.split(' ') ])

for row in sudoku_puzzle:
    for num in row:
        print(num, end=' ')
    print()

verify_puzzle(size_square, sudoku_puzzle)
