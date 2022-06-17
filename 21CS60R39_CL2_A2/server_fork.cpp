#include <iostream>
#include <string.h> 
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/time.h>
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)
#define buffer_size 524308
#define max_records 4000
#define max_clients 5
#define TRUE 1
#define FALSE 0
#define PORT 8888
using namespace std;

int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
struct sembuf pop, vop;
int print_semaphore; // used for printing logs on server so that ouput of two process doesn't got mixed

struct Date
{
    int day, month, year;
};

struct Record
{
    Date d;
    char item[137] = {0}; //Assumption: maximum size of an item name is 136 else it is invalid file
    double price;
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
        else if(valread==0)return 0;
		total += valread;
		if(output[total-1]=='^')stop=true;
	} while (!stop);

	output[total-1] = '\0';
	return total-1;
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
/*check whether a given string is valid price(floating point number with upto 2 decimal)*/
bool is_price(char *buffer)
{
    double d = 0;
    bool flag = true;
    int n = strlen(buffer);
    int i = 0;
    while (i < n && buffer[i] != '.')
    {
        if (buffer[i] >= '0' && buffer[i] <= '9')
        {
        }
        else
        {
            flag = false;
            return flag;
        }
        i++;
    }
    i++;
    int c = 0;
    while (i < n)
    {
        if (buffer[i] >= '0' && buffer[i] <= '9')
        {
            c++;
        }
        else
        {
            flag = false;
            return flag;
        }
        i++;
    }
    if (c > 2)
        return false;
    return true;
}
/*check wheter given date is valid or not*/
bool is_date(Date d)
{
    if (d.month < 1 || d.month > 12 || d.year < 0)
        return false;

    if ((d.year % 400 == 0) || (d.year % 100 != 0 && d.year % 4 == 0))
        days[1] = 29;
    else
        days[1] = 28;

    if (d.day < 1 || d.day > days[d.month - 1])
        return false;
    return true;
}
/*check given string is valid record or not.
And covert valid string to Record structure*/
Record str_to_record(const char *msg)
{
    char d[3] = {0};
    char m[3] = {0};
    char y[5] = {0};
    char item[1024] = {0};
    char price[32] = {0};
    Record x = {0};
    int k = sscanf(msg, "%2s.%2s.%4s\t %[^\t] %[^\n]", d, m, y, item, price);
    if (k != 5 || !is_int(d) || !is_int(m) || !is_int(y) || strlen(item) == 0 || strlen(item) > 136 || !is_price(price))
    {
        return x;
    }
    Date date;
    date.day = atoi(d);
    date.month = atoi(m);
    date.year = atoi(y);
    if (!is_date(date))
    {
        return x;
    }
    x.d = date;
    strcpy(x.item, item);
    x.price = atof(price);
    return x;
}
/*check wether two records are similar or not.
Two records are similar if eithe one of the field is same.
*/
bool is_similar(const Record r1, const Record r2)
{
    if (r1.d.day == r2.d.day)
        return true;
    if (r1.d.month == r2.d.month)
        return true;
    if (r1.d.year == r2.d.year)
        return true;
    if (strcmp(r1.item, r2.item) == 0)
        return true;
    if (r1.price == r2.price)
        return true;
    return false;
}

/*check two Record structure are exactly same or not*/
bool is_equal(const Record r1, const Record r2)
{
    if (r1.d.day != r2.d.day)
        return false;
    if (r1.d.month != r2.d.month)
        return false;
    if (r1.d.year != r2.d.year)
        return false;
    if (strcmp(r1.item, r2.item) != 0)
        return false;
    if (r1.price != r2.price)
        return false;
    return true;
}
/*utility function to store a file content received from a client to array of Record structure
It returns the no of records in a file if valid or else return -1.
*/
int buffer_to_array(Record *array, char *buffer)
{
    char no[64] = {0};
    int idx = first_occur(buffer, '\n');
    strncpy(no, buffer, idx);
    int rows = atoi(no);

    idx++;
    string s = "";
    int j = 0;
    int status = rows;
    int totals = rows;
    const Record errorRecord = {0};
    while (rows > 0)
    {
        if (buffer[idx] == '\n')
        {
            s += '\n';
            array[j] = str_to_record(s.c_str());
            if (is_equal(array[j], errorRecord)) // check wether a given jth record is invalid or not
            {
                cout << "Invalid record at index " << (totals - rows) << ": " << s << endl;

                status = -1;
                break;
            }
            j++;
            rows--;
            s = "";
        }
        else
        {
            s += buffer[idx];
        }
        idx++;
    }
    return status;
}

/*utility function to store a 1D array of Record structure to character buffer to be sent to client*/
void array_to_buffer(int n, Record *array, char *buffer)
{
    int i = 0;
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        sprintf(buffer + j, "%02d.%02d.%04d\t%s\t%0.2f\n", array[i].d.day, array[i].d.month, array[i].d.year, array[i].item, array[i].price);

        j = strlen(buffer);
    }
    int l = strlen(buffer);
    buffer[l - 1] = 0;
}
/*utility function to store a 2D array of Record structure(pair of Records) to character buffer to be sent to client*/
void array_to_buffer_2d(int n, Record **array, char *buffer)
{
    int i = 0;
    int j = 0;
    for (int i = 0; i < n; i++)
    {
        sprintf(buffer + j, "%02d.%02d.%04d\t%s\t%0.2f\t|\t%02d.%02d.%04d\t%s\t%0.2f\n",
                array[i][0].d.day, array[i][0].d.month, array[i][0].d.year, array[i][0].item, array[i][0].price,
                array[i][1].d.day, array[i][1].d.month, array[i][1].d.year, array[i][1].item, array[i][1].price);

        j = strlen(buffer);
    }
    int l = strlen(buffer);
    buffer[l - 1] = 0;
}
/*utility function to compare two Records using date field*/
int date_comparator(const Record r1, const Record r2)
{
    if (r1.d.year != r2.d.year)
    {
        return r1.d.year - r2.d.year;
    }
    if (r1.d.month != r2.d.month)
    {
        return r1.d.month - r2.d.month;
    }
    return r1.d.day - r2.d.day;
}

/*utility function to compare two Records using item_name field*/
int item_comparator(const Record r1, const Record r2)
{
    return strcmp(r1.item, r2.item);
}
/*utility function to compare two Records using price field*/
int price_comparator(const Record r1, const Record r2)
{
    if (r1.price > r2.price)
        return 1;
    if (r1.price < r2.price)
        return -1;
    return 0;
}
/*utility function to check wether a given array is sorted or not with respect to given field*/
bool is_sorted(int n, Record *array, int (*p)(const Record, const Record))
{
    bool flag = true;
    for (int i = 1; i < n; i++)
    {
        if ((*p)(array[i], array[i - 1]) < 0)
        {

            flag = false;
            return flag;
        }
    }
    return flag;
}
/*utility function to merge two sorted array*/
void merge(Record array1[], int start1, int end1, Record array2[], int start2, int end2, int (*p)(Record, Record))
{
    int i = start1;
    int j = start2;
    int k = 0;
    int n = (end1 - start1) + (end2 - start2) + 2;
    Record *aux = (Record *)malloc(n * sizeof(Record));

    while (i <= end1 && j <= end2)
    {
        if ((*p)(array1[i], array2[j]) <= 0)
        {
            aux[k++] = array1[i++];
        }
        else
        {
            aux[k++] = array2[j++];
        }
    }
    while (i <= end1)
    {
        aux[k++] = array1[i++];
    }
    while (j <= end2)
    {
        aux[k++] = array2[j++];
    }
    int idx = start1;
    for (int x = 0; x < k; x++)
    {
        array1[idx++] = aux[x];
    }
    free(aux);
}
/*implementation of merge_sort*/
void merge_sort(Record *array, int start, int end, int (*p)(Record, Record))
{
    if (start < end)
    {
        int mid = (start + end) / 2;
        merge_sort(array, start, mid, p);
        merge_sort(array, mid + 1, end, p);
        merge(array, start, mid, array, mid + 1, end, p);
    }
}
/*utility function to parse character array to array of Record structure
And sort using required comparator.
And again convert sorted array to character buffer before sending to client
*/
void sort_records(char *buffer, char *output)
{
    char field = buffer[0];
    Record *array = (Record *)malloc(max_records * sizeof(Record));

    int rows = buffer_to_array(array, buffer + 2);
    if (rows < 0)
    {
        sprintf(output, "%s", "e:File is invalid\n^");
        free(array);
        return;
    }

    switch (field)
    {
    case 'D':
        merge_sort(array, 0, rows - 1, date_comparator);
        break;
    case 'N':
        merge_sort(array, 0, rows - 1, item_comparator);
        break;
    case 'P':
        merge_sort(array, 0, rows - 1, price_comparator);
        break;
    default:
        break;
    }
    strcpy(output, "s:");
    array_to_buffer(rows, array, output + 2);
    int offset = strlen(output);
    output[offset] = '^';
    output[offset + 1] = '\0';
    free(array);
}
/*utility function to parse character array to array of Record structure.
And check whether arrays are sorted or not with field.
And merge two sorted arrays array1 and array 2 and store to array1.
*/
void merge_records(char *buffer, char *output)
{
    char field = buffer[0];
    Record *array1 = (Record *)malloc(2 * max_records * sizeof(Record));
    Record *array2 = (Record *)malloc(max_records * sizeof(Record));
    int delimeter = first_occur(buffer, '$');
    int rows1 = buffer_to_array(array1, buffer + 2);
    int rows2 = buffer_to_array(array2, buffer + delimeter + 1);
    int (*fun_ptr)(Record, Record) = NULL;
    switch (field)
    {
    case 'D':
        fun_ptr = date_comparator;
        break;
    case 'N':
        fun_ptr = item_comparator;
        break;
    case 'P':
        fun_ptr = price_comparator;
        break;

    default:
        break;
    }
    if (rows1 < 0 || rows2 < 0 || fun_ptr == NULL || !is_sorted(rows1, array1, fun_ptr) || !is_sorted(rows2, array2, fun_ptr))
    {
        free(array1);
        free(array2);
        sprintf(output, "%s", "e:File is invalid or unsorted \n^");
        return;
    }
    merge(array1, 0, rows1 - 1, array2, 0, rows2 - 1, fun_ptr);
    strcpy(output, "s:");
    array_to_buffer(rows1 + rows2, array1, output + 2);
    int offset = strlen(output);
    output[offset] = '^';
    output[offset + 1] = '\0';
    free(array1);
    free(array2);
}
/*utility function to parse character array to array of Record structure.
And find similar pairs of records between two arrays.
Note: as no of maximum similar pairs can be O(n*n).
So to avoid any issues maximum similar pairs are restricted to 10k.
If similar pair count exceeds 20k limit, it will 
*/
void similar_records(char *buffer, char *output)
{
    int delimeter = first_occur(buffer, '$');
    Record *array1 = (Record *)malloc(max_records * sizeof(Record));
    Record *array2 = (Record *)malloc(max_records * sizeof(Record));
    int rows1 = buffer_to_array(array1, buffer);
    int rows2 = buffer_to_array(array2, buffer + delimeter + 1);
    if (rows1 < 0 || rows2 < 0)
    {
        sprintf(output, "%s", "e:File is invalid\n^");
        return;
    }
    Record **array = (Record **)malloc(max_records * sizeof(Record *));
    for (int i = 0; i < max_records; i++)
        array[i] = (Record *)malloc(2 * sizeof(Record));
    
    int k = 0;
    for (int i = 0; i < rows1; i++)
    {
        for (int j = 0; j < rows2; j++)
        {
            if (is_similar(array1[i], array2[j]))
            {
                if (k >= max_records)
                {   for (int i = 0; i < max_records; i++)
                        free(array[i]);

                    sprintf(output, "%s", "s:Too many similar records found. Maximum 4k pairs can be sent\n^");
                    return;
                }
                array[k][0] = array1[i];
                array[k][1] = array2[j];
                k++;
            }
        }
    }
    strcpy(output, "s:");
    if (k == 0)
    {
          
    for (int i = 0; i < max_records; i++)
        free(array[i]);
    sprintf(output + 2, "%s", "No similar records found\n^");
        return;
    }
    else
    {
        array_to_buffer_2d(k, array, output + 2);
    }
    int offset = strlen(output);
    output[offset] = '^';
    output[offset + 1] = '\0';
    
    for (int i = 0; i < max_records; i++)
        free(array[i]);
}
int main(int argc, char *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    bzero((char *)&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = SEM_UNDO;
    pop.sem_op = -1; //wait operation
    vop.sem_op = 1;  //signal operation
    print_semaphore = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT);
    semctl(print_semaphore, 0, SETVAL, 1);
    int opt = 1;
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   (char *)&opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

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
    socklen_t clilen;            //storing length for client address, i.e. 32 bit integer
    clilen = sizeof(cli_addr);
    int p_track[5];
    for (int i = 0; i < 5; i++)
    {
        p_track[i] = -1;
    }
    puts("Waiting for connections ...");
    int client_allowed = max_clients;

    while (true)
    {
        //waiting for the request from a client

        if ((new_socket = accept(server_fd, (struct sockaddr *)&cli_addr,
                                 (socklen_t *)&clilen)) < 0)
        {
            P(print_semaphore);
            perror("accept");
            V(print_semaphore);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < 5; i++)
        {
            if (p_track[i] < 0)
            {
                continue;
            }
            int status;
            int cid = waitpid(p_track[i], &status, WNOHANG);
            if (cid != 0)
            {
                p_track[i] = -1;
                client_allowed++;
            }
        }

        if (client_allowed <= 0)
        {
            const char *msg = "error";
            send(new_socket, msg, 0, 0);
            P(print_semaphore);
            cout << "Connection denied" << endl;
            V(print_semaphore);

            close(new_socket);
            continue;
        }

        int pid = fork();
        if (pid < 0)
        {
            const char *msg = "error";
            send(new_socket, msg, 0, 0);
            close(new_socket);
            P(print_semaphore);
            cout << "Error on fork" << endl;
            V(print_semaphore);

            continue;
        }
        if (pid == 0)
        {
            close(server_fd);
            const char *msg = ("Hello from the server (implemented using fork)\n");
            send(new_socket, msg, strlen(msg), 0);

            char *buffer = (char *)malloc(buffer_size * sizeof(char));
            char *output = (char *)malloc(buffer_size * sizeof(char));
            P(print_semaphore);
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            V(print_semaphore);
            while (true)
            {
                valread = recv_all(new_socket, buffer, buffer_size);
                if (valread == 0)
                {
                    P(print_semaphore);
                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                    V(print_semaphore);
                    close(new_socket);
                    break;
                }
                else
                {
                    P(print_semaphore);
                    cout << "----------------------------------------------" << endl;
                    printf("%s", buffer);
                    cout << "----------------------------------------------" << endl;
                    V(print_semaphore);
                    
                    int op = first_occur(buffer, '\n');
                    char operation[128] = {0};
                    strncpy(operation, buffer, op);
                    if (strcmp(operation, "sort") == 0)
                    {
                        sort_records(buffer + op + 1, output);
                    }
                    else if (strcmp(operation, "merge") == 0)
                    {
                        merge_records(buffer + op + 1, output);
                    }
                    else if (strcmp(operation, "similarity") == 0)
                    {
                        similar_records(buffer + op + 1, output);
                    } 
                    send(new_socket, output, strlen(output), 0);
                }
            }
            free(buffer);
            free(output);

            exit(0);
        }
        else
        {
            close(new_socket);
            client_allowed--;

            for (int i = 0; i < 5; i++)
            {
                if (p_track[i] == -1)
                {
                    p_track[i] = pid;
                    break;
                }
            }
            P(print_semaphore);
            cout << "Maximum clients allowed: " << client_allowed << endl;
            V(print_semaphore);
        }
    }
    return 0;
}
