log_info("TC_02 Initializing")

local tc_t08 = dcclite.dcc0.TC_T08
local tc_t15 = dcclite.dcc0.TC_T15

function tc_t08_turnout_state_change(turnout)
    log_trace("[TC_02] tc_t08_turnout_state_change")
    
    tc_t15:set_state(not tc_t08.state)
end

function tc_t15_turnout_state_change(turnout)
    log_trace("[TC_02] tc_t15_turnout_state_change")
    
    tc_t08:set_state(not tc_t15.state)
end

tc_t08:on_state_change(tc_t08_turnout_state_change)
tc_t15:on_state_change(tc_t15_turnout_state_change)

-- make sure they are in sync
tc_t08_turnout_state_change(tc_t08)

log_info("TC_02 - init OK");
