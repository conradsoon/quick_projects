#include <cstddef>
#include <deque>
#include <vector>
#include <string>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <memory>

enum Direction
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	NOOP
};
class Snake
{
	class Position
	{
	public:
		size_t x;
		size_t y;
	};

public:
	Snake(size_t x, size_t y, Direction dir)
	{
		snakeSegments.push_back({x, y});
		direction = dir;
	};

	Position getHead()
	{
		return snakeSegments.front();
	}
	Position nextHeadPos()
	{
		auto head = getHead();
		switch (direction)
		{
		case UP:
			head.x--;
			break;
		case DOWN:
			head.x++;
			break;
		case LEFT:
			head.y--;
			break;
		case RIGHT:
			head.y++;
			break;
		default:
			throw std::invalid_argument("invalid direction");
			break;
		}
		return head;
	}
	void addHead(Position pos)
	{
		snakeSegments.push_front(pos);
	}
	Position popTail()
	{
		auto tail = snakeSegments.back();
		snakeSegments.pop_back();
		return tail;
	}
	void setDirection(Direction dir)
	{
		direction = dir;
	}

private:
	Direction direction;
	std::deque<Position> snakeSegments;
};

class BoardState
{
	enum TileState
	{
		EMPTY,
		SNAKE,
		FOOD,
		WALL,
	};

public:
	BoardState(size_t width, size_t height)
		: width(width), height(height), snake(width / 2, height / 2, UP)
	{
		// reject if width or height is too small
		if (width < 3 || height < 3)
		{
			throw std::invalid_argument("width or height is too small");
		}
		board.resize(width);
		for (auto &row : board)
		{
			row.resize(height);
		}
		score = 0;
		initWalls();
		generateFruit();
	}

	void setDirection(Direction dir)
	{
		snake.setDirection(dir);
	}
	int moveSnake() // return -1 if game over
	{
		auto nextPos = snake.nextHeadPos();
		// oob checks
		if (nextPos.x < 0 || nextPos.x >= width || nextPos.y < 0 || nextPos.y >= height)
		{
			return -1;
		}
		int res = 0;
		switch (board[nextPos.x][nextPos.y])
		{
		case EMPTY:
		{
			auto tail = snake.popTail();
			board[tail.x][tail.y] = EMPTY;
			snake.addHead(nextPos);
			board[nextPos.x][nextPos.y] = SNAKE;
			break;
		}
		case FOOD:
		{
			snake.addHead(nextPos);
			board[nextPos.x][nextPos.y] = SNAKE;
			generateFruit();
			addScore();
			break;
		}
		case WALL:
		{
			// game over
			res = -1;
			break;
		}
		case SNAKE:
		{
			// game over
			res = -1;
			break;
		}
		default:
		{
			// ??? throw exception
			res = -1;
			break;
		}
		}
		return res;
	}
	std::string &&toString()
	{
		std::string bStr;
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				switch (board[i][j])
				{
				case EMPTY:
					bStr.push_back(' ');
					break;
				case SNAKE:
					bStr.push_back('S');
					break;
				case FOOD:
					bStr.push_back('F');
					break;
				case WALL:
					bStr.push_back('#');
					break;
				}
			}
			bStr.push_back('\n');
		}
		std::cout << bStr << std::endl;
		return std::move(bStr);
	}

private:
	size_t width;
	size_t height;
	size_t score;
	std::vector<std::vector<TileState>> board;
	Snake snake;

	void initWalls()
	{
		for (size_t i = 0; i < width; i++)
		{
			board[i][0] = WALL;
			board[i][height - 1] = WALL;
		}
		for (size_t i = 0; i < height; i++)
		{
			board[0][i] = WALL;
			board[width - 1][i] = WALL;
		}
	}
	void generateFruit()
	{
		while (true)
		{
			int x = rand() % width;
			int y = rand() % height;
			if (board[x][y] == EMPTY)
			{
				board[x][y] = FOOD;
				break;
			}
		}
	}
	void addScore()
	{
		score++;
	}
};

class Game
{
public:
	enum GameState
	{
		RUNNING,
		PAUSED,
		GAMEOVER,
		VICTORY
	};
	void poll()
	{
		while (gameState == RUNNING || gameState == PAUSED)
		{
			Direction dir = getArrowKeyInput();
			if (dir != Direction::NOOP)
			{
				boardState.setDirection(dir);
			}

			if (gameState == RUNNING)
			{
				int res = boardState.moveSnake();
				if (res != 0)
				{
					gameState = GAMEOVER;
				}
			}
			std::cout << boardState.toString() << std::endl;
			usleep(300000); // delay for 0.3 seconds
		}
	}

	Game(size_t width, size_t height) : boardState(width, height)
	{
		gameState = RUNNING;
	};

public:
	GameState gameState;

private:
	BoardState boardState;
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>

	class TerminalAttrGuard
	{
	public:
		TerminalAttrGuard()
		{
			tcgetattr(STDIN_FILENO, &oldt);
			newt = oldt;
			newt.c_lflag &= ~(ICANON | ECHO);
			newt.c_cc[VMIN] = 0;
			newt.c_cc[VTIME] = 0;
			tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		}
		~TerminalAttrGuard()
		{
			tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		}

	private:
		struct termios oldt, newt;
	};
	Direction getArrowKeyInput()
	{
		TerminalAttrGuard guard; // This will automatically reset terminal attributes when the function exits.

		fd_set set;
		struct timeval timeout;

		FD_ZERO(&set);
		FD_SET(STDIN_FILENO, &set);
		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;

		if (select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) > 0)
		{
			int c = getchar();
			if (c == '\033')
			{
				if (getchar() == '[')
				{
					switch (getchar())
					{
					case 'A':
						return Direction::UP;
					case 'B':
						return Direction::DOWN;
					case 'C':
						return Direction::RIGHT;
					case 'D':
						return Direction::LEFT;
					}
				}
			}
		}

		return Direction::NOOP;
	}
};

int main()
{
	auto game = std::make_unique<Game>(10, 10);
	game->poll();
	switch (game->gameState)
	{
	case Game::RUNNING:
		//????
		break;
	case Game::PAUSED:
		//????
		break;
	case Game::GAMEOVER:
		std::cout << "Game Over!" << std::endl;
		break;
	case Game::VICTORY:
		std::cout << "Victory!" << std::endl;
		break;
	default:
		break;
	}
	return 0;
}
