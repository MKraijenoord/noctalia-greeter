#include "compositor/greetd_handover.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <wlr/util/log.h>

void greetd_handover_send(const char* socket_path, const char* message) {
  if (socket_path == NULL || socket_path[0] == '\0') {
    return;
  }
  struct sockaddr_un addr = {.sun_family = AF_UNIX};
  if (strlen(socket_path) >= sizeof(addr.sun_path)) {
    return;
  }
  strcpy(addr.sun_path, socket_path);
  const int fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    return;
  }
  if (sendto(fd, message, strlen(message), MSG_NOSIGNAL, (const struct sockaddr*)&addr, sizeof(addr)) < 0) {
    wlr_log_errno(WLR_ERROR, "failed to send %s to greetd", message);
  }
  close(fd);
}
