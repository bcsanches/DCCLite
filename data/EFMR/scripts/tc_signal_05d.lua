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

local function on_device_change(device)
	if tc_d05_main.inactive and tc_t02.closed and tc_t04.thrown and tc_t06.closed then				
		if signal_d06.aspect == SignalAspects.Stop then				
			signal_d05:set_aspect(SignalAspects.Restricted, "TC_SIGNAL_D05_SCRIPT", msg .. "d06 stop")
		else
			signal_d05:set_aspect(SignalAspects.Clear, "TC_SIGNAL_D05_SCRIPT", msg .. ", not stop")
		end		
	else
		signal_d05:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D05_SCRIPT", tc_d05_main.active and "sensor d05 active (main line blocked)" or "route not set")
	end
end

local function on_section_sig05d_to_sig01e_change(section)
	if section:is_clear() then
		route:unlock()
	else
		route:lock()
	end
end

local function on_section_start_sensor_check(section, sensor)
	if not route:is_route_set() then
		dcclite.dispatcher:panic(section, "[SignalD05::SensorChecker] start sensor activated, but route is not set")

		return false
	end

	return true
end

local function on_section_end_sensor_check(section, sensor)
	return route:is_route_set()	
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

signal_d06:on_aspect_change(on_device_change)

tc_t02:on_state_change(on_device_change)
tc_t04:on_state_change(on_device_change)
tc_t06:on_state_change(on_device_change)

tc_d05_main:on_state_change(on_device_change)

-- initial state
on_device_change(tc_t02)

log_info("[TC_SIGNAL_D05] init ok")