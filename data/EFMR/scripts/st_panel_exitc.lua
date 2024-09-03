
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