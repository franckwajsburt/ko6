/**
 * To simplify device operations, this module provides functions and structures to allocate
 * and release devices dynamically. When the platform is initialized (arch_init) it should detect
 * the hardware and allocates as much devices as it needs. The devices are stored in the kernel heap
 * and linked to each other in a global linked-list (devList). Each device has a "major" number
 * (its tag) indicating his type (tty, icu, ...) and a minor number, which is incremented everytime
 * the a new device of the same type is created. Example:
 * 
 * When the OS starts we have:
 *      devList
 * .-------------------.
 * | .next = &devList  |
 * | .prev = &devList  |
 * .-------------------.
 * 
 * We then allocate a TTY: dev_alloc(TTY_DEV, sizeof(struct tty_s))
 *      devList                tty0
 * .---------------.      .------------------.
 * | .next = tty0  . <--- | .next = devList; |
 * | .prev = tty0  . ---> | .prev = devList; |
 * .---------------.      .------------------.
 *                        | .tag = TTY_DEV   |
 *                        | .no  = 0         |
 *                        .------------------.
 * 
 * Then an ICU: dev_alloc(ICU_DEV, sizeof(struct icu_s))
 *      devList                tty0                    icu0
 * .---------------.      .------------------.      .-------------------.
 * | .next = tty0  . <--- | .next = icu0;    | <--- | .next = devList;  |
 * | .prev = icu0  . ---> | .prev = devList; | ---> | .prev = tty0;     |
 * .---------------.      .------------------.      .-------------------.
 *                        | .tag = TTY_DEV   |      | .tag = ICU_DEV    |
 *                        | .no  = 0         |      | .no  = 0          |
 *                        .------------------.      .-------------------.
 * 
 * And another TTY: dev_alloc(TTY_DEV, sizeof(struct tty_s))
 *      devList                tty0                    icu0                      tty1
 * .---------------.      .------------------.      .-------------------.      .------------------.
 * | .next = tty0  . <--- | .next = icu0;    | <--- | .next = tty1;     | <--- | .next = devList; |
 * | .prev = icu0  . ---> | .prev = devList; | ---> | .prev = tty0;     | ---> | .prev = icu0;    |
 * .---------------.      .------------------.      .-------------------.      .------------------.
 *                        | .tag = TTY_DEV   |      | .tag = ICU_DEV    |      | .tag = TTY_DEV   |
 *                        | .no  = 0         |      | .no  = 0          |      | .no = 1          |
 *                        .------------------.      .-------------------.      .------------------.
 * 
 * Since this module manipulates only dev_s structures, each device should provide macro to 
 * simplify their manipulation, by example, tty module should provide:
 * #define tty_alloc() (struct tty_s*) (dev_alloc(TTY_DEV, sizeof(struct tty_s))->data)
 * #define tty_get(no) (struct tty_s*) (dev_get(TTY_DEV, no)->data)
*/
#ifndef _HAL_DEV_H_
#define _HAL_DEV_H_

#include <common/list.h>
#include <kernel/klibc.h>

enum dev_tag_e {
    CHAR_DEV,
    ICU_DEV,
    DMA_DEV,
    TIMER_DEV
};

struct dev_s {
    enum dev_tag_e tag; // Identify the type of the device (tty, icu, ...)
    unsigned no;        // Minor device number (tty0, tty1, ...)
    list_t list;        // List entry in the global device list
    char data[];        // Device specific data, should be filled with (struct tty_s, struct icu_s, ...)
};

extern list_t devList;

/**
 * \brief   Find the last element added with corresponding tag
 *          To do so, we loop through the device list in a reverse order
 *          Once we found the last device, we add one to his number
 * \param   tag type of the device (tty, icu, ...)
 * \return  the next no (last device of this type no + 1)
*/
unsigned dev_next_no(enum dev_tag_e tag);

/**
 * \brief   Allocate enough size in kernel heap to store device meta data (tag, no, list)
 *          and device data (struct tty_s, struct icu_s, ...) and add it into the global device
 *          list
 * \param   tag type of the device (tty, icu, ...)
 * \param   size size of the device-specific structure (ex: sizeof(struct_s))
 * \return  the allocated device
*/
struct dev_s *dev_alloc(enum dev_tag_e tag, unsigned dsize);

/**
 * \brief   Get a device based on its type and on its minor number
 * \param   tag type of the device
 * \param   no minor number of the device
 * \return  the corresponding device if found, NULL if not
*/
struct dev_s *dev_get(enum dev_tag_e tag, unsigned no);

/**
 * \brief   Release a created device (kfree + list_unlink)
 * \param   dev the device to release
 * \param   dsize size of device-specific structure
 * \return  nothing
*/
void          dev_free(struct dev_s *dev, unsigned dsize);

#endif