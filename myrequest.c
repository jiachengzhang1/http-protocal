#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int httpRequest(char *host, char *path, int *time, int *size, int print)
{
	struct timeval start, end;
	unsigned long timer = 0;

	// get host name and ip
	struct hostent *h = gethostbyname(host);
	if (h == NULL || h->h_addrtype != AF_INET)
	{
		printf("ERROR: Invalid URL.\n");
		return -1;
	}

	struct in_addr ina;
	memmove(&ina, h->h_addr, 4);
	char *ipstr = inet_ntoa(ina);

	// construct socket
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	server_addr.sin_addr.s_addr = inet_addr(ipstr);
	int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client_socket == -1 || client_socket == -2)
	{
		printf("ERROR: There's a problem creating a socket.\n");
		return -1;
	}

	// connect to the server
	gettimeofday(&start, NULL);
	int res = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
	gettimeofday(&end, NULL);
	timer += (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
	if (res == -1)
	{
		printf("ERROR: There's a problem connecting to \"%s\".\n", host);
		return -1;
	}

	char request[1024];
	char *response = malloc(sizeof(char) * 4096);
	int totalSize = 0;

	// construct request header
	sprintf(request, "GET %s HTTP/1.1\r\nHost:%s\r\nConnection:Close\r\n\r\n", path, host);

	// send request
	int ret = send(client_socket, request, strlen(request), 0);
	if (ret < 0)
	{
		printf("There's a probem sending request to \"%s\".\n", host);
		return -1;
	}

	// set request timeout
	struct timeval timeout = {3, 0};
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	// get response
	while (ret > 0)
	{
		const int bufsize = 4096;
		char *buf = (char *)calloc(bufsize - 1, 1);
		gettimeofday(&start, NULL);
		ret = recv(client_socket, buf, bufsize - 1, 0);
		gettimeofday(&end, NULL);
		timer += (1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec) / 1000;
		if (ret < 0)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				printf("ERROR: Recv timeout, error code %d.\n", errno);
			}
			else if (errno == EINTR)
			{
				printf("ERROR: Interrupt by signal, error code %d.\n", errno);
				continue;
			}
			else if (errno == ENOENT)
			{
				printf("ERROR: Recv RST segement, error code %d.\n", errno);
			}
			else
			{
				printf("ERROR: Unknown error! error code %d.\n", errno);
			}
			return -1;
		}

		totalSize += ret;
		const size_t len1 = strlen(buf);
		const size_t len2 = strlen(response);
		response = realloc(response, len1 + len2 + 1);
		strcat(response, buf);
		free(buf);
	}

	*time = (int)timer;
	*size = totalSize;

	if (print)
		printf("%s", strstr(response, "\r\n\r\n")); // print response body

	// clean up
	free(response);
	close(client_socket);
	return 1;
}

int isUrlValid(char *addr)
{
	return !(addr[0] != 'h' || addr[1] != 't' ||
			 addr[2] != 't' || addr[3] != 'p' ||
			 addr[4] != ':' || addr[5] != '/' || addr[6] != '/');
}

int request(char *addr, int *time, int *size, int print)
{
	char address[100], path[100];
	int status;
	if (!isUrlValid(addr))
	{
		printf("ERROR: \"http\" prefix is required in the given URL.\n");
		return -1;
	}

	status = sscanf(addr, "%*[^:]%*[:/]%[^/]%s", address, path);
	if (status == 1)
		return httpRequest(address, "/", time, size, print);

	if (status == 2)
		return httpRequest(address, path, time, size, print);

	return -1;
}