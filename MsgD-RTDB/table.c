//@file: table.c
//
//  Table management routines.
//
// Some of the code in this file has been derived from the original 
// source code by: Jerry Coffin, with improvements by HenkJ
// and Wolthuis.
//@

#include <malloc.h>
#include <string.h>

#include "base/errlib.h"
#include "base/msglib.h"

#include "shmlib.h"
#include "rtdb.h"       // For DB_variable
#include "table.h"

//@Function{API}: hcreate
//
// Create hash table
//
// Initialize the hash_table to the size asked for. Allocates space
// for the correct number of pointers and sets them to NULL.
//@

int hcreate(hash_table *table,  // @P{table}: Pointer to table structure
            int        size)    // @P{size}: Table size (N. of elements)
{                               // @R: Completion code
    int i,ovfl;

    ovfl=size*1.02;   // Compute a convenient overflow size
    table->body = (void *)calloc(ovfl,sizeof(void *));
    if (table->body == NULL) return MEMORY_ALLOC_ERROR;
    table->size=size;
    table->ovfl=ovfl;
    table->nItms=0;
    table->nColls=0;
    table->nInsert=0;
    table->nRemove=0;

    for (i=0;i<ovfl;i++)           // Initialize table elements
        table->body[i] = NULL;

    return NO_ERROR;
}

//@Function: hhash
//
// Compute hash values
//
// Hashes a string to produce an unsigned short, which should be
// sufficient for most purposes.
//
  
// Note: the original hash() function is broken (it also looks to bytes
//       after the '\0'). I've found the current version on the net. It
//       is claimed to be good for ASCII strings.
//@

static unsigned hhash(char *string, int size)
{
    unsigned h = 5381;
    int c;

    while ((c = *string++))
        h = ((h<<5) + h) + c;
    
    return (h%size);
}

//@Function{API}: hput
//
// Insert a variable into an hash table
//
//@

int hput(hash_table *table,   // @P{table}: table to search
         DB_Var     *item)   // @P{item}: Variable item to insert.
	                      // NOTE: The item is not copied into the
                              // table so that the memory area holding
	                      // data must be permanently allocated by the 
	                      // caller.
	                      // @R: completion code
{
    DB_Var *ptr;
    unsigned val = hhash(item->Name,table->size);

    while(val<table->ovfl) {
        if(NULL==(ptr=table->body[val])) break;
        if (0 == strcmp(item->Name, ptr->Name))
             return PLAIN_ERROR(DUPL_NAME_ERROR);      // Duplicate variable
        table->nColls++;
        val++;
    }
    if(val==table->ovfl) return SYSTEM_ERROR(HASH_INSERT_ERROR);
                             // Here ptr points to the next free slot
    table->body[val]=item;   // Add item to list
    table->nItms++;          // Increase element count
    table->nInsert++;
    return val;
}

//@Function{API}: hremove
//
// Deletes a variable from an hash table
//
// This function deletes a variable from an hash table.
// A pointer to the variable content is returned so that the caller can
// perform needed operations (such as freeing variable related memory)
//@

DB_Var *hremove(hash_table *table,  // @P{table}: table
                char       *name)   // @P{name}: name of the item to remove
                                    // @R: pointer to Variable (or NULL if not found)
{
    DB_Var *ptr;
    DB_Var *ret=NULL;
    unsigned val = hhash(name, table->size);

    while(val<table->ovfl) {
        if ((ptr=table->body[val]) && (0==strcmp(name,ptr->Name))) {
            ret=ptr;
            table->body[val]=NULL;
            table->nItms--;     // Decrease element count
            table->nRemove++;
            break;
        }
        val++;
    }
    return ret;
}


//@Function{API}: hsearch
//
// Search variables into an hash table
//
// This function searches an item with given name
//
// Note: The function is not "thread safe". You might want to enclose
//       the call into a critical section.
//@

int hsearch(hash_table *table,     // @P{table}: table.
            char *name         )   // @P{name}: Item to search/insert.
                                   // @R: index of found item, (-1) if not found.
{
    DB_Var *ptr;
    int ret=(-1);
    unsigned val = hhash(name, table->size);

    while(val<table->ovfl) {
        if ((ptr=table->body[val]) && (0==strcmp(name,ptr->Name))) {
            ret=val;
            break;  // Found
        }
	val++;
    }
    return ret;
}



//@Function: sfirst
//
// Searches a sorted list for for the first matching prefix
//
// This function searches an item in a sorted list with partial matching
// of a string. The search matches when the given string matches the prefix
// of the item key.
// If more than one key matches, the index of first matching key is returned
//
//@

static int sfirst(sort_table *stable,  // @P{table}: pointer to table
                   char *key,           // @P{key}: item key
                   int  kl,             // @P{key}: key length
                   int *pos)            // @P{pos}: store here index of item (if found)
                                        //          or position where to store item (if not found)
                                        // @R: 1: found, 0: not found
{
int id0, id1;
int dd,found;

if(kl<=0) {
    if (stable->nItms>0) {
        *pos=0;
        found=1;
    } else
        found=0;
    return found;
}

for(id0=(-1),id1=stable->nItms,found=0;;) {
   int id;
   if((id=(id1-id0)/2)==0) break;
   id+=id0;
   dd=strncmp(key,stable->body[id].key,kl);
   if(dd<0)  {
      id1=id;
      continue;
   }
   if(dd>0)  {
      id0=id;
      continue;
   }
   id1=id;
   found=1;
}
*pos=id1;
return found;
}
      
//@Function{API}: ssearch
//
// Searches a sorted list for a matching key (exact)
//
// This function searches an item in a sorted list for exact matching
// of a string. 
// If more than one key matches, the index of first matching key is returned
//
//@

int ssearch(sort_table *stable,  // @P{table}: pointer to table
            char *key,           // @P{key}: item key
            int *pos)            // @P{pos}: store here index of item (if found)
                                 //          or position where to store item (if not found)
                                 // @R: 1: found, 0: not found
{
int id0, id1;
int dd,found;

for(id0=(-1),id1=stable->nItms,found=0;;) {
   int id;
   if((id=(id1-id0)/2)==0) break;
   id+=id0;
   dd=strcmp(key,stable->body[id].key);
   if(dd<0)  {
      id1=id;
      continue;
   }
   if(dd>0)  {
      id0=id;
      continue;
   }
   id1=id;
   found=1;
}
*pos=id1;
return found;
}


//@Function{API}: screate
//
// Create sorted table
//
// Create and initialize a sorted table to the size asked for. Allocates space
// for the correct number of items and sets them to NULL.
//@

int screate(sort_table *table,  // @P{table}: Pointer to table structure
            int        size,    // @P{size}: Initial table size (N. of elements)
            int        incr)    // @P{incr}: Increment size when extending. If 0
                                //           extension is computed as 25% of initial size
{                               // @R: Completion code
    table->body = calloc(size,sizeof(s_item));
    if (table->body == NULL) return MEMORY_ALLOC_ERROR;
    table->size=size;
    table->nItms=0;
    if(incr<=0) incr=size/4;
    if(incr<1) incr=1;
    table->incr=incr;

    return NO_ERROR;
}

//@Function{API}: sput
//
// Puts an item in sorted list
//
// Adds the given item to the sorted list
//
//@

int sput(sort_table *stable, // @P{table}: pointer to table
         s_item     *item,   // @P{item}: item value.
         int        *pos)    // @P{pos}: position where item has been put
	                     // NOTE: The item is not copied into the
                             // table so that the memory areas holding both
	                     // key and value data must be permanently 
	                     // allocated by the caller.
	                     // @R: completion code
{
int found,id;

if(*(item->key)=='\0') 
    return NULL_CLIST_ERROR;
if(stable->nItms>=stable->size) {        // Extend table, if needed
    stable->size += stable->incr;
    stable->body = realloc((void *)stable->body, sizeof(s_item) * stable->size);
    if(stable->body==NULL) {
        stable->size=0;
        stable->nItms=0;
        return SYSTEM_ERROR(MEMORY_ALLOC_ERROR);
    }
}

found = ssearch(stable,item->key,pos);

if(found) return DUPL_NAME_ERROR;

for(id=stable->nItms;id>*pos;id--)        // Make space for the new key
    stable->body[id]=stable->body[id-1];
      
stable->body[*pos].key=item->key;
stable->body[*pos].value=item->value;
stable->nItms++;
return NO_ERROR;
}

//@Function: sremove
//
// removes an item from sorted list
//
// removes an item at given position from a sorted list
//
//@
int sremove(sort_table *stable, // @P{table}: pointer to table
            int        pos)     // @P{pos}: item position
                                // @R: completion code
{
int id;

if(pos<0 || pos >= stable->nItms) return INDEX_RANGE_ERROR;

for(id=pos;id<(stable->nItms-1);id++) stable->body[id]=stable->body[id+1];
if(stable->nItms>0) stable->nItms--;
return NO_ERROR;
}


//@Function: sremstr
//
// removes an item matching string from sorted list
//
// removes the given item from a sorted list
//
//@

int sremstr(sort_table *stable, // @P{table}: pointer to table
            char       *key)    // @P{key}: item key
	                        // @R: completion code
{
int ret,pos;

if(ssearch(stable,key,&pos))
    ret=sremove(stable,pos);
else
    ret=KEY_NOTFOUND_ERROR;

return ret;
}

//@Procedure{API}: sinit
//
// Initialize sorted table scan function
//
// This function sets up the data required for a scanning operation.
// After initialization valid table elements can be retrieved by calling
// @see{snext}.

void sinit(sort_table *stable, char *key, int len, Listscan *listX)    // Pointer to list status structure
{
   int found,pos;

   listX->table=stable;
   listX->skey=key;
   if(len<0) 
       listX->kl=strlen(listX->skey);
   else
       listX->kl=len;

   if((found = sfirst(stable,listX->skey,listX->kl,&pos)))
       listX->sId= pos;
   else
       listX->sId= (-1);
}

//@Function{API}: snext
//
// Get next element in a table scan
//
// This function is called to get the next element in a full table scan
// Table scanning must be initialized by calling @see{sinit}.
//@

s_item *snext(Listscan   *listX)   //@P{listX}: Pointer to list status structure.
                                   //@R: pointer to item
{
    s_item *ret=NULL;
    sort_table *table=listX->table;

    if(listX->sId<0) return ret;
    ret=&(table->body[listX->sId++]);
    if((listX->sId >= table->nItms) || 
       (strncmp(listX->skey,table->body[listX->sId].key,listX->kl)!=0) ) listX->sId=(-1);
        
    return ret;
}

//@Function{API}: initclist
//
// Initalize a CList
//
//@

CList *initclist(void)
{
   CList *clst;

   clst=(CList *)malloc(sizeof(CList));
   if(!clst) return NULL;
   clst->nItms=0;
   clst->size=0;
   clst->body=NULL;
   return clst;
}


//@Function{API}: addclist
//
// Add an item to CList
//
//@

int addclist(CList  *clst,
             Client *cl,
             char *  fname)
{
   int id;
   cListItem *c;

   if(!clst) return NULL_CLIST_ERROR;

   for(id=0;id<clst->nItms;id++) 
      if( strncmp(clst->body[id].fname,fname,LONG_NAME_LEN)==0) return DUPL_CLIST_ERROR;

   if(clst->nItms>=clst->size) {       // Extend CList
      int nsize=clst->size+CLISTCHUNK;
      c=realloc(clst->body,nsize*sizeof(cListItem));
      if(!c) return MEMORY_ALLOC_ERROR;
      clst->body=c;
      clst->size=nsize;
   }
   strncpy(clst->body[clst->nItms].fname,fname,LONG_NAME_LEN);
   clst->body[clst->nItms].fname[LONG_NAME_LEN]='\0';
   clst->body[clst->nItms++].clPtr=cl;
   return NO_ERROR;
}


//@Function{API}: remclist
//
// removes an item from CList
//
//@

int remclist(CList  *clst,
             char *fname)
{
   int id,nxt,found=0;

   if(!clst) return NOT_IN_CLIST_ERROR;

   for(id=0;id<clst->nItms;id++) 
      if(strncmp(clst->body[id].fname,fname,LONG_NAME_LEN)==0) { found=1; break; }

   if(found) {
      for(nxt=id+1;nxt<clst->nItms;id++,nxt++) {   // Compact array
         strncpy(clst->body[id].fname,clst->body[nxt].fname,LONG_NAME_LEN);
         clst->body[id].fname[LONG_NAME_LEN]='\0';
         clst->body[id].clPtr=clst->body[nxt].clPtr;
      }
      clst->nItms--;
      return NO_ERROR;
   } else
      return NOT_IN_CLIST_ERROR;
}

//@Function{API}: clistempty
//
// Checks if Clist is empty
//
//@

int clistempty(CList *clst)
{
if(!clst) return 1;
return (clst->nItms<=0);
}


//@Function{API}: freeclist
//
// Free clist resources
//
//@

void freeclist(CList *clst)
{
   if(clst) {
       if(clst->body) free(clst->body);
       clst->nItms=0;
       clst->size=0;
       clst->body=NULL;
   }
}



//@Function: InitTableScan
//
// Intialize the scan of a sorted table
//
// This routine must be called to initialize a sorted table
// scanning operation. 
// After initialization NextTableScan can be called to get next variable 
// from the table. At the end NextTableScan returns NULL and EndTableScan() 
// must be called to reset and release resources used.
//
// Table scanning always scans the entire list
//
//@

int InitTableScan(SortedTable* table,Listscan *listX)      // List status structure
{
    sinit(&(table->stable),"",0,listX);
    MUTEX_LOCK(&(table->mutex),"TableScan");
    return NO_ERROR;
}


//@Function: NextTableScan
//
// Get next matching entry in Table List
//
// This routine must be called repeatedly to scan the Tabley table.
// The scanning mechanism must be initialized with a call to InitTableScan()
//@

s_item *NextTableScan(Listscan *listX)   // Returns: pointer to s_item, or NULL
{
    return snext(listX);
}

//@Function{API}: EndTableScan

// Terminates the Table list scanning


// This function must be called at the end of a list scanning
//@

int EndTableScan(SortedTable *table)
{
    MUTEX_UNLOCK(&(table->mutex),"TableScan");
    return NO_ERROR;
}

