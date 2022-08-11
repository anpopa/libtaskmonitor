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

#pragma once

#include <string>

#include "Envelope.pb.h"
#include "IAsyncEnvelope.h"

namespace tkm
{

class EnvelopeWriter : public IAsyncEnvelope, public std::enable_shared_from_this<EnvelopeWriter>
{
public:
  explicit EnvelopeWriter(int fd);

  auto send(const tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status;
  bool flush();

public:
  EnvelopeWriter(EnvelopeWriter const &) = delete;
  void operator=(EnvelopeWriter const &) = delete;

private:
  bool flushInternal();
};

} // namespace tkm
