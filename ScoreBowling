#include <stdio.h>

#define MAX_FRAME 10
#define STRIKE 0
#define SPARE 1
#define NONE 2

struct frame {
	int first;
	int second;
	int result;
};

struct score {
	int first;
	int second;
};

struct frame f[MAX_FRAME];
struct score s[11] = { 8,0,4,3,8,2,4,6,2,6,10,0,9,0,10,0,8,2,10,0,10,10 };

void main() {
	int i, j, sum = 0;
	for (i = 0; i < MAX_FRAME; i++) {
		f[i].first = s[i].first;
		f[i].second = s[i].second;
		if (s[i].first == 10) {
			sum += s[i].first + s[i + 1].first + s[i + 1].second;
			f[i].result = STRIKE;
			if (i < 9 && s[i + 1].first == 10) //10프레임 이외에 더블 스트라이크를 친 경우
				sum += s[i + 2].first;
		}
		else if ((s[i].first + s[i].second) == 10) //spare
		{
			f[i].result = SPARE;
			sum += s[i].first + s[i].second + s[i + 1].first;
		}
		else // NONE
		{
			f[i].result = NONE;
			sum += s[i].first + s[i].second;
		}
		switch (f[i].result)
		{
		case STRIKE:
			printf("%d/%d : %d STRIKE!\n", s[i].first, s[i].second, sum);
			break;
		case SPARE:
			printf("%d/%d : %d Spare\n",s[i].first,s[i].second,sum);
			break;
		default:
			printf("%d/%d : %d\n", s[i].first, s[i].second, sum);
		}
	}
	if (s[9].first + s[9].second == 10)
		printf("Bonus:%d/%d\n", s[10].first, s[10].second);

	printf("total: %d\n", sum);
}
