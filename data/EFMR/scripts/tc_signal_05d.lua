log_info("[TC_SIGNAL_D05] Initializing")

-- signal
local signal_d06 = dcclite.dcc0.TC_SIG_6D
local signal_d05 = dcclite.dcc0.TC_SIG_5D

-- turnouts
local tc_t02 = dcclite.dcc0.TC_T02
local tc_t04 = dcclite.dcc0.TC_T04
local tc_t06 = dcclite.dcc0.TC_T06

-- sensors
local tc_d05_main = dcclite.dcc0.TC_D_MAIN_05D
local tc_d01_main = dcclite.dcc0.TC_D_MAIN_01E

-- signal request button
local tc_d05_clear_request = dcclite.dcc0.TC_ButtonClearSignal05D

local section_sig05d_to_sig01e

local msg = "tc_02 closed, tc_04 thrown, tc_06 closed, "

local route = RouteMonitor:new({
	turnouts = {
		{
			turnout = tc_t02,
			state = TURNOUT_STATES.closed
		},
		{
			turnout = tc_t04,
			state = TURNOUT_STATES.thrown
		},
		{
			turnout = tc_t06,
			state = TURNOUT_STATES.closed
		}
	}
})

local function on_section_sig05d_to_sig01e_change(section)
	if section:is_clear() then
		route:unlock()			
	else
		route:lock("signal_05d script")

		signal_d05:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D05_SCRIPT", "section ocupied")
	end	
end

local function on_section_start_sensor_check(section, sensor)
	if not route:is_route_set() then
		-- this is a problem, the train will get a blocked turnout...
		dcclite.dispatcher:panic(section, "[SignalD05::SensorChecker] start sensor activated, but route is not set")

		return false
	end	

	return true
end

local function on_section_end_sensor_check(section, sensor)
	-- no panic here, the train will get another route if our route is not set, not our concern...
	return route:is_route_set()	
end

local function request_permission_to_clear_signal()
	log_trace("[SignalD05::RequestPermission] request permission to clear signal entered")

	if signal_d05.aspect ~= SignalAspects.Stop then
		log_trace("[SignalD05::RequestPermission] Signal was not stop")

		if section_sig05d_to_sig01e:is_clear() then
			log_trace("[SignalD05::RequestPermission] section is clear, cancelling signal")

			signal_d05:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D05_SCRIPT", "Signal cancelled")
		else
			log_trace("[SignalD05::RequestPermission] section is not clear, signal cannot be changed")
		end

		return true
	end

	if not route:is_route_set() then
		dcclite.dispatcher:panic(section_sig05d_to_sig01e, "[SignalD05::RequestPermission] request permission to clear signal DENIED, route is not set")

		return false
	end

	if not section_sig05d_to_sig01e:is_clear() then
		dcclite.dispatcher:panic(section_sig05d_to_sig01e, "[SignalD05::RequestPermission] request permission to clear signal DENIED, section is not clear")

		return false
	end

	if signal_d06.aspect == SignalAspects.Stop then				
		signal_d05:set_aspect(SignalAspects.Restricted, "TC_SIGNAL_D05_SCRIPT", "route set and SIGNAL D06 stop")
	else
		signal_d05:set_aspect(SignalAspects.Clear, "TC_SIGNAL_D05_SCRIPT", "route set and SIGNAL D06 not on stop aspect")
	end

	log_trace("[SignalD05::RequestPermission] request permission to clear signal finished")
	return true
end

section_sig05d_to_sig01e = Section:new({
	name = "tc_hlx_05d_to_01e",
	start_sensor = tc_d05_main,
	end_sensor = tc_d01_main,
	callback = on_section_sig05d_to_sig01e_change,
	start_sensor_checker = on_section_start_sensor_check,
	end_sensor_checker = on_section_end_sensor_check,
	address = 1460
})

tc_d05_clear_request:on_state_change(request_permission_to_clear_signal)

-- initial state
signal_d05:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D05_SCRIPT", "initial state")

log_info("[TC_SIGNAL_D05] init ok")