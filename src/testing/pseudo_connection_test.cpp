
#include "testing.h"

void PseudoConnectionTest()
{
	HYProblem prob(10, 10);
	prob.AddHint(0, 0, 3, 2, -1);
	prob.AddHint(0, 2, 3, 1, 2);
	prob.AddHint(0, 3, 10, 7, -1);
	prob.AddHint(3, 0, 7, 3, -1);

	HYField sol(prob);
	sol.DetermineBlack(3, 1);

	HYSolver::Solve(sol);
	sol.Debug();
}
