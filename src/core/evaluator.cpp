
#include "heyawake.h"

#include <algorithm>

double HYEvaluator::ADJACENT_BLACK = 1.0;
double HYEvaluator::CELL_CONNECTIVITY = 2.0;
double HYEvaluator::THREE_ROOM = 1.0;
double HYEvaluator::PSEUDO_CONNECTION = 5.0;

HYEvaluator::StepCand SingleCandidate(int cell, int type, double weight)
{
	std::vector<std::pair<int, int> > ret;
	ret.push_back(std::make_pair(cell, type));
	return std::make_pair(weight, ret);
}

void HYEvaluator::CheckAdjacentBlack(HYField &field, StepStore &sto)
{
	int height = field.height, width = field.width;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) if (field.CellStatus(i, j) == HYField::BLACK) {
			std::vector<std::pair<int, int> > cands;

			for (int k = 0; k < 4; ++k) {
				int y = i + HYField::dy[k], x = j + HYField::dx[k];

				if (field.Range(y, x) && field.CellStatus(y, x) == HYField::UNDECIDED) {
					cands.push_back(std::make_pair(field.Id(i, j), 0));
				}
			}

			if (cands.size() > 0) sto.push_back(std::make_pair(ADJACENT_BLACK, cands));
		}
	}
}

void HYEvaluator::CheckCellConnectivity(HYField &field, StepStore &sto)
{
	int height = field.height, width = field.width;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			// TODO: proper way to estimate 'weight' is the distance in the tree

			if (!field.rel_pseudo_con[field.Id(i, j)]) {
				if (!field.conm_ps.CheckValidity(i, j)) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, PSEUDO_CONNECTION));
				}
			}
			if (!field.conm.CheckValidity(i, j)) {
				sto.push_back(SingleCandidate(field.Id(i, j), 0, CELL_CONNECTIVITY));
			}
		}
	}
}

void HYEvaluator::CheckThreeRoom(HYField &field, StepStore &sto)
{
	for (int i = 0; i < field.n_rsets; ++i) {
		if (field.rsets[i].rem_cells == 1) {
			sto.push_back(SingleCandidate(field.rsets[i].xor_id, 1, THREE_ROOM));
		}
	}
}

void HYEvaluator::CheckVirtualRoom(HYField &field, StepStore &sto, int top_y, int top_x, int end_y, int end_x, int hint, double ofs)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;

	int n_black = 0, n_undecided = 0;
	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			HYField::Status st = field.CellStatus(top_y + i, top_x + j);
			if (st == HYField::UNDECIDED) {
				++n_undecided;
			}
			else if (st == HYField::BLACK) ++n_black;
		}
	}

	if (n_black == hint) {
		std::vector<std::pair<int, int> > cands;

		for (int i = top_y; i < end_y; ++i) {
			for (int j = top_x; j < end_x; ++j) {
				if (field.CellStatus(top_y + i, top_x + j) == HYField::UNDECIDED) {
					cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), 0));
				}
			}
		}

		if(cands.size() > 0) sto.push_back(std::make_pair(1.0 + ofs, cands));

		return;
	}

	if (n_undecided == hint - n_black) {
		std::vector<std::pair<int, int> > cands;

		for (int i = top_y; i < end_y; ++i) {
			for (int j = top_x; j < end_x; ++j) {
				if (field.CellStatus(top_y + i, top_x + j) == HYField::UNDECIDED) {
					cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), 1));
				}
			}
		}

		if(cands.size() > 0) sto.push_back(std::make_pair(1.0 + ofs, cands));

		return;
	}

	if (room_h == 3 && room_w == 3) {
		if (hint == 5) {
			std::vector<std::pair<int, int> > cands;

			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					if (field.CellStatus(top_y + i, top_x + j) == HYField::UNDECIDED) {
						cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), 1 ^ ((i + j) & 1)));
					}
				}
			}

			if(cands.size() > 0) sto.push_back(std::make_pair(3.0 + ofs, cands));
			
			return;
		}
	}

	if (room_h == 1 && room_w % 2 == 1 && hint == (room_w + 1) / 2) {
		std::vector<std::pair<int, int> > cands;

		for (int i = 0; i < room_w; ++i) {
			if (field.CellStatus(top_y, top_x + i) == HYField::UNDECIDED) {
				cands.push_back(std::make_pair(field.Id(top_y, top_x + i), 1 ^ (i & 1)));
			}
		}

		if (cands.size() > 0) sto.push_back(std::make_pair(2.0 + ofs, cands));

		return;
	}

	if (room_w == 1 && room_h % 2 == 1 && hint == (room_h + 1) / 2) {
		std::vector<std::pair<int, int> > cands;

		for (int i = 0; i < room_h; ++i) {
			if (field.CellStatus(top_y + i, top_x) == HYField::UNDECIDED) {
				cands.push_back(std::make_pair(field.Id(top_y + i, top_x), 1 ^ (i & 1)));
			}
		}

		if (cands.size() > 0) sto.push_back(std::make_pair(2.0 + ofs, cands));

		return;
	}

}

void HYEvaluator::ShrinkRoom(HYField &field, StepStore &sto, int room_id)
{
	HYField::Room &room = field.rooms[room_id];

	int top_y = 127, top_x = 127, end_y = -1, end_x = -1;
	int rem_hint = room.hint;

	for (int i = room.top_y; i < room.end_y; ++i) {
		for (int j = room.top_x; j < room.end_x; ++j) {
			if (field.CellStatus(i, j) == HYField::UNDECIDED) {
				if (top_y < i) top_y = i;
				if (i < end_y) end_y = i;
				if (top_x < j) top_x = j;
				if (j < end_x) end_x = j;
			}
			else if (field.CellStatus(i, j) == HYField::BLACK) --rem_hint;
		}
	}

	if (top_y > end_y) return;

	++end_y; ++end_x;

	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			if (field.CellStatus(i, j) == HYField::BLACK) ++rem_hint;
		}
	}

	CheckVirtualRoom(field, sto, top_y, top_x, end_y, end_x, rem_hint, (rem_hint == room.hint ? 1.0 : 2.0));
}

void HYEvaluator::CheckRoom(HYField &field, StepStore &sto, int room_id)
{
	HYField::Room &room = field.rooms[room_id];

	CheckVirtualRoom(field, sto, room.top_y, room.top_x, room.end_y, room.end_x, room.hint, 0.0);

	ShrinkRoom(field, sto, room_id);
}

double HYEvaluator::Step(HYField &field)
{
	// (step weight, (cell, white: 0, black: 1) )
	int height = field.height, width = field.width;
	StepStore cand;

	// nanika suru
	CheckAdjacentBlack(field, cand);
	CheckCellConnectivity(field, cand);
	CheckThreeRoom(field, cand);

	if (cand.size() == 0) return -1.0;

	double lval = cand[0].first / cand[0].second.size(); int lp = 0;

	for (int i = 1; i < cand.size(); ++i) {
		double lval2 = cand[i].first / cand[i].second.size();

		if (lval > lval2) {
			lval = lval2;
			lp = i;
		}
	}

	auto& hand_point = cand[lp].second;

	for (auto& pt : hand_point) {
		int y = pt.first / width;
		int x = pt.first % width;

		if (pt.second == 0) field.DetermineWhite(y, x);
		if (pt.second == 1) field.DetermineBlack(y, x);
	}

	double ret = 0;
	for (auto& c : cand) {
		double cand_cost = c.first;

		ret += 1 / (cand_cost + 1e-7); // avoid 0 division
	}

	ret = (1 / ret) - 1e-7;

	return ret;
}

double HYEvaluator::Evaluate(HYProblem &prob)
{
	double ret = 0;

	HYField field;
	HYSolverMethod sol; sol.DisableAll();

	field.SetSolverMethod(sol);
	field.Load(prob);

	while (field.GetStatus() == HYField::NORMAL) {
		double st = Step(field);

		if (st < 0) return -1;
		ret += st;
	}

	if (field.GetStatus() == HYField::SOLVED) return ret;

	return -1;
}
