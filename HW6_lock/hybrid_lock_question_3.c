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
	int count = 0, cc = 0;
	double elapsed_time = 0;
	struct timeval start, end;

	gettimeofday(&start, NULL);
	while(elapsed_time < (double)1) {
		if(pthread_mutex_trylock(&h_lock->m_lock) == 0) {
			if(h_lock->pin > 0) {
				pthread_mutex_unlock(&h_lock->m_lock);
				break;
			}

			else {
				return 0;
			}
		}
		
		if(count % 50000 == 0) {
			gettimeofday(&end, NULL);
			elapsed_time = (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0);
			cc++;
		}
		count++;
	}
	/*
	gettimeofday(&end, NULL);
	printf("cc: %d time: %f\n", cc, (end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1000000.0));
	cc++;
	*/

	h_lock->pin++;
	pthread_mutex_lock(&h_lock->m_lock);
	h_lock->pin--;
	return 0;
}

int hybrid_lock_unlock(struct hybrid_lock *h_lock)
{
	int ret;
	ret = pthread_mutex_unlock(&h_lock->m_lock);
	return ret;
}
