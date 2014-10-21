
#include "testing.h"
#include "../frontend/frontend.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>

std::vector<std::string> ids;
std::vector<HYProblem> probs;
std::vector<double> difficulty;

void RegisterProblem(std::string &id, std::string &fn)
{
	std::ifstream ifs(fn);

	if (!ifs) {
		std::cerr << "File " << fn << " does not exist" << std::endl;
		return;
	}
	ids.push_back(id);

	HYProblem prob = LoadPBProblem(ifs);
	probs.push_back(prob);
}

std::mutex mtx;
int prob_id;

void EvaluateWorker()
{
	int current_prob = -1;
	double current_score;

	for (;;) {
		mtx.lock();

		if (current_prob != -1) difficulty[current_prob] = current_score;
		if (prob_id >= ids.size()) {
			mtx.unlock();
			break;
		}

		current_prob = prob_id++;

		mtx.unlock();

		current_score = HYEvaluator::Evaluate(probs[current_prob]);
	}
}

double CorRel(std::vector<double> &x, std::vector<double> &y)
{
	double ax = 0, ay = 0;
	for (double v : x) ax += v;
	for (double v : y) ay += v;
	ax /= x.size(); ay /= x.size();

	double cov = 0;
	for (int i = 0; i < x.size(); ++i) cov += (x[i] - ax) * (y[i] - ay);
	
	double std_x = 0, std_y = 0;
	for (double v : x) std_x += (v - ax) * (v - ax);
	for (double v : y) std_y += (v - ay) * (v - ay);

	return cov / sqrt(std_x * std_y);
}

void ConsecutiveEvaluator(int argc, char* argv[])
{
	/*
	Usage: ./heyawake [prefix] [start id] [end id]
	Example: ./heyawake probs/ 1 10
	probs/001.txt, ..., probs/010.txt will be processed
	*/

	if (argc < 4) {
		puts("Usage: ./heyawake [prefix] [start id] [end id]");
		return;
	}

	std::string bas_loc = argv[1];
	int sid = atoi(argv[2]), eid = atoi(argv[3]);

	probs.clear();
	for (int i = eid; i >= sid; --i) {
		std::string fn = bas_loc;
		std::string id;
		id.push_back(i / 100 + '0');
		id.push_back(i / 10 % 10 + '0');
		id.push_back(i % 10 + '0');
		fn += id;
		fn += std::string(".txt");

		RegisterProblem(id, fn);
	}

	HYRoomDatabase::Initialize();

	for (int i = 0; i < ids.size(); ++i) difficulty.push_back(-2.0);

	const int n_threads = 4;
	std::vector<std::thread> ths(n_threads);
	prob_id = 0;

	for (auto& th : ths) {
		th = std::thread(EvaluateWorker);
	}

	for (auto& th : ths) {
		th.join();
	}

	for (int i = ids.size() - 1; i >= 0; --i) {
		std::cout << ids[i] << " " << difficulty[i] << std::endl;
	}

	std::vector<double> x, y;
	for (int i = 0; i < ids.size(); ++i) if (difficulty[i] >= 0) {
		y.push_back(difficulty[i]);
		double pid = 0;
		for (auto c : ids[i]) {
			pid *= 10;
			pid += c - '0';
		}
		x.push_back(pid);
	}

	std::cerr << "Correlation coefficient: " << CorRel(x, y) << std::endl;
}
