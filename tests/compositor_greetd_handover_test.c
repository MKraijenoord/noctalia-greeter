#define _POSIX_C_SOURCE 200809L

#include "compositor/greetd_handover.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static bool expect_received(const char* name, int fd, const char* expected) {
  char buffer[64];
  const ssize_t length = recv(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
  const char* actual = buffer;
  if (length < 0) {
    actual = errno == EAGAIN || errno == EWOULDBLOCK ? "<nothing>" : strerror(errno);
  } else {
    buffer[length] = '\0';
  }
  if (strcmp(actual, expected) == 0) {
    return true;
  }
  fprintf(stderr, "%s: expected '%s', got '%s'\n", name, expected, actual);
  return false;
}

int main(void) {
  char directory[] = "/tmp/noctalia-compositor-handover-XXXXXX";
  if (mkdtemp(directory) == NULL) {
    perror("mkdtemp");
    return EXIT_FAILURE;
  }
  struct sockaddr_un addr = {.sun_family = AF_UNIX};
  snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/handover.sock", directory);
  const int greetd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (greetd < 0 || bind(greetd, (const struct sockaddr*)&addr, sizeof(addr)) != 0) {
    perror("bind");
    return EXIT_FAILURE;
  }

  bool passed = true;
  greetd_handover_send(addr.sun_path, "PRESENTED=1");
  passed = expect_received("presented reaches greetd as one datagram", greetd, "PRESENTED=1") && passed;
  passed = expect_received("presented is sent once", greetd, "<nothing>") && passed;

  greetd_handover_send(NULL, "PRESENTED=1");
  greetd_handover_send("", "PRESENTED=1");
  passed = expect_received("no handover socket sends nothing", greetd, "<nothing>") && passed;

  close(greetd);
  unlink(addr.sun_path);
  rmdir(directory);
  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
