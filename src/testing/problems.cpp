
#include "testing.h"

/* 
Format for each room: (top y, top x, height, width, hint)
 */

/* 
Source: http://www.nikoli.com/ja/puzzles/heyawake/rule.html 
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
	{ 5, 4, 2, 3, 0 },
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
	}

	for (int i = 0; i < len; i++) {
		ret.AddHint(src[i][0], src[i][1], src[i][2], src[i][3], src[i][4]);
	}

	return ret;
}
