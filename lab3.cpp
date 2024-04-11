#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <regex>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

void fetchResource(const std::string& host) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    struct hostent *server_info = gethostbyname(host.c_str());
    if (server_info == NULL) {
        std::cerr << "Invalid host" << std::endl;
        close(sock);
        return;
    }
    memcpy(&server.sin_addr.s_addr, server_info->h_addr, server_info->h_length);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return;
    }

    std::string request = "GET / HTTP/1.1\r\nHost: " + host + "\r\n\r\n";
    if (send(sock, request.c_str(), request.length(), 0) < 0) {
        std::cerr << "Failed to send request" << std::endl;
        close(sock);
        return;
    }

    char response[4096];
    std::string responseBody;
    while (true) {
        memset(response, 0, sizeof(response));
        int bytesRead = recv(sock, response, sizeof(response) - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        responseBody += response;
    }

    close(sock);

    // Find links, images, and scripts
    std::regex linkRegex("<link[^>]*>");  // Escape backslashes
    std::regex imageRegex("<img[^>]*>");   // Escape backslashes
    std::regex scriptRegex("<script[^>]*>"); // Escape backslashes

    std::smatch match;
    std::stringstream foundElements;

    // Search for links
    while (std::regex_search(responseBody, match, linkRegex)) {
        foundElements << "Found link: " << match.str() << std::endl;
        responseBody = match.suffix().str();
    }

    // Search for images
    while (std::regex_search(responseBody, match, imageRegex)) {
        foundElements << "Found image: " << match.str() << std::endl;
        responseBody = match.suffix().str();
    }

    // Search for scripts
    while (std::regex_search(responseBody, match, scriptRegex)) {
        foundElements << "Found script: " << match.str() << std::endl;
        responseBody = match.suffix().str();
    }

    // Print the response body and found elements
    std::cout << "Response:" << std::endl;
    std::cout << responseBody << std::endl;

    if (foundElements.str().length() > 0) {
        std::cout << "Found elements:" << std::endl;
        std::cout << foundElements.str();
    }
}

int main() {
    std::string host = "ufba.br";

    fetchResource(host);

    return 0;
}
