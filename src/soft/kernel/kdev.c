/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2023-08-01
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     kernel/kdev.c
  \author   Nolan Bled
  \brief    Generic device management functions

\*------------------------------------------------------------------------------------------------*/

#include <kernel/kdev.h>

static list_t DevList = {      // Do the same thing the function list_init does
    .next = &DevList,
    .prev = &DevList
};

unsigned dev_next_no(enum dev_tag_e tag)
{
    list_foreach_rev(&DevList, item) {
        struct dev_s *dev = list_item(item, struct dev_s, list);
        if (dev->tag == tag)
            return dev->no + 1;
    }
    return 0;
}

struct dev_s *dev_alloc(enum dev_tag_e tag, unsigned dsize)
{
    struct dev_s *dev = kmalloc(sizeof(struct dev_s) + dsize);  // Allocate enough space for 
                                                                // device metadata (tag, no, list)
                                                                // and device-specific data 
                                                                // (ops,...)
    dev->tag = tag;
    dev->no = dev_next_no(tag);
    list_addlast(&DevList, &dev->list);
    return dev;
}

struct dev_s *dev_get(enum dev_tag_e tag, unsigned no)
{
    /**
     * Loop through the list until we find the corresponding entry
     * We could gain some performance by having device-specific linked list
     * but I can't think of a way to do it easily & in a simple manner
    */
    list_foreach(&DevList, item) {
        struct dev_s *dev = list_item(item, struct dev_s, list);
        if (dev->tag == tag && dev->no == no)
            return dev;
    }
    return NULL;
}

void dev_free(struct dev_s *dev, unsigned dsize)
{
    /**
     *  TODO: should we decrement every other device no in the last, ex: should tty 2 become tty 1
     *  if tty 0 is removed ? i don't think so but it could be interesting to think about it
     */
    list_unlink(&dev->list);
    kfree(dev, sizeof(struct dev_s) + dsize);
}
