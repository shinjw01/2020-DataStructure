#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> // strtok(문자열,분리자)필요할것같아서 만약 사용안하면 지우기
#pragma warning(disable:4996)
#define DO 1
#define GAE 2
#define GUL 3
#define YUT 4
#define MO 5
#define BACKDO -1

typedef struct horses* horse_ptr;
struct horses {
	int finish;					//완주했는가
	int visitcorner3;			//corner_node3를 방문했는가?
	horse_ptr carried;			//업혔는가
	void* location;				//현재 위치가 어디인가? 갈림길 진행 방향 정하는 역할+말잡음, 말업음 확인
	char s[4];
}A, B, C, X, Y, Z;

typedef struct common_node* common_ptr;
struct common_node {
	void* llink;
	void* rlink;
	char s[4];
};

typedef struct corner_type1* corner1_ptr;
struct corner_type1 {
	common_ptr llink1;
	common_ptr llink2;
	common_ptr rlink;
	char s[4];
}corner_node1, corner_node4;

typedef struct corner_type2* corner2_ptr;
struct corner_type2 {
	common_ptr llink;
	common_ptr rlink1;
	common_ptr rlink2;
	char s[4];
}corner_node2, corner_node3;

typedef struct special_node* special_ptr;
struct special_node {
	common_ptr luplink;
	common_ptr ldownlink;
	common_ptr ruplink;
	common_ptr rdownlink;
	char s[4];
}middle_node;

int throwing_record[10] = { 0 };			//던져서 나온 눈의 기록
int moving_order[10] = { 0 };				//눈 순서. 눈 순서대로 말을 움직이고 눈의 기록에서 (눈의 순서에서 지워진 눈과) 같은 눈을 지운다.
int backdo_exist = 0;						//첫판에서 백도 나오면 무시하도록 하기 위해 만든 변수
int arrest = 0;								//throw함수에서 말잡힘으로 불려왔는지 확인하도록 하기 위해 만든 변수
int player_order = 1;
int first_backdo = 0;

common_ptr connect_node(void* ptr);								//서로다른타입의 노드끼리 연결할 함수
common_ptr insert_node(common_ptr ptr1, int num);				//common노드 쭉 이어붙일 함수
void throw();									//윷던지는 함수
char choose_horse();							//움직일 말 선택
void input_order(horse_ptr ptr);												//눈 순서 입력, 남은 눈 있으면 말선택함수로 돌아가도록 코딩.
void move_horse(horse_ptr ptr);					//눈 순서대로 움직이기
void catch_enemy(horse_ptr ptr);				//말잡음
void initialize(horse_ptr ptr);
void carry_ally(horse_ptr ptr);						//말업음
void synchronize(ptr);
int Is_record_empty();
void print_board();

void main() {
	int i;
	horse_ptr ptr = NULL;
	common_ptr ptr1, ptr2;
	corner_node3.rlink1 = NULL;		//connect_node 함수에 쓰기 위해 NULL로 초기화
	middle_node.ldownlink = NULL;
	char answer, horse;

	//윷놀이 판 노드 생성 및 연결
	ptr1 = connect_node(&corner_node1);
	ptr1 = insert_node(ptr1, 3);
	ptr1->rlink = &corner_node2;
	corner_node2.llink = ptr1;

	ptr1 = connect_node(&corner_node2);
	ptr1 = insert_node(ptr1, 3);
	ptr1->rlink = &corner_node3;
	corner_node3.llink = ptr1;

	ptr1 = connect_node(&corner_node3);
	ptr1 = insert_node(ptr1, 3);
	ptr1->rlink = &corner_node4;
	corner_node4.llink1 = ptr1;

	ptr1 = connect_node(&corner_node4);
	ptr1 = insert_node(ptr1, 3);
	ptr1->rlink = &corner_node1;
	corner_node1.llink1 = ptr1;

	ptr1 = connect_node(&corner_node2);
	ptr1 = insert_node(ptr1, 1);
	ptr1->rlink = &middle_node;
	middle_node.ruplink = ptr1;

	ptr1 = connect_node(&corner_node3);
	ptr1 = insert_node(ptr1, 1);
	ptr1->rlink = &middle_node;
	middle_node.luplink = ptr1;

	ptr1 = connect_node(&middle_node);
	ptr1 = insert_node(ptr1, 1);
	ptr1->rlink = &corner_node4;
	corner_node4.llink2 = ptr1;

	ptr1 = connect_node(&middle_node);
	ptr1 = insert_node(ptr1, 1);
	ptr1->rlink = &corner_node1;
	corner_node1.llink2 = ptr1;

	printf("<윷놀이>규칙 설명.\n");
	printf("**규칙1: 플레이어1의 말은 A,B,C,플레이어 2의 말은 X,Y,Z입니다. 모든 말을 먼저 골인시키면 승리입니다.\n");
	printf("**규칙2: 도는 1칸, 개는 2칸, 걸은 3칸, 윷은 4칸, 모는 5칸, 백도는 -1칸입니다.\n윷이나 모가 나오면 한 번 더 던집니다. 상대말을 잡아도 한 번 더 던집니다.\n");
	printf("**규칙3: 백도는 오직 판에 있는 말에만 적용시킬 수 있습니다. 만약 모든 말이 판 위에 있지 않다면 턴이 넘어갑니다.\n");
	printf("**규칙4: 백도로 홈에 들어와도 골인입니다.(ex: 도로 한 칸 갔다가 백도 나와서 홈으로 돌아옴)\n또한, 갈림길에서 백도가 나올 경우 기존에 온 방향으로 뒤로 갑니다.\n");
	printf("**규칙5: 같은 편 말을 업을 수 있습니다.\n\n");
	print_board();
	while (1) {
		printf("윷놀이 새 게임을 시작하시려면 y를 눌러주세요.");
		scanf("%c", &answer);
		if (answer != 'y')
			break;

		printf("\n");
		printf("선 플레이어를 선택해주세요(1 또는 2 입력):");
		scanf("%d", &player_order);
		printf("\n------------------------------------------------------------------------\n");
		A.finish = 0, A.carried = 0, A.visitcorner3 = 0, A.carried = NULL, A.location = &corner_node1, strcpy(A.s, " A ");
		B.finish = 0, B.carried = 0, B.visitcorner3 = 0, B.carried = NULL, B.location = &corner_node1, strcpy(B.s, " B ");
		C.finish = 0, C.carried = 0, C.visitcorner3 = 0, C.carried = NULL, C.location = &corner_node1, strcpy(C.s, " C ");
		X.finish = 0, X.carried = 0, X.visitcorner3 = 0, X.carried = NULL, X.location = &corner_node1, strcpy(X.s, " X ");
		Y.finish = 0, Y.carried = 0, Y.visitcorner3 = 0, Y.carried = NULL, Y.location = &corner_node1, strcpy(Y.s, " Y ");
		Z.finish = 0, Z.carried = 0, Z.visitcorner3 = 0, Z.carried = NULL, Z.location = &corner_node1, strcpy(Z.s, " Z ");
		getchar();// y 입력 받을 때 버퍼에 같이 들어온 \n값 받아서 없애주는 중
		while (!((A.finish == 1 && B.finish == 1 && C.finish == 1) || (X.finish == 1 && Y.finish == 1 && Z.finish == 1))) {
			printf("\n플레이어 %d의 차례입니다. 윷을 던지시려면 Enter를 눌러주세요.", player_order);
			printf("\n");
			throw();
			while (!Is_record_empty()) {
				horse = choose_horse();
				switch (horse)
				{
				case 'A':
					ptr = &A;
					break;
				case 'B':
					ptr = &B;
					break;
				case 'C':
					ptr = &C;
					break;
				case 'X':
					ptr = &X;
					break;
				case 'Y':
					ptr = &Y;
					break;
				case 'Z':
					ptr = &Z;
					break;
				}
				input_order(ptr);
				if (!first_backdo) {
					move_horse(ptr);
					if (arrest)
						continue;
				}
				else
					first_backdo--;
				if (ptr->finish == 1)
					printf("%c가 골인했습니다.\n\n", horse);
			}
			if (player_order == 1)
				player_order = 2;
			else
				player_order = 1;
		}
		if (A.finish == 1 && B.finish == 1 && C.finish == 1)
			printf("플레이어 1의 승리입니다! 축하합니다!");
		else
			printf("플레이어 2의 승리입니다! 축하합니다!");
	}
	system("pause");
}

common_ptr connect_node(void* ptr) {		//진행방향으로 common노드 이어붙여서 만들기.
	common_ptr ptr1;
	ptr1 = (common_ptr)malloc(sizeof(struct common_node));
	ptr1->llink = ptr;
	strcpy(ptr1->s, " O ");
	if ((corner1_ptr)ptr == &corner_node1 || (corner1_ptr)ptr == &corner_node4) {							//corner_type1인경우
		strcpy(((corner1_ptr)ptr)->s, " O ");
		((corner1_ptr)ptr)->rlink = ptr1;
	}
	else if (corner_node3.rlink1 == NULL && ((corner2_ptr)ptr == &corner_node2 || (corner2_ptr)ptr == &corner_node3))//corner_type2인데 rlink1 채울 경우
		((corner2_ptr)ptr)->rlink1 = ptr1;
	else if ((corner2_ptr)ptr == &corner_node2 || (corner2_ptr)ptr == &corner_node3) {					//corner_type2인데 rlink2 채울 경우
		strcpy(((corner2_ptr)ptr)->s, " O ");
		((corner2_ptr)ptr)->rlink2 = ptr1;
	}
	else if ((special_ptr)ptr == &middle_node) {														//special_node인경우
		if (!middle_node.ldownlink) {
			strcpy(middle_node.s, " O ");
			middle_node.ldownlink = ptr1;
		}
		else
			middle_node.rdownlink = ptr1;
	}
	return ptr1;
};

common_ptr insert_node(common_ptr ptr1, int num) {					//다른종류의 노드 다음에 이어진 선두 common_node에 이어질 노드 쭉 선언
	int i;
	common_ptr ptr2 = ptr1;
	for (i = 1; i <= num; i++) {
		ptr1 = (common_ptr)malloc(sizeof(struct common_node));
		ptr1->llink = ptr2;
		strcpy(ptr1->s, " O ");
		ptr2->rlink = ptr1;
		ptr2 = ptr1;
	}
	return ptr2;
};

void throw(){
	int i = 0,j,k;
	char str1[10],str2[20];

	if (arrest) {
		printf("\n상대편 말을 잡았습니다. 한 번 더 굴리고, 움직일 말과 말을 움직일 순서를 다시 정합니다.\n");
		for (k = 0; k <= 9; k++)
			moving_order[k] = 0;
	}

	do {
		strcpy(str2, "");
		getchar();//윷놀이 타격감 살리려고
		srand((unsigned)time(NULL));
		j = rand() % 6 + 1;
		if (j == 6)
			j = -1;

		if (arrest)												//말잡았을때 throwing_record 배열 빈칸에 새로 던진거 채워넣기
			for (k = 0; k <= 9; k++) {
				if (throwing_record[k] == 0) {
					throwing_record[k] = j;
					break;
				}
			}

		else
			throwing_record[i] = j;

		switch (j) {
		case BACKDO:
			strcpy(str1, "백도가");
			break;
		case DO:
			strcpy(str1, "도가");
			break;
		case GAE:
			strcpy(str1, "개가");
			break;
		case GUL:
			strcpy(str1, "걸이");
			break;
		case YUT:
			strcpy(str1, "윷이");
			strcpy(str2,"한 번 더 굴립니다.");
			break;
		case MO:
			strcpy(str1, "모가");
			strcpy(str2, "한 번 더 굴립니다.");
			break;
		}
		printf("%s 나왔습니다.%s", str1, str2);
		if (player_order == 1 && (A.location == &corner_node1 || B.location == &corner_node1 || C.location == &corner_node1) && throwing_record[i] == -1)//모든말이판에서출발안했을때윷(모),백도나오면윷먼저쓰고백도쓰게하기위해서
			backdo_exist = 1;
		if (player_order == 2 && (X.location == &corner_node1 || Y.location == &corner_node1 || Z.location == &corner_node1) && throwing_record[i] == -1)
			backdo_exist = 1;
		i++;
	} while ((arrest == 0 && (throwing_record[i - 1] == YUT || throwing_record[i - 1] == MO)) || (arrest == 1 && (throwing_record[k] == YUT || throwing_record[k] == MO)));

	if (arrest) {
		printf("현재 보유한 눈은 ");
		for (i = 0; i < 10; i++) {
			switch (throwing_record[i])
			{
			case DO:
				printf("도 ");
				break;
			case GAE:
				printf("개 ");
				break;
			case GUL:
				printf("걸 ");
				break;
			case YUT:
				printf("윷 ");
				break;
			case MO:
				printf("모 ");
				break;
			case BACKDO:
				printf("백도 ");
				break;
}
}
printf("입니다.\n");
}

};

char choose_horse() {
	char horse;
	while (1) {
		if (player_order == 1) {
			printf("\n움직이고 싶은 말을 선택하세요(A,B,C)");
			scanf("%c", &horse);
			if (!(horse == 'A' || horse == 'B' || horse == 'C')) {
				printf("잘못된 입력입니다. 상관없는 문자를 입력하셨거나 소문자를 입력하셨습니다. 대문자로 말을 입력해주세요.\n\n");
				continue;
			}
		}
		else {
			printf("\n움직이고 싶은 말을 선택하세요(X,Y,Z)");
			scanf("%c", &horse);
			if (!(horse == 'X' || horse == 'Y' || horse == 'Z')) {
				printf("잘못된 입력입니다. 상관없는 문자를 입력하셨거나 소문자를 입력하셨습니다. 대문자로 말을 입력해주세요.\n\n");
				continue;
			}
		}
		return horse;
	}
};

void input_order(horse_ptr ptr) {
	int i = 0;
	char str[30], * token;

	getchar();
	printf("말을 움직이고 싶은 순서를 입력해주세요.(띄어쓰기로만 구분해주세요. ex:윷 도):");
	gets(str);
	token = strtok(str, " ");
	while (token != NULL) {
		if (!strcmp(token, "도"))
			moving_order[i] = DO;
		else if (!strcmp(token, "개"))
			moving_order[i] = GAE;
		else if (!strcmp(token, "걸"))
			moving_order[i] = GUL;
		else if (!strcmp(token, "윷"))
			moving_order[i] = YUT;
		else if (!strcmp(token, "모"))
			moving_order[i] = MO;
		else if (!strcmp(token, "백도"))
			moving_order[i] = BACKDO;
		else {
			printf("\n잘못된 입력입니다.\n");
			input_order(ptr);
			exit(0);
		}
		i++;
		token = strtok(NULL, " ");
	}
	if (backdo_exist == 1 && moving_order[0] == BACKDO && ptr->location == &corner_node1) {
		if (player_order == 1 && ((A.location == &corner_node1) && (B.location == &corner_node1) && (C.location == &corner_node1)) && moving_order[1] == 0) {
			printf("차례를 넘깁니다.\n");
			throwing_record[0] = 0;
			first_backdo++;
		}
		else if (player_order == 2 && ((X.location == &corner_node1) && (Y.location == &corner_node1) && (Z.location == &corner_node1)) && moving_order[1] == 0) {
			printf("차례를 넘깁니다.\n");
			throwing_record[0] = 0;
			first_backdo++;
		}
		else {
			printf("\n잘못된 입력입니다. 출발하지 않은 말을 백도 시킬 수는 없습니다.\n");
			switch (choose_horse())
			{
			case 'A':
				ptr = &A;
				break;
			case 'B':
				ptr = &B;
				break;
			case 'C':
				ptr = &C;
				break;
			case 'X':
				ptr = &X;
				break;
			case 'Y':
				ptr = &Y;
				break;
			case 'Z':
				ptr = &Z;
				break;
			}
			input_order(ptr);
		}
	}
	backdo_exist = 0;
};

void move_horse(horse_ptr ptr) {
	int i = 0, j = 0;
	char str[4];

	arrest = 0;

	while (moving_order[i] != 0) {
		//기존 표기 삭제하기
		if (ptr->location == &corner_node4)
			strcpy(corner_node4.s, " O ");
		else if (ptr->location == &corner_node2 || ptr->location == &corner_node3)
			strcpy(((corner2_ptr)ptr->location)->s, " O ");
		else if (ptr->location == &middle_node)
			strcpy(middle_node.s, " O ");
		else if (ptr->location != &corner_node1)
			strcpy(((common_ptr)ptr->location)->s, " O ");

		//throwing_record 겸사겸사 차례로 초기화(한꺼번에 초기화하지 않는 건 catch_enemy에서 상대편을 잡고 윷을 다시 던져도 수행하다만 기존 눈을 기억하기 위해서)
		for (j = 0; j <= 9; j++) {
			if (throwing_record[j] == moving_order[i]) {
				throwing_record[j] = 0;
				break;
			};
		}

		if (moving_order[i] != -1) {										//백도가 아닐때
			for (j = 1; j <= moving_order[i]; j++) {
				if ((ptr->location == &corner_node1) || (ptr->location == &corner_node4))
					ptr->location = ((corner1_ptr)ptr->location)->rlink;
				else if ((ptr->location == &corner_node2) || (ptr->location == &corner_node3)) {
					if (ptr->location == &corner_node3)
						ptr->visitcorner3 = 1;
					if (j != 1)
						ptr->location = ((corner2_ptr)ptr->location)->rlink1;
					else
						ptr->location = ((corner2_ptr)ptr->location)->rlink2;
				}
				else if (ptr->location == &middle_node) {						//백도가 아닐때 미들노드이면 1.코너2에서왔고지금미들노드에서막출발 2.코너2에서왔고미들노드는중간과정 3.코너3에
					if ((ptr->visitcorner3 == 0) && (j != 1))
						ptr->location = middle_node.ldownlink;
					else
						ptr->location = middle_node.rdownlink;
				}
				else
					ptr->location = ((common_ptr)ptr->location)->rlink;

				if (ptr->location == &corner_node1) {
					ptr->finish = 1;
					break;
				}
			}
		}
		else if (ptr->location == &corner_node4) {							//corner_node4에서의 백도
			if (ptr->visitcorner3)
				ptr->location = corner_node4.llink1;
			else
				ptr->location = corner_node4.llink2;
		}
		else if (ptr->location == &middle_node) {										//가운데 노드 백도
			if (ptr->visitcorner3)
				ptr->location = middle_node.luplink;
			else
				ptr->location = middle_node.ruplink;
		}
		else if (ptr->location != &corner_node1) {										//일반적인 백도
			if (ptr->location == &corner_node2) {
				ptr->location = corner_node2.llink;
			}
			else if (ptr->location == &corner_node3) {									//확률은 희박하지만 코너3에서 코너2까지 백도해서 미들노드로 갈 수도 있으니까
				ptr->visitcorner3 = 0;
				ptr->location = corner_node3.llink;
			}
			else if (ptr->location == corner_node1.rlink) {
				ptr->location = &corner_node1;
				ptr->finish = 1;
			}
			else
				ptr->location = ((common_ptr)ptr->location)->llink;
		}

		catch_enemy(ptr);
		synchronize(ptr);//carry_ally하기전에 한 번 더 해야 2개짜리가 1개 따라잡을때 뒤따라오던 하나가 낙오되는것을 막을 수 있음.
		carry_ally(ptr);
		synchronize(ptr);

		//새로 표기하기
		if (ptr->location == &corner_node4)
			strcpy(corner_node4.s, ptr->s);
		else if (ptr->location == &corner_node2 || ptr->location == &corner_node3)
			strcpy(((corner2_ptr)ptr->location)->s, ptr->s);
		else if (ptr->location == &middle_node)
			strcpy(middle_node.s, ptr->s);
		else if (ptr->location != &corner_node1)
			strcpy(((common_ptr)ptr->location)->s, ptr->s);
		else {
			print_board();
			break;
		}

		print_board();

		if (arrest)
			break;

		moving_order[i] = 0;
		i++;
	}
};

void catch_enemy(horse_ptr ptr) {
	if (ptr->location == &corner_node1)
		exit(0);
	if (player_order == 1) {
		if (ptr->location == X.location && X.finish == 0) {
			initialize(&X);
			arrest = 1;
			throw();
		}
		if (ptr->location == Y.location && Y.finish == 0) {
			initialize(&Y);
			if (arrest == 0) {
				arrest = 1;
				throw();
			}
		}
		if (ptr->location == Z.location && Z.finish == 0) {
			initialize(&Z);
			if (arrest == 0) {
				arrest = 1;
				throw();
			}
		}
	}
	else {
		if (ptr->location == A.location && A.finish == 0) {
			initialize(&A);
			arrest = 1;
			throw();
		}
		if (ptr->location == B.location && B.finish == 0) {
			initialize(&B);
			if (arrest == 0) {
				arrest = 1;
				throw();
			}
		}
		if (ptr->location == C.location && C.finish == 0) {
			initialize(&C);
			if (arrest == 0) {
				arrest = 1;
				throw();
			}
		}
	}
};

void initialize(horse_ptr ptr) {
	ptr->location = &corner_node1;
	ptr->visitcorner3 = 0;
	ptr->carried = NULL;
	if (player_order == 2) {		//지금 턴인 플레이어가 상대편임.
		if (ptr == &A)
			strcpy(ptr->s, " A ");
		else if (ptr == &B)
			strcpy(ptr->s, " B ");
		else if (ptr == &C)
			strcpy(ptr->s, " C ");
	}
	else {
		if (ptr == &X)
			strcpy(ptr->s, " X ");
		else if (ptr == &Y)
			strcpy(ptr->s, " Y ");
		else if (ptr == &Z)
			strcpy(ptr->s, " Z ");
	}
};

void carry_ally(horse_ptr ptr) {
	if (ptr->location == &corner_node1)
		exit(0);
	if (player_order == 1) {
		if ((ptr == &A && (A.location == B.location && A.location == C.location)) || ((ptr == &A || ptr == &B) && A.location == B.location)) {
			B.carried = &A;
			strcpy(A.s, "AB ");
			strcpy(B.s, "AB ");
		}
		else if ((ptr == &A || ptr == &C) && A.location == C.location) {
			C.carried = &A;
			strcpy(A.s, "AC ");
			strcpy(C.s, "AC ");
		}
		else if ((ptr == &C && (A.location == B.location && B.location == C.location)) || ((ptr == &B || ptr == &C) && B.location == C.location)) {
			C.carried = &B;
			strcpy(B.s, "BC ");
			strcpy(C.s, "BC ");
		}
		if (A.location == B.location && B.location == C.location) {
			A.carried = &C;
			B.carried = &A;
			C.carried = &B;
			strcpy(A.s, "ABC");
			strcpy(B.s, "ABC");
			strcpy(C.s, "ABC");
		}
	}
	else {
		if ((ptr == &X && (X.location == Y.location && X.location == Z.location)) || ((ptr == &X || ptr == &Y) && X.location == Y.location)) {
			Y.carried = &X;
			strcpy(X.s, "XY ");
			strcpy(Y.s, "XY ");
		}
		else if ((ptr == &X || ptr == &Z) && X.location == Z.location) {
			Z.carried = &X;
			strcpy(X.s, "XZ ");
			strcpy(Z.s, "XZ ");
		}
		else if ((ptr == &Z && (X.location == Y.location && Y.location == Z.location)) || ((ptr == &Y || ptr == &Z) && Y.location == Z.location)) {
			Z.carried = &Y;
			strcpy(Y.s, "YZ");
			strcpy(Z.s, "YZ ");
		}
		if (X.location == Y.location && Y.location == Z.location) {
			X.carried = &Z;
			Y.carried = &X;
			Z.carried = &Y;
			strcpy(X.s, "XYZ");
			strcpy(Y.s, "XYZ");
			strcpy(Z.s, "XYZ");
		}
	}
};

int Is_record_empty() {
	int i;
	for (i = 0; i < 10; i++)
		if (throwing_record[i] != 0)
			return 0;
	return 1;
};

void synchronize(horse_ptr ptr) {
	if (player_order == 1) {
		if (B.carried != NULL && C.carried != NULL) {
			A.finish = ptr->finish;
			A.visitcorner3 = ptr->visitcorner3;
			B.finish = ptr->finish;
			B.visitcorner3 = ptr->visitcorner3;
			C.finish = ptr->finish;
			C.visitcorner3 = ptr->visitcorner3;
		}
		else if (B.carried != NULL && C.carried == NULL) {
			A.finish = ptr->finish;
			A.visitcorner3 = ptr->visitcorner3;
			B.finish = ptr->finish;
			B.visitcorner3 = ptr->visitcorner3;
		}
		else if (B.carried == NULL && C.carried != NULL) {
			if (C.carried = &A) {
				A.finish = ptr->finish;
				A.visitcorner3 = ptr->visitcorner3;
				C.finish = ptr->finish;
				C.visitcorner3 = ptr->visitcorner3;
			}
			else {
				B.finish = ptr->finish;
				B.visitcorner3 = ptr->visitcorner3;
				C.finish = ptr->finish;
				C.visitcorner3 = ptr->visitcorner3;
			}
		}
	}
	else {
		if (Y.carried != NULL && Z.carried != NULL) {
			X.finish = ptr->finish;
			X.visitcorner3 = ptr->visitcorner3;
			Y.finish = ptr->finish;
			Y.visitcorner3 = ptr->visitcorner3;
			Z.finish = ptr->finish;
			Z.visitcorner3 = ptr->visitcorner3;
		}
		else if (Y.carried != NULL && Z.carried == NULL) {
			X.finish = ptr->finish;
			X.visitcorner3 = ptr->visitcorner3;
			Y.finish = ptr->finish;
			Y.visitcorner3 = ptr->visitcorner3;
		}
		else if (Y.carried == NULL && Z.carried != NULL) {
			if (Z.carried = &X) {
				X.finish = ptr->finish;
				X.visitcorner3 = ptr->visitcorner3;
				Z.finish = ptr->finish;
				Z.visitcorner3 = ptr->visitcorner3;
			}
			else {
				Y.finish = ptr->finish;
				Y.visitcorner3 = ptr->visitcorner3;
				Z.finish = ptr->finish;
				Z.visitcorner3 = ptr->visitcorner3;
			}
		}
	}
}

void print_board() {
	int i;
	common_ptr ptr;
	printf("          <윷놀이 보드판>\n----------------------------------\n");
	//첫줄 인쇄
	printf("     %s", corner_node3.s);
	ptr = corner_node3.llink;
	for (i = 1; i <= 4; i++) {
		printf("-%s", ptr->s);
		if (i <= 3)
			ptr = ptr->llink;
	}
	printf("-%s\n", corner_node2.s);
	//둘째줄 인쇄
	printf("\n     %s", corner_node3.rlink1->s);
	printf("  %s   ", corner_node3.rlink2->s);
	printf("    %s  ", corner_node2.rlink2->s);
	printf("%s\n", corner_node2.llink->s);
	//셋째줄 인쇄
	ptr = corner_node3.rlink1->rlink;
	printf("\n     %s", ptr->s);
	printf("    %s  ", middle_node.luplink->s);
	printf(" %s    ", middle_node.ruplink->s);
	ptr = corner_node2.llink->llink;
	printf("%s\n", ptr->s);
	//넷째줄 인쇄
	printf("\n               %s          \n", middle_node.s);
	//다섯째줄 인쇄
	ptr = corner_node4.llink1->llink;
	printf("\n     %s", ptr->s);
	printf("    %s  ", middle_node.ldownlink->s);
	printf(" %s    ", middle_node.rdownlink->s);
	ptr = corner_node1.rlink->rlink;
	printf("%s\n", ptr->s);
	//여섯째줄 인쇄
	printf("\n     %s", corner_node4.llink1->s);
	printf("  %s     ", corner_node4.llink2->s);
	printf("  %s  ", corner_node1.llink2->s);
	printf("%s\n", corner_node1.rlink->s);
	//마지막줄 인쇄
	printf("\n     %s", corner_node4.s);
	ptr = corner_node4.rlink;
	for (i = 1; i <= 4; i++) {
		printf("-%s", ptr->s);
		if (i <= 3)
			ptr = ptr->rlink;
	}
	printf("-%s\t<-출발점이자 골인점\n----------------------------------\n\n", corner_node1.s);
};