/*
** selectserver.c -- a cheezy multiperson chat server
*/


/////////////////////////////
//    beni valotker       //
//     300106937         ///
////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <string>
#include <dirent.h>
#include <fstream>
#include <sstream>


using namespace std;

#define PORT "9034"   // port we're listening on
#define PATH "/home/beniva/Desktop/code block/server"          //server DIR path
#define SOCK_PATH "echo_socket"

static int counter = 0;

//to hold the id connection
struct ID
{
    int id;
    int massageCounter;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string lsFunc(const char* argv)
{
    DIR* directory;
    struct dirent* ent;
    string buffer;
    string temp;

    ifstream myfiles;
    ofstream myfile;
    myfile.open ("ls.txt");

    if ((directory = opendir(argv)) == NULL)
    {
        fprintf(stderr, "Could not open DIR: %s\n", argv);
    }

    while ((ent = readdir(directory)) != NULL)
    {
        myfile << ent->d_name;
        myfile << endl;
    }
    myfile.close();

    myfiles.open("ls.txt");
    if (myfiles.is_open())
    {
        while (1)
        {
            getline (myfiles,temp);
            buffer = buffer + "\n" + temp;
            if(myfiles.eof())
                break;
        }
        myfile.close();
    }
    closedir(directory);

    if( remove( "ls.txt" ) != 0 )
        perror( "Error deleting file" );

    return buffer;
}

string download(char* filename)
{

    ifstream fin;
    DIR *pDir;
    pDir=opendir(PATH);
    struct dirent *pDirent;
    char buf[100];
    const char* buff = buf;

    while ((pDirent = readdir(pDir)) != NULL)
    {
        if(strcmp(filename, pDirent->d_name) == 0)
        {
            realpath(pDirent->d_name, buf);
            fstream f(buff, fstream::in );
            string s;
            getline( f, s, '\0');
            f.close();
            return s;
        }
    }
    closedir (pDir);
    return "0";
}

bool stringChack(char str[], char str2[])
{

    int i = 0;
    while(1)
    {
        if(str[i] == str2[i])
        {
            i++;
        }
        else
        {
            return 0;
        }

        if(str[i] == '[' && str2[i] == '[')
            return 1;

    }
}

string getid(struct ID* id)
{
    ifstream myfiles;
    ofstream myfile;

    string buffer;
    string temp;

    myfile.open ("connection.txt");
    myfile << "connected list: " << endl;
    for(int i = 0; i < 20; i++ )
    {
        if(id[i].id != 0 )
        {
            myfile << "user id - " <<"[" << id[i].id << "]";
            myfile << endl;
        }
    }
    myfile.close();

    myfiles.open("connection.txt");
    if (myfiles.is_open())
    {
        while (1)
        {
            getline (myfiles,temp);
            buffer = buffer + "\n" + temp;
            if(myfiles.eof())
                break;
        }
        myfile.close();
    }

    if( remove( "connection.txt" ) != 0 )
        perror( "Error deleting file" );

    return buffer;
}

string maxM(struct ID id[])
{
    int maximum = 0, maxNumber = 0;

    ifstream myfiles;
    ofstream myfile;

    string buffer;
    string temp;

    for(int i =0; i < 20 ; i++)
    {
        if(id[i].massageCounter > maximum)
        {
            maximum = id[i].massageCounter;
            maxNumber = i;
        }
    }


    myfile.open("max.txt");
    myfile << "connected: " << endl;
    myfile << "id number " <<"["<< id[maxNumber].id << "]" << " = " << maximum;
    myfile.close();

    myfiles.open("max.txt");
    if (myfiles.is_open())
    {
        while (1)
        {
            getline (myfiles,temp);
            buffer = temp + "\n";
            if(myfiles.eof())
                break;
        }
        myfile.close();
    }

    if( remove( "max.txt" ) != 0 )
        perror( "Error deleting file" );

    return buffer;
}

int kickById(string num, struct ID id[])
{

    int temp = atoi(num.c_str());

    for(int i = 0; i <20 ; i++)
    {
        if(id[i].id == temp)
            return (temp - 100);
    }
    return 0;
}


int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;
    int n;
    int adminfd;
    string buffer;
    char* conToBuf;
    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv, k;

    struct addrinfo hints, *ai, *p;
    struct timeval timeout;
    struct timeval timeout1;
    struct ID id[20] = {0};

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);


    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }
        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL)
    {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1)
    {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    //admin
    int s, s2, len;
    struct sockaddr_un local, remote;
    char str[100];

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }


    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(s, 1) == -1)
    {
        perror("listen");
        exit(1);
    }


    int fdmax1;
    fd_set rfd;
    fd_set rf;

    FD_ZERO(&rfd);
    FD_ZERO(&rf);

    FD_SET(s, &rfd);


    fdmax1 = s;

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    // main loop
    for(;;)
    {
        read_fds = master; // copy it
        rf = rfd;

        if (select(fdmax+1, &read_fds, NULL, NULL, &timeout) == -1)
        {
            perror("select");
            exit(4);
        }

        if (select(fdmax1+1, &rf, NULL, NULL, &timeout) == -1)
        {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))   // we got one!!
            {
                if (i == listener)
                {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *)&remoteaddr,
                                   &addrlen);

                    if (newfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax)      // keep track of the max
                        {
                            fdmax = newfd;
                        }

                        id[newfd].id = newfd + 100;
                        printf("selectserver: new connection from %s on "
                               "socket %d ID number %d\n",
                               inet_ntop(remoteaddr.ss_family,
                                         get_in_addr((struct sockaddr*)&remoteaddr),
                                         remoteIP, INET6_ADDRSTRLEN),
                               newfd, id[newfd].id);
                    }
                }
                else
                {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0)
                    {
                        // got error or connection closed by client
                        if (nbytes == 0)
                        {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        id[i].id = 0;
                        id[i].massageCounter = 0;
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                    else
                    {
                        if(strcmp(buf, "listfiles") == 0)
                        {
                            buffer = lsFunc(PATH);
                            conToBuf = new char[buffer.length()+1];
                            strcpy(conToBuf, buffer.c_str());
                            send(i, conToBuf, strlen(conToBuf), 0);
                        }
                        else if(stringChack(buf, "download["))    //download file from server
                        {
                            char *bufr;
                            string  temp = "";
                            stringstream ss;

                            bufr = buf;
                            for(int f = 0; f < 30; f++)
                            {

                                if(bufr[f] == '[')
                                {
                                    ++f;
                                    while(1)
                                    {
                                        std::string temp2(1,bufr[f++]);
                                        temp += temp2;
                                        if(bufr[f] == ']')
                                            break;
                                    }

                                }
                            }
                            conToBuf = new char[temp.length()+1];
                            strcpy(conToBuf, temp.c_str());
                            buffer = download(conToBuf);
                            if(FD_ISSET(i, &master) && (buffer.compare("0") != 0))
                            {
                                conToBuf = new char[buffer.length()+1];
                                strcpy(conToBuf, buffer.c_str());
                                send(i, conToBuf, strlen(conToBuf), 0);
                            }
                        }
                        else
                        {
                            // we got some data from a client
                            for(j = 0; j <= fdmax; j++)
                            {
                                // send to everyone!
                                if (FD_ISSET(j, &master))
                                {
                                    // except the listener and ourselves
                                    if (j != listener && j != i)
                                    {
                                        if (send(j, buf, nbytes, 0) == -1)
                                        {
                                            perror("send");
                                        }
                                        else
                                        {
                                            id[i].massageCounter++;
                                        }
                                    }
                                }
                            }
                        }
                    } // END handle data from client
                } // END got new incoming connection
            } // END looping through file descriptors

        }
        for(k = 0; k <= fdmax1; k++ )
        {
            if (FD_ISSET(k, &rf))
            {
                if(k == s && counter == 0)
                {
                    printf("Waiting for a connection...\n");
                    socklen_t t = sizeof(remote);

                    if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1)
                    {
                        perror("accept");
                        exit(1);
                    }
                    adminfd = s2;
                    printf("Admin connect\n");
                    counter++;
                    FD_SET(s2, &rfd);
                    fdmax1 = s2;

                }
                else
                {
                    if(n = recv(s2, str, sizeof str, 0) <= 0)
                    {

                        if (n == 0)
                        {
                            // connection closed
                            printf("selectserver: Admin hung up\n");
                        }
                        else
                        {
                            perror("recv");
                        }

                        counter--;
                        close(k); // bye!
                        FD_CLR(k, &rfd); // remove from master set
                    }
                    else
                    {

                        if(strcmp(str, "list") == 0)
                        {
                            buffer = getid(id);
                            conToBuf = new char[buffer.length()+1];
                            strcpy(conToBuf, buffer.c_str());
                            send(s2, conToBuf, strlen(conToBuf), 0);
                        }
                        else if(strcmp(str, "chatmaster") == 0)
                        {
                            string buffer = maxM(id);
                            conToBuf = new char[buffer.length()+1];
                            strcpy(conToBuf, buffer.c_str());
                            send(k, conToBuf, strlen(conToBuf), 0);
                        }
                        else if(stringChack(str, "kick["))
                        {
                            char *bufr;
                            string  temp = "";
                            stringstream ss;

                            bufr = str;
                            for(int f = 0; f < 30; f++)
                            {
                                if(bufr[f] == '[')
                                {
                                    ++f;
                                    while(1)
                                    {
                                        std::string temp2(1,bufr[f++]);
                                        temp += temp2;
                                        if(bufr[f] == ']')
                                            break;

                                    }

                                }
                            }
                            int tempR = kickById(temp, id);
                            if(tempR != 0 && tempR != s && tempR != listener)
                            {
                                close(tempR);
                                FD_CLR(tempR, &master);
                                printf("client with id  %d kick\n", id[tempR].id);
                                id[tempR].id = 0;
                                id[tempR].massageCounter = 0;

                            }
                        }
                        else if(strcmp(str, "kickall") == 0)
                        {
                            for(int i=0; i <= fdmax; i++)
                            {
                                if(id[i].id != 0)
                                {
                                    close(i);
                                    FD_CLR(i, &master);
                                    id[i].id = 0;
                                    id[i].massageCounter = 0;
                                    printf("All client kick\n");
                                }
                            }
                        }
                        else if(stringChack(str, "download["))    //download file from server
                        {
                            char *bufr;
                            string  temp = "" ;
                            stringstream ss;

                            bufr = str;
                            for(int f = 0; f < 30; f++)
                            {

                                if(bufr[f] == '[')
                                {
                                    ++f;
                                    while(1)
                                    {
                                        std::string temp2(1,bufr[f++]);
                                        temp += temp2;
                                        if(bufr[f] == ']')
                                            break;

                                    }

                                }
                            }
                            conToBuf = new char[temp.length()+1];
                            strcpy(conToBuf, temp.c_str());
                            buffer = download(conToBuf);
                            if(FD_ISSET(k, &rfd) && (buffer.compare("0") != 0))
                            {
                                conToBuf = new char[buffer.length()+1];
                                strcpy(conToBuf, buffer.c_str());
                                send(k, conToBuf, strlen(conToBuf), 0);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
