#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "stdbool.h"

#define REQUIRED_ARGC 7
#define MSG_POS 2
#define ERROR 1
#define SUCCESS 0
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFFER_SIZE 1024
#define MANDATORY_ERR "Mandatory args missing!\n"
#define PORT_CHOSEN_ERR "Use a port number between 1025 and 65535\n"
#define CARRIAGE "\r\n"
#define IO_ERR "Unable to create a file descriptor to read socket\n"

int usage(char *progname) {
    fprintf(stderr, "usage: %s -p PORT -r DOCUMENT_DIRECTORY -t AUTH_TOKEN\n", progname);
    exit(ERROR);
}

int errexit(char *format, char *arg) {
    fprintf(stderr, format, arg);
    fprintf(stderr, "\n");
    exit(ERROR);
}

int create_socket_and_listen(char *port_number) {
    struct sockaddr_in sin;
    struct protoent *protoinfo;
    int sd;

    /* Determine protocol */
    if ((protoinfo = getprotobyname(PROTOCOL)) == NULL)
        errexit("cannot find protocol information for %s", PROTOCOL);

    /* Setup endpoint info */
    memset((char *)&sin, 0x0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons((u_short)atoi(port_number));

    /* Socket creation */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0)
        errexit("cannot create socket", NULL);

    /* Bind the socket */
    if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit("cannot bind to port %s", port_number);

    /* Listen for incoming connections */
    if (listen(sd, QLEN) < 0)
        errexit("cannot listen on port %s\n", port_number);

    printf("Server Address: %s\nSocket Created and is listening on port %d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    return sd;
}

/* Accept a connection. Buffer data sent by client */
int accept_a_connection_and_read_request(int listen_fd) {
    struct sockaddr_in client;
    unsigned int cli_len = sizeof(client);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0x0, sizeof(buffer));
    int sd2 = accept(listen_fd, (struct sockaddr *)&client, &cli_len);
    if (sd2 < 0)
        errexit("error accepting connection", NULL);
    printf("New Client connected from port number %d and IP address  %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));

    FILE *fp = fdopen(sd2, "r");
    if (fp == NULL) {
        printf(IO_ERR);
        exit(ERROR);
    }
    printf("Reading Request\n");
    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        printf("RSP: %s", buffer);
    }
    return sd2;
}

void write_to_connection(int conn_fd, char *reply) {
    /* write message to the connection */
    if (write(conn_fd, reply, strlen(reply)) < 0)
        errexit("error writing message: %s", reply);
}

bool isRequestWellFormed(const char *request) {
    // char *sample = "GET /5.txt HTTP/1.1\r\n\r\n";
    return true;
}

int main(int argc, char *argv[]) {
    int opt, sd, sd2;
    bool PORT_GIVEN = false;
    bool ROOT_DIR_GIVEN = false;
    bool AUTH_TOKEN_GIVEN = false;
    char *PORT_NUMBER = NULL;
    char *ROOT_DIR = NULL;
    char *AUTH_TOKEN = NULL;
    // char *REPLY = NULL;

    /* There should be be 7 elements in argv, without which we terminate the program execution. */
    if (argc != REQUIRED_ARGC) {
        usage(argv[0]);
    }

    while (optind < argc) {
        //  To disable the automatic error printing, a colon is added as the first character in optstring:
        if ((opt = getopt(argc, argv, ":p:r:t:")) != -1) {
            switch (opt) {
                case 'p':
                    PORT_GIVEN = true;
                    PORT_NUMBER = optarg;
                    break;
                case 'r':
                    ROOT_DIR_GIVEN = true;
                    ROOT_DIR = optarg;
                    break;
                case 't':
                    AUTH_TOKEN_GIVEN = true;
                    AUTH_TOKEN = optarg;
                    break;
                case '?':
                    printf("Unknown option: %c\n", optopt);
                    break;
                case ':':
                    printf("Missing arg for %c\n", optopt);
                    usage(argv[0]);
                    break;
            }
        } else {
            optind += 1;
        }
    }

    /* Mandatory arguments check */
    if (!PORT_GIVEN || !ROOT_DIR_GIVEN || !AUTH_TOKEN_GIVEN) {
        printf(MANDATORY_ERR);
        usage(argv[0]);
    }

    /* Check for PORT number range */
    int port_given_by_user = atoi(PORT_NUMBER);
    if (port_given_by_user < 1025 || port_given_by_user > 65535) {
        printf(PORT_CHOSEN_ERR);
        usage(argv[0]);
    }

    printf("PORT : %s\n", PORT_NUMBER);
    printf("ROOT_DIR : %s\n", ROOT_DIR);
    printf("AUTH_TOKEN : %s\n", AUTH_TOKEN);

    sd = create_socket_and_listen(PORT_NUMBER);
    sd2 = accept_a_connection_and_read_request(sd);
    // write_to_connection(sd2, REPLY);

    /* close connections and exit */
    printf("Closing connections\n");
    close(sd);
    close(sd2);
    exit(SUCCESS);
}
