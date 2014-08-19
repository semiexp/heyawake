
#include "testing.h"

void ProblemTest()
{
	HYProblem prob = GetTestProblem(0);

	HYField field(prob);
	field.Debug();

	field.DetermineWhite(5, 0);
	field.Debug();
}
