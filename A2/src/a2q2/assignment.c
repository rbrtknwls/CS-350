#include "assignment.h"
#include <stdbool.h>
#include <stdio.h>

bool isBelowRatio(long num_consumers, long num_producers, int ratio) {
    return num_consumers > num_producers * ratio;
}

void
consume_enter(struct resource *resource)
{
    // FILL ME IN
    pthread_mutex_lock(&resource->mutex);
    while (isBelowRatio(resource->num_consumers + 1, resource->num_producers, resource->ratio))
        pthread_cond_wait(&resource->cond, &resource->mutex);
    resource->num_consumers += 1;
    pthread_cond_signal(&resource->cond);

}

void
consume_exit(struct resource *resource)
{
    // Consumer can always exit (increase ratio)
    resource->num_consumers -= 1;
    pthread_cond_signal(&resource->cond);
    pthread_mutex_unlock(&resource->mutex);
}

void
produce_enter(struct resource *resource)
{
    // Producers can always enter (increase ratio)
    // FILL ME IN
    pthread_mutex_lock(&resource->mutex);
    resource->num_producers += 1;
    pthread_cond_signal(&resource->cond);

}

void
produce_exit(struct resource *resource)
{

    while (resource->num_consumers != 0)
        pthread_cond_wait(&resource->cond, &resource->mutex);
    resource->num_producers -= 1;
    pthread_mutex_unlock(&resource->mutex);
    pthread_cond_signal(&resource->cond);

}


