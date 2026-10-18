#include "greetd/greetd_handover.h"

#include "core/log.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace {
  constexpr Logger kLog("greeter");
} // namespace

void notifyHandoverReady() {
  const char* path = std::getenv("GREETD_HANDOVER_SOCKET");
  if (std::getenv("GREETD_HANDOVER_PRESTART") == nullptr || path == nullptr) {
    return;
  }
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  if (std::strlen(path) >= sizeof(addr.sun_path)) {
    return;
  }
  std::strcpy(addr.sun_path, path);
  const int fd = ::socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (fd < 0) {
    return;
  }
  constexpr char kReady[] = "READY=1";
  if (::sendto(fd, kReady, sizeof(kReady) - 1, MSG_NOSIGNAL, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr))
      < 0) {
    kLog.warn("failed to send READY=1 to greetd: {}", std::strerror(errno));
  }
  ::close(fd);
}
