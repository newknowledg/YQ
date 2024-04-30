#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define BUFF 4096
#define STRBUFF 1024
bool printed = false;
int def_ind = 2, color = 0;

union union_value {
	char *cval;
	char **aval;
    struct tree_list **yaval;
	struct tree_list *yval;
};

enum uniontype {string, tree, array, y_array};

enum arrtype {Arr, yArr, NA};

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

void del_object (struct tree_val *object);

void del_tree_list (struct tree_list *self) {
    struct ytree *cur, *next;
    cur = self->head;
    while(cur) {
        next = cur->next;
        free(cur->key);
        del_object(cur->object);
        free(cur->object->value);
        free(cur->object);
        free(cur);
        cur = next;
	}
    free(self);
};

void del_object (struct tree_val *object) {
    if (object->union_type == 0) {
        free(object->value->cval);
    }
    else if (object->union_type == 1) {
        del_tree_list(object->value->yval);
    }
    else if (object->union_type == 2){
        for(int i=0;object->value->aval[i] !=NULL; i++) {
            free(object->value->aval[i]);
        }
    }
    else if (object->union_type == 3){
        for(int i=0;object->value->yaval[i] !=NULL; i++) {
        del_tree_list(object->value->yaval[i]);
        }
        free(object->value->yaval);
    }
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
    if (strcmp(cur->key, query) == 0) {
        return cur;
    }
    if (cur->key[0] < query[0]) {
        if (cur->right)
            cur = search_tree(cur->right, query);
        else
            cur = NULL;
        return cur;
    }
    else {
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
			printf("\e[1;%dm%s :\e[0m %s", color, cur->key, cur->object->value->cval);
            char *n_test = cur->object->value->cval;
            if (n_test[strlen(n_test) - 1] != '\n')
                printf("\n");                
		}
		else if (cur->object->union_type == 1) {
			if (cur_ind > 0) for (i=0; i < cur_ind; i++) printf(" ");
			printf("\e[1;%dm%s :\e[0m\n", color, cur->key);
			print_tree(cur->object->value->yval, stat_ind, cur_ind + stat_ind);
		}
		else if (cur->object->union_type == 2){
			if (cur_ind > 0) for (i=0; i < cur_ind; i++) printf(" ");
			printf("\e[1;%dm%s :\e[0m\n", color, cur->key);
            for(i=0;cur->object->value->aval[i] !=NULL; i++) {
				for (j=0; j < cur_ind + stat_ind; j++) printf(" ");
				printf("- %s",cur->object->value->aval[i]);
                char *n_test = cur->object->value->aval[i];
                if (n_test[strlen(n_test) - 1] != '\n')
                    printf("\n");                
			}
		}
        else if (cur->object->union_type == 3){
			if (cur_ind > 0) for (i=0; i < cur_ind; i++) printf(" ");
            printf("\e[1;%dm%s :\e[0m\n", color, cur->key);
            for(i=0;cur->object->value->yaval[i] !=NULL; i++) {
                for (int j=0; j < cur_ind + stat_ind; j++) printf(" ");
                printf("- \n");
                print_tree(cur->object->value->yaval[i], stat_ind, cur_ind + stat_ind + 2);
            }
        }
	}
    printed = true;
    free(yiter);
}

char* analyze_tree(struct tree_list *ylist, char *query, bool embedded) {
    char qval[STRBUFF], *a_end = '\0';
    struct tree_list *ya_end;
    struct ytree *cur = ylist->head;
    int a_start = 0, i = 0;
    bool y_ap;
	while(1) {
        i = 0, a_start = 0, a_end = '\0', ya_end = NULL, y_ap = false;
        if (query[0] == ')' && embedded) {
            query++;
            for (;query[0] == ' ' || query[0] == '\t'; query++) {}
            return query;
        }
        else if (query[0] == '(') {
            query++;
            query = analyze_tree(ylist, query, true);
            continue;
        }
        if (query[0] == '.') { 
            query++;
            for (i=0;query[0] != '.' && query[0] != '\n' && query[0] != '\0' && query[0] !='[' && query[0] != ' '; query++, i++){
                    if (!(query[0] >= 'A') && !(query [0] <= 'Z') && !(query[0] >= '-') && !(query [0] <= '9') && !(query[0] >= 'a') && !(query [0] <= 'z') && query[0] != '_' || query[0] == '/')
                        return NULL;
                    qval[i] = query[0];
            }
        }
        else if (query[0] != '\0') {
            printf("Query is malformed\n");
            return NULL;
        }
        qval[i] = '\0';
        if (qval[0] == '\0' || qval[0] == '\n'){
                print_tree(ylist, def_ind, 0);
                printed = true;
                return NULL;
        }
        else {
           cur = search_tree(cur, qval); 
        }
	  	if (cur == NULL) {
            printf("null\n");
            return NULL;
        }
        if (query[0] == '[') {
            if (cur->object->union_type == 2 || cur->object->union_type == 3) {
                bool neg = false;
                query++;
                if (query[0] == '-' ) {
                    neg = true;
                    query++;
                }
                if (query[0] == '\0' || query[0] == '\n'){
                    printf("Query is malformed\n");
                    return NULL;
                }
                if (query[0] >='0' && query[0] <= '9') {
                    char numstr[STRBUFF];
                    int numval, a_length = 0;
                    for (i=0;query[0] >='0' && query[0] <= '9';query++,i++)
                        numstr[i] = query[0];
                    numval = atoi(numstr);
                    if (query[0] == '\0' || query[0] == '\n'){
                        printf("Query is malformed\n");
                        break;
                    }
                    if (query[0] == ']') {
                        if (cur->object->union_type == 2) {
                            for(int j=0;cur->object->value->aval[j] !=NULL; j++) 
                                a_length++;
                            if (numval < a_length || neg && numval <= a_length){
                                if (neg){
                                   if (numval > 0) 
                                       numval = a_length  - numval;
                                    else {
                                        printf("Element is out of range\n");
                                        return NULL; 
                                    }
                                }
                                a_start = numval;
                                if (cur->object->value->aval[numval+1])
                                    a_end = cur->object->value->aval[numval+1];
                            }
                            else {
                                printf("Element is out of range\n");
                                return NULL;
                            }
                            query++;
                        }
                        if (cur->object->union_type == 3) {
                            for(int j=0;cur->object->value->yaval[j] !=NULL; j++) 
                                a_length++;
                            if (numval < a_length || neg && numval <= a_length){
                                if (neg){
                                   if (numval > 0) 
                                       numval = a_length  - numval;
                                    else {
                                        printf("Element is out of range\n");
                                        return NULL; 
                                    }
                                }
                                a_start = numval;
                                if (cur->object->value->yaval[numval+1])
                                    ya_end = cur->object->value->yaval[numval+1];
                                y_ap = true;
                            }
                            else {
                                printf("Element is out of range\n");
                                return NULL;
                            }
                            query++;
                        }
                    }
                    else{
                        printf("Query is malformed\n");
                        return NULL;
                    }
                }
                else if (query[0] == ']') {
                    query++;
                }
                else {
                    printf("Query is malformed\n");
                    return NULL;
                }
            }
            else {
                printf("Value is not of type array\n");
                return NULL;
            }
        }

        for (;query[0] == ' ' || query[0] == '\t'; query++) {}

        if (query[0] == '\0' || query[0] == '\n') {
            if (cur->object->union_type == 0) {
                printf("\e[%dm%s :\e[0m %s", color, cur->key, cur->object->value->cval);
                char *n_test = cur->object->value->cval;
                if (n_test[strlen(n_test) - 1] != '\n')
                    printf("\n");                
                printed = true;
                break;
            }
            else if (cur->object->union_type == 1) {
                printf("\e[1;%dm%s :\e[0m\n", color, cur->key);
                print_tree(cur->object->value->yval, def_ind, def_ind);
                printed = true;
                break;
            }
            else if (cur->object->union_type == 2){
                printf("\e[1;%dm%s :\e[0m\n", color, cur->key);
                for(i=a_start;cur->object->value->aval[i] != a_end; i++) {
    				for (int j=0; j < def_ind; j++) printf(" ");
                    printf("- %s",cur->object->value->aval[i]);
                    char *n_test = cur->object->value->aval[i];
                    if (n_test[strlen(n_test) - 1] != '\n')
                        printf("\n");                
                }
                printed = true;
                break;
            }
            else if (cur->object->union_type == 3){
                printf("\e[1;%dm%s :\e[0m\n", color, cur->key);
                for(i=a_start;cur->object->value->yaval[i] != ya_end; i++) {
    				for (int j=0; j < def_ind; j++) printf(" ");
                    printf("- \n");
                    print_tree(cur->object->value->yaval[i], def_ind, def_ind + 2);
                }
                printed = true;
                break;
            }
        }

        if (query[0] == '|') {
            query++;
            if (query[0] == '='){
                char value[STRBUFF];
                int j;
                query++; 
                for (;query[0] == ' ' || query[0] == '\t'; query++) {}
                if (query[0] == '"' || query[0] == '\'') {
                    char cmp = query[0];
                    query++;
                    for (j=0;query[0] != cmp; query++, j++) 
                        value[j] = query[0];
                    value[j] = '\0';
                    query++;
                    if (cur->object->union_type == array && a_end || cur->object->union_type == array && a_start > 0) {
                        strcpy(cur->object->value->aval[a_start], value);
                    }
                    else {
                        char *exchange;
                        del_object(cur->object);
                        cur->object->union_type = string;
                        exchange = malloc(sizeof(value));
                        strcpy(exchange,value);
                        cur->object->value->cval = exchange;
                    }
                }
                else if (query[0] = '[') {
                    char *arr[STRBUFF];
                    int pos = 0;
                    query++;
                    while (query[0] != ']') {
                        if (query[0] == '\0') return NULL;
                        for (;query[0] == ' ' || query[0] == '\t'; query++) {}
                        if (query[0] == '"' || query[0] == '\'') {
                            char cmp = query[0];
                            query++;
                            for (int j=0;query[0] != cmp; query++, j++) 
                                value[j] = query[0];
                            char *exchange;
                            query++;
                            exchange = malloc(sizeof(value));
                            strcpy(exchange,value);
                            arr[pos] = exchange;
                        }
                        for (;query[0] == ' ' || query[0] == '\t'; query++) {}
                        if (query[0] == ',') {
                            pos++, query++;
                        }
                        else if (query[0] != ']'){
                            printf("error\n");
                            return NULL;
                        }
                    }
                    query++;
                    del_object(cur->object);
                    cur->object->union_type = array;
                    cur->object->value->aval = arr;
                }
                if (embedded && query[0] != ')')
                    return query;
            }
            else {
                printf("Query is malformed\n");
                return NULL;
            }
        }

        if (query[0] == '.') {
            if (cur->object->union_type == 1) {
                query = analyze_tree(cur->object->value->yval, query, true);
                if (query == NULL)
                    return NULL;
            }
            else if (cur->object->union_type == 3 && y_ap == true) {
                query = analyze_tree(cur->object->value->yaval[a_start], query, true);
                if (query == NULL)
                    return NULL;
            }
            else {
                printf("Value is not an embedded dict\n");
                return NULL;
            }
            if (printed || embedded && query[0] != ')') {
                return query;
            }
        }

	}
}

int gen_tree(FILE *fptr, struct tree_list *ylist, int ind, bool embedded, fpos_t *pos) {
	int cur_ind, i, new_ind = 0, j = 0;
	bool emptyArr = true, inArr = false;
	char *file = malloc(BUFF), key[STRBUFF], value[STRBUFF], *exchange, *arr[BUFF], word[BUFF];
    struct tree_list **y_arr;
    enum arrtype aType = NA;
	struct ytree *object;
	int catch = 0;

	while (fgets(file, BUFF, fptr) != NULL) {
		cur_ind = 0;
		for (;file[0] == ' ' || file[0] == '\t'; file++) 
			cur_ind++;
		if (file[0] == '#') for (;file[0] != '\n'; file++){}
		if (file[0] == '\n') continue;
		if (inArr == true) {
			if (cur_ind < new_ind || file[0] != '-' && cur_ind == new_ind) {
               if (file[0] == '-' || emptyArr == true) {
				printf("Incorrect YAML format top\n");
				return 1;
               }
               if (aType == Arr) {
                   object->object->union_type = array;
                   object->object->value->aval = arr;
               }
               else if (aType == yArr) {
                   object->object->union_type = y_array;
                   object->object->value->yaval = y_arr;
               }
               ylist->append(ylist, object);
               inArr = false;
               j = 0;
               aType = NA;
			}
			else if (file[0] == '-') {
				file++;
				for (;file[0] == ' ' || file[0] == '\t'; file++) {}
				if (file[0] == '#') for (;file[0] != '\n'; file++){}
				if (emptyArr == true) {
					new_ind = cur_ind;
					emptyArr = false;
				}
				for (i=0;file[0] != ' ' && file[0] != '\t' && file[0] != '\0' && file[0] != ':' && file[0] != '#'; file++, i++)
					key[i] = file[0];
                key[i] = '\0';
                for (;file[0] == ' ' || file[0] == '\t'; file++, i++) {} 
                if (file[0] != ':' && key[0] != '\n') {
                    file -= i;
                    key[0] = '\0';
                }
                else {
                    file++;
                    for (;file[0] == ' ' || file[0] == '\t'; file++, i++) {} 
                }
                if (file[0] == '|') {
                    value[0] = file[0];
                    file++;
                    for (;file[0] == ' ' || file[0] == '\t'; file++, i++) {} 
                    if (file[0] != '\n' && file[0] != '#'){
                        return 1;
                    }
                    else if (file[0] == '#') {
                        for (;file[0] != '\n'; file++){}
                    }
                    value[1] = file[0];
                    i = 2;
                    while (1) {
                        fpos_t t_pos;
                        int t_ind = 0;
                        char *l_test = malloc(BUFF);
                        fgetpos(fptr, &t_pos);
                        fgets(l_test, BUFF, fptr);
                        fsetpos(fptr, &t_pos);
                        for (;l_test[0] == ' ' || l_test[0] == '\t'; l_test++)
                            t_ind++;
                        if (l_test[0] == '-' || t_ind < cur_ind || l_test[0] == '\0') {
                            break;
                        }
                        fgets(file, BUFF, fptr);
                        for (;file[0] == ' ' || file[0] == '\t'; file++)
                            value[i++] = ' ';
                        value[i++] = ' ';
                        value[i++] = ' ';
                        if (file[0] == '#')
                            for (;file[0] != '\n'; file++) {}
                        if (file[0] == '\n')
                            continue;
                        if (file[0] == '"' || file[0] == '\'') {
                            char cmp = file[0];
                            file++;
                            for (;file[0] != cmp; file++, i++) 
                                value[i] = file[0];
                            file++;
                            for (;file[0] == ' ' || file[0] == '\t'; file++) {}
                            if (file[0] == '#')
                                for (;file[0] != '\n'; file++) {}
                            if (file[0] != '\n'){}
                        }
                        else {
                            for (;file[0] != '\0' && file[0] != '#' && file[0] != '|'; file++, i++)
                                value[i] = file[0];
                            for (;file[0] == ' ' || file[0] == '\t'; file++) {}
                            if (file[0] == '#')
                                for (;file[0] != '\n'; file++) {}
                            if (file[0] != '\n') {}
                        }
                        
                    }
                        value[i] = '\0';
                }
                else if (file[0] == '"' || file[0] == '\'') {
                    char cmp = file[0];
                    file++;
                    for (i=0;file[0] != cmp; file++, i++) 
                        value[i] = file[0];
                    file++;
                }
                else {
                    for (i=0;file[0] != '\0' && file[0] != '#' && file[0] != '|'; file++, i++)
                        value[i] = file[0];
                }
				value[i] = '\0';
                if (key[0] != '\0') {
                    if (aType == NA) {
                        aType = yArr;
                        y_arr = malloc(sizeof(*y_arr));
                    }
                    if (aType != yArr) {
                        printf("Can't mix array types\n");
                        return 1;
                    }
                    struct tree_list *new_list = new_tree_list();
                    fpos_t t_pos;
                    char *l_test = malloc(BUFF);
                    int t_ind = 0;
                    if (key[0] != '\n') {
                        struct ytree *ar_obj;
                        ar_obj = new_tree();
                        exchange = malloc(sizeof(key));
                        strcpy(exchange,key);
                        ar_obj->key = exchange;
                        exchange = malloc(sizeof(value));
                        strcpy(exchange,value);
                        ar_obj->object->union_type = string;
                        ar_obj->object->value->cval= exchange;
                        new_list->append(new_list, ar_obj);
                    }
                    bool embed = true;

                    fgetpos(fptr, &t_pos);
                    fgets(l_test, BUFF, fptr);
                    fsetpos(fptr, &t_pos);

                    for (;l_test[0] == ' ' || l_test[0] == '\t'; l_test++)
                        t_ind++;
                    if (l_test[0] == '-' || t_ind < cur_ind || l_test[0] == '\0') {
                        y_arr[j] = new_list;
                    }
                    else if (t_ind > cur_ind) {
                        catch = gen_tree(fptr, new_list, cur_ind, embed, pos);
                        if (catch > 0) return catch;
                        y_arr[j] = new_list;
                    }
                        j++;
                }
                else {
                    if (aType == NA) {
                        aType = Arr;
                        *arr = malloc(sizeof(*arr));
                    }
                    if (aType != Arr) {
                        printf("Can't mix array types\n");
                        return 1;
                    }
                    exchange = malloc(sizeof(value));
                    strcpy(exchange,value);
                    arr[j] = exchange;
                    j++;
                }

				for (;file[0] == ' ' || file[0] == '\t'; file++) {}
				if (file[0] != '\n' && file[0] != '\0') {
					printf("this is not correct YAML2\n");
					return 1;
				}
                fgetpos(fptr,pos);
                if (fgets(file, BUFF, fptr) == NULL){
                   if (aType == Arr) {
                       object->object->union_type = array;
                       object->object->value->aval = arr;
                   }
                   else if (aType == yArr) {
                       object->object->union_type = y_array;
                       object->object->value->yaval = y_arr;
                   }
                   ylist->append(ylist, object);
                }
                fsetpos(fptr,pos);
				continue;
			}
			else {
				printf("Incorrect YAML format\n");
				return 1;
			}

		}
		new_ind = ind;
		object = new_tree();

		if (cur_ind <= ind && embedded == true) {
			break;
		}
		if (embedded == true) fgetpos(fptr,pos);
		for (i=0;file[0] != ' ' && file[0] != '\t' && file[0] != ':' && file[0] != '\n' && file[0] != '#' && file[0] != '\'' && file[0] != '"'; file++, i++){
			key[i] = file[0];
		}
		if (file[0] == '\'' || file[0] == '"') {
			printf("this is not correct YAML3\n");
			return 1;
		}
		key[i] = '\0';
		exchange = malloc(sizeof(key));
		strcpy(exchange,key);
		object->key = exchange;
		for (;file[0] == ' ' || file[0] == '\t'; file++){} 
		if (file[0] == '#') for (;file[0] != '\n'; file++){}

		if (file[0] != ':') {
			printf("this is not correct YAML4\n");
			return 1;
		}
		else{
			file++;
		}
		
		for (;file[0] == ' ' || file[0] == '\t'; file++) {}
		if (file[0] == '#') for (;file[0] != '\n'; file++){}
		
		if (file[0] == '\n') {
            fpos_t t_pos;
            fgetpos(fptr, &t_pos);
            fgets(file, BUFF, fptr);
            fsetpos(fptr, &t_pos);
            for (;file[0] == ' ' || file[0] == '\t'; file++){} 
            if (file[0] == '-') {
                emptyArr = true;
                inArr = true;
                continue;
            }
            else {
                struct tree_list *new_list = new_tree_list();
                bool embed = true;
                catch = gen_tree(fptr, new_list, cur_ind, embed, pos);
                if (catch > 0) return catch;
                object->object->union_type = tree;
                object->object->value->yval= new_list;
                ylist->append(ylist, object);
                continue;
            }
		}
		if (file[0] == '|') {
            value[0] = file[0];
			file++;
            for (;file[0] == ' ' || file[0] == '\t'; file++, i++) {} 
            if (file[0] != '\n' && file[0] != '#'){
				printf("Incorrect YAML format\n");
                return 1;
            }
            else if (file[0] == '#') {
                for (;file[0] != '\n'; file++){}
            }
            value[1] = file[0];
            i = 2;
            int new_ind = 0; 
            int t_ind;
            while (1) {
                fpos_t t_pos;
                t_ind = 0;
                char *l_test = malloc(BUFF);
                fgetpos(fptr, &t_pos);
                fgets(l_test, BUFF, fptr);
                fsetpos(fptr, &t_pos);
                for (;l_test[0] == ' ' || l_test[0] == '\t'; l_test++)
                    t_ind++;
                if (new_ind == 0)
                    new_ind = t_ind;
                if (l_test[0] == '-' || t_ind < new_ind || l_test[0] == '\0') {
                    break;
                }
                fgets(file, BUFF, fptr);
                for (;file[0] == ' ' || file[0] == '\t'; file++) {}
                if (file[0] == '#')
                    for (;file[0] != '\n'; file++) {}
                if (file[0] == '\n')
                    continue;
                for (int h = 0; h < strlen(key) + 1; h++)
                    value[i++] = ' ';
                value[i++] = ' ';
                value[i++] = ' ';
                if (file[0] == '"' || file[0] == '\'') {
                    char cmp = file[0];
                    file++;
                    for (;file[0] != cmp; file++, i++) 
                        value[i] = file[0];
                    file++;
                    for (;file[0] == ' ' || file[0] == '\t'; file++) {}
                    if (file[0] == '#')
                        for (;file[0] != '\n'; file++) {}
                    if (file[0] != '\n') {}
                }
                else {
                    for (;file[0] != '\0' && file[0] != '#' && file[0] != '|'; file++, i++)
                        value[i] = file[0];
                    for (;file[0] == ' ' || file[0] == '\t'; file++) {}
                    if (file[0] == '#')
                        for (;file[0] != '\n'; file++) {}
                    if (file[0] != '\n') {}
                }
                
            }
                value[i] = '\0';
            exchange = malloc(sizeof(value));
            strcpy(exchange,value);
            object->object->union_type = string;
            object->object->value->cval= exchange;
            ylist->append(ylist, object);
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

		for (;file[0] == ' ' || file[0] == '\t'; file++) {}
		if (file[0] == '#') for (;file[0] != '\n'; file++){}
		if (file[0] != '\n') {
			printf("this is not correct YAML1\n");
			return 1;
		}
		ylist->append(ylist, object);
		
	}
	if (embedded == true) fsetpos(fptr, pos);
}

int main(int argc, char **argv){
	FILE *fptr;
	char line[BUFF], *flag, *list[BUFF];
	struct tree_list *ylist = new_tree_list();
	bool embedded = false;
	fpos_t  pos;
	static int stat_ind = 4;
	int catch = 0, j=0;
    argc--;
    argv++;

    for (int i=0;i<argc;i++) {
           flag = argv[i]; 
           if (flag[0] == '-'){
               flag++;
               for (int j = 0; flag[j] != '\0'; j++) {
                   switch (flag[j]) {
                    case 'c':
                        color = 32;
                        break;
                    case 'i':
                        char *c_ind;
                        c_ind = argv[++i];
                        if (c_ind == NULL) {
                            printf("Only integers allowed\n");
                            return 1;
                        }
                        int k = 0;
                        char numstr[STRBUFF];
                        for (k=0;c_ind[0] >='0' && c_ind[0] <= '9';c_ind++, k++)
                            numstr[k] = c_ind[0];
                        if (c_ind[0] != '\0') {
                            printf("Only integers allowed\n");
                            return 1;
                        }
                        numstr[k] = '\0';
                        def_ind = atoi(numstr);
                        break;
                   }
               }
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

	catch = gen_tree(fptr, ylist, 0, embedded, &pos);
	if (catch > 0) return catch;
	free(fptr);

    analyze_tree(ylist, query, false);
    del_tree_list(ylist);
    return 0;
}
