Instructions to execute the client program:
1) create different folders for different clients and place file in respective folders. Later execute using below command.
   ./a.out folder path
   for example: ./a.out client_1/
   After running the client program, no need to provide entire file path in read,insert,etc methods.
2) Files uploaded on server are stored in main folder.
3) Always execute /exit command at client side before terminating the server. Reason: Server program perform cleanup tasks(updating permission file and removing    files when client exits. If you abruptly terminate server program, this cleanup task will not be performed and files remains there and in next run it will throw an error that file already exists.

Design choices: 
1) I am maintaing a permission.txt file for maintaing a permission records.
2) structure of permission record:  file_name   owner  user  permission
3) character '^' is reserved for marking start and end of a message. Reason, for a larger message it might be possible that message internally got fragmented.
4) character '|' is used as a new line character in insert_line functionality.
5) My client is also even-driven program: It waits for two events standard input or event of socket.
6) If a user already have 'E' permission then request for 'V' will get response: user already have a requested permission.

