# quick_cli_snake

## Prerequisites
Ensure you have the following installed:

- A C++ compiler that supports at least C++11 (since we're using std::make_unique).
  For instance, g++ version 4.8.1 or later.
- The make build automation tool (optional).

## Compilation
There are multiple ways to compile this project. Here are two common methods:

### Using g++ directly

Navigate to the directory containing snake.cpp and execute the following command:

```bash
g++ -std=c++11 -o snake_game snake.cpp
