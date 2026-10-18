#include "greetd/greetd_handover.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace {

  bool expect(std::string_view name, std::string_view actual, std::string_view expected) {
    if (actual == expected) {
      return true;
    }
    std::fprintf(
        stderr, "%.*s: expected '%.*s', got '%.*s'\n", static_cast<int>(name.size()), name.data(),
        static_cast<int>(expected.size()), expected.data(), static_cast<int>(actual.size()), actual.data()
    );
    return false;
  }

  std::string receivePending(int fd) {
    char buffer[64];
    const ssize_t length = ::recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT);
    if (length < 0) {
      return errno == EAGAIN || errno == EWOULDBLOCK ? "<nothing>" : std::strerror(errno);
    }
    return std::string(buffer, static_cast<std::size_t>(length));
  }

} // namespace

int main() {
  char directory[] = "/tmp/noctalia-greetd-handover-XXXXXX";
  if (::mkdtemp(directory) == nullptr) {
    std::perror("mkdtemp");
    return 1;
  }
  const std::string socketPath = std::string(directory) + "/handover.sock";
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::strcpy(addr.sun_path, socketPath.c_str());
  const int greetd = ::socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (greetd < 0 || ::bind(greetd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0) {
    std::perror("bind");
    return 1;
  }

  bool passed = true;
  ::setenv("GREETD_HANDOVER_SOCKET", socketPath.c_str(), 1);
  ::unsetenv("GREETD_HANDOVER_PRESTART");
  notifyHandoverReady();
  passed = expect("regular start sends nothing", receivePending(greetd), "<nothing>") && passed;

  ::setenv("GREETD_HANDOVER_PRESTART", "1", 1);
  notifyHandoverReady();
  passed = expect("prestart reports ready", receivePending(greetd), "READY=1") && passed;
  passed = expect("prestart reports ready once", receivePending(greetd), "<nothing>") && passed;

  ::close(greetd);
  ::unlink(socketPath.c_str());
  ::rmdir(directory);
  return passed ? 0 : 1;
}
