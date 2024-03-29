/* Name: BALASHANMUGA PRIYAN RAJAMOHAN
 * Case ID: bxr261
 * Filename: proj3.c
 * Date created: 18 Oct 2022
 * Description: The following code is used to set up a minimal web server that handles GET and TERMINATE requests. Assumes argument size of 512 chars.
 */

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
#define ERROR 1
#define SUCCESS 0
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFFER_SIZE 512
#define MAX_STATUS_LENGTH 50
#define MANDATORY_ERR "Mandatory args missing!\n"
#define PORT_CHOSEN_ERR "Use a port number between 1025 and 65535\n"
#define CARRIAGE "\r\n"
#define IO_ERR "Unable to create a file descriptor to read socket while trying to write data to socket\n"
#define STATUS_400 "HTTP/1.1 400 Malformed Request\r\n\r\n"
#define STATUS_501 "HTTP/1.1 501 Protocol Not Implemented\r\n\r\n"
#define STATUS_405 "HTTP/1.1 405 Unsupported Method\r\n\r\n"
#define STATUS_200_TERMINATE "HTTP/1.1 200 Server Shutting Down\r\n\r\n"  // Terminate Requests
#define STATUS_403 "HTTP/1.1 403 Operation Forbidden\r\n\r\n"             // Terminate Requests
#define STATUS_406 "HTTP/1.1 406 Invalid Filename\r\n\r\n"                // GET Request
#define STATUS_200_GET "HTTP/1.1 200 OK\r\n\r\n"                          // GET Request
#define STATUS_404 "HTTP/1.1 404 File Not Found\r\n\r\n"                  // GET Request

/* Prints usage information and exits the program */
int usage(char *progname) {
    fprintf(stderr, "usage: %s -p PORT -r DOCUMENT_DIRECTORY -t AUTH_TOKEN\n", progname);
    exit(ERROR);
}

/* Prints an error message and exits the program */
int errexit(char *format, char *arg) {
    fprintf(stderr, format, arg);
    fprintf(stderr, "\n");
    exit(ERROR);
}

/* Parses the first line of client's request header and gets the argument field */
void grabArgumentFromRequest(char *request, char *argument) {
    char copy_req[strlen(request) + 1];
    memset(argument, 0x0, BUFFER_SIZE);
    memset(copy_req, 0x0, strlen(request) + 1);
    memcpy(copy_req, request, strlen(request));
    char *token = strtok(copy_req, " ");
    int i = 0;  // indicates the num of elems or tokens
    while (token != NULL) {
        if (i == 1)
            break;
        token = strtok(NULL, " ");
        i++;
    }
    strncpy(argument, token, strlen(token));
}

/* Parses the first line of client's request header and returns 1 for GET request, 2 for TERMINATE request, -1 for neither case */
int figureOutTypeOfRequest(char *request) {
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

/* Checks for 3 tokens in the first line of client's request header. Returns true if 3 tokens were found */
bool isRequestStartingWithCorrectSyntax(char *request) {
    // status 400 check
    char copy_req[strlen(request) + 1];
    int count = 0;
    memset(copy_req, 0x0, strlen(request) + 1);
    memcpy(copy_req, request, strlen(request));
    char *token;
    token = strtok(copy_req, " ");
    while (token != NULL) {
        count += 1;
        token = strtok(NULL, " ");
    }
    if (count == 3) {
        return true;
    } else {
        return false;
    }
}

/* Checks for the use of right protocol in the first line of client's request header and returns true if correct */
bool isRequestContainingTheCorrectProtocol(char *request) {
    // status 501 check
    // check if it is just HTTP and not ALHTTP for example
    char copy_req[strlen(request) + 1];
    memset(copy_req, 0x0, strlen(request) + 1);
    memcpy(copy_req, request, strlen(request));
    char *token = strtok(copy_req, " ");
    int i = 0;  // indicates the num of elems or tokens
    while (token != NULL) {
        if (i == 2)
            break;
        token = strtok(NULL, " ");
        i++;
    }
    char *app_protocol = strtok(token, "/");
    if (strcmp(app_protocol, "HTTP") == 0) {
        return true;
    } else {
        return false;
    }
}

/* Method used can only be GET or TERMINATE. Function parses the first line of client's request header and returns true if either of the two requests were made */
bool isRequestContainingTheCorrectMethod(char *request) {
    // status 405 check
    char copy_req[strlen(request) + 1];
    memset(copy_req, 0x0, strlen(request) + 1);
    memcpy(copy_req, request, strlen(request));
    char *token = strtok(copy_req, " ");
    if ((strcmp(token, "GET") == 0) || (strcmp(token, "TERMINATE") == 0)) {
        return true;
    } else {
        return false;
    }
}

/* Checks client's request header to see if it terminates with a carriage. Returns true if well terminated */
bool isEachLineWellTerminated(char *request) {
    // status 400 check
    if (strstr(request, CARRIAGE) != NULL) {
        return true;
    } else {
        return false;
    }
}

/* Compares actual_cmd_line_token against request argument and returns a boolean indicating whether server should shut down */
bool isServerShuttingDown(char *request_token, char *actual_cmd_line_token, char *status) {
    // Two possible outcomes - either 200(server terminates) or 403(continue accepting new connections)
    memset(status, 0x0, MAX_STATUS_LENGTH);
    if (strcmp(actual_cmd_line_token, request_token) == 0) {
        strcpy(status, STATUS_200_TERMINATE);
        return true;
    } else {
        strcpy(status, STATUS_403);
        return false;
    }
}

/* For a GET request, the argument specified in the header has to begin with a "/". Function returns a boolean indicating if this requirement is met */
bool doesFileNameBeginCorrectly(char *argument_filename) {
    // status 406 check
    if (argument_filename[0] == '/') {
        return true;
    } else {
        return false;
    }
}

/* For a GET request, if the argument specified in the header is nothing but a "/", we have to convert it into "/homepage.html" */
void convertFileNameIfJustASlash(char *argument_filename) {
    if (strcmp(argument_filename, "/") == 0) {
        memset(argument_filename, 0x0, BUFFER_SIZE);
        strcpy(argument_filename, "/homepage.html");
    }
}

/* Checks if the file exists in the specified directory and returns a boolean value indicating if it exists */
bool doesFileExist(char *argument_filename, char *root_dir) {
    bool is_exist = false;
    char temp[BUFFER_SIZE * 2];
    memset(temp, 0x0, BUFFER_SIZE * 2);
    strcat(temp, root_dir);
    strcat(temp, argument_filename);
    FILE *fp = fopen(temp, "r");
    if (fp != NULL) {
        is_exist = true;
        fclose(fp);
    }
    return is_exist;
}

/* Checks for status codes 406, 404 & 200 and updates status accordingly. Calls another function to convert argument into /homepage.html if it was just a "/" */
bool isGetRequestValid(char *argument_filename, char *root_dir, char *status) {
    memset(status, 0x0, MAX_STATUS_LENGTH);
    convertFileNameIfJustASlash(argument_filename);
    if (!doesFileNameBeginCorrectly(argument_filename)) {
        strcpy(status, STATUS_406);
        return false;
    }
    if (!doesFileExist(argument_filename, root_dir)) {
        strcpy(status, STATUS_404);
        return false;
    }
    strcpy(status, STATUS_200_GET);
    return true;
}

/* Creates a socket and returns the socket. Socket is set up to listen for incoming requests */
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

    // printf("Server Address: %s\nSocket Created and is listening on port %d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    return sd;
}

/* Function accepts a connection and checks if client's request is well formed. Also identifies the type of request and returns the connection socket */
int accept_a_connection_and_read_request(int listen_fd, char *status, int *req_type, char *argument) {
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
    // printf("New Client connected from port number %d and IP address  %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));

    FILE *fp = fdopen(sd2, "r");
    if (fp == NULL) {
        printf(IO_ERR);
        exit(ERROR);
    }
    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        if (!isEachLineWellTerminated(buffer)) {
            memset(status, 0x0, MAX_STATUS_LENGTH);
            strcpy(status, STATUS_400);
            break;
        }
        if (!isFirstLineChecksDone) {
            if (!isRequestStartingWithCorrectSyntax(buffer)) {
                memset(status, 0x0, MAX_STATUS_LENGTH);
                strcpy(status, STATUS_400);
                break;
            }
            if (isRequestStartingWithCorrectSyntax(buffer) && !isRequestContainingTheCorrectProtocol(buffer)) {
                memset(status, 0x0, MAX_STATUS_LENGTH);
                strcpy(status, STATUS_501);
            }
            if (isRequestStartingWithCorrectSyntax(buffer) && isRequestContainingTheCorrectProtocol(buffer) && !isRequestContainingTheCorrectMethod(buffer)) {
                memset(status, 0x0, MAX_STATUS_LENGTH);
                strcpy(status, STATUS_405);
            }
            isFirstLineChecksDone = true;
            *req_type = figureOutTypeOfRequest(buffer);
            grabArgumentFromRequest(buffer, argument);
        }
        if (strcmp(buffer, CARRIAGE) == 0)
            break;
    }
    // Handles a case where req is : "GET /small-test.dat HTTP/1.1\r\n"
    if (strcmp(buffer, CARRIAGE) != 0) {
        memset(status, 0x0, MAX_STATUS_LENGTH);
        strcpy(status, STATUS_400);
    }
    return sd2;
}

/* Sends header of server response to client i.e status */
void write_header_to_connection(int conn_fd, char *reply) {
    if (write(conn_fd, reply, strlen(reply)) < 0)
        printf("error writing header to conn: %s", reply);
}

/* Sends data read from requested file over the connection socket to the client */
void write_file_to_connection(char *argument, char *root_dir, int conn_fd) {
    char temp[BUFFER_SIZE * 2];  // will hold the file path
    memset(temp, 0x0, BUFFER_SIZE * 2);
    strcat(temp, root_dir);
    strcat(temp, argument);
    FILE *stream = fopen(temp, "rb");
    if (stream == NULL) {
        printf(IO_ERR);
        return;
        // exit(ERROR);
    }
    int N = 0;
    char reader[BUFFER_SIZE];
    memset(reader, 0x0, BUFFER_SIZE);
    while (!feof(stream)) {
        N = fread(&reader, sizeof(char), BUFFER_SIZE - 1, stream);
        if (N < 0) {
            printf("error reading file from %s", temp);
            return;
        }
        if (write(conn_fd, reader, N) < 0) {
            printf("error writing data from file at %s to conn", temp);
            return;
        }
        memset(reader, 0x0, BUFFER_SIZE);
    }
    fclose(stream);
}

int main(int argc, char *argv[]) {
    bool PORT_GIVEN = false;
    bool ROOT_DIR_GIVEN = false;
    bool AUTH_TOKEN_GIVEN = false;
    bool TERMINATE_SERVER = false;
    int opt, sd;
    int type_of_request_by_client = -1;  //  -1 : Neither GET or Terminate, 1: GET, 2: Terminate
    char *port_number = NULL;            // server listens on this port number
    char *root_dir = NULL;               // argument of -r in cmd line
    char *auth_token = NULL;             // argument of -t in cmd line i.e. auth_token from cmd line to terminate server
    char status[MAX_STATUS_LENGTH];      // status based on client request
    char argument[BUFFER_SIZE];          // the kind of file needed or could be a auth_token from request header

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
                    port_number = optarg;
                    break;
                case 'r':
                    ROOT_DIR_GIVEN = true;
                    root_dir = optarg;
                    break;
                case 't':
                    AUTH_TOKEN_GIVEN = true;
                    auth_token = optarg;
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
    int port_given_by_user = atoi(port_number);
    if (port_given_by_user < 1025 || port_given_by_user > 65535) {
        printf(PORT_CHOSEN_ERR);
        usage(argv[0]);
    }

    sd = create_socket_and_listen(port_number);
    while (1) {
        memset(status, 0x0, MAX_STATUS_LENGTH);
        memset(argument, 0x0, BUFFER_SIZE);
        bool sendFile = false;
        int sd2 = accept_a_connection_and_read_request(sd, status, &type_of_request_by_client, argument);
        if ((strcmp(status, STATUS_400) != 0) && (strcmp(status, STATUS_501) != 0) && (strcmp(status, STATUS_405) != 0)) {
            // handle get or terminate request
            if (type_of_request_by_client == 1) {
                if (isGetRequestValid(argument, root_dir, status)) {
                    sendFile = true;
                }
            } else {
                TERMINATE_SERVER = isServerShuttingDown(argument, auth_token, status);
            }
        }
        write_header_to_connection(sd2, status);
        if (sendFile) {
            write_file_to_connection(argument, root_dir, sd2);
        }
        /* close connections and exit */
        close(sd2);
        if (TERMINATE_SERVER)
            break;
    }
    close(sd);
    exit(SUCCESS);
}
