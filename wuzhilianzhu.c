/*
 *  perfect!
 */

#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <curses.h>

struct path_node {
	int row, col;
	int path;
	struct  path_node *next;
};

struct map_node {
	int y, x;
	char value;
	int path;
	int flag;
};

struct point {
	int row, col;
};

#define ROW 		9
#define COL 		9
#define MAX_PATH 	ROW*COL
#define TICKER 150
#define CHAR_TYPE_NUM 5

#define VISITED 	1
#define UNVISITED 	0

#define CHAR_1		'1'
#define CHAR_2 		'2'
#define CHAR_3 		'3'
#define CHAR_4 		'4'
#define CHAR_5 		'5'
#define CHAR_NONE 	' '
#define CHAR_LINE_1 	"-------------------"
#define CHAR_LINE_2 	"| | | | | | | | | |"

#define bad_row(r) 	((r) < 0 || (r) > 8)
#define bad_col(c) 	((c) < 0 || (c) > 8)
#define arrive_end(r, c) ( ((r) == end_row) && ((c) == end_col) )
#define arrive_begin(r, c) ( ((r) == begin_row) && \
							((c) == begin_col) )

struct map_node map[ROW][COL];
char begin_char;
int no_path, blank_num;
int begin_row, begin_col, end_row, end_col, cursor_row, cursor_col;
struct path_node *head, *tail;
struct point blank_array[MAX_PATH];
char char_array[CHAR_TYPE_NUM] = {CHAR_1, CHAR_2, CHAR_3, CHAR_4, 
	CHAR_5};
struct point weiyi_around[4] = {{-1,0}, {0,1}, {1,0}, {0,-1}};
struct point weiyi_five[4][9] = {
	{{0, -4}, {0, -3}, {0, -2}, {0, -1}, {0, 0}, 
		{0, 1}, {0, 2}, {0, 3}, {0, 4}},
	{{-4, -4}, {-3, -3}, {-2, -2}, {-1, -1}, {0, 0}, 
		{1, 1}, {2, 2}, {3, 3}, {4, 4}},
	{{-4, 0}, {-3, 0}, {-2, 0}, {-1, 0}, {0, 0}, 
		{1, 0}, {2, 0}, {3, 0}, {4, 0}},
	{{-4, 4}, {-3, 3}, {-2, 2}, {-1, 1}, {0, 0}, 
		{1, -1}, {2, -2}, {3, -3}, {4, -4}}
};

void init();
int set_ticker(int n_msec);
void sig_timer(int sig);
void play();
void show_path();
void find_path();
void reset_map_path();
void destroy_list();
void cal_path();
void init_map();
void init_screen();
void game_over();
void generate_new();
void move_up();
void move_down();
void move_left();
void move_right();
void clear_cursor();
void draw_cursor();
void init_blank_array();
int remove_five(int row, int col);
int set_ticker(int n_msec);

int main()
{
	init();
	while(1)
		play();
	return 0;
}

void play()
{
	char ch;
	int first = 1;

	while(1) {
		ch = getch();
		switch (ch) {
			case 'w':
				move_up();
				break;
			case 's':
				move_down();
				break;
			case 'a':
				move_left();
				break;
			case 'd':
				move_right();
				break;
			case ' ':
				if (first) {
					first = 0;
					begin_row = cursor_row;
					begin_col = cursor_col;
					begin_char = map[begin_row][begin_col].value;
					draw_cursor();
				}
				else {
					if (CHAR_NONE != 
							map[cursor_row][cursor_col].value)
						break;
					first = 1;
					end_row = cursor_row;
					end_col = cursor_col;
					find_path();

					cursor_row = begin_row;
					cursor_col = begin_col;
					clear_cursor();
					cursor_row = end_row;
					cursor_col = end_col;
					draw_cursor();
				}
				break;
			case 'q':
				game_over();
				break;
		}
	}
}

void move_up()
{
	clear_cursor();
	if (cursor_row > 0)
		cursor_row --;
	draw_cursor();
}

void move_down()
{
	clear_cursor();
	if (cursor_row < 8)
		cursor_row ++;
	draw_cursor();
}

void move_left()
{
	clear_cursor();
	if (cursor_col > 0)
		cursor_col --;
	draw_cursor();
}

void move_right()
{
	clear_cursor();
	if (cursor_col < 8)
		cursor_col ++;
	draw_cursor();
}

void draw_cursor()
{
	char c;

	move(map[cursor_row][cursor_col].y, 
			map[cursor_row][cursor_col].x);
	c = (char)inch();
	attron(A_STANDOUT);
	addch(c);
	refresh();
}

void clear_cursor()
{
	char c;

	move(map[cursor_row][cursor_col].y, 
			map[cursor_row][cursor_col].x);
	c = (char)inch();
	attroff(A_STANDOUT);
	addch(c);
	refresh();
}

void show_path()
{
	int i;

	cursor_row = begin_row;
	cursor_col = begin_col;
	set_ticker(200);
	while (!arrive_end(cursor_row, cursor_col));
	set_ticker(0);
	for (i = 0; i < blank_num; i ++) {
		if ((blank_array[i].row == end_row) && 
				(blank_array[i].col == end_col)) {
			blank_array[i].row = begin_row;
			blank_array[i].col = begin_col;
			break;
		}
	}
}

int remove_five(int row, int col)
{
	int i, j, k, r, c;

	for (i = 0; i < 4; i ++) {
		for (j = 0; j < 5; j ++) {
			for (k = j; k < j+5; k ++) {
				r = row + weiyi_five[i][k].row;
				c = col + weiyi_five[i][k].col;
				if (begin_char != map[r][c].value) 
					break;
			}
			if (k >= j+5) {
				for (k = j; k < j+5; k ++) {
					r = row + weiyi_five[i][k].row;
					c = col + weiyi_five[i][k].col;
					move(map[r][c].y, map[r][c].x);
					attroff(A_STANDOUT);
					addch(CHAR_NONE);
					refresh();
					map[r][c].value = CHAR_NONE;
				}
				return 1;
			}
		}
	}
	return 0;
}

void find_path()
{
	reset_map_path();
	no_path = 0;
	map[end_row][end_col].path = 0;
	map[end_row][end_col].flag = VISITED;
	head = (struct path_node *)malloc(sizeof(struct path_node));
	*head = (struct path_node){end_row, end_col, 0, NULL};
	tail = head;

	cal_path();
	if (!no_path) {
		show_path();
		map[begin_row][begin_col].value = CHAR_NONE;
		map[end_row][end_col].value = begin_char;
		if (!remove_five(end_row, end_col))
			generate_new();
	}
}

void destroy_list()
{
	struct path_node *p;

	while (head) {
		p = head;
		head = head->next;
		free(p);
	}
}

void cal_path()
{
	int i, r, c;
	struct path_node *p, *tmp;

	while (head) {
		p = head;
		for (i = 0; i < 4; i ++) {
			r = p->row + weiyi_around[i].row;
			c = p->col + weiyi_around[i].col;
			if ((bad_row(r)) || (bad_col(c)))
				continue;
			if (arrive_begin(r, c)) {
				no_path = 0;
				destroy_list();
				return;
			}
			if ((CHAR_NONE == map[r][c].value) && 
					(UNVISITED == map[r][c].flag)) {
				map[r][c].path = p->path + 1;
				map[r][c].flag = VISITED;

				tmp = (struct path_node *)malloc
					(sizeof(struct path_node));
				*tmp = (struct path_node){r, c, p->path+1, NULL};
				tail = tail->next = tmp;
			}
		}
		head = head->next;
		free(p);
	}
	no_path = 1;
	destroy_list();
}

void generate_new()
{
	int i, j, k, m, r, c;

	for (i = 0; i < 3; i ++) {
		k = rand() % blank_num;
		j = rand() % CHAR_TYPE_NUM;
		r = blank_array[k].row;
		c = blank_array[k].col;
		map[r][c].value = char_array[j];
		move(map[r][c].y, map[r][c].x);
		attroff(A_STANDOUT);
		addch(char_array[j]);
		refresh();
		remove_five(r, c);

		for (m = k+1; m < blank_num; m ++) {
			blank_array[m-1].row = blank_array[m].row;
			blank_array[m-1].col = blank_array[m].col;
		}
		blank_num --;
	}
	if (blank_num <= 1) {
		getch();
		game_over();
	}
}

void game_over()
{
	endwin();
	exit(0);
}

void sig_timer(int sig)
{
	int i, r, c;

	move(map[cursor_row][cursor_col].y, 
			map[cursor_row][cursor_col].x);
	addch(CHAR_NONE);
	refresh();
	clear_cursor();
	for (i = 0; i < 4; i ++) {
		r = cursor_row + weiyi_around[i].row;
		c = cursor_col + weiyi_around[i].col;
		if ((bad_row(r)) || (bad_col(c)))
			continue;
		if ((VISITED == map[r][c].flag) && 
				(map[r][c].path < map[cursor_row][cursor_col].path)) {
			cursor_row = r;
			cursor_col = c;
			break;
		}
	}
	move(map[cursor_row][cursor_col].y, 
			map[cursor_row][cursor_col].x);
	addch(begin_char);
	refresh();
	draw_cursor();
}

void init()
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	srand(time(0));
	
	clear();
	init_map();
	init_screen();
	init_blank_array();
	generate_new();
	cursor_row = cursor_col = 0;
	signal(SIGALRM, sig_timer);
}

void init_blank_array()
{
	int i, j;

	blank_num = 0;
	for (i = 0; i < ROW; i ++) {
		for (j = 0; j < COL; j ++) {
			blank_array[blank_num ++] = (struct point){i, j};
		}
	}
}

void reset_map_path()
{
	int i, j;

	for (i = 0; i < ROW; i ++) {
		for (j = 0; j < COL; j ++) {
			map[i][j].path = MAX_PATH;
			map[i][j].flag = UNVISITED;
		}
	}	
}

void init_map()
{
	int i, j;

	for (i = 0; i < ROW; i ++) {
		for (j = 0; j < COL; j ++) {
			map[i][j] = (struct map_node)
			{i*2+1, j*2+1, CHAR_NONE, MAX_PATH, UNVISITED};
		}
	}
}

void init_screen()
{
	int i, j;

	move(0, 0);
	printw(CHAR_LINE_1);
	for (i = 1; i < ROW*2; i ++) {
		move(i, 0);
		printw(CHAR_LINE_2);
		i ++;
		move(i, 0);
		printw(CHAR_LINE_1);
	}
	refresh();

	for (i = 0; i < ROW; i ++) {
		for (j = 0; j < COL; j ++) {
			if (CHAR_NONE != map[i][j].value) {
				move(map[i][j].y, map[i][j].x);
				addch(map[i][j].value);
			}
		}
	}
	refresh();
}

int set_ticker(int n_msec)
{
	struct itimerval timeset;
	long n_sec, n_usec;

	n_sec = n_msec / 1000;
	n_usec = (n_msec % 1000) * 1000L;

	timeset.it_interval.tv_sec = n_sec;
	timeset.it_interval.tv_usec = n_usec;

	timeset.it_value.tv_sec = n_sec;
	timeset.it_value.tv_usec = n_usec;

	return setitimer(ITIMER_REAL, &timeset, NULL);
}
