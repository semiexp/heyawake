
#include "heyawake.h"

#include <algorithm>
#include <cmath>

const double HYEvaluator::ADJACENT_BLACK = 0.1;
const double HYEvaluator::CELL_CONNECTIVITY = 0.5;
const double HYEvaluator::THREE_ROOM = 0.1;
const double HYEvaluator::PSEUDO_CONNECTION = 3.0;

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
					cands.push_back(std::make_pair(field.Id(y, x), 0));
				}
			}

			if (cands.size() > 0) sto.push_back(std::make_pair(ADJACENT_BLACK * cands.size(), cands));
		}
	}
}

void HYEvaluator::CheckCellConnectivity(HYField &field, StepStore &sto)
{
	int height = field.height, width = field.width;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			// TODO: proper way to estimate 'weight' is the distance in the tree

			if (field.CellStatus(i, j) != HYField::UNDECIDED) continue;

			if (!field.rel_pseudo_con[field.Id(i, j)]) {
				if (!field.conm.CheckValidity(i, j)) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, CELL_CONNECTIVITY));
				} else if (!field.conm_ps.CheckValidity(i, j)) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, PSEUDO_CONNECTION));
				}
			} else if (!field.conm.CheckValidity(i, j)) {
				sto.push_back(SingleCandidate(field.Id(i, j), 0, CELL_CONNECTIVITY));
			}
		}
	}
}

void HYEvaluator::CheckThreeRoom(HYField &field, StepStore &sto)
{
	for (int i = 0; i < field.n_rsets; ++i) {
		if (field.rsets[i].rem_cells == 1 && !(field.rsets[i].stat & HYField::BLACK)) {
			sto.push_back(SingleCandidate(field.rsets[i].xor_id, 1, THREE_ROOM));
		}
	}
}

int HYEvaluator::CheckValidityOfPattern(HYField &field, int top_y, int top_x, int end_y, int end_x, std::vector<int> &ys, std::vector<int> &xs)
{
	// Assume that the pattern is formed by diagonally connected black cells

	for (int i = 0; i < ys.size(); ++i) {
		if (field.CellStatus(ys[i], xs[i]) == HYField::WHITE) return PATTERN_OCCUPIED;
	}

	bool mode = true;
	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			if (field.rel_pseudo_con[field.Id(i, j)]) mode = false;
		}
	}

	int st = CheckValidityOfPattern(field, top_y, top_x, end_y, end_x, ys, xs, false);
	if (st != PATTERN_VALID || !mode) return st;

	st = CheckValidityOfPattern(field, top_y, top_x, end_y, end_x, ys, xs, true);
	if (st == PATTERN_DISJOINT) st = PATTERN_PSEUDO_DISJOINT;
}

int HYEvaluator::CheckValidityOfPattern(HYField &field, int top_y, int top_x, int end_y, int end_x, std::vector<int> &ys, std::vector<int> &xs, bool mode)
{
	// Assume that the pattern is formed by diagonally connected black cells

	for (int i = 0; i < ys.size(); ++i) {
		if (field.CellStatus(ys[i], xs[i]) == HYField::WHITE) return PATTERN_OCCUPIED;
	}

	/*
	bool mode = true;
	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			if (field.rel_pseudo_con[field.Id(i, j)]) mode = false;
		}
	}
	*/

	int r_aux = mode ? field.PseudoRoot(field.aux_cell) : field.Root(field.aux_cell);
	int aux_cnt = 0;

	std::vector<int> adjs;

	for (int i = 0; i < ys.size(); ++i) {
		bool aux_flg = false;

		for (int j = 0; j < 4; ++j) {
			int y2 = ys[i] + HYField::dy[j] + HYField::dy[(j + 1) % 4], x2 = xs[i] + HYField::dx[j] + HYField::dx[(j + 1) % 4];
			int bid = field.BlackUnitId(y2, x2); if (bid == -1) continue;
			bid = mode ? field.PseudoRoot(bid) : field.Root(bid);

			if (field.Range(y2, x2) && top_y <= y2 && y2 < end_y && top_x <= x2 && x2 < end_x) continue;
			if (bid == r_aux) aux_flg = true;
			else adjs.push_back(bid);
		}

		if (aux_flg) ++aux_cnt;
	}

	if (aux_cnt >= 2) return PATTERN_DISJOINT;
	std::sort(adjs.begin(), adjs.end());
	for (int i = 1; i < adjs.size(); ++i) if (adjs[i] == adjs[i - 1]) return PATTERN_DISJOINT;

	return PATTERN_VALID;
}

void HYEvaluator::CheckVirtualRoom(HYField &field, StepStore &sto, int top_y, int top_x, int end_y, int end_x, int hint, double ofs)
{
	int room_h = end_y - top_y, room_w = end_x - top_x;

	int n_black = 0, n_undecided = 0;
	for (int i = top_y; i < end_y; ++i) {
		for (int j = top_x; j < end_x; ++j) {
			HYField::Status st = field.CellStatus(i, j);
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
				if (field.CellStatus(i, j) == HYField::UNDECIDED) {
					cands.push_back(std::make_pair(field.Id(i, j), 0));
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
				if (field.CellStatus(i, j) == HYField::UNDECIDED) {
					cands.push_back(std::make_pair(field.Id(i, j), 1));
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

	if ((room_h == 2 && hint == room_w) || (room_w == 2 && hint == room_h)) {
		std::vector<int> ys1, xs1, ys2, xs2;

		for (int i = 0; i < room_h; ++i) {
			for (int j = 0; j < room_w; ++j) {
				if ((i + j) % 2 == 0) {
					ys1.push_back(top_y + i);
					xs1.push_back(top_x + j);
				}
				else {
					ys2.push_back(top_y + i);
					xs2.push_back(top_x + j);
				}
			}
		}

		if (top_y == 1 && top_x == 6) {
			top_y += 0;
		}

		int st1 = CheckValidityOfPattern(field, top_y, top_x, end_y, end_x, ys1, xs1);
		int st2 = CheckValidityOfPattern(field, top_y, top_x, end_y, end_x, ys2, xs2);

		if (st1 == PATTERN_VALID && st2 == PATTERN_VALID) return;

		std::vector<std::pair<int, int> > cands;
		int black_p = (st1 == PATTERN_VALID ? 0 : 1);

		for (int i = 0; i < room_h; ++i) {
			for (int j = 0; j < room_w; ++j) {
				if (field.CellStatus(top_y + i, top_x + j) != HYField::UNDECIDED) continue;

				cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), ((i + j) % 2 == black_p) ? 1 : 0));
			}
		}

		int cause = st1 + st2 - PATTERN_VALID;
		double difficulty =
			cause == PATTERN_OCCUPIED ? 2.0: (cause == PATTERN_DISJOINT ? 3.0 : 6.0);

		if (cands.size() > 0) sto.push_back(std::make_pair(difficulty, cands));

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
				if (i < top_y) top_y = i;
				if (end_y < i) end_y = i;
				if (j < top_x) top_x = j;
				if (end_x < j) end_x = j;
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

void HYEvaluator::CheckAllRoom(HYField &field, StepStore &sto)
{
	for (int i = 0; i < field.n_rooms; ++i) CheckRoom(field, sto, i);
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
	CheckAllRoom(field, cand);

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

	if (hand_point[0].first == 76) {
		int x = 0;
		++x;
	}

	for (auto& pt : hand_point) {
		int y = pt.first / width;
		int x = pt.first % width;

		if (pt.second == 0) field.DetermineWhite(y, x);
		if (pt.second == 1) field.DetermineBlack(y, x);
	}

	// printf("%d %f\n", hand_point.size(), lval);

	double ret = 0;
	for (auto& c : cand) {
		double cand_cost = c.first;

		// ret += 1 / (cand_cost + 1e-7);
		ret += pow(cand_cost, -2.5);
	}

	//ret = (1 / ret) - 1e-7;
	ret = pow(ret, -0.4);

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

		if (st < 0) break;
		ret += st;
	}

	if (field.GetStatus() == HYField::SOLVED) return ret;
	printf("%d / %f\n", field.GetStatus(), ret);
	field.Debug();
	field.Debug2();
	return -1;
}
