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
#define BUFFER_SIZE 512
#define MAX_STATUS_LENGTH 50
#define MANDATORY_ERR "Mandatory args missing!\n"
#define PORT_CHOSEN_ERR "Use a port number between 1025 and 65535\n"
#define CARRIAGE "\r\n"
#define IO_ERR "Unable to create a file descriptor to read socket\n"
#define STATUS_400 "Malformed Request"
#define STATUS_501 "Protocol Not Implemented"
#define STATUS_405 "Unsupported Method"
#define STATUS_200_TERMINATE "Server Shutting Down"  // Terminate Requests
#define STATUS_403 "Operation Forbidden"             // Terminate Requests
#define STATUS_406 "Invalid Filename"                // GET Request
#define STATUS_200_GET "OK"                          // GET Request
#define STATUS_404 "File Not Found"                  // GET Request

int usage(char *progname) {
    fprintf(stderr, "usage: %s -p PORT -r DOCUMENT_DIRECTORY -t AUTH_TOKEN\n", progname);
    exit(ERROR);
}

int errexit(char *format, char *arg) {
    fprintf(stderr, format, arg);
    fprintf(stderr, "\n");
    exit(ERROR);
}

int figureOutTypeOfRequest(char *request) {
    //  -1 : Neither GET or Terminate, 1 : GET, 2: Terminate
    // Modify the value of req_type
    char copy_req[strlen(request) + 1];
    memset(copy_req, 0x0, strlen(request) + 1);
    memcpy(copy_req, request, strlen(request));
    char *token = strtok(copy_req, " ");
    if (strcmp(token, "GET") == 0) {
        return 1;
    } else if (strcmp(token, "TERMINATE") == 0) {
        return 2;
    } else {
        return -1;
    }
}

/* Checks for 3 tokens in the first line of the client's request header */
bool isRequestStartingWithCorrectSyntax(char *request) {
    // status 400 check: tokenize and check for 3 items
    char copy_req[strlen(request) + 1];
    int count = 0;
    memset(copy_req, 0x0, strlen(request) + 1);
    memcpy(copy_req, request, strlen(request));
    char *token;
    token = strtok(copy_req, " ");
    while (token != NULL) {
        printf("tokenising in isRequestStartingWithCorrectSyntax: %s\n", token);
        count += 1;
        token = strtok(NULL, " ");
    }
    if (count == 3) {
        return true;
    } else {
        return false;
    }
}

/* Checks for the right protocol in the first line of the client's request header */
bool isRequestContainingTheCorrectProtocol(char *request) {
    // status 501 check: see if header has "HTTP/" (case-sensitive)
    // tokenize for "/" and check if it is just HTTP and not ALHTTP
    return true;
}

bool isRequestContainingTheCorrectMethod(char *request) {
    // status 405 check: METHOD of request can only be "GET" or "TERMINATE"
    // look for GET or TERMINATE in request
    return true;
}

/* Checks if each line of the client's request header terminates with a carriage */
bool isEachLineWellTerminated(char *request) {
    // status 400 check
    printf("Client_Request: %s", request);
    return true;
}

/* Checks if last two characters of the request header correspond to carriage */
bool isRequestEndingWithCarriage(char *request) {
    // status 400 check
    return true;
}

// return False for err scenarios
bool handleTerminateRequest(char *request_token, char *actual_cmd_line_token) {
    // Check for case-sensitive match
    // Two possible outcomes - either 200 AND terminate server or 403 and continue accepting new connections
    return true;
}

/* For a GET request, the argument specified in the header has to begin with a "/" */
bool doesFileNameBeginCorrectly(char *argument_filename) {
    // status 406 check: see if first character of argument is a "/", else return 406
    return true;
}

/* For a GET request, if the argument specified in the header is nothing but a "/", we have to convert it into "/homepage.html" */
bool isFileNameJustASlash(char *argument_filename) {
    // convert "/" into "/homepage.html"
    return true;
}

// return False for err scenarios
bool handleGetRequest(char *request) {
    // fileName has to be concatenated with root directory(specified via cmd line after -r)
    // if fileFound -> 200 ok, send header with content
    // else -> 404
    return true;
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

/* Accept a connection. Buffer data sent by client. Check if client's request is well formed and identify the type of request */
int accept_a_connection_and_read_request(int listen_fd, char *status, int *req_type) {
    memset(status, 0x0, MAX_STATUS_LENGTH);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0x0, sizeof(buffer));
    *req_type = -1;
    struct sockaddr_in client;
    unsigned int cli_len = sizeof(client);
    bool isFirstLineChecksDone = false;

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
        // printf("RSP: %s", buffer);
        if (!isFirstLineChecksDone) {
            if (!isRequestStartingWithCorrectSyntax(buffer)) {
                strcpy(status, STATUS_400);
                break;
            }
            // if (!isRequestContainingTheCorrectProtocol(buffer)) {
            //     strcpy(status, STATUS_501);
            //     break;
            // }
            // if (!isRequestContainingTheCorrectMethod(buffer)) {
            //     strcpy(status, STATUS_405);
            //     break;
            // }
            isFirstLineChecksDone = true;
            *req_type = figureOutTypeOfRequest(buffer);
        }

        // if (!isEachLineWellTerminated(buffer)) {
        //     strcpy(status, STATUS_400);
        //     break;
        // }
    }

    // if (!isRequestEndingWithCarriage(buffer)) {
    //     strcpy(status, STATUS_400);
    // }
    return sd2;
}

void write_to_connection(int conn_fd, char *reply) {
    /* write message to the connection */
    if (write(conn_fd, reply, strlen(reply)) < 0)
        errexit("error writing message: %s", reply);
}

int main(int argc, char *argv[]) {
    int opt, sd;
    int type_of_request_by_client = -1;  //  -1 : Neither GET or Terminate, 1 : GET, 2: Terminate
    bool PORT_GIVEN = false;
    bool ROOT_DIR_GIVEN = false;
    bool AUTH_TOKEN_GIVEN = false;
    bool terminateServer = false;
    char *PORT_NUMBER = NULL;
    char *ROOT_DIR = NULL;
    char *AUTH_TOKEN = NULL;
    char status[MAX_STATUS_LENGTH];  // status based on client request

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
    while (1) {
        memset(status, 0x0, MAX_STATUS_LENGTH);
        int sd2 = accept_a_connection_and_read_request(sd, status, &type_of_request_by_client);
        if (strlen(status))
            printf("%s\n", status);
        if (type_of_request_by_client == 1) {
            printf("GET request detected\n");
        } else if (type_of_request_by_client == 2) {
            printf("TERMINATE request detected\n");
        } else {
            printf("Not a proper request\n");
        }
        // printf("%lu\n", strlen(status));
        // Switch cases for different inital status and write responses as required
        // ifs for type_of_request_by_client and handle the requests sent to modify status
        // based on status received, send responses or shut down server by setting terminateServer = true
        // write_to_connection(sd2);

        /* close connections and exit */
        printf("Closing connections\n");
        close(sd2);
        if (terminateServer)
            break;
    }
    close(sd);
    exit(SUCCESS);
}
