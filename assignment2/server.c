// Server side C/C++ program to demonstrate Socket programming

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#define PORT 8080
int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    printf("execve=0x%p\n", execve);

    // Pass socket file descriptor
    if(strcmp(argv[0], "child_proc") == 0)
    {
        // pass from server to exec'd child
        // using command line argument: argv
        int socket_to_pass = atoi(argv[1]);
        // read values sent by the client
        valread = read( socket_to_pass, buffer, 1024);
        printf("%s\n",buffer );
        send(socket_to_pass , hello , strlen(hello) , 0 );
        printf("Hello message sent from child process\n");
        exit(0);
    }

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, 
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Socket attachment complete, now we will insert privilege separation logic here.
   

    pid_t pid = fork();

    if(pid<0) {
    	printf("fork failed");
    	exit(1);
    }
    if(pid==0) {
    	// change hello message to verify message is being sent from child
    	hello = "Hello from server child."
    	// obtain ID of "nobody" user
    	struct passwd* nobody_pwd=getpwnam("nobody");
        	// drop privileges to "nobody" user
    	setuid(nobody_pwd -> pw_uid);	

        // handle socket passing from server to exec'd child
    	char str[10];
        snprintf(str, 10, "%d", socket_to_pass);
        char *args[] = {"child_proc", str, NULL};

        // re-exec the process
        int valid = execvp(argv[0], args);
        // check if re-exec worked
        if(valid < 0) {
            printf("could not re-exec process\n");
            exit(1);
        }
    }
    // wait for child process to exit
    wait(NULL);
    return 0;
}
