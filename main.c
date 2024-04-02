#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define BUFF 4096
#define STRBUFF 1024

union union_value {
	char *cval;
	char **aval;
	struct tree_list *yval;
};

enum uniontype {string, tree, array};

struct 	tree_val {
	enum uniontype union_type;
	union union_value *value;
};

struct ytree {
	char *key;
	struct tree_val *object;
	struct ytree *next;
	struct ytree *prev;
	struct ytree *right;
	struct ytree *left;
};

struct tree_list {
	struct ytree *head;
	struct ytree *tail;
	void (*append)(struct tree_list *self, struct ytree *object);
};

struct tree_iter {
	struct ytree *current;
	struct ytree *previous;
	struct ytree* (*next)(struct tree_iter* self);
	void (*prev)(struct tree_iter* self);
};

struct ytree* tree_iterNext (struct tree_iter *self) {
	struct ytree *retval;
	retval = self->current;
	if (retval != NULL) {
	       	self->current = retval->next;
		self->previous = retval;
	}
	return retval;
}

void tree_join(struct ytree *cur, struct ytree *node) {
    if (cur->key[0] < node->key[0]) {
        if (!cur->right)
            cur->right = node;
        else
            tree_join(cur->right, node);
    }
    else {
        if (!cur->left)
            cur->left = node;
        else
            tree_join(cur->left, node);
    }
}

void tree_list_add( struct tree_list *self, struct ytree *node) {
	char *cmp;
	node->next = NULL;
	if (self->head == NULL){
		self->head = node;
	}
	else {
		self->tail->next = node;
        tree_join(self->head, node);
		cmp = self->tail->key;
	/*	if (cmp[0] > node->key[0]) 
			self->tail->right = node;
		else
			self->tail->left = node; */
		node->prev = self->tail;
	}
	self->tail = node;
}

void tree_iterPrev (struct tree_iter *self) {
	struct ytree *retval;
	if (self->previous != NULL) {
		self->current = self->previous;
		self->previous = self->current->prev;
	}
}

struct tree_list *new_tree_list() {
	struct tree_list *p = malloc(sizeof(*p));
	p->head = NULL;
	p->tail = NULL;
	p->append = &tree_list_add;
	return p;
};

struct tree_iter* new_tree_iter(struct tree_list *self) {
	struct tree_iter *it = malloc(sizeof(*it));
	it->current = self->head;
	it->previous = NULL;
	it->next = &tree_iterNext;
	it->prev = &tree_iterPrev;
	return it;
}

struct ytree *new_tree() {
	struct ytree *t = malloc(sizeof(*t));
	t->object = malloc(sizeof(t->object));
	t->object->value = malloc(sizeof(t->object->value));
	return t;
}

struct ytree* search_tree(struct ytree *cur, char *query) {
//    printf("inside search tree\n");
    if (strcmp(cur->key, query) == 0) {
        return cur;
    }
//    printf("no match\n");
    if (cur->key[0] < query[0]) {
  //      printf("test right node\n");
        if (cur->right)
            cur = search_tree(cur->right, query);
        else
            cur = NULL;
        return cur;
    }
    else {
 //       printf("test left node\n");
        if (cur->left)
            cur = search_tree(cur->left, query);
        else
            cur = NULL;
        return cur;
    }
}

void print_tree(struct tree_list *ylist, int stat_ind, int cur_ind) {
	struct tree_iter *yiter;
	struct ytree *cur;
	int i,j;
	yiter = new_tree_iter(ylist);
	while(1) {
		cur = yiter->next(yiter);
	  	if (cur == NULL) break;
		if (cur->object->union_type == 0) {
			if (cur_ind > 0) for (i=0; i < cur_ind; i++) printf(" ");
			printf("%s : %s\n", cur->key, cur->object->value->cval);
		}
		else if (cur->object->union_type == 1) {
			if (cur_ind > 0) for (i=0; i < cur_ind; i++) printf(" ");
			printf("%s :\n", cur->key);
			print_tree(cur->object->value->yval, stat_ind, cur_ind + stat_ind);
		}
		else if (cur->object->union_type == 2){
			if (cur_ind > 0) for (i=0; i < cur_ind; i++) printf(" ");
			printf("%s : | \n", cur->key);
            for(i=0;cur->object->value->aval[i] !=NULL; i++) {
				for (j=0; j < cur_ind + stat_ind; j++) printf(" ");
				printf("- %s\n",cur->object->value->aval[i]);
			}
		}
	}
}

void analyze_tree(struct tree_list *ylist, char *query) {
	int i,j;
    char qval[STRBUFF];
    struct ytree *cur = ylist->head;
//    printf("before while loop\n");
	while(1) {
        if (query[0] == '.') { 
            query++;
            for (i=0;query[0] != '.' && query[0] != '\n' && query[0] != '\0'; query++, i++){
 //               printf("cur char is %c\n", query[0]);
                    if (!(query[0] >= 'A') && !(query [0] <= 'Z') && !(query[0] >= '-') && !(query [0] <= '9') && !(query[0] >= 'a') && !(query [0] <= 'z') && query[0] != '_' || query[0] == '/')
                        return 1;
                    qval[i] = query[0];
            }
        }
        qval[i] = '\0';
        if (qval[0] == '\0' || qval[0] == '\n'){
                print_tree(ylist, 4, 0);
                break;
        }
        else {
           cur = search_tree(cur, qval); 
        }
	  	if (cur == NULL) {
            printf("query doesn't exist\n");
            break;
        }

        for (;query[0] == ' ' || query[0] == '\t'; query++) {}

        if (query[0] == '\0' || query[0] == '\n') {
            if (cur->object->union_type == 0) {
                printf("%s : %s\n", cur->key, cur->object->value->cval);
                break;
            }
            else if (cur->object->union_type == 1) {
                printf("%s :\n", cur->key);
                print_tree(cur->object->value->yval, 4, 4);
                break;
            }
            else if (cur->object->union_type == 2){
                printf("%s : | \n", cur->key);
                int ind = 4;
                for(i=0;cur->object->value->aval[i] !=NULL; i++) {
    				for (j=0; j < ind; j++) printf(" ");
                        printf("- %s\n",cur->object->value->aval[i]);
                }
                break;
            }
        }
        if (query[0] == '.') {
            if (cur->object->union_type == 1)
                analyze_tree(cur->object->value->yval, query);
            else {
                printf("Value is not an embedded dict");
            }
            break;
        }

	}
}

int gen_tree(FILE *fptr, struct tree_list *ylist, int ind, bool embedded, fpos_t *pos) {
	int cur_ind, i, new_ind = 0, j = 0;
	bool emptyArr = true, inArr = false;
	char *file = malloc(BUFF), key[STRBUFF], value[STRBUFF], *exchange, *arr[BUFF], word[BUFF];
	struct ytree *object;
	int catch = 0;

//	printf("new level of the tree base ind: %d\n", ind);
	while (fgets(file, BUFF, fptr) != NULL) {
		cur_ind = 0;
		for (;file[0] == ' ' || file[0] == '\t'; file++) 
			cur_ind++;
		if (file[0] == '#') for (;file[0] != '\n'; file++){}
		if (file[0] == '\n') continue;
		if (inArr == true) {
			if (cur_ind < new_ind) {
			       if (file[0] == '-' || emptyArr == true) {
				printf("Incorrect YAML format top\n");
				return 1;
			       }
				object->object->union_type = array;
				object->object->value->aval = arr;
				ylist->append(ylist, object);
			       inArr = false;
			       j = 0;
//			       for(i=0;object->object->value->aval[i] !=NULL; i++) {
//				       printf("%s\n",object->object->value->aval[i]);
//			       }
			}
			else if (file[0] == '-') {
				file++;
				for (;file[0] == ' ' || file[0] == '\t'; file++) {}
				if (file[0] == '#') for (;file[0] != '\n'; file++){}
				if (emptyArr == true) {
					new_ind = cur_ind;
					emptyArr = false;
//					printf("array not empty\n");
				}
				for (i=0;file[0] != ' ' && file[0] != '\t' && file[0] != '\n' && file[0] != '#'; file++, i++)
					value[i] = file[0];
				value[i] = '\0';
//				printf("value %s\n", value);
				exchange = malloc(sizeof(value));
				strcpy(exchange,value);
				arr[j] = exchange;
				//object->object->value->aval[j] = exchange;
//				printf("value stored : %s\n", arr[j]);
				j++;

				for (;file[0] == ' ' || file[0] == '\t'; file++) {}
				if (file[0] != '\n') {
					printf("this is not correct YAML\n");
					return 1;
				}
				       	fgetpos(fptr,pos);
				continue;
			}
			else {
				printf("Incorrect YAML format\n");
				return 1;
			}

		}
		new_ind = ind;
		object = new_tree();
		if (cur_ind > ind && embedded == false) {
			printf("Incorrect formating\n");
			break;
		}
		if (cur_ind <= ind && embedded == true) {
//			printf("exit new tree\n");
			break;
		}
		if (embedded == true) fgetpos(fptr,pos);
		for (i=0;file[0] != ' ' && file[0] != '\t' && file[0] != ':' && file[0] != '\n' && file[0] != '#' && file[0] != '\'' && file[0] != '"'; file++, i++){
			key[i] = file[0];
		}
		if (file[0] == '\'' || file[0] == '"') {
			printf("this is not correct YAML\n");
			return 1;
		}
		key[i] = '\0';
		exchange = malloc(sizeof(key));
		strcpy(exchange,key);
		object->key = exchange;
		for (;file[0] == ' ' || file[0] == '\t'; file++){} 
		if (file[0] == '#') for (;file[0] != '\n'; file++){}

		if (file[0] != ':') {
			printf("this is not correct YAML\n");
			return 1;
		}
		else{
			file++;
		}
		
		for (;file[0] == ' ' || file[0] == '\t'; file++) {}
		if (file[0] == '#') for (;file[0] != '\n'; file++){}
		
		if (file[0] == '\n') {
//			printf("new tree, ind = %d\n", cur_ind);
			struct tree_list *new_list = new_tree_list();
			bool embed = true;
			catch = gen_tree(fptr, new_list, cur_ind, embed, pos);
			if (catch > 0) return catch;
			object->object->union_type = tree;
			object->object->value->yval= new_list;
			ylist->append(ylist, object);
			continue;
		}
		if (file[0] == '|') {
			file++;
			for (;file[0] == ' ' || file[0] == '\t'; file++) {}
			if (file[0] == '#') for (;file[0] != '\n'; file++){}
			if (file[0] != '\n') {
				printf("Incorrect YAML format\n");
				return 1;
			}
//			printf("array\n");
			emptyArr = true;
			inArr = true;
			*arr = malloc(sizeof(*arr));
			continue;
		}

		if (file[0] == '"' || file[0] == '\'') {
			char cmp = file[0];
			file++;
			for (i=0;file[0] != cmp; file++, i++) 
				value[i] = file[0];
			file++;
		}
		else {
			for (i=0;file[0] != '"' && file[0] != '\'' && file[0] != '\n' && file[0] != '#' && file[0] != '|'; file++, i++) 
				value[i] = file[0];
		}
		value[i] = '\0';
		exchange = malloc(sizeof(value));
		strcpy(exchange,value);
		object->object->union_type = string;
		object->object->value->cval= exchange;
//		printf("key: %s, value: %s\n", object->key, value);	

		for (;file[0] == ' ' || file[0] == '\t'; file++) {}
		if (file[0] == '#') for (;file[0] != '\n'; file++){}
		if (file[0] != '\n') {
			printf("this is not correct YAML\n");
			return 1;
		}
		ylist->append(ylist, object);
		
	}
//	printf("exit loop\n");
	if (embedded == true) fsetpos(fptr, pos);
}

int main(int argc, char **argv){
	FILE *fptr;
	char line[BUFF], *flag, *list[BUFF];
	struct tree_list *ylist = new_tree_list();
	struct tree_iter yiter;
	struct tree_iter *file_iter;
	bool embedded = false;
	fpos_t  pos;
	static int stat_ind = 4;
	int catch = 0, j=0;
    argc--;
    argv++;

    for (int i=0;i<argc;i++) {
           flag = argv[i]; 
           if (flag[0] == '-'){
               printf("flag\n"); 
           }
           else {
                list[j] = flag;
                j++;
           }
    }
    if (j > 2) {
        printf("exited\n");
            return 1;
    }
    char query[STRBUFF];
    strncpy(query, list[0], STRBUFF);

	fptr = fopen(list[1], "r");

	if (fptr == NULL){
		fptr = tmpfile();
        if (!isatty(fileno(stdin)))
            while(fgets(line,BUFF, stdin)) {
                fputs(line, fptr);
            }	
            rewind(fptr);
            if(NULL == fgets(line, BUFF, fptr))  {
                printf("please select a file\n");
                return 1;
            }
            rewind(fptr);
        }
//	printf("before function\n");
	catch = gen_tree(fptr, ylist, 0, embedded, &pos);
	if (catch > 0) return catch;
//    printf("query is : %s\n", query);
    analyze_tree(ylist, query);
//	print_tree(ylist, stat_ind,  0);
	free(fptr);
}
