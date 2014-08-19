
#include "testing.h"

void ProblemTest()
{
	HYProblem prob = GetTestProblem(0);

	HYField field(prob);
	field.Debug();
}
