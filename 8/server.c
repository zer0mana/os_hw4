#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

void* handle_client(void* arg) {
  int client_fd = *(int*)arg;
  struct sockaddr_in client_addr;
  socklen_t addr_size = sizeof(struct sockaddr_in);

  char request[256];
  memset(request, 0, sizeof(request));

  if (recvfrom(client_fd, request, sizeof(request), 0,
               (struct sockaddr *)&client_addr, &addr_size) == -1) {
    perror("recvfrom");
    pthread_exit(NULL);
  }

  if (strcmp(request, "status") == 0) {
    char response[1024];
    sprintf(response, "Статус: Одноместные номера - %d, двухместные номера - %d\n",
            hotel.single_rooms, available_double_rooms());
    sendto(client_fd, response, strlen(response), 0,
           (struct sockaddr *)&client_addr, addr_size);
  } else {
    int is_male = atoi(request);
    if (hotel.single_rooms > 0) {
      hotel.single_rooms--;
      char response[] = "Вы заселены в одноместный номер\n";
      sendto(client_fd, response, strlen(response), 0,
             (struct sockaddr *)&client_addr, addr_size);
    } else {
      int room_no = occupy_double_room(is_male);
      if (room_no != -1) {
        char response[256];
        sprintf(response, "Вы заселены в двухместный номер %d\n", room_no);
        sendto(client_fd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, addr_size);
      } else {
        char response[] = "Нет свободных номеров\n";
        sendto(client_fd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, addr_size);
      }
    }
  }

  close(client_fd);
  free(arg);
  pthread_exit(NULL);
}

int main() {
  int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_fd == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(8080);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind");
    return 1;
  }

  init_hotel();

  while (1) {
    int* client_fd = malloc(sizeof(int));
    *client_fd = accept(server_fd, NULL, NULL);
    if (*client_fd == -1) {
      perror("accept");
      continue;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, client_fd);
    pthread_detach(thread_id);
  }

  return 0;
}
