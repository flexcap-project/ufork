/*******************************************************************************
 *  The BYTE UNIX Benchmarks - Release 3
 *          Module: spawn.c   SID: 3.3 5/15/91 19:30:20
 *
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Ben Smith, Rick Grehan or Tom Yagerat BYTE Magazine
 *	ben@bytepb.byte.com   rick_g@bytepb.byte.com   tyager@bytepb.byte.com
 *
 *******************************************************************************
 *  Modification Log:
 *  $Header: spawn.c,v 3.4 87/06/22 14:32:48 kjmcdonell Beta $
 *  August 29, 1990 - Modified timing routines (ty)
 *  October 22, 1997 - code cleanup to remove ANSI C compiler warnings
 *                     Andy Kahn <kahn@zk3.dec.com>
 *
 ******************************************************************************/
char SCCSid[] = "@(#) @(#)spawn.c:3.3 -- 5/15/91 19:30:20";
/*
 *  Process creation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sfork.h>

unsigned long start;
unsigned long end;
unsigned long mid;

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
	int	slave, duration;
	int	status;
	char *fixed_workload = getenv("UNIXBENCH_FIXED_WORKLOAD");

    duration = 100;

	iter = 0;
	start = ustime();
	while (1) {
		if ((slave = DO_SFORK()) == 0) {
			/* slave .. boring */
			/* kill it right away */
            sfork_syscalls.sfork_exit(0);
			//exit(0);
		} else if (slave < 0) {
			/* woops ... */
			fprintf(stderr,"Fork failed at iteration %lu\n", iter);
			perror("Reason");
			sfork_syscalls.sfork_exit(2);
		} else {
			/* master */
			//mid = ustime();
			//fprintf(stderr, "parent latency %ld\n", mid - start);
			sfork_syscalls.yield();
			//while (!sfork_syscalls.sfork_wait3(&status, 0, NULL));
			sfork_syscalls.sfork_wait3(&status, 0, NULL);
		}
		if (status != 0) {
			fprintf(stderr,"Bad wait status: 0x%x\n", status);
			return 2;
		}
		iter++;
		if (iter == duration) {
			report();
            return 0;
        }
		}
}