
local main_line_quad_inverter = dcclite.dcc0.SL_BP_MAIN_INVERTER;

local sl_bp_main_d01 = dcclite.dcc0.SL_BP_MAIN_D01;
local sl_bp_main_d02 = dcclite.dcc0.SL_BP_MAIN_D02;

function on_section01_state_change(section)
    log_info("section 01 state change")

    if section.state == section_states.clear then
        log_trace("[SoledadeBarraMonitor] train left the block")

        return
    end

    -- train is leaving Soledade and entered the block?
    if section.state == section_states.up_start then
        log_trace("[SoledadeBarraMonitor] train leaving soledade")
    -- train is aproaching Soledade and entered the block?
    elseif section.state == section_states.down_start then
        log_trace("[SoledadeBarraMonitor] train aproaching soledade")
    end



end

function on_sensor_change_test(sensor)

    log_trace("[SoledadeBarraMonitor] on_sensor_change_test")

    main_line_quad_inverter:set_state(sensor.active);
    
end


local section01 = Section:new({
    start_sensor = sl_bp_main_d01,
    end_sensor = sl_bp_main_d02,
    callback = on_section01_state_change
})


--sl_bp_main_d01:on_state_change(on_sensor_change_test);

log_info("SL - BP - Monitor initializing");

--on_section01_state_change(section01)


