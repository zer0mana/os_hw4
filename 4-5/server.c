#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SINGLE_ROOMS 10
#define DOUBLE_ROOMS 15
#define TOTAL_ROOMS (SINGLE_ROOMS + DOUBLE_ROOMS)

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

  int is_male = atoi(request);
  int double_room_index;
  if (is_male) {
    if (hotel.single_rooms > 0) {
      hotel.single_rooms--;
      printf("Мужчина заселяется в одноместный номер. Оставшиеся номера: %d "
             "одноместных, %d двухместных\n",
             hotel.single_rooms, available_double_rooms());
    } else if ((double_room_index = occupy_double_room(is_male)) != -1) {
      printf("Мужчина заселяется в двухместный номер #%d. Оставшиеся номера: "
             "%d одноместных, %d двухместных\n",
             double_room_index, hotel.single_rooms, available_double_rooms());
    } else {
      printf("Мужчина уходит, нет свободных номеров.\n");
    }
  } else {
    if (hotel.single_rooms > 0) {
      hotel.single_rooms--;
      printf("Женщина заселяется в одноместный номер. Оставшиеся номера: %d "
             "одноместных, %d двухместных\n",
             hotel.single_rooms, available_double_rooms());
    } else if ((double_room_index = occupy_double_room(is_male)) != -1) {
      printf("Женщина заселяется в двухместный номер #%d. Оставшиеся номера: "
             "%d одноместных, %d двухместных\n",
             double_room_index, hotel.single_rooms, available_double_rooms());
    } else {
      printf("Женщина уходит, нет свободных номеров.\n");
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
    exit(1);
  }

  char *ip_address = argv[1];
  int port = atoi(argv[2]);

  init_hotel();

  int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd == -1) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_in addr, client_addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip_address);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) ==
      -1) {
    perror("bind");
    exit(1);
  }

  socklen_t addr_size = sizeof(struct sockaddr_in);

  while (1) {
    handle_client(server_fd, client_addr, addr_size);
  }

  return 0;
}
