#include <iostream>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#define permision_file "permission.txt"
#define buffer_size 262156
#define TRUE 1
#define FALSE 0
#define PORT 8888
#define max_clients 5
using namespace std;

int client_id[max_clients];
int client_socket[max_clients];
int counter = 0;

struct Record
{
    char file_name[256] = {0};
    int owner = -1;
    int other = -1;
    char permit = 'V';
};

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

void replace_char(char* buffer,char old,char latest){
    int n = strlen(buffer);
    for (int i = 0; i < n; i++)
    {
        if (buffer[i] == old)
        {
            buffer[i] = latest;
        }
    }
 
}

/*utility function to find the index of first occurence of character c in buffer*/
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
/*check whether a given string is valid integer or not*/
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
            printf("Listener on port %d \n", PORT);

            flag = false;
            return flag;
        }
    }
    return true;
}
/*Utility function for NLINEX command*/
int no_of_lines(const char *file)
{   FILE *fp = fopen(file, "r");
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
void read_line(const char *file, int start, int end, char *buffer)
{

    FILE *fp = fopen(file, "r");
    int count = 0;
    char c;

    char temp_msg[1024] = {0};
    int p = 0;
    while (fgets(temp_msg, 1024, fp) != NULL)
    {
        if (count >= start && count <= end)
        {
            strcpy(buffer + p, temp_msg);
            p = strlen(buffer);
        }
        count++;
        if (count > end)
            break;
    }
    p = strlen(buffer);
    if (buffer[p - 1] == '\n')
        buffer[p - 1] = '\0';
    fclose(fp);
}

/*Utility function for READX command*/
void delete_line(const char *file, int start, int end)
{
    int n = no_of_lines(file);
    FILE *f1, *f2;
    f1 = fopen(file, "r");
    f2 = fopen("temp.txt", "w");
    int count = 0;
    char temp_msg[1024] = {0};
    int p = 0;

    while (fgets(temp_msg, 1024, f1) != NULL)
    {
        if (count < start || count > end)
        {
            if (end == n - 1 && count == start - 1)
            {
                int len = strlen(temp_msg);
                temp_msg[len - 1] = 0;
            }
            fprintf(f2, "%s", temp_msg);
        }
        count++;
    }
    fclose(f1);
    fclose(f2);
    remove(file);
    rename("temp.txt", file);
}
/*Utility function for INSERTX command*/
void insert_line(const char *file, int idx, char *new_msg, int n)
{
    FILE *f1, *f2;
    /*special case
	if total lines are n. Thus valid index is [0,n-1]. 
	So, if line has to inserted at index n. we just need to append it 
	*/
    if (n == idx)
    {
        f1 = fopen(file, "a");
        if (n != 0)
        {
            fprintf(f1, "%c", '\n');
        }
        fprintf(f1, "%s", new_msg);

        fclose(f1);
        return;
    }
    f1 = fopen(file, "r");
    f2 = fopen("temp.txt", "w");
    int count = 0;
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
    remove(file);
    rename("temp.txt", file);
}

void list_clients(char *response, int self)
{
    int j = 0;
    char output[1024] = {0};
    sprintf(output, "s:List of active users\nsocket_id \t client_id\n%5d \t\t %7d    (you)\n", client_socket[self], client_id[self]);

    for (int i = 0; i < max_clients; i++)
    {
        if (i != self && client_id[i] > 0)
        {
            j = strlen(output);
            sprintf(output + j, "%5d \t\t %7d\n", client_socket[i], client_id[i]);
        }
    }
    j = strlen(output);
    output[j - 1] = '^';
    output[j] = 0;
    int len = strlen(output);
    sprintf(response, "%07d^", len);
    strcat(response, output);
}

Record str_to_record(const char *msg)
{
    Record record = {0};
    char file_name[256] = {0};
    int owner, other;
    char permit;
    int j=sscanf(msg, "%s\t%d\t%d\t%c", file_name, &owner, &other, &permit);
    strcpy(record.file_name, file_name);
    record.owner = owner;
    record.other = other;
    record.permit = permit;
    return record;
}
void list_files(char *response)
{
    char *output = (char *)malloc(buffer_size * sizeof(char));
    sprintf(output, "s:%s \t \t %s \t %s \t %s \t %s\n", "file", "owner", "user", "permission", "lines");
    char temp_msg[1024] = {0};
    FILE *f1 = fopen(permision_file, "r");
    int offset;
    while (fgets(temp_msg, 1024, f1) != NULL)
    {
        Record r = str_to_record(temp_msg);
         offset=strlen(output);
         int no=no_of_lines(r.file_name);
         sprintf(output+offset,"%s \t %d \t %d \t %6c \t %4d\n",r.file_name,r.owner,r.other,r.permit,no);
    }
    offset = strlen(output);
    output[offset-1] = '^';
    output[offset] = '\0';
    int len = strlen(output);
    sprintf(response, "%07d^", len);
    strcat(response, output);
    free(output);
}
bool has_permit(const char *file_name, int user, char permit)
{
    FILE *fp = fopen(permision_file, "r");
    char temp_msg[1024] = {0};
    bool flag = false;
    while (fgets(temp_msg, 1024, fp) != NULL)
    {
        Record r = str_to_record(temp_msg);
        if (strcmp(r.file_name, file_name) == 0 && user == r.other && (r.permit == permit || r.permit == 'E'))
        {
            flag = true;
            break;
        }
    }
    fclose(fp);
    return flag;
}
void cleanup(int user)
{
    printf("User %d exited\n", user);
    FILE *f1 = fopen(permision_file, "r");
    FILE *f2 = fopen("temp.txt", "w");
    char temp_msg[1024] = {0};
    bool flag = false;
    bool first_record = true;
    while (fgets(temp_msg, 1024, f1) != NULL)
    {
        Record r = str_to_record(temp_msg);

        if (r.owner != user && r.other != user)
        {
            remove_char(temp_msg, '\n');
            if (first_record)
            {
                fprintf(f2, "%s", temp_msg);
                first_record = false;
            }
            else
                fprintf(f2, "\n%s", temp_msg);
        }
        if(r.owner==user)
        {
            remove(r.file_name);
        }
    }
    fclose(f1);
    fclose(f2);
    remove(permision_file);
    rename("temp.txt", permision_file);
}

void update_record(int owner, int other, const char *file_name, int permit)
{
    FILE *fp = fopen(permision_file, "r");
    FILE *temp = fopen("temp.txt", "w");
    char temp_msg[1024] = {0};
    int count = 0;
    char ch='\n';

    while (fgets(temp_msg, 1024, fp) != NULL)
    {
        Record r = str_to_record(temp_msg);
        if (strcmp(r.file_name, file_name) != 0 || other != r.other)
        {
            fprintf(temp, "%s", temp_msg);
            ch=temp_msg[strlen(temp_msg)-1];
         
        }
    }
    if (ch == '\n')
        fprintf(temp, "%s \t%d \t%d \t%c", file_name, owner, other, permit);
    else
        fprintf(temp, "\n%s \t%d \t%d \t%c", file_name, owner, other, permit);
    fclose(temp);
    fclose(fp);
    remove(permision_file);
    rename("temp.txt", permision_file);
}
/*utility function to store a content of buffer to file*/
void save_file(const char *file_name, const char *buffer)
{
    FILE *f = fopen(file_name, "w");
    int n = strlen(buffer);
    for (int i = 0; i < n; i++)
    {
        fputc(buffer[i], f);
    }
    fclose(f);
}

void upload_file(const char *buffer, char *response, int self)
{
    int idx = first_occur(buffer, '\n');
    char file_name[128] = {0};
    strncpy(file_name, buffer, idx);
    char output[1024] = {0};
    if (access(file_name, F_OK) != -1)
    {
        printf("File %s already exists on server\n", file_name);
        sprintf(output, "e:File %s already exists on server^", file_name);
    }
    else
    {
        int owner = client_id[self];
        save_file(file_name, buffer + idx + 1);
        update_record(owner, owner, file_name, 'E');
        printf("File %s uploaded successfully\n", file_name);
        int j = 0;
        sprintf(output, "s:File %s uploaded successfully^", file_name);
    }
    int len = strlen(output);
    sprintf(response, "%07d^", len);
    strcat(response, output);
}

void download_file(const char *file_name, char *response, int self)
{
    char *output = (char *)malloc(buffer_size * sizeof(char));
    if (access(file_name, F_OK) == -1)
    {
        printf("File %s doesn't exist on server\n", file_name);
        sprintf(output, "e:File %s doesn't exist on server^", file_name);
    }
    else if (!has_permit(file_name, client_id[self], 'V'))
    {
        printf("You don't have permission to downlaod file %s\n", file_name);
        sprintf(output, "e:You don't have permission to download file %s^", file_name);
    }
    else
    {
        int owner = client_id[self];
        sprintf(output, "d:%s\n", file_name);
        int offset = strlen(output);
        read_file(file_name, output + offset);
        offset = strlen(output);
        output[offset] = '^';
        output[offset + 1] = '\0';
    }
    sprintf(response, "%07d^", (int)strlen(output));
    strcat(response, output);
    free(output);
}

int id_to_socket_index(int cli_id)
{
    int sock = -1;
    for (int i = 0; i < max_clients; i++)
    {
        if (client_id[i] == cli_id)
        {
            sock = i;
            break;
        }
    }
    return sock;
}
bool is_owner(const char *file_name, int owner)
{
    FILE *fp = fopen(permision_file, "r");
    char temp_msg[1024] = {0};
    int count = 0;
    bool flag = false;
    while (fgets(temp_msg, 1024, fp) != NULL)
    {
        Record r = str_to_record(temp_msg);
        if (strcmp(r.file_name, file_name) == 0 && r.owner == owner)
        {
            flag = true;
            break;
        }
    }
    fclose(fp);
    return flag;
}

void invite(const char *buffer, char *response, int self)
{
    char file_name[128] = {0};
    char cli_id[128] = {0};
    char field[128] = {0};
    char output[1024] = {0};
    int sock_index = -1;
    sscanf(buffer, "%s\n%s\n%s\n", file_name, cli_id, field);
    if (access(file_name, F_OK) == -1)
    {
        printf("File %s doesn't exist on server\n", file_name);
        sprintf(output, "e:File %s doesn't exist on server^", file_name);
    }
    else if (!is_owner(file_name, client_id[self]))
    {
        sprintf(output, "e:you are not owner^");
    }
    else if ((sock_index = id_to_socket_index(atoi(cli_id))) == -1)
    {
        printf("User %d is not active\n", atoi(cli_id));
        sprintf(output, "e:User %d is not active^", atoi(cli_id));
    }
    else if (sock_index == self)
    {
        sprintf(output, "e:You are the owner of file %s! You already have full autorization. !!!^", file_name);
    }
    else if (has_permit(file_name, atoi(cli_id), field[0]))
    {
        cout << "permission already given" << endl;
        printf("User %d already has requested permission\n", atoi(cli_id));
        sprintf(output, "e:User %d already has requested permission^", atoi(cli_id));
    }
    else
    {
        char temp[1024] = {0};
        char msg[1050] = {0};
        sprintf(temp, "i:%d\n%s\n%c\n^", client_id[self], file_name, field[0]);
        sprintf(msg, "%07d^", (int)strlen(temp));
        strcat(msg, temp);
        send(client_socket[sock_index], msg, strlen(msg), 0);
        sprintf(output, "s:Invitation for file: %s has been sent to user %d^", file_name, atoi(cli_id));
    }
    sprintf(response, "%07d^", (int)strlen(output));
    strcat(response, output);
}

void invite_response(const char *buffer, char *response, int self)
{
    char status[128] = {0};
    char file_name[128] = {0};
    int owner;
    char field[128] = {0};
    sscanf(buffer, "%s\n%d\n%s\n%s\n", status, &owner, file_name, field);
    int sock_index = id_to_socket_index(owner);
    char output[1024] = {0};
    if (strcmp(status, "reject") == 0)
    {
        sprintf(output, "s:You have rejected the invitation for file %s^", file_name);
        char msg[1050] = {0};
        char temp[1024] = {0};
        sprintf(temp, "s:Client %d has rejected the invite for file %s^", client_id[self], file_name);
        sprintf(msg, "%7d^", (int)strlen(temp));
        strcat(msg, temp);
        send(client_socket[sock_index], msg, strlen(msg), 0);
    }
    else if (sock_index < 0 || access(file_name, F_OK) == -1)
    {
        sprintf(output, "e:You couldn't be made collaborator(either owner left or file got deleted)^");
    }
    else if (has_permit(file_name, client_id[self], field[0]))
    {
        cout << "permission already given" << endl;
        sprintf(output, "e:You are already collaborator for file %s^", file_name);
        char msg[1050] = {0};
        char temp[1024] = {0};
        sprintf(temp, "e:User %d already has requested permission for file %s^", client_id[self], file_name);
        sprintf(msg, "%7d^", (int)strlen(temp));
        strcat(msg, temp);
        send(client_socket[sock_index], msg, strlen(msg), 0);
    }

    else
    {
        sprintf(output, "s:You are the collaborator for file %s^", file_name);
        update_record(client_id[sock_index], client_id[self], file_name, field[0]);
        char msg[1050] = {0};
        char temp[1024] = {0};
        sprintf(temp, "s:Client %d has accepted invite for file %s^", client_id[self], file_name);
        sprintf(msg, "%7d^", (int)strlen(temp));
        strcat(msg, temp);
        send(client_socket[sock_index], msg, strlen(msg), 0);
    }
    sprintf(response, "%07d^", (int)strlen(output));
    strcat(response, output);
}
void process_read(const char *buffer, char *response, int self)
{
    char file_name[128] = {0};
    int start;
    int end;
    sscanf(buffer, "%s %d %d", file_name, &start, &end);
    char *output = (char *)malloc(buffer_size * sizeof(char));
    if (access(file_name, F_OK) == -1)
    {
        printf("File %s doesn't exist on server\n", file_name);
        sprintf(output, "e:File %s doesn't exist on server^", file_name);
    }
    else if (!has_permit(file_name, client_id[self], 'V'))
    {
        printf("You don't have a permission to read the file %s\n", file_name);
        sprintf(output, "e:You dont have a permission to read the file %s^", file_name);
    }
    else
    {
        int n = no_of_lines(file_name);
        if (start < 0)
            start += n;
        if (end < 0)
            end += n;
        if (start < 0 || start >= n || end < 0 || end >= n || start > end)
        {
            cout << "Invalid Index" << endl;
            sprintf(output, "e:You have entered invalid index in the file %s^", file_name);
        }
        else
        {
            sprintf(output, "s:Lines from index %d to %d from file %s\n", start, end, file_name);
            int offset = strlen(output);
            read_line(file_name, start, end, output + offset);
            offset = strlen(output);
            output[offset] = '^';
            output[offset + 1] = 0;
        }
    }
    sprintf(response, "%07d^", (int)strlen(output));
    strcat(response, output);
    free(output);
}
void process_insert(const char *buffer, char *response, int self)
{
    char file_name[128] = {0};
    int param, idx;
    char rest[1024] = {0};
    char new_line[1024] = {0};
    int k = sscanf(buffer, "%s %d %[^\n]", file_name, &param, rest);
    char *output = (char *)malloc(buffer_size * sizeof(char));
    if (access(file_name, F_OK) == -1)
    {
        printf("File %s doesn't exist on server\n", file_name);
        sprintf(output, "e:File %s doesn't exist on server^", file_name);
    }
    else if (!has_permit(file_name, client_id[self], 'E'))
    {
        printf("You don't have a permission to insert the line in the file %s\n", file_name);
        sprintf(output, "e:You dont have a permission to insert the line in the file %s^", file_name);
    }
    else
    {
        int n = no_of_lines(file_name);
        if (param == 1)
        {
            idx = n;
            sscanf(rest, "\" %[^\"]", new_line);
        }
        else
        {
            sscanf(rest, "%d \" %[^\"]", &idx, new_line);
        }
        if (idx < 0)
            idx += n;
        if (idx < 0 || idx > n)
        {
            cout << "Invalid Index" << endl;
            sprintf(output, "e:You have entered invalid index in the file %s^", file_name);
        }
        else
        {   replace_char(new_line,'|','\n');
            insert_line(file_name, idx, new_line, n);
            
            sprintf(output, "s:Reqeusted lines have been inserted in file %s\n",file_name);
            int offset = strlen(output);
            read_file(file_name,output + offset);
            offset = strlen(output);
            output[offset] = '^';
            output[offset + 1] = 0;
           
            //sprintf(output, "s:Requested line has been inserted in file %s^", file_name);
        }
    }
    sprintf(response, "%07d^", (int)strlen(output));
    strcat(response, output);
    free(output);
}
void process_delete(const char *buffer, char *response, int self)
{
    char file_name[128] = {0};
    int start;
    int end;
    sscanf(buffer, "%s %d %d", file_name, &start, &end);
    char *output = (char *)malloc(buffer_size * sizeof(char));

    if (access(file_name, F_OK) == -1)
    {
        printf("File %s doesn't exist on server\n", file_name);
        sprintf(output, "e:File %s doesn't exist on server^", file_name);
    }
    else if (!has_permit(file_name, client_id[self], 'E'))
    {
        printf("You don't have a permission to delete the lines from the file %s\n", file_name);
        sprintf(output, "e:You dont have a permission to delete the lines from the file %s^", file_name);
    }
    else
    {
        int n = no_of_lines(file_name);
        if (start < 0)
            start += n;
        if (end < 0)
            end += n;
        if (start < 0 || start >= n || end < 0 || end >= n || start > end)
        {
            cout << "Invalid Index" << endl;
            sprintf(output, "e:You have entered invalid index in the file %s^", file_name);
        }
        else
        {
            delete_line(file_name, start, end);
            sprintf(output, "s:Lines from index %d to %d from file %s have been deleted\n", start, end, file_name);
            int offset = strlen(output);
            read_file(file_name,output + offset);
            offset = strlen(output);
            output[offset] = '^';
            output[offset + 1] = 0;
          
        }
    }
    sprintf(response, "%07d^", (int)strlen(output));
    strcat(response, output);
    free(output);
}

int main(int argc, char *argv[])
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket,
        activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char *buffer = (char *)malloc(buffer_size * sizeof(char));
    char *output = (char *)malloc(buffer_size * sizeof(char));
    FILE *fp = fopen(permision_file, "w");
    fclose(fp);
    //set of socket descriptors
    fd_set readfds;

    //a message
    const char *message = "Hello from the server (implemented using select)\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
        client_id[i] = -1;
    }
    //create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    //try to specify maximum of 5 pending connections for the master socket
    if (listen(master_socket, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    int client_allowed = max_clients;
    while (TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            //highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error\n");
        }
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            if (client_allowed > 0)
            {
                printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                cout << "Maximum clients allowed: " << client_allowed << endl;
            }
            else
            {
                printf("connection request denied\n");
                send(new_socket, message, 0, 0);
                close(new_socket);
                continue;
            }
            //send new connection greeting message
            if (send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            //add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                //if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    client_id[i] = 10000 + counter;
                    counter = (counter + 1) % 1000;
                    cout << "counter " << counter << endl;
                    client_allowed--;
                    break;
                }
            }
        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                valread = recv_all(sd, buffer, 8);
                if ((valread) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_allowed++;
                    cleanup(client_id[i]);
                    client_socket[i] = 0;
                    client_id[i] = -1;
                }

                else
                {
                    buffer[valread] = 0;
                    int len = atoi(buffer);
                    valread = recv_all(sd, buffer, len);
                    printf("%s\n", buffer);
                    cout << "----------------------------------------------" << endl;
                    int op = first_occur(buffer, '\n');
                    char operation[128] = {0};
                    strncpy(operation, buffer, op);
                    if (strcmp(buffer, "users") == 0)
                    {
                        list_clients(output, i);
                    }

                    else if (strcmp(operation, "upload") == 0)
                    {
                        upload_file(buffer + op + 1, output, i);
                    }
                    else if (strcmp(operation, "download") == 0)
                    {
                        download_file(buffer + op + 1, output, i);
                    }
                    else if (strcmp(operation, "files") == 0)
                    {
                        list_files(output);
                    }
                    else if (strcmp(operation, "interrupt") == 0)
                    {
                        invite(buffer + op + 1, output, i);
                    }
                    else if (strcmp(operation, "response") == 0)
                    {
                        invite_response(buffer + op + 1, output, i);
                    }
                    else if (strcmp(operation, "read") == 0)
                    {
                        process_read(buffer + op + 1, output, i);
                    }
                    else if (strcmp(operation, "insert") == 0)
                    {
                        process_insert(buffer + op + 1, output, i);
                    }
                    else if (strcmp(operation, "delete") == 0)
                    {
                        process_delete(buffer + op + 1, output, i);
                    }
                    send(sd, output, strlen(output), 0);
                    memset(output, 0, sizeof(output));
                }
            }
        }
    }
    free(buffer);
    free(output);
    return 0;
}
