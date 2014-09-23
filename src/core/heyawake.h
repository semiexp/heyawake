
#include <vector>

class HYProblem;
class HYField;
class HYSolver;

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

class HYConnectionManager
{
	typedef int CellCord;
	typedef int CellId;

	CellId *unit_root;
	CellId aux;
	CellCord height, width;

public:
	HYConnectionManager() : unit_root(nullptr), aux(-1), height(-1), width(-1) {}
	HYConnectionManager(CellCord height, CellCord width, CellId *space);
	void Load(const HYConnectionManager &src) { memcpy(unit_root, src.unit_root, sizeof(CellId) * (height * width + 1)); }

	CellId *GetPointer() const { return unit_root; }

	bool Range(CellCord y, CellCord x) { return 0 <= y && y < height && 0 <= x && x < width; }
	CellId Id(CellCord y, CellCord x) { return y * width + x; }
	
	CellId Root(CellId p) { return unit_root[p] < 0 ? p : (unit_root[p] = Root(unit_root[p])); }
	void Join(CellId p, CellId q);
	bool CheckValidity(CellCord y, CellCord x);
};

class HYField
{
	struct Cell;
	struct RestrictedSet;
	struct Room;

	//typedef char Status;
	//typedef char CellCord;
	//typedef short CellId;
	//typedef short RSetId;
	//typedef short RoomId;
	typedef int Status;
	typedef int CellCord;
	typedef int CellId;
	typedef int RSetId;
	typedef int RoomId;

	struct Cell
	{
		Status stat;

		RoomId room_id;
		RSetId n_conds, *cond;
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

		bool CellInclude(CellCord y, CellCord x) { return top_y <= y && y < end_y && top_x <= x && x < end_x; }
	};

	CellCord height, width;
	HYConnectionManager conm, conm_ps; // normal manager, pseudo connection manager
	RSetId n_rsets;
	RoomId n_rooms;
	Status status;
	CellId aux_cell;
	CellId progress;
	Cell *field;
	RestrictedSet *rsets;
	Room *rooms;
	bool *rel_pseudo_con;

	char *pool;
	int sz_pool;
	
	static int dx[4];
	static int dy[4];

	CellId Id(CellCord y, CellCord x) { return y * width + x; }
	bool Range(CellCord y, CellCord x) { return 0 <= y && y < height && 0 <= x && x < width; }
	Status CellStatus(CellCord y, CellCord x) { return field[Id(y, x)].stat; }
	RoomId CellRoomId(CellCord y, CellCord x) { return field[Id(y, x)].room_id; }
	CellId BlackUnitId(CellCord y, CellCord x) { return Range(y, x) ? (CellStatus(y, x) == BLACK ? Id(y, x) : -1) : aux_cell; }
	bool IsBlackOrOutOfRange(CellCord y, CellCord x) { return !Range(y, x) || CellStatus(y, x) == BLACK; }

	CellId Root(CellId p) { return conm.Root(p); }
	CellId PseudoRoot(CellId p) { return conm_ps.Root(p); }
	Status AssureConnectivity(CellCord y, CellCord x);

	void CheckPseudoConnection(CellCord y, CellCord x);

	Status Exclude(CellId cid);
	Status ExcludeFromRSet(RSetId sid, CellId cid);
	Status SolveRestrictedSet(RSetId sid);
	Status SolveRoom(RoomId rid);
	Status WhiteRestriction(RoomId id);
	Status SolveRoomWithDatabase(RoomId rid);
	Status SolveVirtualRoomWithDatabase(int top_y, int top_x, int end_y, int end_x, int hint);
	Status SolveVirtualRoom(int top_y, int top_x, int end_y, int end_x, int hint);
	Status SolveTrivialRoom(RoomId rid);

	friend class HYSolver;
	friend class HYConnectionManager;

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
	~HYField();

	Status Load(HYProblem &prob);

	Status DetermineWhite(CellCord y, CellCord x);
	Status DetermineBlack(CellCord y, CellCord x);

	Status GetStatus() { return status; }
	CellId GetProgress() { return progress; }

	void Debug();
	void Debug2();
};

class HYSolver
{
public:
	static HYField::Status AssureConnectivity(HYField &field);
	static HYField::Status CheckAllRoom(HYField &field);
	static HYField::Status CheckPseudoConnection(HYField &field);

	static HYField::Status Solve(HYField &field);
	static HYField::Status Assume(HYField &field, int depth = 1);
};

class HYRoomDatabase
{
	static std::vector<std::vector<int> > room;
	static std::vector<std::vector<std::vector<int> > > detail;

	static int index[10][10][10];
	static int limit[10][10];

	static void Visit(int y, int x, int state, int hint, int height, int width, std::vector<int> &sto, std::vector<std::vector<int> > &sto_detail);
	static void PreCalc(int height, int width, int hint);

public:
	static void Initialize();
	static bool IsAvailable(int height, int width, int hint) { return height <= 9 && width <= 9 && hint <= 9 && hint >= 0 && index[height][width][hint] >= 0; }
	static std::vector<int> &Fetch(int height, int width, int hint) { return room[index[height][width][hint]]; }
	static std::vector<std::vector<int> > &FetchDetail(int height, int width, int hint) { return detail[index[height][width][hint]]; }
	static int Limit(int height, int width) { return (height <= 9 && width <= 9) ? limit[height][width] : 100; }
};
