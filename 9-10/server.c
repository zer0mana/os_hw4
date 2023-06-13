#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define SINGLE_ROOMS 10
#define DOUBLE_ROOMS 15
#define TOTAL_ROOMS (SINGLE_ROOMS + DOUBLE_ROOMS)
#define MAX_CLIENTS 30  // define the maximum number of clients the server can handle concurrently

typedef struct {
  int occupants;
  int is_male;
} double_room;

typedef struct {
  int single_rooms;
  double_room double_rooms[DOUBLE_ROOMS];
} hotel_status;

hotel_status hotel;

void init_hotel() {
  hotel.single_rooms = SINGLE_ROOMS;
  for (int i = 0; i < DOUBLE_ROOMS; i++) {
    hotel.double_rooms[i].occupants = 0;
    hotel.double_rooms[i].is_male = -1;
  }
}

int occupy_double_room(int is_male) {
  for (int i = 0; i < DOUBLE_ROOMS; i++) {
    if (hotel.double_rooms[i].occupants < 2 &&
        (hotel.double_rooms[i].is_male == is_male ||
         hotel.double_rooms[i].is_male == -1)) {
      hotel.double_rooms[i].occupants++;
      hotel.double_rooms[i].is_male = is_male;
      return i;
    }
  }
  return -1;
}

int available_double_rooms() {
  int available = 0;
  for (int i = 0; i < DOUBLE_ROOMS; i++) {
    if (hotel.double_rooms[i].occupants < 2) {
      available++;
    }
  }
  return available;
}

void handle_client(int client_fd, struct sockaddr_in client_addr,
                   socklen_t addr_size) {
  char request[256];
  memset(request, 0, sizeof(request));

  if (recvfrom(client_fd, request, sizeof(request), 0,
               (struct sockaddr *)&client_addr, &addr_size) == -1) {
    perror("recvfrom");
    exit(1);
  }

  if (strcmp(request, "status") == 0) {
    char response[1024];
    sprintf(response, "Статус: Одноместные номера - %d, двухместные номера - %d\n",
            hotel.single_rooms, available_double_rooms());
    sendto(client_fd, response, strlen(response), 0,
           (struct sockaddr *)&client_addr, addr_size);
  } else {
    int is_male = atoi(request);
    char *gender_str = is_male ? "Мужчина" : "Женщина";

    if (hotel.single_rooms > 0) {
      hotel.single_rooms--;
      printf("%s заселяется в одноместный номер. Оставшиеся номера: %d "
             "одноместных, %d двухместных\n",
             gender_str, hotel.single_rooms, available_double_rooms());
    } else {
      int room_number = occupy_double_room(is_male);
      if (room_number != -1) {
        printf("%s заселяется в двухместный номер #%d. Оставшиеся номера: %d "
               "одноместных, %d двухместных\n",
               gender_str, room_number + 1, hotel.single_rooms,
               available_double_rooms());
      } else {
        printf("%s уходит, нет свободных номеров.\n", gender_str);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }

  int port = atoi(argv[1]);

  int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd == -1) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(server_fd, (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr_in)) == -1) {
    perror("bind");
    close(server_fd);
    exit(1);
  }

  init_hotel();

  printf("Server running on port %d\n", port);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    handle_client(server_fd, client_addr, addr_size);
  }

  return 0;
}
