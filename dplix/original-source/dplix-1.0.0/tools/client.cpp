//
// An attempt to connect to the Dell 3010cn printer
// by passing in an intercepted message...
//

#include <stdio.h>                /* perror() */
#include <stdlib.h>               /* atoi() */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>               /* read() */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char *argv[])
{
//	int clientSocket,
//	remotePort,
//	status = 0;
//	struct hostent *hostPtr = NULL;
//	struct sockaddr_in serverName = { 0 };
//	char buffer[256] = "";
//	char *remoteHost = NULL;

	if (2 != argc)
	{
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

//	remoteHost = argv[1];
//	remotePort = atoi(argv[2]);

	printf("Creating client socket...\n");

	int clientSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (-1 == clientSocket)
	{
		herror("socket()");
		exit(1);
	}

	// need to resolve the remote server name or IP address
	printf("Resolving server...\n");
	char remoteHost[] = "192.168.0.10";
	int remotePort = 9100;

	hostent * hostPtr = gethostbyname(remoteHost);

	if (NULL == hostPtr)
	{
		// This doesn't work.
		hostPtr = gethostbyaddr(remoteHost, strlen(remoteHost), AF_INET);

		if (NULL == hostPtr)
		{
			herror("Error resolving server address");
			exit(1);
		}
	}

	printf("Connecting to server...\n");

	sockaddr_in serverName = { 0 };

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(remotePort);
	memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

	int status = connect(clientSocket, (sockaddr *)&serverName, sizeof(serverName));

	if (-1 == status)
	{
		herror("connect()");
		exit(1);
	}

	// Write payload to printer here...

	printf("Writing payload to server...\n");

	FILE * fp = fopen(argv[1], "rb");

	if (NULL == fp)
	{
		perror("fopen()");
	}

	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	rewind(fp);

	char * payload = new char[fileSize];

	if (fread(payload, 1, fileSize, fp) == fileSize)
	{
//		printf("Need to send this stuff over the wire...[%u bytes]\n", fileSize);
//		int num = write(clientSocket, (void *)((char *)(buffer + bytesWritten)),
//			bytesToSend - bytesWritten);
		int num = write(clientSocket, (void *)payload, fileSize);

		if (num < 0)
		{
			perror("write()");

//			if (errno != EINTR)
//			{
//				exit(1);
//			}
		}
		else
		{
			printf("Wrote %u bytes to the server.\n", num);
		}
	}

	delete[] payload;
	fclose(fp);
#if 0
	/*
	 * Client application specific code goes here
	 *
	 * e.g. receive messages from server, respond,
	 * etc. */
	while (0 < (status = read(clientSocket, buffer, sizeof(buffer) - 1)))
	{
		printf("%d: %s", status, buffer);
	}

	if (-1 == status)
	{
		perror("read()");
	}
#endif

	close(clientSocket);

	return 0;
}

#if 0
/*
 * Listing 3:
 * Handling short writes
 * Ivan Griffin (ivan.griffin@ul.ie)
 */
int bytesToSend = 0,
    bytesWritten = 0,
    num = 0;

/*
 * somewhere here bytesToSend, buffer, and
 * fileDesc must be set up.
 */

for (bytesWritten = 0; bytesWritten < bytesToSend;
     bytesWritten += num)
{
    num = write(fileDesc,
        (void *)( (char *)buffer +
      (char *)bytesWritten ),
        bytesToSend - bytesWritten);

    if (num < 0)
    {
        perror("write()");

        if (errno != EINTR)
        {
            exit(1);
        }
    }
}
#endif
