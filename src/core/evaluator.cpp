
#include "heyawake.h"
#include "../util/util.h"

#include <algorithm>
#include <cmath>
#include <memory>

const double HYEvaluator::ADJACENT_BLACK = 0.1;
const double HYEvaluator::CELL_CONNECTIVITY = 0.6;
const double HYEvaluator::THREE_ROOM = 0.5;
const double HYEvaluator::PSEUDO_CONNECTION = 2.0;

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

	HYConnectionTree tree(height * width + 1);
	//field.Debug();
	field.CreateConnectionTree(tree);

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			// TODO: proper way to estimate 'weight' is the distance in the tree

			if (field.CellStatus(i, j) != HYField::UNDECIDED) continue;
			/*
			if (!field.rel_pseudo_con[field.Id(i, j)]) {
				if (!field.conm.CheckValidity(i, j)) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, CELL_CONNECTIVITY));
				} else if (!field.conm_ps.CheckValidity(i, j)) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, PSEUDO_CONNECTION));
				}
			} else if (!field.conm.CheckValidity(i, j)) {
				sto.push_back(SingleCandidate(field.Id(i, j), 0, CELL_CONNECTIVITY));
			}
			*/
			
			if (!field.rel_pseudo_con[field.Id(i, j)] && !field.conm_ps.CheckValidity(i, j)) {
				double cost = pow(field.conm_ps.CheckCost(i, j, tree), 0.2) * CELL_CONNECTIVITY;
				sto.push_back(SingleCandidate(field.Id(i, j), 0, cost));
			} else if (!field.conm.CheckValidity(i, j)) {
				double cost = pow(field.conm.CheckCost(i, j, tree), 0.2) * CELL_CONNECTIVITY;
				sto.push_back(SingleCandidate(field.Id(i, j), 0, cost));
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

	return st;
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

		if (field.CellStatus(ys[i], xs[i]) != HYField::BLACK) {
			for (int j = 0; j < 4; ++j) {
				int y2 = ys[i] + HYField::dy[j] + HYField::dy[(j + 1) % 4], x2 = xs[i] + HYField::dx[j] + HYField::dx[(j + 1) % 4];
				int bid = field.BlackUnitId(y2, x2); if (bid == -1) continue;
				bid = mode ? field.PseudoRoot(bid) : field.Root(bid);

				if (field.Range(y2, x2) && top_y <= y2 && y2 < end_y && top_x <= x2 && x2 < end_x) continue;
				if (bid == r_aux) aux_flg = true;
				else adjs.push_back(bid);
			}
		}

		if (aux_flg) ++aux_cnt;
	}

	if (aux_cnt >= 2) return PATTERN_DISJOINT;
	std::sort(adjs.begin(), adjs.end());
	for (int i = 1; i < adjs.size(); ++i) if (adjs[i] == adjs[i - 1]) return PATTERN_DISJOINT;

	return PATTERN_VALID;
}

void HYEvaluator::CheckWhiteRestriction(HYField &field, StepStore &sto, int rid)
{
	HYField::Room &room = field.rooms[rid];
	if (room.hint == -1) return;

	int height = field.height, width = field.width;
	std::vector<std::pair<int, int> > cands;

	if (0 < room.top_y && room.end_y < height) {
		int res = 0, n_black = 0;

		for (int j = room.top_x; j < room.end_x; ++j) {
			int row_black = 0;
			for (int i = room.top_y; i < room.end_y; ++i) {
				if (field.CellStatus(i, j) == HYField::BLACK) ++row_black;
			}

			if (row_black > 0) n_black += row_black;
			else if (field.CellStatus(room.top_y - 1, j) == HYField::WHITE && field.CellStatus(room.end_y, j) == HYField::WHITE) ++res;
		}

		if (res + n_black == room.hint) {
			for (int j = room.top_x; j < room.end_x; ++j) {
				bool flg = field.CellStatus(room.top_y - 1, j) == HYField::WHITE && field.CellStatus(room.end_y, j) == HYField::WHITE;

				for (int i = room.top_y; i < room.end_y; ++i) {
					if (field.CellStatus(i, j) == HYField::BLACK) flg = false;
				}

				if (!flg) {
					for (int i = room.top_y; i < room.end_y; ++i) {
						if (field.CellStatus(i, j) == HYField::UNDECIDED) {
							cands.push_back(std::make_pair(field.Id(i, j), 0));
						}
					}
				}
			}
		}
	}

	if (0 < room.top_x && room.end_x < width) {
		int res = 0, n_black = 0;

		for (int i = room.top_y; i < room.end_y; ++i) {
			int row_black = 0;
			for (int j = room.top_x; j < room.end_x; ++j) {
				if (field.CellStatus(i, j) == HYField::BLACK) ++row_black;
			}

			if (row_black > 0) n_black += row_black;
			else if (field.CellStatus(i, room.top_x - 1) == HYField::WHITE && field.CellStatus(i, room.end_x) == HYField::WHITE) ++res;
		}

		if (res + n_black == room.hint) {
			for (int i = room.top_y; i < room.end_y; ++i) {
				bool flg = field.CellStatus(i, room.top_x - 1) == HYField::WHITE && field.CellStatus(i, room.end_x) == HYField::WHITE;

				for (int j = room.top_x; j < room.end_x; ++j) {
					if (field.CellStatus(i, j) == HYField::BLACK) flg = false;
				}

				if (!flg) {
					for (int j = room.top_x; j < room.end_x; ++j) {
						if (field.CellStatus(i, j) == HYField::UNDECIDED) {
							cands.push_back(std::make_pair(field.Id(i, j), 0));
						}
					}
				}
			}
		}
	}

	if (cands.size() > 0) sto.push_back(std::make_pair(1.0 * cands.size(), cands));
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

		if (cands.size() > 0) sto.push_back(std::make_pair(0.5 * cands.size() + ofs, cands));

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

		if (cands.size() > 0) sto.push_back(std::make_pair(1.0 * cands.size() + ofs, cands));

		return;
	}

	if (room_h == 3 && room_w == 3) {
		if (hint == 5) {
			std::vector<std::pair<int, int> > cands;

			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					if (field.CellStatus(top_y + i, top_x + j) == HYField::UNDECIDED && (1 ^ ((i + j) & 1)) == 1) {
						cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), 1 ^ ((i + j) & 1)));
					}
				}
			}

			if (cands.size() > 0) sto.push_back(std::make_pair(1.0 * cands.size() + ofs, cands));
			
			return;
		}

		if (hint == 4) {
			std::vector<std::pair<int, int> > cands;
			std::vector<std::pair<int, int> > black_cand;

			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					if ((i + j) % 2 == 0) {
						black_cand.push_back(std::make_pair(top_y + i, top_x + j));
					}
					if (field.CellStatus(top_y + i, top_x + j) == HYField::UNDECIDED && (i + j) % 2 == 1) {
						cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), 0));
					}
				}
			}

			for (int i = 0; i < 5; ++i) {
				if (i == 2) continue;

				std::vector<int> ys, xs;
				for (int j = 0; j < 5; ++j) if (i != j) {
					ys.push_back(black_cand[j].first);
					xs.push_back(black_cand[j].second);
				}

				int st = CheckValidityOfPattern(field, top_y, top_x, end_y, end_x, ys, xs);

				if (st != PATTERN_VALID) {
					if (field.CellStatus(black_cand[i].first, black_cand[i].second) == HYField::UNDECIDED) {
						cands.push_back(std::make_pair(field.Id(black_cand[i].first, black_cand[i].second), 1));
					}
				}
			}

			if (cands.size() > 0) sto.push_back(std::make_pair(1.4 * cands.size() + ofs, cands));

			return;
		}
	}

	if (room_h == 1 && room_w % 2 == 1 && hint == (room_w + 1) / 2) {
		std::vector<std::pair<int, int> > cands;

		for (int i = 0; i < room_w; ++i) {
			if (field.CellStatus(top_y, top_x + i) == HYField::UNDECIDED && (1 ^ (i & 1)) == 1) {
				cands.push_back(std::make_pair(field.Id(top_y, top_x + i), 1 ^ (i & 1)));
			}
		}

		if (cands.size() > 0) sto.push_back(std::make_pair(0.9 * cands.size() + ofs, cands));

		return;
	}

	if (room_w == 1 && room_h % 2 == 1 && hint == (room_h + 1) / 2) {
		std::vector<std::pair<int, int> > cands;

		for (int i = 0; i < room_h; ++i) {
			if (field.CellStatus(top_y + i, top_x) == HYField::UNDECIDED && (1 ^ (i & 1)) == 1) {
				cands.push_back(std::make_pair(field.Id(top_y + i, top_x), 1 ^ (i & 1)));
			}
		}

		if (cands.size() > 0) sto.push_back(std::make_pair(0.9 * cands.size() + ofs, cands));

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

		std::vector<std::pair<int, int> > cands;

		if (!(st1 == PATTERN_VALID && st2 == PATTERN_VALID)) {
			int black_p = (st1 == PATTERN_VALID ? 0 : 1);

			for (int i = 0; i < room_h; ++i) {
				for (int j = 0; j < room_w; ++j) {
					if (field.CellStatus(top_y + i, top_x + j) != HYField::UNDECIDED) continue;

					if ((i + j) % 2 == black_p) cands.push_back(std::make_pair(field.Id(top_y + i, top_x + j), ((i + j) % 2 == black_p) ? 1 : 0));
				}
			}
		}
		int cause = st1 + st2 - PATTERN_VALID;
		double difficulty = 
			(cause == PATTERN_VALID || cause == PATTERN_OCCUPIED) ? 1.1 : (cause == PATTERN_DISJOINT ? 1.1 : 2.2);

		if (room_h == 2 && hint == room_w) {
			for (int i = 0; i < room_w; ++i) {
				if ((i == 0 && top_x != 0) || (i == room_w - 1 && end_x != field.width)) continue;

				if (field.Range(top_y - 1, top_x + i) && field.CellStatus(top_y - 1, top_x + i) == HYField::UNDECIDED)
					cands.push_back(std::make_pair(field.Id(top_y - 1, top_x + i), 0));

				if (field.Range(top_y + 2, top_x + i) && field.CellStatus(top_y + 2, top_x + i) == HYField::UNDECIDED)
					cands.push_back(std::make_pair(field.Id(top_y + 2, top_x + i), 0));
			}
		}

		if (room_w == 2 && hint == room_h) {
			for (int i = 0; i < room_h; ++i) {
				if ((i == 0 && top_y != 0) || (i == room_h - 1 && end_y != field.height)) continue;

				if (field.Range(top_y + i, top_x - 1) && field.CellStatus(top_y + i, top_x - 1) == HYField::UNDECIDED)
					cands.push_back(std::make_pair(field.Id(top_y + i, top_x - 1), 0));

				if (field.Range(top_y + i, top_x + 2) && field.CellStatus(top_y + i, top_x + 2) == HYField::UNDECIDED)
					cands.push_back(std::make_pair(field.Id(top_y + i, top_x + 2), 0));
			}
		}

		if (cands.size() > 0) sto.push_back(std::make_pair(difficulty * cands.size() + ofs, cands));

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

	CheckVirtualRoom(field, sto, top_y, top_x, end_y, end_x, rem_hint, 0.2); // (rem_hint == room.hint ? 0.5 : 0.6));
}

void HYEvaluator::SeparateRoom(HYField &field, StepStore &sto, int room_id)
{
	HYField::Room &room = field.rooms[room_id];

	int room_h = room.end_y - room.top_y, room_w = room.end_x - room.top_x;
	int hint = room.hint;

	std::unique_ptr<int[]> uft(new int[room_h * room_w]);
	UnionFind uf(room_h * room_w, uft.get());

	for (int i = 0; i < room_h; ++i) {
		for (int j = 0; j < room_w; ++j) {
			if (field.CellStatus(i + room.top_y, j + room.top_x) == HYField::UNDECIDED) {
				if (i + 1 < room_h && field.CellStatus(i + 1 + room.top_y, j + room.top_x) == HYField::UNDECIDED) {
					uf.join((i + 1) * room_w + j, i * room_w + j);
				}
				if (j + 1 < room_w && field.CellStatus(i + room.top_y, j + 1 + room.top_x) == HYField::UNDECIDED) {
					uf.join(i * room_w + j, i * room_w + (j + 1));
				}
			}
			else if (field.CellStatus(i + room.top_y, j + room.top_x) == HYField::BLACK) {
				--hint;
			}
		}
	}

	bool update;
	do{
		update = false;

		for (int i = 0; i < room_h; ++i) {
			for (int j = 0; j < room_w; ++j) if (field.CellStatus(i + room.top_y, j + room.top_x) == HYField::UNDECIDED && uf.root(i * room_w + j) == i * room_w + j) {
				int top_y = 127, top_x = 127, end_y = -1, end_x = -1;
				int id = i * room_w + j;

				for (int y = 0; y < room_h; ++y) {
					for (int x = 0; x < room_w; ++x) {
						if (field.CellStatus(y + room.top_y, x + room.top_x) == HYField::UNDECIDED && uf.root(y * room_w + x) == id) {
							if (y < top_y) top_y = y;
							if (end_y < y) end_y = y;
							if (x < top_x) top_x = x;
							if (end_x < x) end_x = x;
						}
					}
				}

				++end_y; ++end_x;

				for (int y = top_y; y < end_y; ++y) {
					for (int x = top_x; x < end_x; ++x) {
						if (field.CellStatus(y + room.top_y, x + room.top_x) == HYField::UNDECIDED) {
							int id2 = y * room_w + x;
							if (uf.root(id) != uf.root(id2)) {
								update = true;
								uf.join(id, id2);
							}
						}
					}
				}
			}
		}
	} while (update);

	std::vector<int> top_ys, top_xs, end_ys, end_xs, m_blacks;

	for (int i = 0; i < room_h; ++i) {
		for (int j = 0; j < room_w; ++j) {
			if (field.CellStatus(i + room.top_y, j + room.top_x) == HYField::UNDECIDED && uf.root(i * room_w + j) == i * room_w + j) {
				int top_y = 127, top_x = 127, end_y = -1, end_x = -1;
				int id = i * room_w + j;

				for (int y = 0; y < room_h; ++y) {
					for (int x = 0; x < room_w; ++x) {
						if (field.CellStatus(y + room.top_y, x + room.top_x) == HYField::UNDECIDED && uf.root(y * room_w + x) == id) {
							if (y < top_y) top_y = y;
							if (end_y < y) end_y = y;
							if (x < top_x) top_x = x;
							if (end_x < x) end_x = x;
						}
					}
				}

				++end_y; ++end_x;

				for (int y = top_y; y < end_y; ++y) {
					for (int x = top_x; x < end_x; ++x) {
						if (field.CellStatus(y + room.top_y, x + room.top_x) == HYField::BLACK) ++hint;
					}
				}

				if ((end_y - top_y) * (end_x - top_x) > 30) return;

				int m_black = field.MaximumBlackCells(top_y, top_x, end_y, end_x, -1);

				top_ys.push_back(top_y + room.top_y);
				top_xs.push_back(top_x + room.top_x);
				end_ys.push_back(end_y + room.top_y);
				end_xs.push_back(end_x + room.top_x);
				m_blacks.push_back(m_black);
			}
		}
	}

	if (m_blacks.size() <= 1) return;

	int tot = 0;
	for (int m : m_blacks) tot += m;

	if (tot == hint) {
		for (int i = 0; i < top_ys.size(); ++i) {
			CheckVirtualRoom(field, sto, top_ys[i], top_xs[i], end_ys[i], end_xs[i], m_blacks[i], 0.3);
		}
	}
}

void HYEvaluator::CheckRoom(HYField &field, StepStore &sto, int room_id)
{
	HYField::Room &room = field.rooms[room_id];
	if (room.hint == -1) return;

	CheckVirtualRoom(field, sto, room.top_y, room.top_x, room.end_y, room.end_x, room.hint, 0.0);

	ShrinkRoom(field, sto, room_id);
	CheckWhiteRestriction(field, sto, room_id);
	SeparateRoom(field, sto, room_id);
}

void HYEvaluator::CheckAllRoom(HYField &field, StepStore &sto)
{
	for (int i = 0; i < field.n_rooms; ++i) CheckRoom(field, sto, i);
}

int HYEvaluator::SolveArea(HYField &field, int top_y, int top_x, int end_y, int end_x)
{
	int height = field.height, width = field.width;

	if (top_y < 0) top_y = 0;
	if (end_y >= height) end_y = height;
	if (top_x < 0) top_x = 0;
	if (end_x >= width) end_x = width;

	int cur_progress;
	int n_steps = 0;

	do {
		cur_progress = field.GetProgress();

		// three room
		for (int i = 0; i < field.n_rsets; ++i) {
			if (field.rsets[i].rem_cells == 1 && !(field.rsets[i].stat & HYField::BLACK)) {
				int loc = field.rsets[i].xor_id;
				int y = loc / width, x = loc % width;

				if (top_y <= y && y < end_y && top_x <= x && x < end_x) {
					field.DetermineBlack(y, x);
				}
			}
		}

		// adjacent black
		for (int i = top_y; i < end_y; ++i) {
			for (int j = top_x; j < end_x; ++j) {
				bool flg = false;
				for (int k = 0; k < 4; ++k) {
					int y = i + HYField::dy[k], x = j + HYField::dx[k];

					if (field.Range(y, x) && field.CellStatus(y, x) == HYField::BLACK) {
						flg = true;
					}
				}

				if (flg) field.DetermineWhite(i, j);
			}
		}

		// room check
		for (int i = top_y; i < end_y; ++i) {
			for (int j = top_x; j < end_x; ++j) {
				int rid = field.field[field.Id(i, j)].room_id;
				auto room = field.rooms[rid];

				if (room.top_y == i && room.top_x == j && room.end_y <= end_y && room.end_x <= end_x) {
					field.SolveRoom(rid);
				}
			}
		}
		++n_steps;
	} while (field.GetStatus() == HYField::NORMAL && cur_progress != field.GetProgress());

	return n_steps;
}

void HYEvaluator::CheckAssumption(HYField &field, StepStore &sto)
{
	int height = field.height, width = field.width;

	HYSolverMethod method;
	method.DisableAll();
	method.virtual_room = true;

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			if (field.CellStatus(i, j) != HYField::UNDECIDED) continue;

			HYField f_black = field, f_white = field;
			f_black.SetSolverMethod(method);
			f_white.SetSolverMethod(method);

			f_black.DetermineBlack(i, j);
			f_white.DetermineWhite(i, j);


			for (int k = 1; k < 6; ++k) {
				int s_black = SolveArea(f_black, i - k, j - k, i + (k + 1), j + (k + 1));

				int s_white = SolveArea(f_white, i - k, j - k, i + (k + 1), j + (k + 1));

				if ((f_black.GetStatus() & HYField::INCONSISTENT) && (f_white.GetStatus() & HYField::INCONSISTENT)) {
					f_black.Debug();
				}
				if (f_black.GetStatus() & HYField::INCONSISTENT) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, 1.8 * sqrt(k * s_black)));
					break;
				}
				if (f_white.GetStatus() & HYField::INCONSISTENT) {
					sto.push_back(SingleCandidate(field.Id(i, j), 1, 1.8 * sqrt(k * s_white)));
					break;
				}
			}

			/*
			if (!(f_white.GetStatus() & HYField::INCONSISTENT) && !(f_black.GetStatus() & HYField::INCONSISTENT)) {
				// insanity
				f_black.SetSolverMethod(HYSolverMethod());
				f_white.SetSolverMethod(HYSolverMethod());
				HYSolver::Solve(f_black);
				HYSolver::Solve(f_white);

				if (f_black.GetStatus() & HYField::INCONSISTENT) {
					sto.push_back(SingleCandidate(field.Id(i, j), 0, 7.0));
				}
				if (f_white.GetStatus() & HYField::INCONSISTENT) {
					sto.push_back(SingleCandidate(field.Id(i, j), 1, 7.0));
				}
			}
			*/
		}
	}
}

double HYEvaluator::Step(HYField &field, std::vector<std::pair<int, int> > &last)
{
	// (step weight, (cell, white: 0, black: 1) )
	int height = field.height, width = field.width;
	StepStore cand;

	// nanika suru
	CheckAdjacentBlack(field, cand);
	CheckCellConnectivity(field, cand);
	CheckThreeRoom(field, cand);
	CheckAllRoom(field, cand);

	CheckAssumption(field, cand);

	if (cand.size() == 0) return -1.0;
	/*
	if (last.size() > 0) {
		for (int i = 0; i < cand.size(); ++i) {
			double aux_score = cand[i].first;

			for (auto loc : cand[i].second) {
				int y = loc.first / width, x = loc.first % width;
				double s = 10;

				for (auto lp : last) {
					int yl = lp.first / width, xl = lp.first % width;
					int m = (y > yl ? (y - yl) : (yl - y)) + (x > xl ? (x - xl) : (xl - x));
					s = std::min(s, m + 0.0);
					//if (m == 0) fprintf(stderr, "%d %d %d %d\n", y, x, yl, xl);
					//s += m;// pow(m, 0.5);
				}

				//s /= last.size();

				aux_score += s * 0.01;
			}

			cand[i].first = aux_score;
		}
	}
	*/
	double lval = cand[0].first / cand[0].second.size(); int lp = 0;

	for (int i = 1; i < cand.size(); ++i) {
		double lval2 = cand[i].first / cand[i].second.size();

		if (lval > lval2) {
			lval = lval2;
			lp = i;
		}
	}

	int rem_cells = height * width - field.GetProgress();
	auto& hand_point = cand[lp].second;

	for (auto& pt : hand_point) {
		int y = pt.first / width;
		int x = pt.first % width;

		if (pt.second == 0) field.DetermineWhite(y, x);
		if (pt.second == 1) field.DetermineBlack(y, x);
	}

	last.clear();
	last = hand_point;

	double ret = 0;
	for (auto& c : cand) {
		double cand_cost = c.first / c.second.size();

		// ret += 1 / (cand_cost + 1e-7);
		ret += pow(cand_cost, -1.0);
	}

	ret = pow(ret, -(1 / 1.0)) * hand_point.size(); //*pow(rem_cells, 0.2);
	//printf("%dcells (%d) %f pts (%f pts each)\n", hand_point.size(), hand_point[0].first, ret, ret / hand_point.size());
	return ret;
}

double HYEvaluator::Evaluate(HYProblem &prob)
{
	double ret = 0;

	HYField field;
	HYSolverMethod sol; sol.DisableAll();

	field.SetSolverMethod(sol);
	field.Load(prob);

	std::vector<std::pair<int, int> > last;

	while (field.GetStatus() == HYField::NORMAL) {
		double st = Step(field, last);

		if (st < 0) break;
		ret += st;
	}

	if (field.GetStatus() == HYField::SOLVED) return ret / pow(field.height * field.width, 0.6) * 150;
	// printf("%d / %f\n", field.GetStatus(), ret);
	// field.Debug();
	return -1;
}
