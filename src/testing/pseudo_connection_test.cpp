
#include "testing.h"

void PseudoConnectionTest()
{
	HYRoomDatabase::Initialize();

	HYProblem prob(10, 10);
	prob.AddHint(0, 0, 3, 2, -1);
	prob.AddHint(0, 2, 3, 1, 2);
	prob.AddHint(0, 3, 10, 7, -1);
	prob.AddHint(3, 0, 2, 2, 2);
	prob.AddHint(3, 2, 7, 1, -1);
	prob.AddHint(5, 0, 5, 3, -1);

	HYField sol(prob);

	HYSolver::Solve(sol);
	sol.Debug();
}
