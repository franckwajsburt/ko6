/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/list.h
  \author   Franck Wajsburt
  \brief    Management of double chained lists

  This file contains the functions (inline & define) necessary to manage double chained lists.
  These are based on embedded structures (list_t) with two pointers each which are linked together.
  - A list consists of only two types of objects:
     1. a type for the root structure with an embedded list_t member
     2. a type for all items in the list also with an embedded list_t member
  - The list_t member of the root is connected to the list_t member that are in the items
  - it is possible to calculate the address of the container, by knowing:
     - the memory address of the embedded structure (list_t),
     - the type C of the item that contains the (list_t) member and
     - the member name of the embedded member (list_t) in the item

  struct root_s {       struct item_s {         Note that the list_t member can be placed
      type1 member_1;       type1 member_a;     anywhere in the root_s and item_s structures
      ...                   ...                 and it can be nammed anything. The chosen names
      list_t root;          list_t item;        depends onthe use of the structures.
      typen member_n;       typem members_m;    Generally, root_s and item_s do not have the
  };                    };                      same size (warning, the scheme can be misleading)

      struct root_s               struct root_s     struct item_s     struct item_s
      .------------.              .------------.    .------------.    .------------.
      |  member_1  |              |  member_1  |    |  member_a  |    |  member_a  |
      |    ...     |              |    ...     |    |    ...     |    |    ...     |
      |.----------.|              |.----------.|    |.----------.|    |.----------.|
  .--->|   next   |----.      .--->|   next   |----->|   next   |----->|   next   |----.
  |.---|   prev   |<--.|      |.---|   prev   |<-----|   prev   |<-----|   prev   |<--.|
  ||  |'----------'|  ||      ||  |'----------'|    |'----------'|    |'----------'|  ||
  ||  |  member_n  |  ||      ||  |  member_n  |    |  member_m  |    |  member_m  |  ||
  ||  '------------'  ||      ||  '------------'    '------------'    '------------'  ||
  |'------------------'|      |'------------------------------------------------------'|
  '--------------------'      '--------------------------------------------------------'
        Empty list                           list with 2 items

  API  (see detailled descriptions bellow with an example at the end)
  -------------------------------------------------------------------

  - test
      unsigned list_isempty  (list_t * root)
      unsigned list_isfirst  (list_t * root, list_t * item)
      unsigned list_islast   (list_t * root, list_t * item)
  - consultation
      list_t * list_first    (list_t * root)
      list_t * list_last     (list_t * root)
      list_t * list_next     (list_t * item)
      list_t * list_prev     (list_t * item)
      list_foreach           (list_t * ROOT, list_t * ITEM)
      list_item              (list_t * ITEM, typeof item_s, MEMBER)
  - modification
      void list_init         (list_t * root)
      void list_addfirst     (list_t * current_item, list_t * added_item)
      void list_addlast      (list_t * current_item, list_t * added_item)
      list_t * list_unlink   (list_t * item)
      list_t * list_getfirst (list_t * root)
      list_t * list_getlast  (list_t * root)
      void list_replace      (list_t * old_item, list_t * new_item)
  - miscellaneous
      unsigned list_nbobj    (list_t * root)

\*------------------------------------------------------------------------------------------------*/

#ifndef _LIST_H_
#define _LIST_H_
#ifndef __DEPEND__      // this condition allows to not include stdarg.h when makedepend is used
#include <stddef.h>     // gcc's builtin include with offsetof() (https://bit.ly/3lBw3p6)
#endif

// EXTRACT_STATIC = YES for Doxygen to document the static functions

/**
 * \brief The 2-pointers structure to embed in the root of a list and in each of its items
 */
typedef struct list_s {
    struct list_s * next;   ///< toward another same type struct in a root or a item list
    struct list_s * prev;   ///< backward
} list_t;


// TEST FUNCTIONS ----------------------------------------------------------------------------------


/**
 * \brief   Tests if a list don't have any item.
 * \param   root root of list
 * \return  true if there is no item,  false otherwise
 */
static inline unsigned list_isempty (list_t * root) {
    return root == root->next;
}

/**
 * \brief   Tests if an item is the first item of the list root.
 * \param   root root of list
 * \param   item an item of the list
 * \return  true if elmenent is the first one, false otherwise
 */
static inline unsigned list_isfirst (list_t * root, list_t * item) {
    return item == root->next;
}

/**
 * \brief   Tests if an item is the last item of the list root.
 * \param   root root of list
 * \param   item an item of the list
 * \return  true if elmenent is the last one, false otherwise
 */
static inline unsigned list_islast (list_t *  root, list_t * item) {
    return item == root->prev;
}

// CONSULTATION FUNCTIONS --------------------------------------------------------------------------


/**
 * \brief   Give the first item of the list, do not detache it
 * \param   root root of list
 * \return  A pointer to the first elenent if there is, NULL if the list is empty
 */
static inline list_t * list_first (list_t * root) {
    return (list_isempty(root)) ? NULL : root->next;
}
#define list_next(item) list_first(item)

/**
 * \brief   Give the last item of the list, do not detache it.
 * \param   root root of list
 * \return  A pointer to the last elenent if there is, NULL if the list is empty
 */
static inline list_t * list_last (list_t * root) {
    return (list_isempty(root)) ? NULL : root->prev;
}
#define list_prev(item) list_last(item)

/**
 * \brief   C instruction to iterate through a list of items from first to last.
 *          list_foreach will iterate as many times as there are items in the ROOT list.
 *          At each iteration, ITEM is defined with the current item:
 *          the first, the second, until the last.
 *          If you need several foreach() in the same function, you have to create a block
 *          (by adding braces): { foreach(root, item){ your instructions } }
 *          in order to handle the implicit local variables needed for list_foreach().
 * \param   ROOT root of list
 * \param   ITEM : create a list_t pointer, named _ITEM
 */
#define list_foreach(ROOT,ITEM)                 \
    list_t * _NEXT, * ITEM;                     \
    for(ITEM = (ROOT)->next, _NEXT=(ITEM)->next;\
        (ITEM) != (ROOT);                       \
        ITEM = _NEXT, _NEXT = _NEXT->next )

/**
 * \brief   C instruction to iterate through a list of items from last to first.
 *          list_foreach will iterate as many times as there are items in the ROOT list.
 *          At each iteration, ITEM is defined with the current item:
 *          the first, the second, until the last.
 *          If you need several foreach() in the same function, you have to create a block
 *          (by adding braces): { foreach(root, item){ your instructions } }
 *          in order to handle the implicit local variables needed for list_foreach().
 * \param   ROOT root of list
 * \param   ITEM : create a list_t pointer, named _ITEM
 */
#define list_foreach_rev(ROOT,ITEM)             \
    list_t * _NEXT, * ITEM;                     \
    for(ITEM = (ROOT)->prev, _NEXT=(ITEM)->prev;\
        (ITEM) != (ROOT);                       \
        ITEM = _NEXT, _NEXT = _NEXT->prev )

/**
 * \brief   Macro to get the address of the container of a list item.
 * \param   ITEM : pointer the list_t variable created by foreach()
 * \param   TYPE C tyoe of the container
 * \param   MEMBER member name of the list item in the container TYPE
 * \return  A pointer to the container
 */
#define list_item(ITEM,TYPE,MEMBER) ((TYPE*)((void*)ITEM-offsetof(TYPE,MEMBER)))

// MODIFICATION FUNCTIONS --------------------------------------------------------------------------

/**
 * \brief   Initialize of the list root.
 * \param   root a list root
 */
static inline void list_init (list_t * root) {
    root->next = root->prev = root;
}

/**
 * \brief   Add a new added item after the current item which is root the most often.
 *          then the added item is placed at the beginning of the list
 *          list_addfirst and list_addnext are synonymous, you can use either one.
 *          depending on the readability of the code
 * \param   root is the current item
 * \param   added_item is the added item
 * \return  No return but the list has changed.
 */
static inline void list_addfirst (list_t * root, list_t * added_item) {
    added_item->next = root->next;
    added_item->prev = root;
    root->next->prev = added_item;
    root->next = added_item;
}
#define list_addnext list_addfirst

/**
 * \brief   Add a new added item before the current item which is root the most often.
 *          then the added item is placed at the end of the list
 *          list_addlast and list_addprev are synonymous, you can use either one.
 *          depending on the readability of the code
 * \param   root the current item
 * \param   added_item the added item
 * \return  No return but the list has changed.
 */
static inline void list_addlast (list_t * root, list_t * added_item) {
    list_addfirst(root->prev, added_item);
}
#define list_addprev list_addlast

/**
 * \brief   Unlink (i.e. detach) an item of list.
 * \param   item the item to unlink
 * \return  The unlinked item (given in param)
 */
static inline list_t * list_unlink (list_t * item) {
    item->prev->next = item->next;
    item->next->prev = item->prev;
    return item;
}

/**
 * \brief   Unlink (i.e. detach) the first item of list.
 * \param   root a list root
 * \return  The unlinked item.
 */
static inline list_t * list_getfirst (list_t * root) {
    return (list_isempty(root)) ? NULL : list_unlink (root->next);
}

/**
 * \brief   Unlink (i.e. detach) the last item of list.
 * \param   root a list root
 * \return  The unlinked item.
 */
static inline list_t * list_getlast (list_t * root) {
    return (list_isempty(root)) ? NULL : list_unlink (root->prev);
}

/**
 * \brief   Replace an old item by a new one, or an old root by a new one.
 *          When params are roots, that can be used to move an entire list
 *          new_item is not a list before to call list_replace
 * \param   old_item a list item to unlink, that can be also a root
 * \param   new_item a list item to add, that can be also a root
 */
static inline void list_replace (list_t * old_item, list_t * new_item) {
    old_item->next->prev = old_item->prev->next = new_item ;
    new_item->next = old_item->next;
    new_item->prev = old_item->prev;
    list_init(old_item);
}

// MISCELLANEOUS -----------------------------------------------------------------------------------

/**
 * \brief   calculate the number of object of a list
 * \param   root root of list
 * \return  a number or 0 if the list is empty
 */
static inline unsigned list_nbobj (list_t * root) {
    size_t nbobj = 0;
    list_foreach (root, item) nbobj++;
    return nbobj;
}

/**
 * \brief   Add a new added item in a sorted list after root element
 * \param   root is the beginng of the list
 * \param   added_item is the added item into the sorted list
 * \param   cmp is a pointer to a function that compares items A and B, it returns (A - B)
 * \return  No return but the list has changed, all items are sorted by increasing value
 */
static inline void list_addsort (list_t *root, list_t *added_item, int (*cmp)(list_t*A,list_t*B) ) {
    list_foreach (root, curr_item) {
        if (cmp (curr_item, added_item)>=0) {
            list_addprev (curr_item, added_item);
            return;
        }
    }
    list_addlast (root, added_item);
}

#endif// __LIST_H_

/*------------------------------------------------------------------------------------------------*\
  To test these list functions, open a terminal and proceed as follows:
  $ cp list.h list.c
  $ gcc -DTEST -o list list.c
  $ ./list
\*------------------------------------------------------------------------------------------------*/
#ifdef TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "list.h"

typedef struct family_s {
    char *lastname;
    list_t root;
} family_t;

typedef struct person_s {
    char *firstname;
    list_t item;
} person_t;

char * question (char * mess) {
    char buf[256];
    fprintf (stdout, mess);
    fgets(buf, sizeof(buf), stdin);
    buf[strlen(buf)-1] = 0; // delete \n
    return strdup (buf);
}

int cmp_name (list_t * curr, list_t * new) {
    person_t *person_curr = list_item (curr, person_t, item); 
    person_t *person_new  = list_item (new , person_t, item); 
    return strcmp (person_curr->firstname, person_new->firstname);
}

void print_familly (family_t *family) {
    printf ("family %s : ", family->lastname);
    list_foreach (&family->root, i) {
        person_t *ptr = list_item (i, person_t, item);
        printf ("%s, ", ptr->firstname);
    }
    printf ("\b\b \n");
} 

int main () {
    family_t archi2;
    person_t jean_claude;
    person_t monique;

    archi2.lastname = "Archi2";
    jean_claude.firstname = "Jean-Claude";
    monique.firstname = "Monique";

    printf("\nbuilt a LIFO list\n");
    person_t * moi = malloc (sizeof(person_t));
    moi->firstname = question ("Entrez votre nom : ");
    list_init (&archi2.root);
    list_addfirst (&archi2.root, &monique.item);
    list_addfirst (&archi2.root, &jean_claude.item);
    list_addfirst (&archi2.root, &moi->item);
    print_familly (&archi2);

    printf("\nBuilt a sorted list\n");
    list_init (&archi2.root);
    while (1) {
        person_t * new = malloc (sizeof(person_t));
        new->firstname = question ("Entrez votre nom : ");
        list_addsort (&archi2.root, &new->item, cmp_name);
        print_familly (&archi2);
    }

    return 0;
}
#endif
