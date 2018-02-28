#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <iostream>

using namespace std;

#define SOCK_PATH "/home/beniva/Desktop/code block/selectServer/echo_socket"

int main(void)
{
    int s, len;
    struct sockaddr_un remote;
    char buf[256];
    char str[100];
    string input;


    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1)
    {
        perror("connect");
        exit(1);
    }

    printf("Connected.\n");

    pid_t childPID;
    int var_lcl = 0;
    childPID = fork();

    while(1)
    {
        if(childPID >= 0) // fork was successful
        {
            if(childPID == 0) // child process
            {
                getline(cin, input);
                strcpy(buf, input.c_str());
                cout<<endl;
                if ((send(s, buf,sizeof(buf),0))== -1)
                {
                    fprintf(stderr, "Failure Sending Message\n");
                    close(s);
                }
            }
            else //Parent process
            {
                cout<<"send: "<<endl;
                if ((len = recv(s, buf,sizeof(buf), 0)) == -1)
                {
                    perror("recv");
                }
                cout<<".............................................."<<endl;

                buf[len] = '\0';

                cout<<"client recive: "<<endl;
                cout<<buf<<endl<<".............................................."<<endl;

            }
        }
        else // fork failed
        {
            printf("\n Fork failed\n");
            return 1;
        }

    }


    close(s);

    return 0;
}
