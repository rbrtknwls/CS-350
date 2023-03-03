#include "assignment.h"
#include <stdbool.h>
#include <stdio.h>

bool isBelowRatio(struct resource *resource) {
    return resource->num_consumers > resource->num_producers * resource->ratio;
}

void
consume_enter(struct resource *resource)
{
    printf("CONSUME ENTER [p: %ld, c: %ld]\n", resource->num_consumers,  resource->num_producers);
    // FILL ME IN
    /*pthread_mutex_lock(&resource->mutex);
    if (isBelowRatio(resource))
        pthread_cond_wait(&resource->cond, &resource->mutex);
    resource->num_consumers += 1;
    pthread_cond_signal(&resource->cond);
    pthread_mutex_unlock(&resource->mutex);*/

}

void
consume_exit(struct resource *resource)
{
    // Consumer can always exit (increase ratio)
    printf("CONSUME EXIT [p: %ld, c: %ld]\n", resource->num_consumers,  resource->num_producers);
    pthread_mutex_lock(&resource->mutex);
    resource->num_consumers -= 1;
    pthread_cond_signal(&resource->cond);
    pthread_mutex_unlock(&resource->mutex);
}

void
produce_enter(struct resource *resource)
{
    // Producers can always enter (increase ratio)
    printf("PRODUCE ENTER [p: %ld, c: %ld]\n", resource->num_consumers,  resource->num_producers);
    // FILL ME IN
    pthread_mutex_lock(&resource->mutex);
    resource->num_producers += 1;
    pthread_cond_signal(&resource->cond);
    pthread_mutex_unlock(&resource->mutex);
}

void
produce_exit(struct resource *resource)
{
    printf("PRODUCE EXIT [p: %ld, c: %ld]\n", resource->num_consumers,  resource->num_producers);
    /*pthread_mutex_lock(&resource->mutex);

    if (isBelowRatio(resource))
        pthread_cond_wait(&resource->cond, &resource->mutex);
    resource->num_consumers += 1;
    pthread_mutex_unlock(&resource->mutex);*/

}


