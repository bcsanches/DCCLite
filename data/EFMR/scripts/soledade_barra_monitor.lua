log_info("SL - BP - Monitor initializing");

local main_line_quad_inverter = dcclite.dcc0.SL_BP_MAIN_INVERTER;

local hlx_quad_inverter = dcclite.dcc0.INV_HELIX_TC_SOL;

local sl_bp_main_d01 = dcclite.dcc0.SL_BP_MAIN_D01;
local sl_bp_main_d02 = dcclite.dcc0.SL_BP_MAIN_D02;
local sl_bp_main_d03 = dcclite.dcc0.SL_BP_MAIN_D03;
local sl_bp_main_d04 = dcclite.dcc0.SL_BP_MAIN_D04;
local sl_bp_main_d05 = dcclite.dcc0.SL_BP_MAIN_D05;

local sl_bp_triangle_turnout = dcclite.dcc0.HLX_T06;

local sl_bp_d01_reset_button = dcclite.dcc0.SL_BP_ResetButton;

local on_hlx_quad_inverter_state_change

function on_section01_state_change(section)
    log_info("section 01 state change: " .. section:get_state_name())

    if section:is_clear() then
        log_trace("[SoledadeBarraMonitor] train left the block")     

        on_hlx_quad_inverter_state_change(hlx_quad_inverter)        

        return
    end

    if section.state == SECTION_STATES.up_start then
        log_trace("[SoledadeBarraMonitor] train leaving soledade")    
    elseif section.state == SECTION_STATES.down_start then

        -- train is aproaching Soledade and entered the block?
        log_trace("[SoledadeBarraMonitor] train aproaching soledade")
    elseif section.state == SECTION_STATES.up then
        log_trace("[SoledadeBarraMonitor] train leaving soledade - block complete")    
    elseif section.state == SECTION_STATES.down then
        log_trace("[SoledadeBarraMonitor] train aproaching soledade - block complete")    
    end
    
    main_line_quad_inverter:set_state(false);
    
end

function on_section02_state_change(section)
    log_info("section 02 state change: " .. section:get_state_name())

    if section:is_clear() then
        log_trace("[SoledadeBarraMonitor] train left the block")     

        on_hlx_quad_inverter_state_change(hlx_quad_inverter)        

        return
    end

    if section.state == SECTION_STATES.up_start then
        log_trace("[SoledadeBarraMonitor] train on midway from soledade")    
    elseif section.state == SECTION_STATES.down_start then

        -- train is aproaching Soledade and entered the block?
        log_trace("[SoledadeBarraMonitor] train on midway to soledade")
    elseif section.state == SECTION_STATES.up then
        log_trace("[SoledadeBarraMonitor] train on midway leaving soledade - block complete")    
    elseif section.state == SECTION_STATES.down then
        log_trace("[SoledadeBarraMonitor] train on midway aproaching soledade - block complete")    
    end
end


function on_tsection03_state_change(tsection)    
    log_trace("[SoledadeBarraMonitor] on_tsection03_state_change")
end

local section01 = Section:new({
    name = "sl_bp_main_s01",
    start_sensor = sl_bp_main_d01,
    end_sensor = sl_bp_main_d02,
    callback = on_section01_state_change,
    address = 1691
})

local section02 = Section:new({
    name = "sl_bp_main_s02",
    start_sensor = sl_bp_main_d02,
    end_sensor = sl_bp_main_d03,
    callback = on_section02_state_change,
    address = 1692
})

local section03 = TSection:new({
    name = "sl_bp_main_s03",
    start_sensor = sl_bp_main_d03,
    closed_sensor = sl_bp_main_d04,
    thrown_sensor = sl_bp_main_d05,
    turnout = sl_bp_triangle_turnout,
    callback = on_tsection03_state_change,    
    address = 1693
})

function on_hlx_quad_inverter_state_change(quad_inverter)

    log_trace("[SoledadeBarraMonitor] on_hlx_quad_inverter_state_change")

    if not section01:is_clear() then
        log_trace("[SoledadeBarraMonitor] section is busy, ignoring")

        return
    end
    
    log_trace("[SoledadeBarraMonitor] section is clear, updating")
    main_line_quad_inverter:set_state(hlx_quad_inverter.active);
end

function on_sl_bp_d01_reset_button_state_change(turnout)
    log_trace("[SoledadeBarraMonitor] on_sl_bp_d01_reset_button_state_change")

    section01:reset()
end

hlx_quad_inverter:on_state_change(on_hlx_quad_inverter_state_change)
sl_bp_d01_reset_button:on_state_change(on_sl_bp_d01_reset_button_state_change)

on_section01_state_change(section01)
on_hlx_quad_inverter_state_change(hlx_quad_inverter)

log_info("SL - BP - init OK");
