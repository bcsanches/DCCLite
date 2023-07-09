
local main_line_quad_inverter = dcclite.dcc0.SL_BP_MAIN_INVERTER;

local hlx_quad_inverter = dcclite.dcc0.INV_HELIX_TC_SOL;

local sl_bp_main_d01 = dcclite.dcc0.SL_BP_MAIN_D01;
local sl_bp_main_d02 = dcclite.dcc0.SL_BP_MAIN_D02;

function on_hlx_quad_inverter_state_change(quad_inverter)
    if section:is_clear() then
        return
    end
    
    main_line_quad_inverter:set_state(hlx_quad_inverter.active);
end

function on_section01_state_change(section)
    log_info("section 01 state change: " .. section:get_state_name())

    if section:is_clear() then
        log_trace("[SoledadeBarraMonitor] train left the block")     

        main_line_quad_inverter:set_state(true);

        return
    end

    if section.state == SECTION_STATES.up_start then
        log_trace("[SoledadeBarraMonitor] train leaving soledade")    
    elseif section.state == SECTION_STATES.down_start then

        -- train is aproaching Soledade and entered the block?
        log_trace("[SoledadeBarraMonitor] train aproaching soledade")

    end
    
    main_line_quad_inverter:set_state(hlx_quad_inverter.active);
    
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

hlx_quad_inverter:on_state_change(on_hlx_quad_inverter_state_change)

on_section01_state_change(section01)


