#include <pthread.h>
#include "hybrid_lock.h"

int hybrid_lock_init(struct hybrid_lock *h_lock)
{
	int ret;

	h_lock->pin = 0;
	ret = pthread_mutex_init(&h_lock->m_lock, NULL);
	
	return ret;
}

int hybrid_lock_destroy(struct hybrid_lock *h_lock)
{
	int ret;
	ret = pthread_mutex_destroy(&h_lock->m_lock);
	return ret;
}

int hybrid_lock_lock(struct hybrid_lock *h_lock)
{
	int result, count = 0;
	struct timeval start, end;

	gettimeofday(&start, NULL);
	while(1) {
		if(count % 50000 == 0) {
			gettimeofday(&end, NULL);
			if((end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0) >= (double)1) {
				hybrid_lock_privilege(h_lock);
				return 0;
			}
		}

		result = pthread_mutex_trylock(&h_lock->m_lock);

		if(result == 0 && h_lock->pin > 0) {
			pthread_mutex_unlock(&h_lock->m_lock);
		}

		else if(result == 0 && h_lock->pin == 0) {
			return 0;
		}
		
		count++;
	}
}

int hybrid_lock_unlock(struct hybrid_lock *h_lock)
{
	int ret;
	ret = pthread_mutex_unlock(&h_lock->m_lock);
	return ret;
}

void hybrid_lock_privilege(struct hybrid_lock *h_lock)
{
	h_lock->pin++;
	pthread_mutex_lock(&h_lock->m_lock);
	h_lock->pin--;
}
