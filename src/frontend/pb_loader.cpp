
#include "frontend.h"
#include <string>
#include <vector>

std::vector<int> ReadLineAndSplit(std::istream &in)
{
	std::string line;
	std::getline(in, line);

	std::vector<int> ret;
	int tmp = 0;

	if (line[line.size() - 1] >= '0' && line[line.size() - 1] <= '9') line.push_back(' ');

	for (char c : line) {
		if (c >= '0' && c <= '9') {
			tmp *= 10;
			tmp += c - '0';
		} else {
			ret.push_back(tmp);
			tmp = 0;
		}
	}

	return ret;
}

HYProblem LoadPBProblem(std::istream &in)
{
	int height = ReadLineAndSplit(in)[0];
	int width = ReadLineAndSplit(in)[0];
	int n_hint = ReadLineAndSplit(in)[0];
	
	HYProblem prob(height, width);

	for (int i = 0; i < n_hint; ++i) {
		auto ln = ReadLineAndSplit(in);
		if (ln.size() == 4) ln.push_back(-1);

		prob.AddHint(ln[0], ln[1], ln[2] - ln[0] + 1, ln[3] - ln[1] + 1, ln[4]);
	}

	return prob;
}
