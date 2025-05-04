# CSP Library

A C library for solving Constraint Satisfaction Problems (CSP) developed as a university project, including advanced algorithms like forward checking.

## What is a CSP?

A Constraint Satisfaction Problem (CSP) is a mathematical problem defined by:

- A set of variables
- A domain of possible values for each variable
- A set of constraints between these variables

This library provides tools to efficiently model and solve such problems.

## Features

- Creation and manipulation of variables with domains
- Definition of binary and n-ary constraints
- Solving algorithms:
  - Simple backtracking
  - Forward checking
  - Arc consistency (AC-3)
- Variable and value selection heuristics

TODO: Verify information and complete it

## Installation

```bash
git clone git@github.com:Groupe-X-L2-INFO-LRU/csp-lib.git
cd csp-lib
cmake ..
make
```

## Compiling instructions

```bash
mkdir build #if not already made
cmake ..` #create MakeFile from the `CMakeList.txt` on root directory
make

make test # to launch all tests
make <executable name> # to launch a specific exectuable
```

## Developpement instructions

- Before compiling, don't forget to go on the **build** directory to keep the main directory clean
- Create an test file for each function that you create
- Don't forget to do a **cmake ..** to refresh the MakeFile
- Respect the naming convention
- Comment your code
- Generate _Doxygen_ documentation
- Before commit, enter the next commands :

```bash
clang-tidy src/* #check the syntax of your code
clang-format -i src/\* #reformat your code
```

## Requirements

- C compiler supporting C23
- CMake 3.30+

## Usage Example

To solve sudoku, enter this command :
`./solve-sudoku <puzzle file>`

The puzzle file must be resolvable and must respect the input format : each line correspond to a sudoku line and the hidden numbers are replace by a '.'
For more informations, see [puzzle.txt](puzzle.txt) file

## Contribution

Please respect the directives writen on [Contributing.md](CONTRIBUTING.md)

## Contributors

- [Quentin Sautiere](https://github.com/SautiereQDev)
- [Linares Julien](https://github.com/KyozuFR)
  TODO: Add your name or pseudonym and link of your profile

## License

MIT - All right reserved
For more informations, please check [LiCENSE](LICENSE)

```

```
