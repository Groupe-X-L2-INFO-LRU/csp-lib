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

TODO: complete installation directives

## Requirements

- C compiler supporting C23
- CMake 3.30+

## Usage Example

TODO: Add example code when lib is completed

## Contribution

Please respect the directives writen on [Contributing.md](CONTRIBUTING.md)

## Contributors

- [Quentin Sautiere](https://github.com/SautiereQDev)
- [Linares Julien](https://github.com/KyozuFR)
  TODO: Add your name or pseudonym and link of your profile

## License

MIT - All right reserved
For more informations, please check [LiCENSE](LICENSE)