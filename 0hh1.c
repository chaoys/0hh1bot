#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>

#define	NIL	0
#define	RED	1
#define	BLUE	2
#define NCOLOR	3

struct node {
	int row;
	int col;
};

struct state {
	TAILQ_ENTRY(state) next;
	char *matrix;
	struct node node;
};

TAILQ_HEAD(game, state) game;
int level;

static int matrix_size(void)
{
	return (level * level * sizeof(char));
}
static char matrix_get(struct state *s, int row, int col)
{
	return s->matrix[row*level+col];
}
static void matrix_set(struct state *s, int row, int col, char color)
{
	s->matrix[row*level+col] = color;
}
static struct state *new_state(struct state *s)
{
	struct state *snew = (struct state *)malloc(sizeof(*snew));
	assert(snew != NULL);
	memset(snew, 0, sizeof(*snew));
	snew->matrix = (char *)malloc(matrix_size());
	if (s) {
		memcpy(snew->matrix, s->matrix, matrix_size());
	} else {
		memset(snew->matrix, 0, matrix_size());
	}
	return snew;
}
// create new state and set color
static struct state *new_state_color(struct state *s, char c)
{
	struct state *ns = new_state(s);
	matrix_set(ns, s->node.row, s->node.col, c);
	return ns;
}
static void free_state(struct state *s)
{
	free(s->matrix);
	free(s);
}
static void dump_state(struct state *s, const char *msg)
{
	int i, j;
	char c;
	printf("%s\n", msg);
	for (i = 0; i < level; i++) {
		for (j = 0; j < level; j++) {
			c = matrix_get(s, i, j);
			if (c == NIL) {
				printf(". ");
			} else if (c == RED) {
				printf("R ");
			} else {
				printf("B ");
			}
		}
		putchar('\n');
	}
	putchar('\n');
}
static void dump_state_raw(struct state *s)
{
	int i, j;
	char c;
	for (i = 0; i < level; i++) {
		for (j = 0; j < level; j++) {
			c = matrix_get(s, i, j);
			printf("%d", c);
		}
	}
	putchar('\n');
}
static int get_first_nil_node(struct state *s)
{
	int i, j;
	for (i = 0; i < level; i++) {
		for (j = 0; j < level; j++) {
			if (matrix_get(s, i, j) == NIL) {
				s->node.row = i;
				s->node.col = j;
				return 1;
			}
		}
	}
	return 0;
}
// ((row,col,color)(row,col,color)...)
static struct state *load_initial_state(char *arg)
{
	struct state *s;
	char *str;
	int len, n;
	int row, col;
	char color;

	s = new_state(NULL);
	len = strlen(arg);
	assert(arg[0] == '(');
	assert(arg[len-1] == ')');
	arg[len-1] = 0;
	str = arg + 1;
	while ((*str != 0)) {
		n = sscanf(str, "(%d,%d,%hhd)", &row, &col, &color);
		if (n != 3) {
			printf("bad initial state\n");
			free_state(s);
			exit(1);
		}
		assert((row >= 0) && (col >= 0) && (color >= 0));
		assert((row < level) && (col < level) && (color < NCOLOR));
		matrix_set(s, row, col, color);
		str = strchr(str, ')');
		str++;
	}
	return s;
}
static int validate_row3(struct state *s)
{
	int i, j;
	char c1, c2, c3;
	for (i = 0; i < level; i++) {
		for (j = 0; j < level - 2; j++) {
			c1 = matrix_get(s, i, j);
			c2 = matrix_get(s, i, j+1);
			c3 = matrix_get(s, i, j+2);
			if ((c1 == c2) && (c2 == c3) && (c3 != NIL)) {
				return 1;
			}
		}
	}
	return 0;
}
static int validate_col3(struct state *s)
{
	int i, j;
	char c1, c2, c3;
	for (i = 0; i < level; i++) {
		for (j = 0; j < level - 2; j++) {
			c1 = matrix_get(s, j, i);
			c2 = matrix_get(s, j+1, i);
			c3 = matrix_get(s, j+2, i);
			if ((c1 == c2) && (c2 == c3) && (c3 != NIL)) {
				return 1;
			}
		}
	}
	return 0;
}
static int validate_row_color_eq(struct state *s)
{
	int nb, nr;
	int i, j;
	char c;
	for (i = 0; i < level; i++) {
		nb = nr = 0;
		for (j = 0; j < level; j++) {
			c = matrix_get(s, i, j);
			if (c == BLUE) {
				nb++;
			} else if (c == RED) {
				nr++;
			}
		}
		if ((nb + nr == level) && (nb != nr)) {
			return 1;
		}
	}
	return 0;
}
static int validate_col_color_eq(struct state *s)
{
	int nb, nr;
	int i, j;
	char c;
	for (i = 0; i < level; i++) {
		nb = nr = 0;
		for (j = 0; j < level; j++) {
			c = matrix_get(s, j, i);
			if (c == BLUE) {
				nb++;
			} else if (c == RED) {
				nr++;
			}
		}
		if ((nb + nr == level) && (nb != nr)) {
			return 1;
		}
	}
	return 0;

}
static int validate_row_eq(struct state *s)
{
	int i, j, k;
	for (i = 0; i < level - 1; i++) {
		for (j = 0; j < level; j++) {
			// ignore NIL
			if (matrix_get(s, i, j) == NIL) {
				goto next;
			}
		}
		for (k = i + 1; k < level; k++) {
			for (j = 0; j < level; j++) {
				// ignore NIL
				if (matrix_get(s, i, j) == NIL) {
					goto nexti;
				}
			}
			if (!memcmp(s->matrix+i*level, s->matrix+k*level, level)) {
//				printf("validate row eq failed\n");
				return 1;
			}
nexti:
			j = 0;
		}
next:
		j = 0;
	}
	return 0;
}
static int validate_col_eq(struct state *s)
{
	int i, j, k;
	int rc = 0;
	char *col1, *col2;
	col1 = (char *)malloc(sizeof(char)*level);
	assert(col1);
	col2 = (char *)malloc(sizeof(char)*level);
	assert(col2);
	for (i = 0; i < level - 1; i++) { // col
		for (j = 0; j < level; j++) { // row
			col1[j] = matrix_get(s, j, i);
			if (col1[j] == NIL) {
				goto next;
			}
		}
		for (k = i+1; k < level; k++) {
			for (j = 0; j < level; j++) { // row
				col2[j] = matrix_get(s, j, k);
				if (col2[j] == NIL) {
					goto nexti;
				}
			}
			if (!memcmp(col1, col2, level)) {
				rc = 1;
//				printf("validate col eq failed\n");
				goto out;
			}
nexti:
			rc = 0;
		}
next:
		rc = 0;
	}
out:
	free(col1);
	free(col2);
	return rc;
}
static int validate_matrix(struct state *s)
{
	int rc = 0;
	rc |= validate_row3(s);
	rc |= validate_col3(s);
	rc |= validate_row_color_eq(s);
	rc |= validate_col_color_eq(s);
	rc |= validate_row_eq(s);
	rc |= validate_col_eq(s);
	return rc;
}
// all the complicate things
static int row3(struct state *s)
{
	int i, j;
	char c1, c2, c3;
	int rc = 0;
	for (i = 0; i < level; i++) {
		for (j = 0; j < level; j++) {
			c1 = matrix_get(s, i, j);
			if (c1 != NIL) {
				continue;
			}
			if (j < 2) {
				// look forward
				c2 = matrix_get(s, i, j+1);
				c3 = matrix_get(s, i, j+2);
				if ((c2 == c3) && (c2 != NIL)) {
//					printf("row3 f i %d j %d %hhd %hhd\n", i, j, c2, c3);
					matrix_set(s, i, j, NCOLOR-c2);
					rc = 1;
				}
			} else if (j > level-3) {
				// look backward
				c2 = matrix_get(s, i, j-1);
				c3 = matrix_get(s, i, j-2);
				if ((c2 == c3) && (c2 != NIL)) {
//					printf("row3 b i %d j %d %hhd %hhd\n", i, j, c2, c3);
					matrix_set(s, i, j, NCOLOR-c2);
					rc = 1;
				}
			} else {
				// both
				c2 = matrix_get(s, i, j+1);
				c3 = matrix_get(s, i, j+2);
				if ((c2 == c3) && (c2 != NIL)) {
					matrix_set(s, i, j, NCOLOR-c2);
					rc = 1;
				} else {
					c2 = matrix_get(s, i, j-1);
					c3 = matrix_get(s, i, j-2);
					if ((c2 == c3) && (c2 != NIL)) {
						matrix_set(s, i, j, NCOLOR-c2);
						rc = 1;
					}
				}
			}
		}
	}
	return rc;
}
static int col3(struct state *s)
{
	int i, j;
	char c1, c2, c3;
	int rc = 0;
	for (i = 0; i < level; i++) { // col
		for (j = 0; j < level; j++) { // row
			c1 = matrix_get(s, j, i);
			if (c1 != NIL) {
				continue;
			}
			if (j < 2) {
				// look forward
				c2 = matrix_get(s, j+1, i);
				c3 = matrix_get(s, j+2, i);
				if ((c2 == c3) && (c2 != NIL)) {
					matrix_set(s, j, i, NCOLOR-c2);
					rc = 1;
				}
			} else if (j > level-3) {
				// look backward
				c2 = matrix_get(s, j-1, i);
				c3 = matrix_get(s, j-2, i);
				if ((c2 == c3) && (c2 != NIL)) {
					matrix_set(s, j, i, NCOLOR-c2);
					rc = 1;
				}
			} else {
				// both
				c2 = matrix_get(s, j+1, i);
				c3 = matrix_get(s, j+2, i);
				if ((c2 == c3) && (c2 != NIL)) {
					matrix_set(s, j, i, NCOLOR-c2);
					rc = 1;
				} else {
					c2 = matrix_get(s, j-1, i);
					c3 = matrix_get(s, j-2, i);
					if ((c2 == c3) && (c2 != NIL)) {
						matrix_set(s, j, i, NCOLOR-c2);
						rc = 1;
					}
				}
			}
		}
	}
	return rc;
}
static int row_color_eq(struct state *s)
{
	int nb, nr;
	int rc = 0;
	int i, j;
	char c;
	for (i = 0; i < level; i++) {
		nb = nr = 0;
		for (j = 0; j < level; j++) {
			c = matrix_get(s, i, j);
			if (c == BLUE) {
				nb++;
			} else if (c == RED) {
				nr++;
			}
		}
		if (nb + nr == level) {
			continue;
		}
		if (nb == (level/2)) {
			c = RED;	
		} else if (nr == (level/2)) {
			c = BLUE;
		} else {
			continue;
		}
		for (j = 0; j < level; j++) {
			if (matrix_get(s, i, j) == NIL) {
				matrix_set(s, i, j, c);
				rc = 1;
			}
		}
	}
	return rc;
}
static int col_color_eq(struct state *s)
{
	int nb, nr;
	int rc = 0;
	int i, j;
	char c;
	for (i = 0; i < level; i++) { // col
		nb = nr = 0;
		for (j = 0; j < level; j++) { // row
			c = matrix_get(s, j, i);
			if (c == BLUE) {
				nb++;
			} else if (c == RED) {
				nr++;
			}
		}
		if (nb + nr == level) {
			continue;
		}
		if (nb == (level/2)) {
			c = RED;	
		} else if (nr == (level/2)) {
			c = BLUE;
		} else {
			continue;
		}
		for (j = 0; j < level; j++) { // row
			if (matrix_get(s, j, i) == NIL) {
				matrix_set(s, j, i, c);
				rc = 1;
			}
		}
	}
	return rc;

}
static struct state *compute_determined_state(struct state *s)
{
	struct state *ns;
	int rc;

	ns = new_state(s);

	do {
		rc = 0;
		rc |= row3(ns);
		rc |= col3(ns);
		rc |= row_color_eq(ns);
		rc |= col_color_eq(ns);
		/*
		if (row3(ns)) {
			printf("row3\n");
			rc = 1;
		}
		if (col3(ns)) {
			printf("col3\n");
			rc = 1;
		}
		if (row_color_eq(ns)) {
			printf("row eq\n");
			rc = 1;
		}
		if (col_color_eq(ns)) {
			printf("col eq\n");
			rc = 1;
		}
		dump_state(ns, "loop");
		*/
	} while (rc);

	return ns;
}
static int guess(struct state *s)
{
	struct state *ns, *ns_tmp;
	if (!get_first_nil_node(s)) {
		// done
		return 0;
	}
//	printf("guess row %d col %d\n", s->node.row, s->node.col);
	ns_tmp = new_state_color(s, RED);
	ns = compute_determined_state(ns_tmp);
	free_state(ns_tmp);
//	dump_state(ns, "before validate");
	if (validate_matrix(ns)) {
//		printf("next\n");
		goto next;
	}
	TAILQ_INSERT_TAIL(&game, ns, next);
//	dump_state(ns, "before guess");
	if (guess(ns) == 0) {
		return 0;
	}
	// backtrace
	TAILQ_REMOVE(&game, ns, next);
next:
	free_state(ns);
//	printf("guess2 row %d col %d\n", s->node.row, s->node.col);
	ns_tmp = new_state_color(s, BLUE);
	ns = compute_determined_state(ns_tmp);
	free_state(ns_tmp);
	if (validate_matrix(ns)) {
//		printf("fail\n");
		goto fail;
	}
	TAILQ_INSERT_TAIL(&game, ns, next);
//	dump_state(ns, "before guess2");
	if (guess(ns) == 0) {
		return 0;
	}
	// backtrace
	TAILQ_REMOVE(&game, ns, next);
fail:
	free_state(ns);
	return 1;
}
static void free_game(void)
{
}
int main(int argc, char **argv)
{
	struct state *state1, *state2, *last;

	assert(argc == 3);

	TAILQ_INIT(&game);

	level = atoi(argv[1]);
	assert((level > 0) && (level % 2 == 0));

	state1 = load_initial_state(argv[2]);
//	dump_state(state1, "initial state");
	TAILQ_INSERT_TAIL(&game, state1, next);

	state2 = compute_determined_state(state1);
//	dump_state(state2, "determined state");
	TAILQ_INSERT_TAIL(&game, state2, next);

	// let's guess, depth first
	assert(0 == guess(state2));
	
	last = TAILQ_LAST(&game, game);
	dump_state_raw(last);
//	dump_state(last, "final");

	free_game();

	return 0;
}
