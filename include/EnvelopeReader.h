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

#pragma once

#include <string>

#include "Envelope.pb.h"
#include "IAsyncEnvelope.h"

namespace tkm
{

class EnvelopeReader : public IAsyncEnvelope, public std::enable_shared_from_this<EnvelopeReader>
{
public:
  explicit EnvelopeReader(int fd);

  auto next(tkm::msg::Envelope &envelope) -> IAsyncEnvelope::Status;

public:
  EnvelopeReader(EnvelopeReader const &) = delete;
  void operator=(EnvelopeReader const &) = delete;
};

} // namespace tkm
