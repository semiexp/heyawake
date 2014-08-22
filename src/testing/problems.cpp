
#include "testing.h"

/* 
Format for each room: (top y, top x, height, width, hint)
 */

/* 
Source: http://www.nikoli.com/en/puzzles/heyawake/rule.html
 */

int problem1[][5] = 
{
	{ 0, 0, 2, 2, 2 },
	{ 0, 2, 2, 1, -1 },
	{ 0, 3, 2, 3, -1 },
	{ 0, 6, 1, 4, -1 },
	{ 1, 6, 1, 3, -1 },
	{ 1, 9, 4, 1, -1 },
	{ 2, 0, 2, 4, -1 },
	{ 2, 4, 3, 2, -1 },
	{ 2, 6, 3, 3, 5 },
	{ 4, 0, 3, 1, -1 },
	{ 4, 1, 2, 1, 0 },
	{ 4, 2, 3, 2, 1 },
	{ 5, 4, 2, 3, -1 },
	{ 5, 7, 2, 3, 2 },
	{ 6, 1, 1, 1, 1 },
	{ 7, 0, 3, 1, 2 },
	{ 7, 1, 3, 2, -1 },
	{ 7, 3, 1, 3, -1 },
	{ 7, 6, 3, 2, -1 },
	{ 7, 8, 1, 2, -1 },
	{ 8, 3, 2, 3, 3 },
	{ 8, 8, 2, 2, -1 }
};

/*
Source: http://www.nikoli.com/en/puzzles/heyawake/ Sample 1
*/

int problem2[][5] = 
{
	{ 0, 0, 2, 2, -1 },
	{ 0, 2, 2, 3, -1 },
	{ 0, 5, 2, 3, -1 },
	{ 0, 8, 2, 2, 0 },
	{ 2, 0, 3, 2, -1 },
	{ 2, 2, 3, 3, 5 },
	{ 2, 5, 1, 3, -1 },
	{ 2, 8, 3, 2, 1 },
	{ 3, 5, 2, 3, 0 },
	{ 5, 0, 3, 2, 1 },
	{ 5, 2, 2, 3, 2 },
	{ 5, 5, 3, 3, 2 },
	{ 5, 8, 3, 2, -1 },
	{ 7, 2, 1, 3, -1 },
	{ 8, 0, 2, 2, 0 },
	{ 8, 2, 2, 3, -1 },
	{ 8, 5, 2, 3, 1 },
	{ 8, 8, 2, 2, -1 }
};

/*
Source: http://www.nikoli.com/en/puzzles/heyawake/ Sample 2
*/

int problem3[][5] =
{
	{ 0, 0, 1, 1, 1 },
	{ 0, 1, 1, 4, -1 },
	{ 0, 5, 1, 1, 1 },
	{ 0, 6, 1, 4, -1 },
	{ 1, 0, 2, 5, 4 },
	{ 1, 5, 2, 5, 4 },
	{ 3, 0, 1, 1, 1 },
	{ 3, 1, 1, 4, -1 },
	{ 3, 5, 3, 5, 4 },
	{ 4, 0, 2, 5, 4 },
	{ 6, 0, 4, 2, 3 },
	{ 6, 2, 4, 3, 2 },
	{ 6, 5, 4, 2, 4 },
	{ 6, 7, 4, 1, -1 },
	{ 6, 8, 4, 2, -1 }
};

/*
Source: http://www.nikoli.com/en/puzzles/heyawake/ Sample 3
*/

int problem4[][5] =
{
	{ 0, 0, 3, 2, 2 },
	{ 0, 2, 3, 2, 2 },
	{ 0, 4, 1, 4, -1 },
	{ 0, 8, 3, 2, -1 },
	{ 1, 4, 2, 4, 3 },
	{ 3, 0, 3, 2, 1 },
	{ 3, 2, 3, 3, 5 },
	{ 3, 5, 3, 3, -1 },
	{ 3, 8, 3, 2, 3 },
	{ 6, 0, 2, 2, 2 },
	{ 6, 2, 1, 3, -1 },
	{ 6, 5, 1, 3, 2 },
	{ 6, 8, 3, 2, 2 },
	{ 7, 2, 3, 2, 2 },
	{ 7, 4, 3, 2, 2 },
	{ 7, 6, 3, 2, -1 },
	{ 8, 0, 2, 2, 1 },
	{ 9, 8, 1, 2, -1 }
};

HYProblem GetTestProblem(int pid)
{
	HYProblem ret;
	int (*src)[5] = nullptr, len = 0;

	switch (pid) {
	case 0:
		ret = HYProblem(10, 10);
		src = problem1;
		len = sizeof(problem1) / (5 * sizeof(int));
		break; 
	case 1:
		ret = HYProblem(10, 10);
		src = problem2;
		len = sizeof(problem2) / (5 * sizeof(int));
		break;
	case 2:
		ret = HYProblem(10, 10);
		src = problem3;
		len = sizeof(problem3) / (5 * sizeof(int));
		break;
	case 3:
		ret = HYProblem(10, 10);
		src = problem4;
		len = sizeof(problem4) / (5 * sizeof(int));
		break;
	}

	for (int i = 0; i < len; i++) {
		ret.AddHint(src[i][0], src[i][1], src[i][2], src[i][3], src[i][4]);
	}

	return ret;
}
