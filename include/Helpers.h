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

#pragma once

#include <cstdint>
#include <sched.h>
#include <string>

#include "Collector.pb.h"

namespace tkm
{

auto jnkHsh(const char *key) -> uint64_t;

auto base64Encode(unsigned char const *bytes_to_encode, unsigned int in_len) -> std::string;
auto base64Decode(std::string const &encoded_string) -> std::string;

bool sendCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor);
bool readCollectorDescriptor(int fd, tkm::msg::collector::Descriptor &descriptor);

auto readLink(std::string const &path) -> std::string;
auto getContextId(pid_t pid) -> uint64_t;

void tkmLibCheckVersion(const std::string &vstr);

} // namespace tkm
