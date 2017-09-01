//@File:  table.h
//
// Definitions and prototype for the module table.c
//@

#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>      // For size_t

#include "base/rtdblib.h"     // For Variable

#include "types.h"            // For Client structure


// Here follows the prototype definitions
//

int hcreate(hash_table *table, int size);
int hput(hash_table *table, DB_Var *v);
DB_Var *hremove(hash_table *table, char *name);
int hsearch(hash_table *table, char *name);

int screate(sort_table *table, int size, int incr);
int sput(sort_table *table, s_item *v, int *pos);
int sremove(sort_table *table, int pos);
int sremstr(sort_table *table, char *name);
int ssearch(sort_table *table, char *name, int *pos);
void sinit(sort_table *table, char *key, int len, Listscan *l);
s_item *snext(Listscan *listX);

CList * initclist(void);
int addclist(CList *clst, Client *cl, char *fname);
int remclist(CList *clst, char *fname);
int clistempty(CList *clst);
void freeclist(CList *clst);

int InitTableScan(SortedTable* table,Listscan *listX) ;
s_item *NextTableScan(Listscan *listX);
int EndTableScan(SortedTable *table);


#endif /* TABLE_H */

