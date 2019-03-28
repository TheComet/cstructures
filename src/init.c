#include "cstructures/init.h"
#include "cstructures/memory.h"

/* ------------------------------------------------------------------------- */
int
cstructures_init(void)
{
    return cstructures_memory_init();
}

/* ------------------------------------------------------------------------- */
void
cstructures_deinit(void)
{
    cstructures_memory_deinit();
}
