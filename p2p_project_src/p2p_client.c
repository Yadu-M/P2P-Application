/* p2p_client.c - main

Name: Tamim Rahman (Client)
ID: 500967494
Section: 03
Partner: Yadu Krishnan Madhu (Server)

*/
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <errno.h>

#define BUFSIZE 64

struct pdu
{
    char type;
    char data[255];
};

struct DPDU
{
    char type;
    char contentName[255];
};

struct APDU
{
    char type;
    char peerName[255];
    char contentName[255];
};

struct RPDU
{
    char type;
    char peerName[255];
    char contentName[255];
    char address[255];
};

char *removeSpace(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
    {
        str += 1;
    }
    if (*str == 0)
    {
        return str;
    }
    end = str + strlen(str) - 1;
    while ((isspace((unsigned char)*end) && (end > str)))
    {
        end -= 1;
    }
    end[1] = '\0';
    return str;
}

int main(int argc, char *argv[])
{
    int s, n, type;
    char *host = "placeholder";
    char *port = "5000";

    port = argv[2];
    host = argv[1];

    struct hostent *phe;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    sin.sin_port = htons((u_short)atoi(port));
    sin.sin_addr.s_addr = inet_addr(host)
        phe = gethostbyname(host) if (phe)
    {
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    }
    else if (INADDR_NONE == sin.sin_addr.s_addr)
    {
        fprintf(stderr, "Error: Connection issue \n");
        exit(1);
    }

    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        fprintf(stderr, "Error: Unable to create the socket\n");
        exit(1);
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        fprintf(stderr, "Error: Unable to connect on %s %s \n", host, port);
        exit(1);
    }
    struct RPDU client_content_pdu;
    struct APDU search_pdu;
    struct pdu cpdu;
    struct pdu res;
    struct DPDU dpdu;
    struct RPDU content_pdu;
    struct pdu cpdu_res;

    while (1)
    {
        int user_choice;
        printf("---------P2P Content Downloader----------\n");
        printf("What would you like to do?: \n");
        printf("1. Host Content \n");
        printf("2. Download Content \n");
        scanf("%d", &user_choice);
        if (user_choice == 1)
        {
            struct sockaddr_in content_addr;
            content_addr.sin_addr.s_addr = htonl(INADDR_ANY);
            content_addr.sin_port = htons(0);
            content_addr.sin_family = AF_INET;
            int stcp;
            int addr_len;
            stcp = socket(AF_INET, SOCK_STREAM, 0);

            if (bind(stcp, (struct sockaddr *)&content_addr, sizeof(content_addr)) < 0)
            {
                fprintf(stderr, "Error: Socket Bind issue");
            }
            char con_host_ip[INET_ADDRSTRLEN];
            char full_address[255];
            addr_len = sizeof(struct sockaddr_in);
            int check = getsockname(stcp, (struct sockaddr *)&content_addr, &addr_len);
            char ip_address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(content_addr.sin_addr), ip_address, INET_ADDRSTRLEN);

            content_pdu.type = 'R';
            printf("Please enter your ip address: \n");
            scanf("%s", con_host_ip);
            printf("Please select a name for your peer: \n");
            scanf("%s", content_pdu.peerName);
            printf("Please enter the name of the content you would like to host: \n");
            scanf("%s", content_pdu.contentName);

            snprintf(full_address, sizeof(full_address), "%s:%d", con_host_ip, ntohs(content_addr.sin_port));
            strcpy(content_pdu.address, full_address);

            write(s, &content_pdu, 765);
            n = read(s, &res, sizeof(res));
            if (res.type == 'A')
            {
                printf("Index Server successfully registered your content!\n");
                if (listen(stcp, 5) < 0)
                {
                    perror("listen");
                    exit(EXIT_FAILURE);
                }
                printf("Receiving content download requests on %d\n", ntohs(content_addr.sin_port));
                fd_set rfds, afds;
                FD_ZERO(&afds);
                FD_SET(stcp, &afds);
                FD_SET(0, &afds);
                int new_sd;
                char buf[255];
                struct sockaddr_in client;

                while (1)
                {
                    memcpy(&rfds, &afds, sizeof(rfds));

                    if (select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0)
                    {
                        perror("select");
                        exit(EXIT_FAILURE);
                    }

                    if (FD_ISSET(0, &rfds))
                    {
                        n = read(0, buf, BUFSIZE);
                        printf("buf here is %s\n", buf);
                    }

                    if (FD_ISSET(stcp, &rfds))
                    {
                        new_sd = accept(stcp, (struct sockaddr *)&client, &addr_len);
                        if (new_sd != -1)
                        {
                            printf("Content Registration\n");
                            char sbuf[255];
                            int n;
                            struct DPDU serverres;
                            n = read(new_sd, &serverres, 255);

                            if (serverres.type == 'D')
                            {
                                cpdu.type = 'C';
                                char *rem;
                                rem = removeSpace(serverres.contentName);

                                FILE *fp = fopen(rem, "r");
                                if (fp != NULL)
                                {
                                    size_t bytesRead;
                                    while ((n = fread(sbuf, 1, 255, fp)) > 0)
                                    {
                                        strcpy(cpdu.data, sbuf);
                                        write(new_sd, &cpdu, 255);
                                    }
                                    fclose(fp);
                                    close(stcp);
                                }
                                else
                                {
                                    printf("Error: File not found \n");
                                    write(stcp, "NO FILE", n);
                                    close(stcp);
                                    return 1;
                                }
                                return 0;
                            }
                        }
                    }
                }
            }
            else if (res.type = 'E')
            {
                printf("Peer name already exists.\n");
                printf("Please choose another peer name.\n");
            }
        }

        else if (user_choice == 2)
        {
            search_pdu.type = 'S';
            printf("Please enter the name of the content you would like to downlaod: \n");
            scanf("%s", search_pdu.contentName);
            printf("Please enter the name of the peer you would like to download from: \n");
            scanf("%s", search_pdu.peerName);

            write(s, &search_pdu, 510);
            n = read(s, &res, sizeof(res));
            if (n < 0)
            {
                fprintf(stderr, "Error: Issue with reading response\n");
            }

            if (res.type == 'S')
            {
                printf("Index Server: Content has been found!\n");
                printf("Address for Content: %s\n", res.data);

                char str[128];
                char *ptr;
                strcpy(str, res.data);
                strtok_r(str, ":", &ptr);
                printf("'%s'  '%s'\n", str, ptr);

                int sd;
                struct sockaddr_in server;
                struct hostent *hp;
                if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
                {
                    fprintf(stderr, "Error: Unable to form socket\n");
                    exit(1);
                }

                bzero((char *)&server, sizeof(struct sockaddr_in));
                server.sin_family = AF_INET;
                server.sin_port = htons(atoi(ptr));
                if (hp = gethostbyname(str))
                    bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
                else if (inet_aton(host, (struct in_addr *)&server.sin_addr))
                {
                    fprintf(stderr, "Can't get server's address\n");
                    exit(1);
                }
                if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
                {
                    fprintf(stderr, "Can't connect \n");
                    exit(1);
                }

                char cbuf[255];
                printf("Connected to content server!\n");

                dpdu.type = 'D';
                strcpy(dpdu.contentName, search_pdu.contentName);
                size_t contentNameLength = strlen(search_pdu.contentName);

                printf("File name sent to content server: %s\n", search_pdu.contentName);
                size_t bytes_written = write(sd, &dpdu, 255);

                if (bytes_written < 0)
                {
                    perror("Error writing to socket");
                    // Handle the error, e.g., return or exit
                }
                printf("Bytes written from client: %d\n", bytes_written);
                printf("Preparing to send you content...\n");

                char *rem = removeSpace(search_pdu.contentName);
                char rbuf[255];

                FILE *fp = fopen(rem, "w");

                if (fp == NULL)
                {
                    perror("Error opening file for writing.");
                    close(sd);
                    exit(1);
                }

                while ((n = read(sd, &cpdu_res, 255)) > 0)
                {
                    if (cpdu_res.type == 'C')
                    {
                        fprintf(fp, cpdu_res.data);
                    }
                }

                fclose(fp);
                printf("Success: File has been downloaded!\n");
                close(sd);

                char con_host_ip[INET_ADDRSTRLEN];

                printf("Transforming into a content serving peer...\n");
                int stcp;
                struct sockaddr_in content_addr;
                int addr_len;
                stcp = socket(AF_INET, SOCK_STREAM, 0);
                content_addr.sin_family = AF_INET;
                content_addr.sin_port = htons(0);
                content_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                if (bind(stcp, (struct sockaddr *)&content_addr, sizeof(content_addr)) < 0)
                {
                    perror("Error: Socket binding error");
                    exit(EXIT_FAILURE);
                }

                addr_len = sizeof(struct sockaddr_in);
                int check = getsockname(stcp, (struct sockaddr *)&content_addr, &addr_len);

                char ip_address[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(content_addr.sin_addr), ip_address, INET_ADDRSTRLEN);

                char full_address[255];

                printf("Please enter your ip address: \n");
                scanf("%s", con_host_ip);
                printf("Enter your peer name: \n");
                scanf("%s", client_content_pdu.peerName);

                client_content_pdu.type = 'R';
                strcpy(client_content_pdu.contentName, search_pdu.contentName);
                snprintf(full_address, sizeof(full_address), "%s:%d", con_host_ip, ntohs(content_addr.sin_port));

                printf("Peer address and port: %s\n", full_address);
                strcpy(client_content_pdu.address, full_address);

                write(s, &client_content_pdu, 765);

                n = read(s, &res, sizeof(res));
                if (n < 0)
                {
                    fprintf(stderr, "Read failed\n");
                }

                if (res.type == 'A')
                {
                    printf("Index Server successfully registered your content!\n");
                    if (listen(stcp, 5) < 0)
                    {
                        perror("listen");
                        exit(EXIT_FAILURE);
                    }

                    printf("Content is hosted on port: %d \n", ntohs(content_addr.sin_port));
                }

                fd_set rfds, afds;
                FD_ZERO(&afds);
                FD_SET(stcp, &afds);
                FD_SET(0, &afds);

                int new_sd;
                char buf[255];
                struct sockaddr_in client;

                while (1)
                {
                    memcpy(&rfds, &afds, sizeof(rfds));

                    if (select(FD_SETSIZE, &rfds, NULL, NULL, NULL) < 0)
                    {
                        perror("select");
                        exit(EXIT_FAILURE);
                    }
                    if (FD_ISSET(0, &rfds))
                    {
                        n = read(0, buf, BUFSIZE);
                        printf("buf here is %s\n", buf);
                    }

                    if (FD_ISSET(stcp, &rfds))
                    {
                        new_sd = accept(stcp, (struct sockaddr *)&client, &addr_len);
                        if (new_sd != -1)
                        {
                            printf("Received content download request!\n");

                            char sbuf[255];
                            int n;
                            struct DPDU serverres;
                            n = read(new_sd, &serverres, 255);

                            if (serverres.type == 'D')
                            {
                                cpdu.type = 'C';
                                char *rem;
                                rem = removeSpace(serverres.contentName);
                                FILE *fp = fopen(rem, "r");
                                if (fp == NULL)
                                {
                                    printf("Error: File not found \n");
                                    write(stcp, "NO FILE", n);
                                    close(stcp);
                                    return 1;
                                }
                                else
                                {
                                    size_t bytesRead;
                                    while ((n = fread(sbuf, 1, 255, fp)) > 0)
                                    {
                                        strcpy(cpdu.data, sbuf);
                                        write(new_sd, &cpdu, 255);
                                    }
                                    fclose(fp);
                                    close(stcp);
                                }
                                return 0;
                            }
                        }
                    }
                }
                close(sd);
                return 0;
            }
        }
        else if (res.type = 'E')
        {
            printf("Error: Content not found!\n");
        }
    }
}
