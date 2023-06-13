#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void request_status(char* ip_address, int port) {
  int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_fd == -1) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip_address);

  char request[256];
  memset(request, 0, sizeof(request));
  sprintf(request, "status");

  if (sendto(client_fd, request, strlen(request), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    perror("sendto");
    exit(1);
  }

  char response[1024];
  memset(response, 0, sizeof(response));
  socklen_t addr_size = sizeof(struct sockaddr_in);

  if (recvfrom(client_fd, response, sizeof(response), 0, (struct sockaddr*)&addr, &addr_size) == -1) {
    perror("recvfrom");
    exit(1);
  }

  printf("Состояние системы: %s\n", response);

  close(client_fd);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
    exit(1);
  }

  char* ip_address = argv[1];
  int port = atoi(argv[2]);

  request_status(ip_address, port);

  return 0;
}
