/**
 * COE 768 Final Project
 * Index Server
 *
 * Name              : Yadu Krishnan Madhu
 * Section           : 06
 * Student Number    : 500975010
 *
 * Partner Name      : Tamim Rahman (Client)
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#define MAX_SIZE 255

// General PDU format
struct pdu
{
    char type;
    char data[MAX_SIZE];
};

// General structure for the Send protocol. Type must be 'A' for ack, 'S' for search
// and 'R' for conttent registration
struct SPDU
{
    char type;
    char address[MAX_SIZE];
};

// General structure of registered peer
struct RegisteredPeer
{
    char type;
    char name[MAX_SIZE];
    char contentname[MAX_SIZE];
    char address[MAX_SIZE];
};
struct RegisteredPeer registry[10]; // This registry contains all the entries

// function returns whether the recieved entry exists in the registry (1 if exists, 0 otherwise).
int checkForEntry(size_t numElements, const char *peer_name, const char *content_name)
{
    int i;
    for (i = 0; i < numElements; ++i)
    {
        if (strcmp(registry[i].name, peer_name) == 0 && strcmp(registry[i].contentname, content_name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// function returns the pointer to the required entry in the register. NULL if not found (error)
const char *findAddr(size_t numElements, const char *peer_name, const char *content_name)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        printf("%s \n", registry[i].name);
        printf("%s \n", registry[i].contentname);

        if (strcmp(registry[i].name, peer_name) == 0 && strcmp(registry[i].contentname, content_name) == 0)
        {
            return registry[i].address;
        }
    }
    return NULL;
}

// function adds the entry into the registry, throws error if exceeds max number of entries (10).
void addEntry(int index, const char *s, const char *f, const char *c)
{
    // Check if the index is within bounds
    if (index >= 0 && index < 10)
    {
        strncpy(registry[index].name, s, sizeof(registry[index].name) - 1);
        strncpy(registry[index].contentname, f, sizeof(registry[index].contentname) - 1);
        strncpy(registry[index].address, c, sizeof(registry[index].address) - 1);

        // Set up string termination for peer name, content name and address.
        registry[index].name[sizeof(registry[index].name) - 1] = '\0';
        registry[index].contentname[sizeof(registry[index].contentname) - 1] = '\0';
        registry[index].address[sizeof(registry[index].address) - 1] = '\0';
    }
}

// Prints all the contents of the server
void printContentServers(size_t numElements)
{
    int i;
    for (i = 0; i < numElements; i++)
    {
        if (!checkIndex(i))
        {
            printf("Peer Name: %s \n", registry[i].name);
            printf("Content Name: %s \n", registry[i].contentname);
            printf("Content Address: %s \n\n", registry[i].address);
        }
    }
}

int checkIndex(size_t index)
{
    // Check if the peername field is an empty string or some default value
    return registry[index].name[0] == '\0';
}

int main(int argc, char *argv[])
{
    struct sockaddr_in fsin;        // the from address of a client
    struct RegisteredPeer spdu;     // Struct for send PDU
    struct pdu response;            // Struct pdu for sending back reponse
    int sock, n, addrLen;           // server socket, n to keep track, address length
    struct sockaddr_in sin;         // Incoming socket connection
    int s, type;                    // socket descriptor and socket type
    int registeredServersIndex = 0; // setting the default index location,
    char *portNum = "5000";         // Setting port number
    char buf[100];                  // input buffer to temporarily store incoming packets
    char *pts;

    switch (argc)
    {
    case 1:
        break;
    case 2:
        portNum = argv[1];
        break;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;

    // Setting up port number
    sin.sin_port = htons((__u_short)atoi(portNum));

    // Allocating socket
    s = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind the socket
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        fprintf(stderr, "Unable to bind socket\n")
    };

    while (1)
    {

        addrLen = sizeof(fsin);

        // reading incoming UDP connection for registering/deregistering content from peers
        if ((n = recvfrom(s, (void *)&spdu, sizeof(spdu), 0, (struct sockaddr *)&fsin, &addrLen)) < 0)
        {
            fprintf(stderr, "Error: unable to receive UDP packets\n")
        };

        // Following section checks for the incoming PDU type and executes appropriate actions based on it

        // If 'S' type then execute the search for the content within the registry
        if (spdu.type == 'R')
        {
            printf("New content Registration\n");

            // Checks for entry. If entry already exists, then error is sent back since duplicate entries are not allowed
            if !(checkForEntry(10, spdu.name, spdu.contentname))
            {
                response.type = 'A';
                addEntry(registeredServersIndex, spdu.name, spdu.contentname, spdu.address);
                printf("Successfully registered new content and server.\n");
                printContentServers(10);
                registeredServersIndex++;
                (void)sendto(s, &response, strlen(response.data) + 1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
            }
            else
            { // If no dupe is found, a ACK is sent back to indicate success.
                printf("Error: Peer name and content name already registered\n");
                response.type = 'E';
                (void)sendto(s, &response, strlen(response.data) + 1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
            }

        } // If 'R' type, then a registration of the content is executed
        else if (spdu.type == 'S')
        {
            printf("Content Download request sent by content client \n");
            printf("Recieved Peer name is: %s\n", spdu.name);
            printf("Recieved Content name is %s\n", spdu.contentname);

            // Retrieves the address of the content server which contains the required content
            const char *foundAddress = findAddr(10, spdu.name, spdu.contentname);

            if (foundAddress == NULL)
            {
                printf("Error: Content entry not found\n");
                printf("List of available content: \n");
                printContentServers(10);
            }
            else
            {
                response.type = 'S';
                strncpy(response.data, foundAddress, MAX_SIZE);
                printf("Success: Content Found\n");
            }
            // Sends the reponse
            (void)sendto(s, &response, strlen(response.data) + 1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
        }
    }
}
