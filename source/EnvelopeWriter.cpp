/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     EnvelopeWriter Class
 * @details   IPC Envelope writer helper class
 *-
 */

#include <cstring>
#include <errno.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "EnvelopeWriter.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

namespace pbio = google::protobuf::io;

namespace tkm
{

EnvelopeWriter::EnvelopeWriter(int fd)
: IAsyncEnvelope("EnvelopeWriter", fd)
{
}

auto EnvelopeWriter::send(const tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status
{
  uint64_t envelopeSize = envelope.ByteSizeLong();
  std::scoped_lock lk(m_mutex);

  if ((envelopeSize > UINT32_MAX) || (envelopeSize > GAsyncBufferSize)) {
    return Status::Error;
  }

  if ((envelopeSize + sizeof(uint64_t)) > (sizeof(m_buffer) - m_bufferOffset)) {
    if (!flushInternal()) {
      return Status::Error;
    }
  }

  pbio::ArrayOutputStream outputArray(m_buffer + m_bufferOffset,
                                      static_cast<int>(sizeof(m_buffer) - m_bufferOffset));
  pbio::CodedOutputStream codedOutput(&outputArray);

  const auto initialBufferOffset = m_bufferOffset;
  codedOutput.WriteVarint32(static_cast<uint32_t>(envelopeSize));
  m_bufferOffset += sizeof(uint64_t);

  if (!envelope.SerializeToCodedStream(&codedOutput)) {
    m_bufferOffset = initialBufferOffset;
    return Status::Error;
  }
  m_bufferOffset += envelopeSize;

  return Status::Ok;
}

bool EnvelopeWriter::flush()
{
  std::scoped_lock lk(m_mutex);
  return flushInternal();
}

bool EnvelopeWriter::flushInternal()
{
  const auto maxRetry = 4;
  const auto sleepTimeMs = 250;
  auto sizeToSend = m_bufferOffset;
  auto sendRetry = 0;
  auto status = true;

  while ((sizeToSend > 0) && status) {
    auto retVal = ::send(
        m_fd, m_buffer + (m_bufferOffset - sizeToSend), sizeToSend, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (retVal < 0) {
      if (errno == EWOULDBLOCK || (EWOULDBLOCK != EAGAIN && errno == EAGAIN)) {
        if (sendRetry++ < maxRetry) {
          usleep(1000 * sleepTimeMs);
        } else {
          status = false;
        }
      } else {
        status = false;
      }
    } else if (retVal == 0) {
      status = false; // File descriptor closed
    } else {
      sizeToSend -= static_cast<uint64_t>(retVal);
    }
  }

  // Time to reset the buffer
  bufferReset();

  return status;
}

} // namespace tkm
