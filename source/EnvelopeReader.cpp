/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     EnvelopeReader Class
 * @details   IPC Envelope reader helper class
 *-
 */

#include <cstring>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include "EnvelopeReader.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace pbio = google::protobuf::io;

namespace tkm
{

EnvelopeReader::EnvelopeReader(int fd)
: IAsyncEnvelope("EnvelopeReader", fd)
{
}

auto EnvelopeReader::next(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status
{
  std::scoped_lock lk(m_mutex);

  auto retVal =
      recv(m_fd, m_buffer + m_bufferOffset, sizeof(m_buffer) - m_bufferOffset, MSG_DONTWAIT);
  if (retVal < 0) {
    if (errno == EWOULDBLOCK || (EWOULDBLOCK != EAGAIN && errno == EAGAIN)) {
      if (m_bufferOffset == 0) {
        return Status::Again;
      }
    } else {
      return Status::Error;
    }
  } else if (retVal == 0) {
    if (m_bufferOffset == 0) {
      return Status::EndOfFile;
    }
  } else {
    m_bufferOffset += retVal;
  }

  // Check if we have a complete envelope
  pbio::ArrayInputStream inputArray(m_buffer, m_bufferOffset);
  pbio::CodedInputStream codedInput(&inputArray);
  uint32_t messageSize;

  codedInput.ReadVarint32(&messageSize);
  if ((messageSize + sizeof(uint64_t)) > m_bufferOffset) {
    // We don't have the complete message
    return Status::Again;
  }

  codedInput.PushLimit(messageSize);
  if (!envelope.ParseFromCodedStream(&codedInput)) {
    bufferReset();
    return Status::Error;
  }

  if ((messageSize + sizeof(uint64_t)) < m_bufferOffset) {
    memmove(m_buffer,
            m_buffer + (sizeof(uint64_t) + messageSize),
            (m_bufferOffset - (sizeof(uint64_t) + messageSize)));
    m_bufferOffset -= messageSize + sizeof(uint64_t);
  } else {
    bufferReset();
  }

  return Status::Ok;
}

} // namespace tkm
