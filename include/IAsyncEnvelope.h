/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     IAsyncEnvelope Class
 * @details   AsyncEnvelope interface
 *-
 */

#pragma once

#include <fcntl.h>
#include <memory>
#include <mutex>
#include <string.h>
#include <string>
#include <unistd.h>

namespace tkm
{

constexpr size_t GAsyncBufferSize = 1048576;

class IAsyncEnvelope
{
public:
  enum class Status { Ok, Error, Again, EndOfFile };

public:
  explicit IAsyncEnvelope(const std::string &name, int fd)
  : m_name(name)
  , m_fd(fd)
  {
    fcntl(fd, F_SETFL, O_NONBLOCK);
  }

  ~IAsyncEnvelope()
  {
    if (m_closeFdOnDelete) {
      if (m_fd > 0) {
        ::close(m_fd);
      }
    }
  }

public:
  IAsyncEnvelope(IAsyncEnvelope const &) = delete;
  void operator=(IAsyncEnvelope const &) = delete;

  [[nodiscard]] int getFD() const { return m_fd; }

  void setCloseOnDelete(bool state) { m_closeFdOnDelete = state; }

protected:
  void bufferReset()
  {
    ::memset(&m_buffer, 0, sizeof(m_buffer));
    m_bufferOffset = 0;
  }

protected:
  std::string m_name{};
  int m_fd = -1;
  std::mutex m_mutex;
  size_t m_bufferOffset = 0;
  bool m_closeFdOnDelete = false;
  unsigned char m_buffer[GAsyncBufferSize]{};
};

} // namespace tkm
