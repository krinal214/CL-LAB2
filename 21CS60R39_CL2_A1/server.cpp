#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
#define server_file "server_file.txt"

using namespace std;

/*utility function to remove a character from a string*/
void remove_char(char *buffer, char c)
{
	int n = strlen(buffer);
	int j = 0;
	for (int i = 0; i < n; i++)
	{
		if (buffer[i] != c)
		{
			buffer[j++] = buffer[i];
		}
	}
	buffer[j] = '\0';
}


/*utility function to return index of first occurence of character in string*/
int first_occur(char *buffer, char c)
{
	int idx = -1;
	int n = strlen(buffer);
	for (int i = 0; i < n; i++)
	{
		if (buffer[i] == c)
		{
			idx = i;
			break;
		}
	}
	if (idx == -1)
		idx = n;
	return idx;
}


/*Utility function to determine wether a string is valid integer or not*/
bool is_int(char *buffer)
{
	int p = 0;
	bool flag = true;
	if (buffer[0] == '-' || (buffer[0] >= '0' && buffer[0] <= '9'))
	{
	}
	else
	{
		flag = false;
		return flag;
	}
	int n = strlen(buffer);
	for (int i = 1; i < n; i++)
	{
		if (buffer[i] < '0' || buffer[i] > '9')
		{
			flag = false;
			return flag;
		}
	}
	return true;
}
/*Utility function for NLINEX command*/
int no_of_lines()
{
	FILE *fp = fopen(server_file, "r");
	int count = 0;
	char temp_msg[1024] = {0};
	while (fgets(temp_msg, 1024, fp) != NULL)
	{
		count++;
	}
	fclose(fp);
	return count;
}

/*Utility function for READX command*/
void read_line(int idx, char *buffer)
{

	FILE *fp = fopen(server_file, "r");
	int count = 0;
	char c;
	int j = 0;
	char temp_msg[1024] = {0};
	while (fgets(temp_msg, 1024, fp) != NULL)
	{
		if (count == idx)
		{
			strcpy(buffer, temp_msg);
			remove_char(buffer, '\n');
			break;
		}
		count++;
	}
	fclose(fp);
}
/*Utility function for INSERTX command*/
void update_file(int idx, char *new_msg, int n)
{
	FILE *f1, *f2;
	/*special case
	if total lines are n. Thus valid index is [0,n-1]. 
	So, if line has to inserted at index n. we just need to append it 
	*/
	if (n == idx)
	{
		f1 = fopen(server_file, "a");
		if (n != 0)
		{
			fprintf(f1, "%c", '\n');
		}
		fprintf(f1, "%s", new_msg);

		fclose(f1);
		return;
	}
	f1 = fopen(server_file, "r");
	f2 = fopen("temp.txt", "w");
	int count = 0;
	char c;
	char temp_msg[1024] = {0};
	/*make changes in the temporary files and later replace current file by temporary file*/
	while (fgets(temp_msg, 1024, f1) != NULL)
	{
		if (count < idx)
		{
			fprintf(f2, "%s", temp_msg);
		}
		else if (count == idx)
		{
			fprintf(f2, "%s", new_msg);
			fprintf(f2, "%c", '\n');
			fprintf(f2, "%s", temp_msg);
		}
		else
		{
			fprintf(f2, "%s", temp_msg);
		}
		count++;
	}

	fclose(f2);
	fclose(f1);
	remove(server_file);
	rename("temp.txt", server_file);
}

/*Read a command from buffer and determine the action specified in the command.
And store the message in msg array
*/
void perform_action(char *buffer, char *msg)
{

	int op = first_occur(buffer, ' ');

	if (op < 0)
	{
		strcpy(msg, "Invalid Operation");
		printf("Invalid Operation\n");
		return;
	}
	char operation[1024] = {0};

	strncpy(operation, buffer, op); //seperate the command from parameters
	operation[op] = '\0';
	if (strcmp(operation, "NLINEX") == 0 && buffer[op] == '\0')
	{
		int lines = no_of_lines();
		char temp_msg[32] = {0};
		sprintf(temp_msg, "%d", lines);
		strcpy(msg, "No of lines in a file: ");
		strcat(msg, temp_msg);
		printf("NLINEX command has been executed successfully\n");
		return;
	}
	else if (strcmp(operation, "READX") == 0 && (buffer[op] == '\0' || is_int(buffer + op + 1)))
	{
		int linen = 0;
		int totallines = no_of_lines();

		if (buffer[op] != '\0')
			linen = atoi(buffer + op + 1);
		if (!(linen >= -totallines && linen < totallines))
		{
			strcpy(msg, "Invaid Index");
			printf("Invalid Index\n");
			return;
		}
		if (linen < 0) // recalcuate the  index in case of reverse indexing
		{
			linen += totallines;
		}
		read_line(linen, msg);
		printf("READX command has been executed successfully\n");
	}
	else if (strcmp(operation, "INSERTX") == 0 && buffer[op + 1] != '\0')
	{
		int s = first_occur(buffer + op + 1, ' ');
		char k[1024] = {0};
		strncpy(k, buffer + op + 1, s);
		k[s] = '\0';
		int totallines = no_of_lines();
		char new_msg[1024] = {0};
		int line_idx;
		if (is_int(k))
		{
			line_idx = atoi(k);
			if (!(line_idx >= -totallines && line_idx <= totallines))
			{
				strcpy(msg, "Invalid Index");
				printf("Invalid Index\n");
				return;
			}
			if (line_idx < 0)
			{
				line_idx += totallines;
			}
			strcpy(new_msg, buffer + op + s + 2);
		}
		else
		{
			line_idx = totallines;
			strcpy(new_msg, buffer + op + 1);
		}

		update_file(line_idx, new_msg, totallines);
		strcpy(msg, "Line has been updated");
		printf("INSERTX command has been executed successfully\n");
	}
	else
	{
		strcpy(msg, "Invalid Operation");
		printf("Invalid Operation\n");
	}
}
int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	bzero((char *)&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	int opt = 1;
	char buffer[1024] = {0};
	char msg[1024] = {0};

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				   &opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
			 sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 5) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in cli_addr; //storing client address
	socklen_t clilen;			 //storing length for client address, i.e. 32 bit integer
	clilen = sizeof(cli_addr);

	while (true)
	{
		//waiting for the request from a client
		if ((new_socket = accept(server_fd, (struct sockaddr *)&cli_addr,
								 (socklen_t *)&clilen)) < 0)
		{
			perror("error while accepting");
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Connection has been established \n");
		}
		/*Once connection is establieshed it accept the commands from a client 
		until QUIT command is received*/
		while (true)
		{
			valread = read(new_socket, buffer, 1024);
			remove_char(buffer, '\n'); //Command sent by client is terminated by \n. Thus it is required to remove it
			if (strcmp(buffer, "QUIT") == 0)
			{
				printf("Terminating the Session\n");
				strcpy(msg,"QUIT");
				cout<<msg<<endl;
				send(new_socket, msg, strlen(msg), 0);

				break;
			}
			perform_action(buffer, msg); // execute the relevant command and store the o/p in msg array.
			send(new_socket, msg, strlen(msg), 0); //sent back output to client

			for (int i = 0; i < 1024; i++)
			{
				buffer[i] = 0;
				msg[i] = 0;
			}
		}
	}

	return 0;
}
