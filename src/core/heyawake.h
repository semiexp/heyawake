
#include <vector>

class hy_field;

class hy_problem
{
	int H, W;
	std::vector<int> rY, rX, rH, rW, rHint;

	friend class hy_field;

public:
	hy_problem() : H(0), W(0) { }
	hy_problem(int H, int W) : H(H), W(W) {}

	void add_hint(int y, int x, int h, int w, int hint) {
		rY.push_back(y);
		rX.push_back(x);
		rH.push_back(h);
		rW.push_back(w);
		rHint.push_back(hint);
	}
};

class hy_field
{
	struct cell;
	struct restricted_set;
	struct room;

	struct cell
	{
		int status;

		int roomId;
		int cond; // !!! vector !!!
	};

	struct restricted_set
	{
		int status;
		int xCells; // xor of ids of currently involving cells
	};

	struct room
	{
		int y, x, height, width, hint;

		room() : y(-1), x(-1), height(0), width(0), hint(-1) {}
		room(int y, int x, int height, int width, int hint) : y(y), x(x), height(height), width(width), hint(hint) {}
	};

	int H, W, nRSets, nRooms;
	cell *field;
	restricted_set *rsets;
	room *rooms;

public:
	static const int UNDECIDED = 0;
	static const int WHITE = 1;
	static const int BLACK = 2;

	static const int NORMAL = 0;
	static const int SOLVED = 1;
	static const int INCONSISTENT = 2;

	hy_field();
	hy_field(hy_problem &prob);
	hy_field(const hy_field &field);

	void load(hy_problem &prob);

	int determine_white(int y, int x);
	int determine_black(int y, int x);

	int solve_restricted_set(int sid);
	int solve_room(int rid); // (* TODO : necessary? *)

	void debug();
};
