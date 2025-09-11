
/*******************************************************************************
 *  The BYTE UNIX Benchmarks - Release 3
 *          Module: context1.c   SID: 3.3 5/15/91 19:30:18
 *
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Ben Smith, Rick Grehan or Tom Yager
 *	ben@bytepb.byte.com   rick_g@bytepb.byte.com   tyager@bytepb.byte.com
 *
 *******************************************************************************
 *  Modification Log:
 *  $Header: context1.c,v 3.4 87/06/22 14:22:59 kjmcdonell Beta $
 *  August 28, 1990 - changed timing routines--now returns total number of
 *                    iterations in specified time period
 *  October 22, 1997 - code cleanup to remove ANSI C compiler warnings
 *                     Andy Kahn <kahn@zk3.dec.com>
 *
 ******************************************************************************/
char SCCSid[] = "@(#) @(#)context1.c:3.3 -- 5/15/91 19:30:18";
/*
 *  Context switching via synchronized unbuffered pipe i/o
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sfork.h>
#include <uk/sched.h>

unsigned long start;
unsigned long end;

long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

unsigned long iter;

void report()
{
	end = ustime();
	fprintf(stderr, "COUNT|%lu|1|lps, time taken %ld us\n", iter, end - start);
	//exit(0);
}

int main(argc, argv)
int	argc;
char	*argv[];
{
	int duration;
	unsigned long	check;
	int	p1[2], p2[2];
	ssize_t ret;
	char *fixed_workload = getenv("UNIXBENCH_FIXED_WORKLOAD");

	// if (argc != 2) {
	// 	fprintf(stderr, "Usage: context duration\n");
	// 	exit(1);
	// }

	// duration = atoi(argv[1]);

	/* set up alarm call */
    duration = 100000;
	iter = 0;
	//  if (fixed_workload == NULL)
	//  	wake_me(duration, report);

	//signal(SIGPIPE, SIG_IGN);

	if (pipe(p1) || pipe(p2)) {
		perror("pipe create failed");
		exit(1);
	}

	start = ustime();

	if (DO_SFORK()) {	/* parent process */
		while (1) {
			if ((ret = write(p1[1], (char *)&iter, sizeof(iter))) != sizeof(iter)) {
				if ((ret == -1) && (errno == EPIPE)) {
					alarm(0);
					report();
                    return 11;
				}
				if ((ret == -1))
					perror("master write failed");
				return 12;
			}
			sfork_syscalls.yield();
			if ((ret = read(p2[0], (char *)&check, sizeof(check))) != sizeof(check)) {
				if ((ret == 0)) { /* end-of-stream */
					alarm(0);
					report(); 
                    return 0;
				}
				if ((ret == -1))
					perror("master read failed");
				return 13;
			}
			//sfork_syscalls.printf("chck %d, itr %d\n", check, iter);
			if (check != iter) {
				fprintf(stderr, "Master sync error: expect %lu, got %lu\n",
					iter, check);
				return 2;
			}
			iter++;
			if (iter == duration) {
				report();
                return 0;
            }
            sfork_syscalls.yield();
		}
	}
	else { /* child process */
		while (1) {
			if ((ret = read(p1[0], (char *)&check, sizeof(check))) != sizeof(check)) {
				if ((ret == 0)) { /* end-of-stream */
					alarm(0);
					report(); 
                    return 0;
				}
				if ((ret == -1))
					perror("slave read failed");
				sfork_syscalls.sfork_exit(1);
			}
			//sfork_syscalls.printf("check %d, iter %d\n", check, iter);
			if (check != iter) {
				fprintf(stderr, "Slave sync error: expect %lu, got %lu\n",
					iter, check);
				sfork_syscalls.sfork_exit(2);
			}
			sfork_syscalls.yield();
			if ((ret = write(p2[1], (char *)&iter, sizeof(iter))) != sizeof(check)) {
				if ((ret == -1) && (errno == EPIPE)) {
					alarm(0);
					report(); 
                    return 0;
				}
				if ((ret == -1))
					perror("slave write failed");
				sfork_syscalls.sfork_exit(1);
			}
			iter++;
            if (iter == duration) {
              sfork_syscalls.sfork_exit(0);
            }
            sfork_syscalls.yield();
		}
	}
}