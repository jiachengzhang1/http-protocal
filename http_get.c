#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <limits.h>

#include "myrequest.h" // my http request implementation

void printHelp()
{
	char *FOUR_SPACES = "    ";
	char *SIX_SPACES = "      ";
	char *urlCmd = "call --url < Full URL >";
	char *helpCmd = "call --help";
	char *profileCmd = "call --profile < Num of Requests > < Full URL >";

	char *urlMsg = "make a http request to the url (\"http://\" prefix is required), for example, http://www.google.com,\n      then print the response.";
	char *helpMsg = "print out the usage.";
	char *profileMsg = "send number of requests to the General Assignment sit by default, then print out the performance,\n      use the optional < Full URL > field to customize URL (\"http://\" prefix is required).";

	printf("usage:\n");
	printf("  Note: It supports \"http\" requests only. \"https\" may result \"301, Moved Pernimally.\"\n\n");
	printf("%s%s\n%s%s\n\n", FOUR_SPACES, urlCmd, SIX_SPACES, urlMsg);
	printf("%s%s\n%s%s\n\n", FOUR_SPACES, helpCmd, SIX_SPACES, helpMsg);
	printf("%s%s\n%s%s\n\n", FOUR_SPACES, profileCmd, SIX_SPACES, profileMsg);
}

void printStat(int succ, int numRequest, int fastestTime,
			   int slowestTime, int totalTime,
			   int medianTime, int smallestSize, int largestSize)
{
	if (succ > 0)
	{
		float succRate = (float)succ / (float)numRequest * 100;

		printf("   Number of requests:      %d\n", numRequest);
		printf("   Fastest time:            %d ms\n", fastestTime);
		printf("   Slowest time:            %d ms\n", slowestTime);
		printf("   Average time:            %d ms\n", totalTime / numRequest);
		printf("   Median time:             %d ms\n", medianTime);
		printf("   Request success rate:    %.0f %%\n", succRate);
		printf("   Smallest response size:  %d bytes\n", smallestSize);
		printf("   Largest response size:   %d bytes\n\n", largestSize);
	}
	else
	{
		printf("   No successful requests.\n");
	}
}

int main(int argc, char **argv)
{
	static char DEFAULT_ADDRESS[100] = "http://assignment.jiachengzhang.workers.dev";
	int option_index = 0;
	int opt;
	int status;
	int ret;
	int size;
	int succ;
	char *addr;
	int numRequest, midRequest, k;
	int slowestTime, fastestTime, totalTime, medianTime, time;
	int smallestSize, largestSize;
	int i;

	static struct option long_options[] = {
		{"url", 1, NULL, 'u'},
		{"help", 0, NULL, 'h'},
		{"profile", 1, NULL, 'p'},
		{0, 0, 0, 0}};

	if (argv[1] == NULL || (argv[1][0] != '-' && argv[1][1] != '-'))
	{
		printf("\"--help\" option tells you how to use it.\n");
		exit(0);
	}

	while ((opt = getopt_long(argc, argv, "a:b:c:d", long_options, &option_index)) != -1)
	{
		switch (opt)
		{
		case 'u':
			addr = argv[optind - 1];
			ret = request(addr, &time, &size, 1);
			break;
		case 'h':
			printHelp();
			break;
		case 'p':
			if (argv[optind] != NULL)
				addr = argv[optind];
			else
				addr = DEFAULT_ADDRESS;
			numRequest = atoi(argv[optind - 1]);
			midRequest = (numRequest + 1) / 2;
			slowestTime = INT_MIN;
			fastestTime = INT_MAX;
			totalTime = 0;
			medianTime = 0;
			k = 0;

			smallestSize = INT_MAX;
			largestSize = INT_MIN;
			succ = 0;

			printf("Profiling host: %s ...\n\n", addr);

			for (i = 0; i < numRequest; i++)
			{
				status = request(addr, &time, &size, 0);
				if (status == 1)
				{
					succ++;
					totalTime += time; // total time spent

					// find slowest time
					if (time > slowestTime)
						slowestTime = time;

					// find fastest time
					if (time < fastestTime)
						fastestTime = time;

					// find median time
					if (succ == midRequest && numRequest % 2 == 1)
						medianTime = time;
					if ((k == 1 || succ == midRequest) && numRequest % 2 == 0)
					{
						medianTime += time;
						if (k == 1)
							medianTime /= 2;
						k++;
					}

					// find smallest response size
					if (size < smallestSize)
						smallestSize = size;

					// find largest response size
					if (size > largestSize)
						largestSize = size;
				}
			}

			printStat(succ, numRequest, fastestTime, slowestTime, totalTime, medianTime, smallestSize, largestSize);

			break;
		default:
			printf("Invalid command, use --help for help.\n");
		}
	}
	return 0;
}