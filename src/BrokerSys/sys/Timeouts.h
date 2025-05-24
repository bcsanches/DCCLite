// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <chrono>

namespace dcclite::broker::sys
{
	using namespace std::chrono_literals;

	auto constexpr NETWORK_DEVICE_TIMEOUT_TICKS = 10s;
	auto constexpr NETWORK_DEVICE_CONFIG_RETRY_TIME = 100ms;
	auto constexpr NETWORK_DEVICE_STATE_TIMEOUT = 250ms;

	auto constexpr NETWORK_DEVICE_SYNC_TIMEOUT = 100ms;

	auto constexpr NETWORK_DEVICE_PING_TIMEOUT = 2s;

	auto constexpr TASK_DOWNLOAD_EEPROM_RETRY_TIMEOUT = 100ms;
	auto constexpr TASK_DOWNLOAD_EEPROM_DOWNLOAD_WAIT = 25ms;
	auto constexpr TASK_DOWNLOAD_EEPROM_WAIT_CONNECTION = 250ms;

	auto constexpr TASK_RENAME_DEVICE_TIMEOUT = 50ms;

	auto constexpr TASK_CLEAR_EEPROM_TIMEOUT = 50ms;	

	auto constexpr TASK_SERVO_PROGRAMMER_TIMEOUT = 50ms;

	auto constexpr TASK_NETWORK_TEST_DEFAULT_TIMEOUT = 20ms;

	auto constexpr FILE_WATCHER_IGNORE_TIME = 100ms;

	auto constexpr LOCONET_THINK_TIME = 20ms;
	auto constexpr LOCONET_PURGE_INTERVAL = 100s;
	auto constexpr LOCONET_PURGE_TIMEOUT = 200s;

	auto constexpr SIGNAL_FLASH_INTERVAL = 500ms;
	auto constexpr SIGNAL_WAIT_STATE_TIMEOUT = 250ms;

	auto constexpr THROTTLE_SERVICE_THINK_TIME = 20ms;

	auto constexpr ZERO_CONF_SERVICE_PACKET_INTERVAL = 50ms;
	auto constexpr ZERO_CONF_SERVICE_SLEEP_TIME = 100ms;

	auto constexpr BONJOUR_SERVICE_PACKET_INTERVAL = 100ms;
	auto constexpr BONJOUR_SERVICE_SLEEP_TIME = 100ms;
}
