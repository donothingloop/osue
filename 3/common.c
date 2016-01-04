#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

sig_atomic_t exitsig = 0;
const char *progname = NULL;

int shm_fd = -1;
struct SharedStructure *shared = NULL;

const char *SEM_NAME[] = { "/battlesA", "/battles1", "/battles2", "/battlesZ", "/battlesS" };
sem_t *sem[] = { SEM_FAILED, SEM_FAILED, SEM_FAILED, SEM_FAILED, SEM_FAILED };

const char FIELD_CHAR[] = { '#' , '~', 'x', ' ' };

void usage(void)
{
	(void)fprintf(stderr, "USAGE: %s\n", progname);
}

void bail_out(const char *s)
{
	fprintf(stderr, "error: function '%s'\n",s);
	exit(EXIT_FAILURE);
}

int allocate_shared(int owner)
{
	const int oflag = (owner == 0 ? 0 : O_CREAT) | O_RDWR;
	shm_fd = shm_open(SHMEM_NAME, oflag, 0600);
	if(shm_fd == -1) return 0;

	if(owner != 0){
		if(ftruncate(shm_fd, sizeof(*shared)) == -1) return 0;
	}
	shared = mmap(NULL, sizeof(*shared), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
	(void)close(shm_fd);
	
	if(shared == MAP_FAILED){
		shared = NULL;
		return 0;
	}
	return 1;
}

int ship_check(const struct Ship *const ship)
{
	int i;
	const struct Coord *c = ship->c;

	if(c[0].x < 0 || c[0].y < 0 || c[0].x >= FIELD_W || c[0].y >= FIELD_H)
		return 0;

	for(i = 1; i < SHIP_COORDS; i++){
		if(c[i].x < 0 || c[i].y < 0 || c[i].x >= FIELD_W || c[i].y >= FIELD_H)
			return 0;

		if(abs(c[i].x - c[i-1].x) != 1 || abs(c[i].y - c[i-1].y) != 1)
			return 0;
	}
	return 1;

}

static void (*signal_handler_cb)(int);

static void minimal_handler(int signr){
	exitsig = 1;
	if(signal_handler_cb != NULL)
		signal_handler_cb(signr);
}

int setup_signal_handler(void (*handler)(int))
{
	struct sigaction sa = {
		.sa_handler = minimal_handler,
		.sa_flags = SA_NOCLDSTOP
	};

	signal_handler_cb = handler;

	if(sigfillset(&sa.sa_mask) < 0)
		return 0;

	return sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1 ? 0 : 1;
}

void free_common_ressources(void)
{
	int i;
	for(i = 0; i < LEN(sem); i++){
		if(sem[i] != NULL){
			(void)sem_close(sem[i]);
			(void)sem_unlink(SEM_NAME[i]);
			sem[i] = SEM_FAILED;
		}
	}

	if(shm_fd != -1){
		if(shared != NULL){
			munmap(shared, sizeof(*shared));
			shared = NULL;
		}
		(void)shm_unlink(SHMEM_NAME);
	}
	
}

void sem_wait_cb(sem_t *s, void (*cb)(void))
{
	int x;
	do{
		x = sem_wait(s);
		if(exitsig != 0) cb();
	}while(x == -1 && errno == EINTR);
}

