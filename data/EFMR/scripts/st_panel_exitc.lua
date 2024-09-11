
log_info("Staring Panel Exit C - starting");

local decoders = {}

local dcc = dcclite.dcc0

-- create a table with the led, power button and relays info
for i = 1, 12 do
	local num = string.format("%02d", i)	

	decoders[i] = 
	{		
		led = dcc["ST_PNL_EXTC_LED_PWR_" .. num],
		pwr = dcc["ST_PNL_EXTC_BTN_PWR_" .. num],
		relay = dcc["STC_P" .. num]
	}	
end

function on_power_button_state_change(button, relay)
	if not button.active then
		return
	end
	
	log_trace("[StagingPanelExitC] Button pressed")

    relay:set_state(not relay.active)
end

function on_relay_state_change(relay, led)
    log_trace("[StagingPanelExitC] Relay state change")

    led:set_state(relay.active)
end

for i = 1, #decoders do
	local data = decoders[i]

	--now connect each button to a power relay
	data.pwr:on_state_change(
		function()
			on_power_button_state_change(data.pwr, data.relay)
		end
	)

	-- when relay change state, turn on / off the led
	data.relay:on_state_change(
		function()
			on_relay_state_change(data.relay, data.led)
		end
	)
end

--/////////////////////////////////////////////////////////////////////////////
--
-- Routes
--
--/////////////////////////////////////////////////////////////////////////////

local routes = {
	{
		name = "STC_EXT_TRK_01",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_01,
		led = dcc.ST_PNL_EXTC_LED_TRK_01
	},
	{
		name = "STC_EXT_TRK_02",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = false
			},
			{
				turnout = dcc.STC_T10,
				state = false
			},
			{
				turnout = dcc.STC_T11,
				state = true
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_02,
		led = dcc.ST_PNL_EXTC_LED_TRK_02
	},
	{
		name = "STC_EXT_TRK_03",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = false
			},
			{
				turnout = dcc.STC_T10,
				state = false
			},
			{
				turnout = dcc.STC_T11,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_03,
		led = dcc.ST_PNL_EXTC_LED_TRK_03
	},
	{
		name = "STC_EXT_TRK_04",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = false
			},
			{
				turnout = dcc.STC_T10,
				state = true
			},
			{
				turnout = dcc.STC_T12,
				state = true
			},
			{
				turnout = dcc.STC_T13,
				state = true
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_04,
		led = dcc.ST_PNL_EXTC_LED_TRK_04
	},	
	{
		name = "STC_EXT_TRK_05",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = false
			},
			{
				turnout = dcc.STC_T10,
				state = true
			},
			{
				turnout = dcc.STC_T12,
				state = true
			},
			{
				turnout = dcc.STC_T13,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_05,
		led = dcc.ST_PNL_EXTC_LED_TRK_05
	},
	{
		name = "STC_EXT_TRK_06",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = false
			},
			{
				turnout = dcc.STC_T10,
				state = true
			},
			{
				turnout = dcc.STC_T12,
				state = false
			},
			{
				turnout = dcc.STC_T14,
				state = true
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_06,
		led = dcc.ST_PNL_EXTC_LED_TRK_06
	},
	{
		name = "STC_EXT_TRK_07",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = false
			},
			{
				turnout = dcc.STC_T10,
				state = true
			},
			{
				turnout = dcc.STC_T12,
				state = false
			},
			{
				turnout = dcc.STC_T14,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_07,
		led = dcc.ST_PNL_EXTC_LED_TRK_07
	},
	{
		name = "STC_EXT_TRK_08",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = true
			},
			{
				turnout = dcc.STC_T15,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_08,
		led = dcc.ST_PNL_EXTC_LED_TRK_08
	},
	{
		name = "STC_EXT_TRK_09",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = true
			},
			{
				turnout = dcc.STC_T15,
				state = true
			},
			{
				turnout = dcc.STC_T16,
				state = true
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_09,
		led = dcc.ST_PNL_EXTC_LED_TRK_09
	},
	{
		name = "STC_EXT_TRK_10",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = true
			},
			{
				turnout = dcc.STC_T15,
				state = true
			},
			{
				turnout = dcc.STC_T16,
				state = false
			},
			{
				turnout = dcc.STC_T17,
				state = true
			},
			{
				turnout = dcc.STC_T18,
				state = true
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_10,
		led = dcc.ST_PNL_EXTC_LED_TRK_10
	},
	{
		name = "STC_EXT_TRK_11",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = true
			},
			{
				turnout = dcc.STC_T15,
				state = true
			},
			{
				turnout = dcc.STC_T16,
				state = false
			},
			{
				turnout = dcc.STC_T17,
				state = true
			},
			{
				turnout = dcc.STC_T18,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_11,
		led = dcc.ST_PNL_EXTC_LED_TRK_11
	},
	{
		name = "STC_EXT_TRK_12",
		turnouts = {
			{
				turnout = dcc.STC_T08,
				state = true
			},
			{
				turnout = dcc.STC_T09,
				state = true
			},
			{
				turnout = dcc.STC_T15,
				state = true
			},
			{
				turnout = dcc.STC_T16,
				state = false
			},
			{
				turnout = dcc.STC_T17,
				state = false
			}
		},
		button = dcc.ST_PNL_EXTC_BTN_PAS_12,
		led = dcc.ST_PNL_EXTC_LED_TRK_12
	}
}

function set_route(button, route_info)
	if not button.active then
		return
	end

	log_trace("[StagingPanelExitC] Route button pressed for " .. route_info.name)

	local turnouts = route_info.turnouts

	for i = 1, #turnouts do

		local turnout_info = turnouts[i]

		turnout_info.turnout:set_state(turnout_info.state)
	end	
end	

function check_route(route_info)
	log_trace("[StagingPanelExitC] Checking route " .. route_info.name)

	local turnouts = route_info.turnouts

	for i = 1, #turnouts do

		local turnout_info = turnouts[i]

		if turnout_info.turnout.state ~= turnout_info.state then
			route_info.led:set_state(false)

			log_trace("[StagingPanelExitC] Route failed for turnout " .. turnout_info.turnout.name)

			return
		end
	end	

	log_trace("[StagingPanelExitC] Route set for " .. route_info.name)
	route_info.led:set_state(true)
end

for i = 1, #routes do
	local info = routes[i]

	info.button:on_state_change(
		function(button)
			set_route(button, info)
		end
	)

	local turnouts = info.turnouts
	for i = 1, #turnouts do

		local turnout_info = turnouts[i]

		turnout_info.turnout:on_state_change(
			function()
				check_route(info)
			end
		)
	end

	check_route(info)
end

log_info("Staring Panel Exit C - init ok");