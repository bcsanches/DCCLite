log_info("[TC_SIGNAL_D11] Initializing")

-- signal
local signal_d11 = dcclite.dcc0.TC_SIG_11D

-- turnouts
local tc_t02 = dcclite.dcc0.TC_T02

function on_device_change(device)
	if tc_t02.thrown then
		signal_d11:set_aspect(SignalAspects.Clear, "TC_SIGNAL_D11_SCRIPT", "tc_02 thrown")
	else
		signal_d11:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D11_SCRIPT", "tc_02 closed")
	end
end

tc_t02:on_state_change(on_device_change)

on_device_change(tc_t02)

log_info("[TC_SIGNAL_D11] init ok")