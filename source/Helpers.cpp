/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Helper methods
 * @details   Verious helper methods
 *-
 */

#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#if __has_include(<filesystem>)
#include <filesystem>
using namespace std::filesystem;
#else
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#endif

#include "Envelope.pb.h"
#include "Helpers.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

constexpr size_t GDescBufferSize = 1024;
namespace pbio = google::protobuf::io;

namespace tkm
{

auto jnkHsh(const char *key) -> uint64_t
{
  uint64_t hash, i;

  for (hash = i = 0; i < strlen(key); ++i) {
    hash += (uint64_t) key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}

bool sendCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor)
{
  tkm::msg::collector::Message message{};
  tkm::msg::Envelope envelope{};

  // We pack an empty descriptor to calculate envelope size
  message.set_type(tkm::msg::collector::Message_Type_Descriptor);
  message.mutable_data()->PackFrom(descriptor);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  unsigned char buffer[GDescBufferSize]{};
  pbio::ArrayOutputStream outputArray(buffer, sizeof(buffer));
  pbio::CodedOutputStream codedOutput(&outputArray);

  size_t envelopeSize = envelope.ByteSizeLong();
  if ((envelopeSize > UINT32_MAX) || (envelopeSize > sizeof(buffer))) {
    return false;
  }

  codedOutput.WriteVarint32(static_cast<uint32_t>(envelopeSize));
  if (!envelope.SerializeToCodedStream(&codedOutput)) {
    return false;
  }

  if (send(fd, buffer, envelopeSize + sizeof(uint64_t), MSG_WAITALL) !=
      (static_cast<ssize_t>(envelopeSize + sizeof(uint64_t)))) {
    return false;
  }

  return true;
}

bool readCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor)
{
  tkm::msg::collector::Message message{};
  tkm::msg::Envelope envelope{};

  // We pack an empty descriptor to calculate envelope size
  message.set_type(tkm::msg::collector::Message_Type_Descriptor);
  message.mutable_data()->PackFrom(descriptor);
  envelope.mutable_mesg()->PackFrom(message);
  envelope.set_target(tkm::msg::Envelope_Recipient_Monitor);
  envelope.set_origin(tkm::msg::Envelope_Recipient_Collector);

  unsigned char buffer[GDescBufferSize]{};
  pbio::ArrayInputStream inputArray(buffer, sizeof(buffer));
  pbio::CodedInputStream codedInput(&inputArray);

  if (recv(fd, buffer, sizeof(uint64_t), MSG_WAITALL) != static_cast<ssize_t>(sizeof(uint64_t))) {
    return false;
  }

  uint32_t messageSize;
  codedInput.ReadVarint32(&messageSize);
  if (messageSize > (sizeof(buffer) - sizeof(uint64_t))) {
    return false;
  }

  if (recv(fd, buffer + sizeof(uint64_t), messageSize, MSG_WAITALL) != messageSize) {
    return false;
  }

  codedInput.PushLimit(static_cast<int>(messageSize));
  if (!envelope.ParseFromCodedStream(&codedInput)) {
    return false;
  }

  envelope.mesg().UnpackTo(&message);
  if (message.type() != tkm::msg::collector::Message_Type_Descriptor) {
    return false;
  }

  message.data().UnpackTo(&descriptor);
  return true;
}

auto readLink(std::string const &path) -> std::string
{
  char buff[PATH_MAX] = {0};
  ssize_t len = ::readlink(path.c_str(), buff, sizeof(buff) - 1);

  if (len > 0) {
    return std::string(buff);
  }

  return std::string("NA");
}

auto getContextId(pid_t pid) -> uint64_t
{
  std::stringstream ctxStr;

  for (int i = 0; i < 10; i++) {
    std::string procPath{};

    switch (i) {
    case 0: /* cgroup */
      procPath = "/proc/" + std::to_string(pid) + "/ns/cgroup";
      break;
    case 1: /* ipc */
      procPath = "/proc/" + std::to_string(pid) + "/ns/ipc";
      break;
    case 2: /* mnt */
      procPath = "/proc/" + std::to_string(pid) + "/ns/mnt";
      break;
    case 3: /* net */
      procPath = "/proc/" + std::to_string(pid) + "/ns/net";
      break;
    case 4: /* pid */
      procPath = "/proc/" + std::to_string(pid) + "/ns/pid";
      break;
    case 5: /* pid_for_children */
      procPath = "/proc/" + std::to_string(pid) + "/ns/pid_for_children";
      break;
    case 6: /* time */
      procPath = "/proc/" + std::to_string(pid) + "/ns/time";
      break;
    case 7: /* time_for_children */
      procPath = "/proc/" + std::to_string(pid) + "/ns/time_for_children";
      break;
    case 8: /* user */
      procPath = "/proc/" + std::to_string(pid) + "/ns/user";
      break;
    case 9: /* uts */
      procPath = "/proc/" + std::to_string(pid) + "/ns/uts";
      break;
    default: /* never reached */
      break;
    }

    if (exists(procPath)) {
      ctxStr << readLink(procPath);
    }
  }

  return jnkHsh(ctxStr.str().c_str());
}

void tkmLibCheckVersion(const std::string &vstr)
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  if (vstr != TKMLIB_VERSION) {
    throw std::runtime_error("TaskMonitor Library header mismatch");
  }
}

} // namespace tkm
