#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void request_room(int is_male, char *ip_address, int port) {
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
  sprintf(request, "%d", is_male);

  if (sendto(client_fd, request, strlen(request), 0, (struct sockaddr *)&addr,
             sizeof(struct sockaddr_in)) == -1) {
    perror("sendto");
    exit(1);
  }

  close(client_fd);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <is_male> <ip> <port>\n", argv[0]);
    exit(1);
  }

  int is_male = atoi(argv[1]);
  char *ip_address = argv[2];
  int port = atoi(argv[3]);

  request_room(is_male, ip_address, port);

  return 0;
}
