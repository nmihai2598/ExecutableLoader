#include "utils.h"

static TL create_cel(void *info)
{
	TL new;

	new = calloc(1, sizeof(Tlist));
	new->info = info;
	new->urm = NULL;
	return new;
}

void add_list(TL *list, void *info)
{
	TL p;

	if (!*list) {
		*list = create_cel(info);
		return;
	}
	for (p = *list; p->urm != NULL; p = p->urm)
			;
	p->urm = create_cel(info);
}
void *get_el_list(TL *list, so_compare *compare)
{
	TL i, ant, max, antmax;
	void *info;

	max = *list;
	antmax = NULL;
	for (i = max->urm, ant = max; i != NULL; ant = i, i = i->urm) {
		if (compare(max->info, i->info)) {
			max = i;
			antmax = ant;
		}
	}
	info = max->info;
	if (!antmax) {
		*list = max->urm;
		free(max);
	} else {
		antmax->urm = max->urm;
		free(max);
	}
	return info;
}
void *get_list(TL *list, void *io, so_compare *compare, int *count)
{
	TL find, L, ant, aux, val;

	find = NULL;
	L = *list, ant = NULL;
	while (L != NULL) {
		if (compare(L->info, io)) {
			(*count)++;
			val = L;
			if (!ant) {
				*list = L->urm;
				L = L->urm;
			} else {
				ant->urm = L->urm;
				L = L->urm;
			}
			val->urm = NULL;
			if (!find) {
				find = val;
				aux = find;
			} else {
				aux->urm = val;
				aux = aux->urm;
			}
		} else {
			ant = L;
			L = L->urm;
		}
	}
	return find;
}

void *get_queue(AQ queue)
{
	TL get;
	void *info;

	get = TOP(queue);
	if (!get)
		return NULL;
	TOP(queue) = TOP(queue)->urm;
	info = get->info;
	free(get);
	return info;
}

void add_queue(AQ queue, void *info, so_compare *compare)
{
	TL list, ant, new;

	if (!TOP(queue)) {
		TOP(queue) = create_cel(info);
		return;
	}
	for (list = TOP(queue), ant = NULL; list != NULL;
			ant = list, list = list->urm) {
		if (compare(list->info, info)) {
			if (!ant) {
				TOP(queue) = create_cel(info);
				TOP(queue)->urm = list;
			} else {
				new = create_cel(info);
				new->urm = list;
				ant->urm = new;
			}
			return;
		}
	}
	ant->urm = create_cel(info);
}

void add_list_queue(AQ queue, TL list, so_compare *compare)
{
	TL aux;

	while (list != NULL) {
		add_queue(queue, list->info, compare);
		aux = list;
		list = list->urm;
		free(aux);
	}
}
