// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	printf("Connected to server\n");
		
	char command[1024];
	int c=1;
		while(true){
	printf("\nEnter the command(Enter QUIT for exit)\n");
	fgets(command, 1024, stdin);
	send(sock, command, strlen(command), 0);
	printf("Command: %s", command);
	valread = read(sock, buffer, 1024);
	printf("Output: %s\n", buffer);
	 if(strcmp(buffer,"QUIT")==0)break;
	 for(int i=0;i<1024;i++){
		 buffer[i]=0;
		 command[i]=0;
	 }
		}
	return 0;
}
