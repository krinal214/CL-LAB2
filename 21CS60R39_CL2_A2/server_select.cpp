
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
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#define buffer_size 524308
#define max_records 4000
#define TRUE 1
#define FALSE 0
#define PORT 8888
#define max_clients 5
using namespace std;

int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
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
    cout<<rows1<<" "<<rows2<<endl;
    for (int i = 0; i < rows1; i++)
    {
        for (int j = 0; j < rows2; j++)
        {
            if (is_similar(array1[i], array2[j]))
            {
                if (k >= max_records)
                {   for (int i = 0; i < max_records; i++)
                        free(array[i]);

                    sprintf(output, "%s", "s:Too many similar records found. Maximum 4k pairs can be sent.\n^");
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
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[max_clients],
        activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char *buffer[5];
    char *output[5];
    for (int i = 0; i < max_clients; i++)
    {
        buffer[i] = (char *)malloc(buffer_size * sizeof(char));
        output[i] = (char *)malloc(buffer_size * sizeof(char));
    }
    int buffer_pointer[max_clients];

    //set of socket descriptors
    fd_set readfds;

    //a message
    const char *message = "Hello from the server (implemented using select)\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
        buffer_pointer[i] = 0;
    }

    //create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
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

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error\n");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
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
                    buffer_pointer[i] = 0;
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
                if ((valread = read(sd, buffer[i] + buffer_pointer[i], buffer_size - buffer_pointer[i])) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_allowed++;
                    client_socket[i] = 0;
                }

                else

                {
                    buffer_pointer[i] += valread;
                    if (buffer[i][buffer_pointer[i] - 1] == '^')
                    {
                        buffer[i][buffer_pointer[i] - 1] = '\0';
                        cout << "----------------------------------------------" << endl;
                        printf("%s", buffer[i]);
                        cout << "----------------------------------------------" << endl;

                        int op = first_occur(buffer[i], '\n');
                        char operation[128] = {0};
                        strncpy(operation, buffer[i], op);
                        if (strcmp(operation, "sort") == 0)
                        {
                            sort_records(buffer[i] + op + 1, output[i]);
                        }
                        else if (strcmp(operation, "merge") == 0)
                        {
                            merge_records(buffer[i] + op + 1, output[i]);
                        }
                        else if (strcmp(operation, "similarity") == 0)
                        {
                            similar_records(buffer[i] + op + 1, output[i]);
                        }
                        buffer_pointer[i]=0;
                        buffer[i][0]=0;
                        send(sd, output[i], strlen(output[i]), 0);
                    }
                }
            }
        }
    }
    for (int i = 0; i < max_clients; i++)
    {
        free(buffer[i]);
        free(output[i]);
    }

    return 0;
}
