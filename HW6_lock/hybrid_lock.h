#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

struct hybrid_lock {
	int pin;
	pthread_mutex_t m_lock;
};
int hybrid_lock_init(struct hybrid_lock *h_lock);
int hybrid_lock_destroy(struct hybrid_lock *h_lock);
int hybrid_lock_lock(struct hybrid_lock *h_lock);
int hybrid_lock_unlock(struct hybrid_lock *h_lock);

// Mutex lock with privilege
void hybrid_lock_privilege(struct hybrid_lock *h_lock);
