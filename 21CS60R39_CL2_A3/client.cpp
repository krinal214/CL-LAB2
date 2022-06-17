
#include <iostream>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <ios> //used to get stream size
#include <limits>
#include <termios.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#define permision_file "permission.txt"
#define buffer_size 262156
#define TRUE 1
#define FALSE 0
#define PORT 8888
using namespace std;

char folder_path[128] = {0};

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
int recv_all(int sock, char *output, int maxlen)
{
    int arg = 0;

    int total = 0;
    bool stop = false;
    do
    {
        int valread = read(sock, output + total, maxlen - total);
        if (valread < 0)
            return -1;
        if (valread == 0)
            return 0;
        total += valread;
        if (output[total - 1] == '^')
            stop = true;
    } while (!stop && total < maxlen);

    output[total - 1] = '\0';
    return total - 1;
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
    if (len > (buffer_size - 10))
    {
        return true;
    }
    return false;
}
/*utility function to read a content from file and save to buffer*/
void read_file(const char *file_name, char *buffer)
{
    FILE *fp = fopen(file_name, "r");
    int j = 0;
    char c = getc(fp);
    while (c != EOF)
    {
        buffer[j++] = c;
        c = getc(fp);
    }
    buffer[j] = 0;
    fclose(fp);
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

void replace_char(char *buffer, char old, char latest)
{
    int n = strlen(buffer);
    for (int i = 0; i < n; i++)
    {
        if (buffer[i] == old)
        {
            buffer[i] = latest;
        }
    }
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
void list_users(int sock)
{
    char *msg = (char *)malloc(1050 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));
    sprintf(temp, "%s", "users");
    int offset = strlen(temp);
    offset = strlen(temp);
    temp[offset] = '^';
    temp[offset + 1] = '\0';
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}
void upload_file(const char *file, int sock)
{

    char file_name[512] = {0};
    strcpy(file_name, folder_path);
    strcat(file_name, file);
    if (access(file_name, F_OK) == -1)
    {
        cout << "File doesn't exist" << endl;
        return;
    }
    if (is_large_file(file_name))
    {
        cout << "File is large (max file size=256k Bytes)" << endl;
        return;
    }
    char *msg = (char *)malloc(buffer_size * sizeof(char));
    char *temp = (char *)malloc((buffer_size - 10) * sizeof(char));
    sprintf(temp, "%s\n%s\n", "upload", file);
    int offset = strlen(temp);
    read_file(file_name, temp + offset);
    offset = strlen(temp);
    temp[offset] = '^';
    temp[offset + 1] = '\0';
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}

void download_file(const char *file, int sock)
{

    char file_name[512] = {0};
    strcpy(file_name, folder_path);
    strcat(file_name, file);
    if (access(file_name, F_OK) != -1)
    {
        cout << "File with the same name already exists! Existing file will be overwritten!!" << endl;
    }
    char *msg = (char *)malloc(1050 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));
    sprintf(temp, "%s\n%s^", "download", file);
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}

void list_files(int sock)
{
    char *msg = (char *)malloc(1050 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));
    sprintf(temp, "%s", "files");
    int offset = strlen(temp);
    offset = strlen(temp);
    temp[offset] = '^';
    temp[offset + 1] = '\0';
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}

void invite(const char *command, int sock)
{
    char file_name[128] = {0};
    char client_id[128] = {0};
    char field[128] = {0};

    int k = sscanf(command, "%[^ ] %[^ ] %[^\n]", file_name, client_id, field);
    if (k != 3 || strlen(field) != 1 || strlen(client_id) != 5 || !is_int(client_id) || !(field[0] == 'V' || field[0] == 'E'))
    {
        cout << "Invalid parameters" << endl;
        return;
    }
    char *msg = (char *)malloc(1050 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));
    sprintf(temp, "%s\n%s\n%d\n%c\n", "interrupt", file_name, atoi(client_id), field[0]);
    int offset = strlen(temp);
    temp[offset] = '^';
    temp[offset + 1] = '\0';
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}

void process_invite(const char *buffer, int sock)
{
    char msg[32] = {0};
    char file_name[128] = {0};
    int owner;
    char field[128] = {0};
    char ch;
    sscanf(buffer, "%d\n%s\n%s\n", &owner, file_name, field);
    cout << "Invite for a collaboration:" << endl;
    printf("File: %s \t owner:%d \t permission:%c\n", file_name, owner, field[0]);
    cout << "Enter your response (y for accept/ any key for reject)" << endl;
    read(0, msg, 32);
    char temp[1024] = {0};
    sprintf(temp, "response\n");
    int offset = strlen(temp);
    if (tolower(msg[0]) == 'y')
    {
        sprintf(temp + offset, "accept\n%s^", buffer);
    }
    else
    {
        sprintf(temp + offset, "reject\n%s^", buffer);
    }
    char response[1050] = {0};
    sprintf(response, "%07d^", (int)strlen(temp));
    strcat(response, temp);
    send_all(sock, response, strlen(response));
}
void read_line(const char *command, int sock)
{
    char file_name[128] = {0};
    char st[128] = {0};
    char en[128] = {0};
    int start, end;
    int k = sscanf(command, "%[^ ] %[^ ] %[^\n]", file_name, st, en);
    char *msg = (char *)malloc(1050 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));
    if (k == 1)
    {
        start = 0;
        end = -1;
    }
    else if (k == 2 && is_int(st))
    {
        end = start = atoi(st);
    }
    else if (k == 3 && is_int(st) && is_int(en))
    {
        start = atoi(st);
        end = atoi(en);
    }
    else
    {
        cout << "Invalid parameters" << endl;
        return;
    }
    sprintf(temp, "%s\n%s %d %d^", "read", file_name, start, end);
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}

void insert_line(const char *command, int sock)
{
    char file_name[128] = {0};
    char rest[1024] = {0};
    char st[64] = {0};
    char new_line[1024] = {0};
    int k = sscanf(command, "%[^ ] %[^\n]", file_name, rest);
    char *msg = (char *)malloc(2048 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));
    if (k < 2)
    {
        cout << "Invalid parameters" << endl;
        return;
    }
    int len = strlen(rest);
    int param = 2;
    if (rest[0] == '\"' && rest[len - 1] == '\"' && len >= 2)
    {
        param = 1;
        strcpy(new_line, rest);
    }
    else
    {
        int idx = first_occur(rest, ' ');
        strncpy(st, rest, idx);
        strcpy(new_line, rest + idx + 1);
        if (!is_int(st) || strlen(new_line) < 2 || new_line[0] != '\"' || new_line[strlen(new_line) - 1] != '\"')
        {
            cout << "Invalid parameters" << endl;
            return;
        }
        param = 2;
    }

    if (param == 1)
        sprintf(temp, "%s\n%s %d %s^", "insert", file_name, param, new_line);
    else
        sprintf(temp, "%s\n%s %d %d %s^", "insert", file_name, param, atoi(st), new_line);
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}
void delete_line(const char *command, int sock)
{
    char file_name[128] = {0};
    char st[128] = {0};
    char en[128] = {0};
    int start, end;
    int k = sscanf(command, "%[^ ] %[^ ] %[^\n]", file_name, st, en);
    char *msg = (char *)malloc(1050 * sizeof(char));
    char *temp = (char *)malloc(1024 * sizeof(char));

    if (k == 1)
    {
        start = 0;
        end = -1;
    }
    else if (k == 2 && is_int(st))
    {
        end = start = atoi(st);
    }
    else if (k == 3 && is_int(st) && is_int(en))
    {
        start = atoi(st);
        end = atoi(en);
    }
    else
    {
        cout << "Invalid parameters" << endl;
        return;
    }
    sprintf(temp, "%s\n%s %d %d^", "delete", file_name, start, end);
    sprintf(msg, "%07d^", (int)strlen(temp));
    strcat(msg, temp);
    send_all(sock, msg, strlen(msg));
    free(msg);
    free(temp);
}
int main(int argc, char *argv[])
{
    int opt = TRUE;
    int addrlen, new_socket,
        activity, i, valread, sd;
    int max_sd;
    int sock;
    strcpy(folder_path, argv[1]);

    struct sockaddr_in serv_addr;

    char *output = (char *)malloc(buffer_size * sizeof(char));

    // set of socket descriptors
    fd_set readfds;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
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
    char buffer[1024] = {0};
    valread = read(sock, buffer, 1024);
    if (valread == 0)
    {
        cout << "connection request denied" << endl;
        return 0;
    }
    cout << buffer << endl;
    bool terminate = false;
    while (TRUE)
    {
        if (terminate)
        {
            close(sock);
            break;
        }

        cout << "Waiting for an Event..." << endl;
        // clear the socket set
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sock, &readfds);
        max_sd = sock;

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error\n");
        }

        if (FD_ISSET(0, &readfds))
        {

            char command[1024] = {0};
            read(0, command, 1024);
            remove_char(command, '\n');
            cout << "Entered command: " << command << endl;
            int op = first_occur(command, ' ');
            char operation[128] = {0};
            strncpy(operation, command, op);
            if (strcmp(command, "/users") == 0)
            {
                list_users(sock);
            }
            else if (strcmp(operation, "/upload") == 0)
            {
                upload_file(command + op + 1, sock);
            }
            else if (strcmp(operation, "/download") == 0)
            {
                download_file(command + op + 1, sock);
            }
            else if (strcmp(command, "/files") == 0)
            {
                list_files(sock);
            }
            else if (strcmp(operation, "/invite") == 0)
            {
                invite(command + op + 1, sock);
            }
            else if (strcmp(operation, "/read") == 0)
            {
                read_line(command + op + 1, sock);
            }
            else if (strcmp(operation, "/insert") == 0)
            {
                insert_line(command + op + 1, sock);
            }
            else if (strcmp(operation, "/delete") == 0)
            {
                delete_line(command + op + 1, sock);
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

        if (FD_ISSET(sock, &readfds))
        {
            int valread = recv_all(sock, output, 8);
            if (valread == 0)
            {
                terminate = true;
                break;
            }
            output[valread] = 0;
            int len = atoi(output);
            valread = recv_all(sock, output, len);
            if (output[0] == 'i') //  i indicates interrupt message for collaboration
            {
                process_invite(output + 2, sock);
            }
            else if (output[0] == 'd') // d indicates download the file
            {
                char file_name[128] = {0};
                strcpy(file_name, folder_path);
                int idx = first_occur(output + 2, '\n');
                strncat(file_name, output + 2, idx);
                save_file(file_name, output + idx + 3);
                cout << "Response from server:" << endl;
                printf("File %s downloaded successfully\n", file_name);
            }
            else if (output[0] == 's') // s indicates success message
            {
                cout << "Response from server:" << endl;
                cout << output + 2 << endl;
            }
            else // e indicates error message
            {
                cout << "Response from server:" << endl;
                cout << "error: " << (output + 2) << endl;
            }
        }
        cout << "----------------------------------" << endl;
    }
    free(output);
    return 0;
}
