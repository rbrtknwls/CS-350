#include "assignment.h"

void
consume_enter(struct resource *resource)
{
    // FILL ME IN

    resource->num_consumers += 1;
}

void
consume_exit(struct resource *resource)
{
    // Consumer can always exit

    resource->num_consumers -= 1;
}

void
produce_enter(struct resource *resource)
{
    // Producers can
    // FILL ME IN
    resource->num_producers += 1;

}

void
produce_exit(struct resource *resource)
{
    // FILL ME IN
    resource->num_consumers -= 1;

}


