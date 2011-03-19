#include "tests/lib.h" 

#define QUEUE_MAX_MEMBERS 20

#define QUEUE_LEN (QUEUE_MAX_MEMBERS+1)
#define NEXTPOS(n) ((n+1) % QUEUE_LEN)

typedef int customer_t;
typedef int haircutter_t;

usr_cond_t queue_cond;
usr_lock_t queue_lock;

customer_t queue[QUEUE_LEN]; /* Circular buffer */

int queue_start_pos; /* First element on queue */
int queue_end_pos;   /* Last element on queue+1 */


int get_customer_id(customer_t customer) {
    return (int) customer;
}

int get_haircutter_id(haircutter_t haircutter) {
    return (int) haircutter;
}

void customer(customer_t customer) {
    syscall_lock_acquire(&queue_lock);

    /* Check if queue is full */
    if(NEXTPOS(queue_end_pos) == queue_start_pos) {
        pprintf("Customer %d is leaving because the queue is full\n", get_customer_id(customer));
        syscall_lock_release(&queue_lock);
        syscall_exit(-1);
    }

    pprintf("Customer %d is entering queue\n", get_customer_id(customer));

    /* Insert into queue */
    queue[queue_end_pos] = customer;
    queue_end_pos = NEXTPOS(queue_end_pos);

    /* Signal, release lock and exit */
    syscall_condition_signal(&queue_cond);
    syscall_lock_release(&queue_lock);
    syscall_exit(0);
}

customer_t get_customer(haircutter_t haircutter) {
    /* Get lock and wait for customers */
    syscall_lock_acquire(&queue_lock);
    while(queue_start_pos == queue_end_pos) {
        pprintf("Haircutter %d is going to sleep\n", get_haircutter_id(haircutter));
        syscall_condition_wait(&queue_cond, &queue_lock);
    }

    /* Get customer */
    customer_t customer = queue[queue_start_pos];

    /* Increase start pos */
    queue_start_pos = NEXTPOS(queue_start_pos);

    /* Release lock and return */
    syscall_lock_release(&queue_lock);
    return customer;
}

void do_cutting(haircutter_t haircutter, customer_t customer) {
    pprintf("Haircutter %d is beginning to cut customer %d\n",
            get_haircutter_id(haircutter),
            get_customer_id(customer));

    /* Wait */
    int n;
    for(n = 0; n < 100000; n++) { /* no-op */ }

    pprintf("Haircutter %d is finished to cut customer %d\n",
            get_haircutter_id(haircutter),
            get_customer_id(customer));

}

void haircutter(haircutter_t haircutter) {
    while(1) {
        customer_t customer = get_customer(haircutter);
        do_cutting(haircutter, customer);
    }
}


int main(void) {
    int n;

    syscall_lock_create(&queue_lock);
    syscall_condition_create(&queue_cond);

    queue_start_pos = queue_end_pos = 0;

    syscall_fork(haircutter, 0);
    syscall_fork(haircutter, 1);
    syscall_fork(haircutter, 2);

    for(n = 0; n <= 6; n++)
        syscall_fork(customer, n);

    return 0;
}
