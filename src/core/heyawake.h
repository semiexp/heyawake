
#include <vector>

class HYField;

class HYProblem
{
	int height, width;
	std::vector<int> top_y, top_x, end_y, end_x, hint;

	friend class HYField;

public:
	HYProblem() : height(0), width(0) { }
	HYProblem(int h, int w) : height(h), width(w) {}

	void AddHint(int y, int x, int h, int w, int ht) {
		top_y.push_back(y);
		top_x.push_back(x);
		end_y.push_back(y + h);
		end_x.push_back(x + w);
		hint.push_back(ht);
	}
};

class HYField
{
	struct Cell;
	struct RestrictedSet;
	struct Room;

	typedef char Status;
	typedef char CellCord;
	typedef short CellId;
	typedef short RSetId;
	typedef short RoomId;

	struct Cell
	{
		Status stat;

		RoomId room_id;
		RSetId n_conds, *cond; // !!! vector !!!
	};

	struct RestrictedSet
	{
		Status stat;
		CellCord rem_cells;
		CellId xor_id; // xor of ids of currently involving cells
	};

	struct Room
	{
		CellCord top_y, top_x, end_y, end_x;
		CellId hint;

		Room() : top_y(-1), top_x(-1), end_y(-1), end_x(-1), hint(-1) {}
		Room(CellCord ty, CellCord tx, CellCord ey, CellCord ex, CellId hint) : top_y(ty), top_x(tx), end_y(ey), end_x(ex), hint(hint) {}
	};

	CellCord height, width;
	RSetId n_rsets;
	RoomId n_rooms;
	Status status;
	Cell *field;
	RestrictedSet *rsets;
	Room *rooms;
	char *pool;
	int sz_pool;

	CellId Id(CellCord y, CellCord x) { return y * width + x; }
	bool Range(CellCord y, CellCord x) { return 0 <= y && y < height && 0 <= x && x < width; }

	Status Exclude(CellId cid);
	Status ExcludeFromRSet(RSetId sid, CellId cid);
	Status SolveRestrictedSet(RSetId sid);
	Status SolveRoom(RoomId rid); // (* TODO : necessary? *)
	Status SolveTrivialRoom(RoomId rid); 

public:
	static const Status UNDECIDED = 0;
	static const Status WHITE = 1;
	static const Status BLACK = 2;

	static const Status NORMAL = 0;
	static const Status SOLVED = 1;
	static const Status INCONSISTENT = 2;

	HYField();
	HYField(HYProblem &prob);
	HYField(const HYField &src);

	Status Load(HYProblem &prob);

	Status DetermineWhite(CellCord y, CellCord x);
	Status DetermineBlack(CellCord y, CellCord x);

	void Debug();
};
