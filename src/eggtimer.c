#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static struct {
	enum{
		TIMER_EXIT,
		TIMER_BELL,
		TIMER_EXEC
	} mode;
	int paused;
	unsigned remaining;
	char **exec_command;
} timer_state;

void print_usage_message(const char *name){
	fprintf(stderr,
			"Usage: %s [-be] [-h hours] [-m minutes] [-s seconds]"
			" [exec command]\n"
			"Options:\n"
			"\t-b - Emit a BEL character when timer completes\n"
			"\t\tNote: Cannot be used with -e\n"
			"\t-e - Exec a command when timer completes\n"
			"\t\tCommand to exec should be given after options\n"
			"\t\tNote: Cannot be used with -b\n"
			"\t-h - Specifies a number of hours for timer\n"
			"\t-m - Specifies a number of minutes for timer\n"
			"\t-s - Specifies a number of seconds for timer\n"
			"If neither -b nor -e is specified, %s will exit when timer"
			" completes\n"
			"While the timer is running, it can be paused by sending the"
			" process SIGINT\n"
			"(Ctrl+C on most terminals)\n"
			"Sending SIGINT again will resume\n",
			name, name);
}

void sig_handler(int sig){
	unsigned int remaining;

	/* Pause alarm, if still running */
	remaining = alarm(0);

	/* Handle signal */
	switch(sig){
		/* Alarm triggered */
		case SIGALRM:
			switch(timer_state.mode){
				case TIMER_BELL:
					write(1, "\a", 1);
					/* FALLTHRU */
				case TIMER_EXIT:
					exit(0);
				case TIMER_EXEC:
					execvp(	timer_state.exec_command[0],
							timer_state.exec_command);
					fputs("Error: Exec failed\n", stderr);
					exit(-1);
			}
			break;
		/* Pause signal sent */
		case SIGINT:
			if(timer_state.paused){
				puts("Timer resumed");
				timer_state.paused = 0;
				alarm(timer_state.remaining);
			} else{
				printf("Timer paused, %u seconds remaining\n", remaining);
				timer_state.paused = 1;
				timer_state.remaining = remaining;
			}
			break;
	}
}

int main(int argc, char **argv){
	char *check;
	int opt, timer_set;
	unsigned int timer;
	unsigned long temp;

	/* Check for options */
	if(argc == 1){
		print_usage_message(argv[0]);
		return -1;
	}

	/* Clear timer */
	timer = 0;
	timer_set = 0;
	timer_state.mode = TIMER_EXIT;
	timer_state.paused = 0;

	/* Read options */
	while((opt = getopt(argc, argv, "h:m:s:eb")) != -1){
		switch(opt){
			case 'h':
				if(*optarg == '-'){
					fputs("Error: Negative hours not permitted\n", stderr);
					return -1;
				}
				temp = strtoul(optarg, &check, 10);
				if(*check != '\0'){
					fputs("Error: Invalid hours\n", stderr);
					return -1;
				}
				if(temp > UINT_MAX || (temp == ULONG_MAX && errno)){
					fputs("Error: Hours out of range\n", stderr);
					return -1;
				}
				if((temp * 3600) > (UINT_MAX - timer)){
					fputs("Error: Timer value overflow\n", stderr);
					return -1;
				}
				timer += temp * 3600;
				timer_set = 1;
				break;
			case 'm':
				/* Pre-check for negtive input */
				if(*optarg == '-'){
					fputs("Error: Negative minutes not permitted\n", stderr);
					return -1;
				}

				/* Convert input to number */
				temp = strtoul(optarg, &check, 10);
				if(*check != '\0'){
					fputs("Error: Invalid minutes\n", stderr);
					return -1;
				}
				if(temp > UINT_MAX || (temp == ULONG_MAX && errno)){
					fputs("Error: Minutes out of range\n", stderr);
					return -1;
				}

				/* Check if we will overflow our timer */
				if((temp * 60) > (UINT_MAX - timer)){
					fputs("Error: Timer value overflow\n", stderr);
					return -1;
				}

				/* Set the timer */
				timer += temp * 60;
				timer_set = 1;
				break;
			case 's':
				/* Pre-check for negtive input */
				if(*optarg == '-'){
					fputs("Error: Negative seconds not permitted\n", stderr);
					return -1;
				}

				/* Convert input to number */
				temp = strtoul(optarg, &check, 10);
				if(*check != '\0'){
					fputs("Error: Invalid seconds\n", stderr);
					return -1;
				}
				if(temp > UINT_MAX || (temp == ULONG_MAX && errno)){
					fputs("Error: Seconds out of range\n", stderr);
					return -1;
				}

				/* Check if we will overflow our timer */
				if(temp > (UINT_MAX - timer)){
					fputs("Error: Timer value overflow\n", stderr);
					return -1;
				}

				/* Set the timer */
				timer += temp;
				timer_set = 1;

				break;
			case 'e':
				timer_state.mode = TIMER_EXEC;
				break;
			case 'b':
				timer_state.mode = TIMER_BELL;
				break;
			default:
				print_usage_message(argv[0]);
				return -1;
		}
	}

	/* Make sure we have a time */
	if(!timer_set || !timer){
		fputs("Error: No time given\n", stderr);
		return -1;
	}

	/* Set action */
	if(timer_state.mode == TIMER_EXEC){
		if(argv[optind] == NULL){
			fputs("Error: Exec mode specified, but no command given\n", stderr);
			return -1;
		}
		timer_state.exec_command = argv + optind;
	}

	/* Start signal handler */
	signal(SIGALRM, sig_handler);
	signal(SIGINT, sig_handler);
	
	/* Start timer */
	alarm(timer);

	/* Wait for timer */
	for(;;){
		pause();
	}

	return 0;
}
