#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> // strtok(���ڿ�,�и���)�ʿ��ҰͰ��Ƽ� ���� �����ϸ� �����
#pragma warning(disable:4996)
#define DO 1
#define GAE 2
#define GUL 3
#define YUT 4
#define MO 5
#define BACKDO -1

typedef struct horses* horse_ptr;
struct horses {
	int finish;					//�����ߴ°�
	int visitcorner3;			//corner_node3�� �湮�ߴ°�?
	horse_ptr carried;			//�����°�
	void* location;				//���� ��ġ�� ����ΰ�? ������ ���� ���� ���ϴ� ����+������, ������ Ȯ��
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

int throwing_record[10] = { 0 };			//������ ���� ���� ���
int moving_order[10] = { 0 };				//�� ����. �� ������� ���� �����̰� ���� ��Ͽ��� (���� �������� ������ ����) ���� ���� �����.
int backdo_exist = 0;						//ù�ǿ��� �鵵 ������ �����ϵ��� �ϱ� ���� ���� ����
int arrest = 0;								//throw�Լ����� ���������� �ҷ��Դ��� Ȯ���ϵ��� �ϱ� ���� ���� ����
int player_order = 1;
int first_backdo = 0;

common_ptr connect_node(void* ptr);								//���δٸ�Ÿ���� ��峢�� ������ �Լ�
common_ptr insert_node(common_ptr ptr1, int num);				//common��� �� �̾���� �Լ�
void throw();									//�������� �Լ�
char choose_horse();							//������ �� ����
void input_order(horse_ptr ptr);												//�� ���� �Է�, ���� �� ������ �������Լ��� ���ư����� �ڵ�.
void move_horse(horse_ptr ptr);					//�� ������� �����̱�
void catch_enemy(horse_ptr ptr);				//������
void initialize(horse_ptr ptr);
void carry_ally(horse_ptr ptr);						//������
void synchronize(ptr);
int Is_record_empty();
void print_board();

void main() {
	int i;
	horse_ptr ptr = NULL;
	common_ptr ptr1, ptr2;
	corner_node3.rlink1 = NULL;		//connect_node �Լ��� ���� ���� NULL�� �ʱ�ȭ
	middle_node.ldownlink = NULL;
	char answer, horse;

	//������ �� ��� ���� �� ����
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

	printf("<������>��Ģ ����.\n");
	printf("**��Ģ1: �÷��̾�1�� ���� A,B,C,�÷��̾� 2�� ���� X,Y,Z�Դϴ�. ��� ���� ���� ���ν�Ű�� �¸��Դϴ�.\n");
	printf("**��Ģ2: ���� 1ĭ, ���� 2ĭ, ���� 3ĭ, ���� 4ĭ, ��� 5ĭ, �鵵�� -1ĭ�Դϴ�.\n���̳� �� ������ �� �� �� �����ϴ�. ��븻�� ��Ƶ� �� �� �� �����ϴ�.\n");
	printf("**��Ģ3: �鵵�� ���� �ǿ� �ִ� ������ �����ų �� �ֽ��ϴ�. ���� ��� ���� �� ���� ���� �ʴٸ� ���� �Ѿ�ϴ�.\n");
	printf("**��Ģ4: �鵵�� Ȩ�� ���͵� �����Դϴ�.(ex: ���� �� ĭ ���ٰ� �鵵 ���ͼ� Ȩ���� ���ƿ�)\n����, �����濡�� �鵵�� ���� ��� ������ �� �������� �ڷ� ���ϴ�.\n");
	printf("**��Ģ5: ���� �� ���� ���� �� �ֽ��ϴ�.\n\n");
	print_board();
	while (1) {
		printf("������ �� ������ �����Ͻ÷��� y�� �����ּ���.");
		scanf("%c", &answer);
		if (answer != 'y')
			break;

		printf("\n");
		printf("�� �÷��̾ �������ּ���(1 �Ǵ� 2 �Է�):");
		scanf("%d", &player_order);
		printf("\n------------------------------------------------------------------------\n");
		A.finish = 0, A.carried = 0, A.visitcorner3 = 0, A.carried = NULL, A.location = &corner_node1, strcpy(A.s, " A ");
		B.finish = 0, B.carried = 0, B.visitcorner3 = 0, B.carried = NULL, B.location = &corner_node1, strcpy(B.s, " B ");
		C.finish = 0, C.carried = 0, C.visitcorner3 = 0, C.carried = NULL, C.location = &corner_node1, strcpy(C.s, " C ");
		X.finish = 0, X.carried = 0, X.visitcorner3 = 0, X.carried = NULL, X.location = &corner_node1, strcpy(X.s, " X ");
		Y.finish = 0, Y.carried = 0, Y.visitcorner3 = 0, Y.carried = NULL, Y.location = &corner_node1, strcpy(Y.s, " Y ");
		Z.finish = 0, Z.carried = 0, Z.visitcorner3 = 0, Z.carried = NULL, Z.location = &corner_node1, strcpy(Z.s, " Z ");
		getchar();// y �Է� ���� �� ���ۿ� ���� ���� \n�� �޾Ƽ� �����ִ� ��
		while (!((A.finish == 1 && B.finish == 1 && C.finish == 1) || (X.finish == 1 && Y.finish == 1 && Z.finish == 1))) {
			printf("\n�÷��̾� %d�� �����Դϴ�. ���� �����÷��� Enter�� �����ּ���.", player_order);
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
					printf("%c�� �����߽��ϴ�.\n\n", horse);
			}
			if (player_order == 1)
				player_order = 2;
			else
				player_order = 1;
		}
		if (A.finish == 1 && B.finish == 1 && C.finish == 1)
			printf("�÷��̾� 1�� �¸��Դϴ�! �����մϴ�!");
		else
			printf("�÷��̾� 2�� �¸��Դϴ�! �����մϴ�!");
	}
	system("pause");
}

common_ptr connect_node(void* ptr) {		//����������� common��� �̾�ٿ��� �����.
	common_ptr ptr1;
	ptr1 = (common_ptr)malloc(sizeof(struct common_node));
	ptr1->llink = ptr;
	strcpy(ptr1->s, " O ");
	if ((corner1_ptr)ptr == &corner_node1 || (corner1_ptr)ptr == &corner_node4) {							//corner_type1�ΰ��
		strcpy(((corner1_ptr)ptr)->s, " O ");
		((corner1_ptr)ptr)->rlink = ptr1;
	}
	else if (corner_node3.rlink1 == NULL && ((corner2_ptr)ptr == &corner_node2 || (corner2_ptr)ptr == &corner_node3))//corner_type2�ε� rlink1 ä�� ���
		((corner2_ptr)ptr)->rlink1 = ptr1;
	else if ((corner2_ptr)ptr == &corner_node2 || (corner2_ptr)ptr == &corner_node3) {					//corner_type2�ε� rlink2 ä�� ���
		strcpy(((corner2_ptr)ptr)->s, " O ");
		((corner2_ptr)ptr)->rlink2 = ptr1;
	}
	else if ((special_ptr)ptr == &middle_node) {														//special_node�ΰ��
		if (!middle_node.ldownlink) {
			strcpy(middle_node.s, " O ");
			middle_node.ldownlink = ptr1;
		}
		else
			middle_node.rdownlink = ptr1;
	}
	return ptr1;
};

common_ptr insert_node(common_ptr ptr1, int num) {					//�ٸ������� ��� ������ �̾��� ���� common_node�� �̾��� ��� �� ����
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
		printf("\n����� ���� ��ҽ��ϴ�. �� �� �� ������, ������ ���� ���� ������ ������ �ٽ� ���մϴ�.\n");
		for (k = 0; k <= 9; k++)
			moving_order[k] = 0;
	}

	do {
		strcpy(str2, "");
		getchar();//������ Ÿ�ݰ� �츮����
		srand((unsigned)time(NULL));
		j = rand() % 6 + 1;
		if (j == 6)
			j = -1;

		if (arrest)												//��������� throwing_record �迭 ��ĭ�� ���� ������ ä���ֱ�
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
			strcpy(str1, "�鵵��");
			break;
		case DO:
			strcpy(str1, "����");
			break;
		case GAE:
			strcpy(str1, "����");
			break;
		case GUL:
			strcpy(str1, "����");
			break;
		case YUT:
			strcpy(str1, "����");
			strcpy(str2,"�� �� �� �����ϴ�.");
			break;
		case MO:
			strcpy(str1, "��");
			strcpy(str2, "�� �� �� �����ϴ�.");
			break;
		}
		printf("%s ���Խ��ϴ�.%s", str1, str2);
		if (player_order == 1 && (A.location == &corner_node1 || B.location == &corner_node1 || C.location == &corner_node1) && throwing_record[i] == -1)//��縻���ǿ�����߾���������(��),�鵵����������������鵵�����ϱ����ؼ�
			backdo_exist = 1;
		if (player_order == 2 && (X.location == &corner_node1 || Y.location == &corner_node1 || Z.location == &corner_node1) && throwing_record[i] == -1)
			backdo_exist = 1;
		i++;
	} while ((arrest == 0 && (throwing_record[i - 1] == YUT || throwing_record[i - 1] == MO)) || (arrest == 1 && (throwing_record[k] == YUT || throwing_record[k] == MO)));

	if (arrest) {
		printf("���� ������ ���� ");
		for (i = 0; i < 10; i++) {
			switch (throwing_record[i])
			{
			case DO:
				printf("�� ");
				break;
			case GAE:
				printf("�� ");
				break;
			case GUL:
				printf("�� ");
				break;
			case YUT:
				printf("�� ");
				break;
			case MO:
				printf("�� ");
				break;
			case BACKDO:
				printf("�鵵 ");
				break;
}
}
printf("�Դϴ�.\n");
}

};

char choose_horse() {
	char horse;
	while (1) {
		if (player_order == 1) {
			printf("\n�����̰� ���� ���� �����ϼ���(A,B,C)");
			scanf("%c", &horse);
			if (!(horse == 'A' || horse == 'B' || horse == 'C')) {
				printf("�߸��� �Է��Դϴ�. ������� ���ڸ� �Է��ϼ̰ų� �ҹ��ڸ� �Է��ϼ̽��ϴ�. �빮�ڷ� ���� �Է����ּ���.\n\n");
				continue;
			}
		}
		else {
			printf("\n�����̰� ���� ���� �����ϼ���(X,Y,Z)");
			scanf("%c", &horse);
			if (!(horse == 'X' || horse == 'Y' || horse == 'Z')) {
				printf("�߸��� �Է��Դϴ�. ������� ���ڸ� �Է��ϼ̰ų� �ҹ��ڸ� �Է��ϼ̽��ϴ�. �빮�ڷ� ���� �Է����ּ���.\n\n");
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
	printf("���� �����̰� ���� ������ �Է����ּ���.(����θ� �������ּ���. ex:�� ��):");
	gets(str);
	token = strtok(str, " ");
	while (token != NULL) {
		if (!strcmp(token, "��"))
			moving_order[i] = DO;
		else if (!strcmp(token, "��"))
			moving_order[i] = GAE;
		else if (!strcmp(token, "��"))
			moving_order[i] = GUL;
		else if (!strcmp(token, "��"))
			moving_order[i] = YUT;
		else if (!strcmp(token, "��"))
			moving_order[i] = MO;
		else if (!strcmp(token, "�鵵"))
			moving_order[i] = BACKDO;
		else {
			printf("\n�߸��� �Է��Դϴ�.\n");
			input_order(ptr);
			exit(0);
		}
		i++;
		token = strtok(NULL, " ");
	}
	if (backdo_exist == 1 && moving_order[0] == BACKDO && ptr->location == &corner_node1) {
		if (player_order == 1 && ((A.location == &corner_node1) && (B.location == &corner_node1) && (C.location == &corner_node1)) && moving_order[1] == 0) {
			printf("���ʸ� �ѱ�ϴ�.\n");
			throwing_record[0] = 0;
			first_backdo++;
		}
		else if (player_order == 2 && ((X.location == &corner_node1) && (Y.location == &corner_node1) && (Z.location == &corner_node1)) && moving_order[1] == 0) {
			printf("���ʸ� �ѱ�ϴ�.\n");
			throwing_record[0] = 0;
			first_backdo++;
		}
		else {
			printf("\n�߸��� �Է��Դϴ�. ������� ���� ���� �鵵 ��ų ���� �����ϴ�.\n");
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
		//���� ǥ�� �����ϱ�
		if (ptr->location == &corner_node4)
			strcpy(corner_node4.s, " O ");
		else if (ptr->location == &corner_node2 || ptr->location == &corner_node3)
			strcpy(((corner2_ptr)ptr->location)->s, " O ");
		else if (ptr->location == &middle_node)
			strcpy(middle_node.s, " O ");
		else if (ptr->location != &corner_node1)
			strcpy(((common_ptr)ptr->location)->s, " O ");

		//throwing_record ����� ���ʷ� �ʱ�ȭ(�Ѳ����� �ʱ�ȭ���� �ʴ� �� catch_enemy���� ������� ��� ���� �ٽ� ������ �����ϴٸ� ���� ���� ����ϱ� ���ؼ�)
		for (j = 0; j <= 9; j++) {
			if (throwing_record[j] == moving_order[i]) {
				throwing_record[j] = 0;
				break;
			};
		}

		if (moving_order[i] != -1) {										//�鵵�� �ƴҶ�
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
				else if (ptr->location == &middle_node) {						//�鵵�� �ƴҶ� �̵����̸� 1.�ڳ�2�����԰����ݹ̵��忡������� 2.�ڳ�2�����԰�̵�����߰����� 3.�ڳ�3��
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
		else if (ptr->location == &corner_node4) {							//corner_node4������ �鵵
			if (ptr->visitcorner3)
				ptr->location = corner_node4.llink1;
			else
				ptr->location = corner_node4.llink2;
		}
		else if (ptr->location == &middle_node) {										//��� ��� �鵵
			if (ptr->visitcorner3)
				ptr->location = middle_node.luplink;
			else
				ptr->location = middle_node.ruplink;
		}
		else if (ptr->location != &corner_node1) {										//�Ϲ����� �鵵
			if (ptr->location == &corner_node2) {
				ptr->location = corner_node2.llink;
			}
			else if (ptr->location == &corner_node3) {									//Ȯ���� ��������� �ڳ�3���� �ڳ�2���� �鵵�ؼ� �̵���� �� ���� �����ϱ�
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
		synchronize(ptr);//carry_ally�ϱ����� �� �� �� �ؾ� 2��¥���� 1�� ���������� �ڵ������ �ϳ��� �����Ǵ°��� ���� �� ����.
		carry_ally(ptr);
		synchronize(ptr);

		//���� ǥ���ϱ�
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
	if (player_order == 2) {		//���� ���� �÷��̾ �������.
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
	printf("          <������ ������>\n----------------------------------\n");
	//ù�� �μ�
	printf("     %s", corner_node3.s);
	ptr = corner_node3.llink;
	for (i = 1; i <= 4; i++) {
		printf("-%s", ptr->s);
		if (i <= 3)
			ptr = ptr->llink;
	}
	printf("-%s\n", corner_node2.s);
	//��°�� �μ�
	printf("\n     %s", corner_node3.rlink1->s);
	printf("  %s   ", corner_node3.rlink2->s);
	printf("    %s  ", corner_node2.rlink2->s);
	printf("%s\n", corner_node2.llink->s);
	//��°�� �μ�
	ptr = corner_node3.rlink1->rlink;
	printf("\n     %s", ptr->s);
	printf("    %s  ", middle_node.luplink->s);
	printf(" %s    ", middle_node.ruplink->s);
	ptr = corner_node2.llink->llink;
	printf("%s\n", ptr->s);
	//��°�� �μ�
	printf("\n               %s          \n", middle_node.s);
	//�ټ�°�� �μ�
	ptr = corner_node4.llink1->llink;
	printf("\n     %s", ptr->s);
	printf("    %s  ", middle_node.ldownlink->s);
	printf(" %s    ", middle_node.rdownlink->s);
	ptr = corner_node1.rlink->rlink;
	printf("%s\n", ptr->s);
	//����°�� �μ�
	printf("\n     %s", corner_node4.llink1->s);
	printf("  %s     ", corner_node4.llink2->s);
	printf("  %s  ", corner_node1.llink2->s);
	printf("%s\n", corner_node1.rlink->s);
	//�������� �μ�
	printf("\n     %s", corner_node4.s);
	ptr = corner_node4.rlink;
	for (i = 1; i <= 4; i++) {
		printf("-%s", ptr->s);
		if (i <= 3)
			ptr = ptr->rlink;
	}
	printf("-%s\t<-��������� ������\n----------------------------------\n\n", corner_node1.s);
};