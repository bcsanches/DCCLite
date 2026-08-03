log_info("[TC_SIGNAL_D05] Initializing")

-- signal
local signal_d06 = dcclite.dcc0.TC_SIG_6D
local signal_d05 = dcclite.dcc0.TC_SIG_5D

-- turnouts
local tc_t02 = dcclite.dcc0.TC_T02
local tc_t04 = dcclite.dcc0.TC_T04
local tc_t06 = dcclite.dcc0.TC_T06

local function on_device_change(device)
	if tc_t02.closed and tc_t04.thrown and tc_t06.closed then
		local msg = "tc_02 closed, tc_04 thrown, tc_06 closed, "

		if signal_d06.aspect == SignalAspects.Stop then
			signal_d05:set_aspect(SignalAspects.Restricted, "TC_SIGNAL_D05_SCRIPT", msg .. "d06 stop")
		else
			signal_d05:set_aspect(SignalAspects.Clear, "TC_SIGNAL_D05_SCRIPT", msg .. ", not stop")
		end
	else
		signal_d05:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D05_SCRIPT", "route not set")
	end
end

tc_t02:on_state_change(on_device_change)
tc_t04:on_state_change(on_device_change)
tc_t06:on_state_change(on_device_change)

signal_d06:on_aspect_change(on_device_change)

-- initial state
on_device_change(tc_t02)

log_info("[TC_SIGNAL_D05] init ok")