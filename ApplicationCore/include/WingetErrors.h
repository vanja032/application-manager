// WingetErrors.h

#pragma once

#include <cstdint>

namespace WingetErrors
{
constexpr uint32_t UpdateNotApplicable = 0x8A15002B;
constexpr uint32_t UpgradeVersionNotNewer = 0x8A15004F;
constexpr uint32_t UpgradeVersionUnknown = 0x8A150050;
} // namespace WingetErrors