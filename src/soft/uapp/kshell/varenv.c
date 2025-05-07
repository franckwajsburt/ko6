#include <varenv.h>

struct varenv *varenv_create(void)
{
    struct varenv *new = malloc(sizeof*new);
    new->v = NULL;
    new->attr = 0;

    return new;
}


void varenv_destroy(struct varenv *victim)
{
    if (!victim) return;

    if (victim->v) free(victim->v);

    free(victim);
}


int varenv_value_set(struct varenv *var, const char * v)
{
    if (!var) return 0;

    if (var->v) free(var->v);

    var->v = strdup(v);
    var->attr = 0;

    return 1;
}


int varenv_attr_set(struct varenv *var, int mask)
{
    if (!var) return 0;

    var->attr |= mask; // note that it's just a logic OR

    return 1;
}


int varenv_attr_unset(struct varenv *var, int mask)
{
    if (!var) return 0;

    var->attr &= ~mask;

    return 1;
}
