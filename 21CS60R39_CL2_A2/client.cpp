#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#define max_file_size 262154
#define max_records 4000
#define PORT 8888
using namespace std;

/*it might be possible that socket cannot send the no of bytes requested in a single go 
so keep repeating until all bytes sent*/
bool send_all(int sock, char *buf, int len)
{
	int total = 0;
	int bytesleft = len;
	int n = 0;
	while (total < bytesleft)
	{
		n = send(sock, buf + total, bytesleft, 0);
		if (n == -1)
			return false;
		total += n;
		bytesleft -= n;
	}
	return true;
}

/*it might be possible that socket cannot read the no of bytes requested in a single go 
so keep repeating until all bytes received.
To identify the end of a message there are two appraoches. 
1) send no of bytes first
2) mark end with a different character ----> I have used this appraoch.(^ is used to identify the end of a message)
*/
int recv_all(int sock, char *output, int maxlen)
{
	int arg = 0;

	int total = 0;
	bool stop=false;
	do
	{
		int valread = read(sock, output + total, maxlen - total);
		if (valread < 0)
			return -1;
		if(valread==0)
			return 0;
		total += valread;
		if(output[total-1]=='^')stop=true;
	} while (!stop);

	output[total-1] = '\0';
	return total-1;
}

/*utility function to find no of lines in a file*/
int no_of_lines(const char *file_name)
{
	FILE *fp = fopen(file_name, "r");
	int count = 0;
	char temp_msg[1024] = {0};
	while (fgets(temp_msg, 1024, fp) != NULL)
	{
		count++;
	}
	fclose(fp);
	return count;
}
/*Determine wether file size is greater than max size allowed or not*/
bool is_large_file(const char *file_name)
{
	FILE *f = fopen(file_name, "r");
	if (f == NULL)
		return true;
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fclose(f);
	if (len > (max_file_size - 10))
	{
		return true;
	}
	int lines = no_of_lines(file_name);
	if (lines > max_records)
		return true;
	return false;
}
/*remove a character from a character buffer */
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

/*finds the first occurence of the character in c in array*/
int first_occur(const char *buffer, char c)
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

/*check wether a string is integer or not*/
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
/*utility function to store a content of buffer to file*/
void save_file(const char *file_name, char *buffer)
{
	FILE *f = fopen(file_name, "w");
	int n = strlen(buffer);
	for (int i = 0; i < n; i++)
	{
		fputc(buffer[i], f);
	}
	fclose(f);
}
/*utility function to read a content from file and save to buffer*/
void read_file(const char *file_name, char *buffer)
{
	int count = 0;
	int n = no_of_lines(file_name);
	FILE *fp = fopen(file_name, "r");
	sprintf(buffer, "%d", n);
	int j = strlen(buffer);
	sprintf(buffer + j, "%c", '\n');
	j++;
	char c = getc(fp);
	while (c != EOF)
	{
		buffer[j++] = c;
		c = getc(fp);
	}
	buffer[j] = '\n';
	buffer[j + 1] = 0;
	fclose(fp);
}
/*utility function to parse sort command and store content of files in a buffer
Also receives the output from a server and saves sorted records in a file.
*/
void sort_file(char *command, int sock)
{
	char file_name[128] = {0};
	char field[128] = {0};

	int idx1 = first_occur(command, ' ');
	int k = sscanf(command, "%[^ ] %[^\n]", file_name, field);

	if (k != 2 || strlen(field) != 1 || !(field[0] == 'D' || field[0] == 'N' || field[0] == 'P'))
	{
		cout << "Invalid field parameters" << endl;
		return;
	}
	if (access(file_name, F_OK) == -1)
	{
		cout << "File doesn't exist" << endl;
		return;
	}
	if (is_large_file(file_name))
	{
		cout << "File is large (max file size=256k Bytes and maximum records=4k" << endl;
		return;
	}
	char *msg = (char *)malloc(2 * max_file_size * sizeof(char));
	char *output = (char *)malloc(2 * max_file_size * sizeof(char));
	sprintf(msg, "%s\n%c\n", "sort", field[0]);
	int offset = strlen(msg);
	read_file(file_name, msg + offset);
	offset = strlen(msg);
	msg[offset] = '^';
	msg[offset + 1] = '\0';
	//cout << msg;
	send_all(sock, msg, strlen(msg));
	int valread = recv_all(sock, output, 2 * max_file_size);

	cout << "Bytes received: " << valread << endl;
	if (output[0] == 's')
	{
		save_file(file_name, output + 2);
		cout << "File sorted successfully" << endl;
	}
	else
	{
		cout << "error: " << (output + 2) << endl;
	}

	free(msg);
	free(output);
}
/* Utility function to parse merge command and prepare a buffer to store content of two files.
Also receives the output from a server and saves to specified file.
Note: I am combining two files in a single message separated by $.
*/
void merge_file(char *command, int sock)
{
	char file_name1[128] = {0};
	char file_name2[128] = {0};
	char file_name3[128] = {0};
	char field[128] = {0};
	int k = sscanf(command, "%[^ ] %[^ ] %[^ ] %[^\n]", file_name1, file_name2, file_name3, field);
	if (k != 4 || strlen(field) != 1 || !(field[0] == 'D' || field[0] == 'N' || field[0] == 'P'))
	{
		cout << "Invalid parameters" << endl;
		return;
	}
	if (access(file_name1, F_OK) == -1)
	{
		cout << "File 1 doesn't exist" << endl;
		return;
	}
	if (access(file_name2, F_OK) == -1)
	{
		cout << "File 2 doesn't exist" << endl;
		return;
	}
	if (is_large_file(file_name1))
	{
		cout << "File is large (max file size=256k Bytes and maximum records=4k" << endl;
		return;
	}
	if (is_large_file(file_name2))
	{
		cout << "File is large (max file size=256k Bytes and maximum records=4k" << endl;
		return;
	}
	if (access(file_name3, F_OK) != -1)
	{
		cout << "File 3 already exist. Enter different name." << endl; //It prevents from overwriting a existing file
		return;
	}
	char *msg = (char *)malloc(2 * max_file_size * sizeof(char));
	char *output = (char *)malloc(2 * max_file_size * sizeof(char));

	sprintf(msg, "%s\n%c\n", "merge", field[0]);
	int offset = strlen(msg);
	read_file(file_name1, msg + offset);
	offset = strlen(msg);
	msg[offset] = '$';
	read_file(file_name2, msg + offset + 1);
	offset = strlen(msg);
	msg[offset] = '^';
	msg[offset + 1] = '\0';
	send_all(sock, msg, strlen(msg));
	int valread = recv_all(sock, output, 2 * max_file_size);
	cout << "Bytes received: " << valread << endl;
	if (output[0] == 's')
	{
		save_file(file_name3, output + 2);
		cout << "File merged successfully" << endl;
	}
	else
	{
		cout << "error: " << (output + 2) << endl;
	}
	free(msg);
	free(output);
}
/*utility function to parse command for similarity check between files.
It shows the retured pairs of similar records
*/
void similarity_file(char *command, int sock)
{
	char file_name1[128] = {0};
	char file_name2[128] = {0};
	int k = sscanf(command, "%[^ ] %[^\n]", file_name1, file_name2);
	if (k != 2)
	{
		cout << "Invalid parameters" << endl;
		return;
	}
	if (access(file_name1, F_OK) == -1)
	{
		cout << "File 1 doesn't exist" << endl;
		return;
	}
	if (access(file_name2, F_OK) == -1)
	{
		cout << "File 2 doesn't exist" << endl;
		return;
	}
	if (is_large_file(file_name1))
	{
		cout << "File is large (max file size=256k Bytes and maximum records=4k" << endl;
		return;
	}
	if (is_large_file(file_name2))
	{
		cout << "File is large (max file size=256k Bytes and maximum records=4k" << endl;
		return;
	}

	char *msg = (char *)malloc(2 * max_file_size * sizeof(char));
	char *output = (char *)malloc(2 * max_file_size * sizeof(char));
	sprintf(msg, "%s\n", "similarity");
	int offset = strlen(msg);
	read_file(file_name1, msg + offset);
	offset = strlen(msg);
	msg[offset] = '$';
	read_file(file_name2, msg + offset+1);
	offset = strlen(msg);
	msg[offset] = '^';
	msg[offset+1]='\0';
	//cout << msg;
	send_all(sock, msg, strlen(msg));
	int valread = recv_all(sock, output, 2 * max_file_size);
	cout << "Bytes received: " << valread << endl;
	
	if (output[0] == 's')  
	{
		cout << (output + 2) << endl;
	}
	else
	{
		cout << "error: " << (output + 2) << endl;
	}
	free(msg);
	free(output);
}


int main(int argc, char const *argv[])
{
	int sock = 0, valread;
	struct sockaddr_in serv_addr;

	char *buffer = (char *)malloc(1024 * sizeof(char));
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

	char command[1024] = {0};
	int c = 1;
	valread = read(sock, buffer, 1024);
	if (valread == 0)
	{
		cout << "connection request denied" << endl;
		free(buffer);
		return 0;
	}
	cout << buffer << endl;
	memset(buffer, 0, sizeof(buffer));
	while (true)
	{

		memset(command, 0, sizeof(command));
		printf("Enter the command\n");
		fgets(command, 1024, stdin);
		remove_char(command, '\n');
		int op = first_occur(command, ' ');
		char operation[128] = {0};
		strncpy(operation, command, op);
		cout << "operation: " << operation << endl;
		if (strcmp(operation, "/sort") == 0)
		{
			sort_file(command + op + 1, sock);
		}
		else if (strcmp(operation, "/merge") == 0)
		{
			merge_file(command + op + 1, sock);
		}
		else if (strcmp(operation, "/similarity") == 0)
		{
			similarity_file(command + op + 1, sock);
		}
		else if (strcmp(command, "/exit") == 0)
		{
			cout << "Exit" << endl;
			send(sock, buffer, 0, 0);
			break;
		}
		else
		{
			cout << "Invalid Operation" << endl;
		}
	}
	free(buffer);

	return 0;
}
