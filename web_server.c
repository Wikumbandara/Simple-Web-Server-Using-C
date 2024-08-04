#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h> 
#include <windows.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define DEFAULT_PORT 8080
#define MAX_REQUEST_SIZE 1024

// Function to send an HTTP response
void sendResponse(int new_socket, const char *response) {
    send(new_socket, response, strlen(response), 0);
}

// Function to send a file over HTTP
void sendFile(int new_socket, const char *filename, const char *contentType) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("File not found: %s\n", filename);
        
        // Check if 404.html exists
        fp = fopen("404.html", "rb");
        if (fp) {
            contentType = "Content-Type: text/html\r\n\r\n";
            filename = "404.html";
        } else {
            sendResponse(new_socket, "HTTP/1.1 404 Not Found\r\n\r\n");
            sendResponse(new_socket, "404 - File Not Found");
            return;
        }
    }

    struct stat st;
    if (stat(filename, &st) != 0) {
        printf("Error getting file size: %s\n", filename);
        fclose(fp);
        sendResponse(new_socket, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
        sendResponse(new_socket, "500 - Internal Server Error");
        return;
    }
    int fileSize = st.st_size;

    char *contents = malloc(fileSize);
    if (!contents) {
        fclose(fp);
        printf("Memory allocation failed\n");
        sendResponse(new_socket, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
        sendResponse(new_socket, "500 - Internal Server Error");
        return;
    }

    fread(contents, 1, fileSize, fp);
    fclose(fp);

    sendResponse(new_socket, "HTTP/1.1 200 OK\r\n");
    sendResponse(new_socket, contentType);
    send(new_socket, contents, fileSize, 0);

    free(contents);
}

// Function to route the request and serve the appropriate file
void RouteMe(int new_socket, char *request) {
    char *contentType = "Content-Type: text/html\r\n\r\n";
    char *filename = "index.html";

    if (strcmp(request, "/") != 0) {
        memmove(request, request + 1, strlen(request));
        if (strstr(request, ".html")) {
            contentType = "Content-Type: text/html\r\n\r\n";
        } else if (strstr(request, ".jpg") || strstr(request, ".jpeg")) {
            contentType = "Content-Type: image/jpeg\r\n\r\n";
        } else if (strstr(request, ".png")) {
            contentType = "Content-Type: image/png\r\n\r\n";
        }  else if (strstr(request, ".pdf")) {
            contentType = "Content-Type: application/pdf\r\n\r\n";
        } else if (strstr(request, ".gif")) {
            contentType = "Content-Type: image/gif\r\n\r\n"; 
        } else if (strstr(request, ".css")) {
            contentType = "Content-Type: text/css\r\n\r\n";
        } else if (strstr(request, ".js")) {
            contentType = "Content-Type: application/javascript\r\n\r\n";
        } else if (strstr(request, ".txt")) {
            contentType = "Content-Type: text/plain\r\n\r\n";
        } else if (strstr(request, ".mp4")) {
            contentType = "Content-Type: video/mp4\r\n\r\n";
        } else {
            contentType = "Content-Type: application/octet-stream\r\n\r\n";
        }
        filename = request;
    }
    
    sendFile(new_socket, filename, contentType);
}

int main() {
    WSADATA wsa;
    int create_socket, new_socket;
    int addrlen;
    char buffer[MAX_REQUEST_SIZE];
    struct sockaddr_in address;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket.\n");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(DEFAULT_PORT);

    if (bind(create_socket, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        closesocket(create_socket);
        WSACleanup();
        return 1;
    }

    printf("Server started on port %d\n", DEFAULT_PORT);

    if (listen(create_socket, 20) < 0) {
        perror("server: listen");
        exit(1);
    }

    while (1) {
        addrlen = sizeof(address);
        if ((new_socket = accept(create_socket, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            perror("server: accept");
            exit(1);
        }

        printf("Incoming request identified\n");

        recv(new_socket, buffer, MAX_REQUEST_SIZE, 0);
        strtok(buffer, " ");
        char *route = strtok(NULL, " ");
        printf("%s\n", route);
        RouteMe(new_socket, route);

        closesocket(new_socket);
    }

    closesocket(create_socket);
    WSACleanup(); 
    return 0;
}
