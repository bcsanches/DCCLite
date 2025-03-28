--[[

The Signal D06 has a diagram like this:

         HLX_T03                     HLX_T02        HLX_T01
---------|----O----|-------O-----|-------------O------------|-------- -> Staging Exit - Stop
    |-O     \ HLX_DTC02  HLX_DTC01     /   HLX_DMT03    \
     D06     \                        /                  \--|--O----- -> Staging Entrance - Clear
              \   ---------------|---/                       HLX_DTC00
			   | / 
			   |/  HLX_T04                               HLX_T08
			   |                                            /---> Coronel Fulgêncio (down line - internal - Stop)
               |                       HLX_T07             /
			   |\  HLX_T05           ---------------------------> Coronel Fulgêncio (up line - external - RESTRICTED)
			   | \                      /                  
			   |  \--------------------/                    
			   |
			   |
sl_bp_main_d04 O  /
			   | /
			   |/    HLX_T06
			   O     sl_bp_main_s03
			   |
			   O		sl_bp_main_s02
			   |
			   O		sl_bp_main_s01 / sl_bp_main_d01
			   |
			   v Soledade - Aproach

]]--

log_info("[TC_SIGNAL_D06] Initializing")

-- signal
local signal_d06 = dcclite.dcc0.TC_SIG_6D

-- turnouts
local hlx_t08 = dcclite.dcc0.HLX_T08
local hlx_t07 = dcclite.dcc0.HLX_T07
local hlx_t06 = dcclite.dcc0.HLX_T06
local hlx_t05 = dcclite.dcc0.HLX_T05
local hlx_t04 = dcclite.dcc0.HLX_T04
local hlx_t03 = dcclite.dcc0.HLX_T03
local hlx_t02 = dcclite.dcc0.HLX_T02
local hlx_t01 = dcclite.dcc0.HLX_T01

-- sensors for helix route
local hlx_sensor_dtc02 = dcclite.dcc0.HLX_DTC02
local hlx_sensor_dtc01 = dcclite.dcc0.HLX_DTC01
local hlx_sensor_dtc00 = dcclite.dcc0.HLX_DTC00

local hlx_sensor_dmt03 = dcclite.dcc0.HLX_DMT03

-- soledade branch sections
-- local sl_bp_main_s03 = dcclite.dcc0.sl_bp_main_s03
-- local sl_bp_main_s02 = dcclite.dcc0.sl_bp_main_s02
-- local sl_bp_main_s01 = dcclite.dcc0.sl_bp_main_s01

-- soledade branch sensors
local sl_bp_main_d01 = dcclite.dcc0.SL_BP_MAIN_D01

SIGNAL_STATES = {
    automatic = 0,
	helix_path_clear = 1,
	helix_path_busy = 2,
	helix_path_exiting = 4,
	soledade_path_clear = 8,
	soledade_path_busy = 16,
	soledade_path_exiting = 32 
}

local signal_state = SIGNAL_STATES.automatic

function set_stop_aspect(reason)
	log_info(reason)

	signal_d06:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D06_SCRIPT", reason)
	signal_state = SIGNAL_STATES.automatic
end

function set_helix_down_aspect()
	log_info("TC_SIGNAL_D06 Route to staging CLEAR")
	signal_d06:set_aspect(SignalAspects.Clear, "TC_SIGNAL_D06_SCRIPT", "Path to staging entrance is set")

	signal_state = SIGNAL_STATES.helix_path_clear
end

function set_soledade_branch_aspect()
	log_info("[TC_SIGNAL_D06] Soledade path CLEAR")
	signal_d06:set_aspect(SignalAspects.Aproach, "TC_SIGNAL_D06_SCRIPT", "Path to Soledade is set")

	signal_state = SIGNAL_STATES.soledade_path_clear
end

function on_train_entered_helix_down(device)

	log_info("[TC_SIGNAL_D06] on_train_entered_helix_down " .. device.name)

	-- sensor turned off? We do not care...
	if not device.active then
		log_trace("[TC_SIGNAL_D06] on_train_entered_helix_down sensor is off")

		return
	end

	-- is train heading to soledade?
	if signal_state == SIGNAL_STATES.soledade_path_clear then	

		-- now wait for train to reach end sensor
		signal_state = SIGNAL_STATES.soledade_path_busy

		signal_d06:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D06_SCRIPT", "Train entered Soledade path " .. signal_state)

	elseif signal_state == SIGNAL_STATES.helix_path_clear then
		log_trace("[TC_SIGNAL_D06] on_train_entered_helix_down sensor is on, now path is busy, signal is STOP " .. signal_state)

		-- now wait for train to reach end sensor
		signal_state = SIGNAL_STATES.helix_path_busy

		signal_d06:set_aspect(SignalAspects.Stop, "TC_SIGNAL_D06_SCRIPT", "Train entered helix path to staging entrance " .. signal_state)
	end	
end

function on_helix_exit_sensor(sensor)

	log_trace("[TC_SIGNAL_D06] on_helix_exit_sensor sensor")

	if (signal_state == SIGNAL_STATES.helix_path_busy) and sensor.active then				
		log_trace("[TC_SIGNAL_D06] on_helix_exit_sensor sensor ACTIVE")

		signal_state = SIGNAL_STATES.helix_path_exiting

	elseif (signal_state == SIGNAL_STATES.helix_path_exiting) and sensor.inactive then
		log_trace("[TC_SIGNAL_D06] on_helix_exit_sensor INACTIVE - resetting")

		--reset signal
		signal_state = SIGNAL_STATES.automatic

		-- check state
		on_device_change(sensor)
	end
end

function on_soledade_branch_exit_sensor(sensor)
	if (signal_state == SIGNAL_STATES.soledade_path_busy) and sensor.active then				
		signal_state = SIGNAL_STATES.soledade_path_exiting		
	elseif (signal_state == SIGNAL_STATES.soledade_path_exiting) and sensor.inactive then

		--reset signal
		signal_state = SIGNAL_STATES.automatic

		-- check state
		on_device_change(sensor)
	end
end

--[[
function on_helix_path_section_change(section)
    log_info("[TC_SIGNAL_D06] [on_helix_path_section_change] " .. section:get_state_name())

    if section:is_clear() then
        log_trace("[TC_SIGNAL_D06] [on_helix_path_section_change] train left the block")        

        return
    end

    if section.state == SECTION_STATES.up_start then
        log_trace("[TC_SIGNAL_D06] [on_helix_path_section_change] train leaving TC")    
    elseif section.state == SECTION_STATES.down_start then
        -- train is aproaching Soledade and entered the block?
        log_trace("[TC_SIGNAL_D06] [on_helix_path_section_change] train aproaching TC")
    elseif section.state == SECTION_STATES.up then
        log_trace("[TC_SIGNAL_D06] [on_helix_path_section_change] train entering staging - block complete")    
    elseif section.state == SECTION_STATES.down then
        log_trace("[TC_SIGNAL_D06] [on_helix_path_section_change] train entering TC - block complete")    
    end            
end
--]]

function is_helix_path_reserved()
	return (signal_state ~= SIGNAL_STATES.helix_path_clear) and (signal_state ~= SIGNAL_STATES.helix_path_busy) and (signal_state ~= SIGNAL_STATES.helix_path_exiting)
end

function on_device_change(device)

	log_info("[TC_SIGNAL_D06] on_device_change " .. device.name)

	-- Are we going to Soledade or up the helix?
    if hlx_t03.thrown then		
		
		log_info("[TC_SIGNAL_D06] hlx_t03 thrown - Soledade or Helix UP path")

		-- Is path blocked?
		if hlx_t04.thrown then		
        	set_stop_aspect("[TC_SIGNAL_D06] hlx_t04 thrown - Soledade or Helix blocked - STOP")

        	return
		end

		-- Are we going up the helix?
		if hlx_t05.thrown then
			log_info("[TC_SIGNAL_D06] hlx_t05 thrown - going up to Helix")

			-- Is path blocked?
			if hlx_t07.closed then				
				set_stop_aspect("[TC_SIGNAL_D06] hlx_t07 closed - path blocked - STOP")
        		
				return
			end

			-- Is route set to down line (internal)?
			if hlx_t08.thrown then
				set_stop_aspect("[TC_SIGNAL_D06] hlx_t08 thrown - path to wrong line - STOP")
        		
				return
			end

			-- Going up... give a restricted, as line is incomplete
			log_info("[TC_SIGNAL_D06] hlx_t07 thrown - going up the helix - RESTRICTED")
			signal_d06:set_aspect(SignalAspects.Restricted, "TC_SIGNAL_D06_SCRIPT", "Path to Helix UP is set")
			return
		end

		-- ok, we are heading towards to Soledade
		log_info("[TC_SIGNAL_D06] going to Soledade")

		-- Is path blocked?
		if hlx_t06.thrown then			
			set_stop_aspect("[TC_SIGNAL_D06] hlx_t06 thrown - path to Soledade blocked - STOP")
        		
			return
		end

		-- if heading down to SOLEDADE, ignore sensors...
		if (signal_state == SIGNAL_STATES.soledade_path_busy) or (signal_state == SIGNAL_STATES.soledade_path_exiting) then
			-- do not modify signal, wait for sensors
			log_trace("[TC_SIGNAL_D06] path is busy by soledade, waiting sensors: " .. signal_state)
			return
		end

		--[[ Sections are buggy, ignore for now
		if sl_bp_main_s03.active then
			set_stop_aspect("[TC_SIGNAL_D06] sl_bp_main_s03 IN USE - STOP")
        		
			return
		end

		if sl_bp_main_s02.active then			
			set_stop_aspect("[TC_SIGNAL_D06] sl_bp_main_s02 IN USE - STOP")
        		
			return
		end

		if sl_bp_main_s01.active then			
			set_stop_aspect("[TC_SIGNAL_D06] sl_bp_main_s01 IN USE - STOP")
        		
			return
		end
		]]--

		-- ok, soledade path is clear
		set_soledade_branch_aspect()
		return

    end

	-- We are heading down the helix
	log_trace("[TC_SIGNAL_D06] heading down the helix")

	-- is path blocked?
    if hlx_t02.thrown then		
        set_stop_aspect("[TC_SIGNAL_D06] hlx_t02 thrown - path down helix blocked - stop " .. signal_state)

        return
    end

	-- is path toward helix exit?
	if hlx_t01.closed then		
        set_stop_aspect("[TC_SIGNAL_D06] hlx_t01 closed - path set to staging entrance - stop " .. signal_state)

        return
    end

	-- path is down to helix entrance, ok, right path... but

	-- if heading down the helix, ignore sensors...
	if not is_helix_path_reserved() then
		-- do not modify signal, wait for sensors
		log_trace("[TC_SIGNAL_D06] path is busy by helix, waiting sensors: " .. signal_state)
		return
	end

	-- is block ocupied?
	if hlx_sensor_dtc02.active then		
		set_stop_aspect("[TC_SIGNAL_D06] DTC02 ACTIVE - path to Helix is ocupied - stop " .. signal_state)

		return
	end

	-- is block ocupied?
	if hlx_sensor_dtc01.active then		
		set_stop_aspect("[TC_SIGNAL_D06] DTC01 ACTIVE - path to Helix is ocupied - stop " .. signal_state)

		return
	end

	if hlx_sensor_dmt03.active then
		set_stop_aspect("[TC_SIGNAL_D06] DMT03 ACTIVE - path to Helix is ocupied - stop " .. signal_state)

		return
	end

	if hlx_sensor_dtc00.active then
		set_stop_aspect("[TC_SIGNAL_D06] DTC00 ACTIVE - path to Helix is ocupied - stop " .. signal_state)

		return
	end	

	-- Finally path clear and no blocks ocupied...
	set_helix_down_aspect()
end

hlx_t08:on_state_change(on_device_change)
hlx_t07:on_state_change(on_device_change)
hlx_t06:on_state_change(on_device_change)
hlx_t05:on_state_change(on_device_change)
hlx_t04:on_state_change(on_device_change)
hlx_t03:on_state_change(on_device_change)
hlx_t02:on_state_change(on_device_change)
hlx_t01:on_state_change(on_device_change)

hlx_sensor_dtc02:on_state_change(on_device_change)
hlx_sensor_dtc01:on_state_change(on_device_change)
hlx_sensor_dtc00:on_state_change(on_device_change)
hlx_sensor_dmt03:on_state_change(on_device_change)

hlx_sensor_dtc02:on_state_change(on_train_entered_helix_down)
hlx_sensor_dtc00:on_state_change(on_helix_exit_sensor)

sl_bp_main_d01:on_state_change(on_soledade_branch_exit_sensor)

--[[
helix_down_path_section = Section:new({
	name = "tc_stg_ent_s01",
	start_sensor = hlx_sensor_dtc02,
	end_sensor = hlx_sensor_dtc00,
	callback = on_helix_path_section_change,
	address = 1880
})
]]--

-- set initial state
on_device_change(signal_d06)

log_info("[TC_SIGNAL_D06] - init OK " .. signal_state)
