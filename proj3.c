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
#define PORT_POS 1
#define MSG_POS 2
#define ERROR 1
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFLEN 1024
#define MANDATORY_ERR "Mandatory args missing!\n"
#define PORT_CHOSEN_ERR "Use a port number between 1025 and 65535\n"

int usage(char *progname) {
    fprintf(stderr, "usage: %s -p PORT -r DOCUMENT_DIRECTORY -t AUTH_TOKEN\n", progname);
    exit(ERROR);
}

int errexit(char *format, char *arg) {
    fprintf(stderr, format, arg);
    fprintf(stderr, "\n");
    exit(ERROR);
}

int main(int argc, char *argv[]) {
    // struct sockaddr_in sin;
    // struct sockaddr addr;
    // struct protoent *protoinfo;
    // unsigned int addrlen;
    // int sd, sd2;
    int opt;  // option
    bool PORT_GIVEN = false;
    bool ROOT_DIR_GIVEN = false;
    bool AUTH_TOKEN_GIVEN = false;
    char *PORT_NUMBER = NULL;
    char *ROOT_DIR = NULL;
    char *AUTH_TOKEN = NULL;

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

    // /* determine protocol */
    // if ((protoinfo = getprotobyname(PROTOCOL)) == NULL)
    //     errexit("cannot find protocol information for %s", PROTOCOL);

    // /* setup endpoint info */
    // memset((char *)&sin, 0x0, sizeof(sin));
    // sin.sin_family = AF_INET;
    // sin.sin_addr.s_addr = INADDR_ANY;
    // sin.sin_port = htons((u_short)atoi(argv[PORT_POS]));

    // /* allocate a socket */
    // /*   would be SOCK_DGRAM for UDP */
    // sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    // if (sd < 0)
    //     errexit("cannot create socket", NULL);

    // /* bind the socket */
    // if (bind(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    //     errexit("cannot bind to port %s", argv[PORT_POS]);

    // /* listen for incoming connections */
    // if (listen(sd, QLEN) < 0)
    //     errexit("cannot listen on port %s\n", argv[PORT_POS]);

    // /* accept a connection */
    // addrlen = sizeof(addr);
    // sd2 = accept(sd, &addr, &addrlen);
    // if (sd2 < 0)
    //     errexit("error accepting connection", NULL);

    // /* write message to the connection */
    // if (write(sd2, argv[MSG_POS], strlen(argv[MSG_POS])) < 0)
    //     errexit("error writing message: %s", argv[MSG_POS]);

    // /* close connections and exit */
    // close(sd);
    // close(sd2);
    // exit(0);
}
