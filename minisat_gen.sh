#!/bin/sh

MINISAT=./minisat/MiniSat_v1.14_linux
$MINISAT cnf/1.cnf cnf/1.out
$MINISAT cnf/2.cnf cnf/2.out
