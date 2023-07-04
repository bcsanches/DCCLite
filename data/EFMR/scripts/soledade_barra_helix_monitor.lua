
local hlx_t04 = dcclite.dcc0.HLX_T04;
local hlx_t05 = dcclite.dcc0.HLX_T05;
local hlx_t06 = dcclite.dcc0.HLX_T06;

local hlx_quad_inverter = dcclite.dcc0.INV_HELIX_TC_SOL;

function on_t04_change(decoder)

    log_info("hlx_t04 change event");

    local t04_closed = hlx_t04.closed;    

    if t04_closed then

        log_info("hlx_t04 closed - activating quad inverter");
        hlx_quad_inverter:set_state(true);
        
    else

        log_info("hlx_t04 thrown - disabling quad inverter");
        hlx_quad_inverter:set_state(false);        

    end

    -- Ponta do triangulo esta sentido subida, então não importa a posição do t04
    --[[
    if(hlx_t06.thrown) then
        return;
    end

    -- saida para soledade esta sentido helix (subida)
    if(hlx_t05.thrown) then
        return;
    end
    ]]--
    
end

function on_t05_change(decoder)
    
end

function initialize()
    log_trace("Hello from 'on_state_change' ");

    on_t04_change();

    local t04_thrown = hlx_t04.thrown;
    local t05_thrown = hlx_t05.thrown;

    if(t04_thrown and t05_thrown) then
        log_info("hlx_t04 and hlx_t05 closed - set route to normal");

        return;
    end
        

    if(hlx_t04.closed or hlx_t05.closed) then
        log_info("hlx_t04 or hlx_t05 closed - set route to reverse");

        return;
    end

end


log_info("Hello from monitor");

hlx_t04:on_state_change(on_t04_change);
hlx_t05:on_state_change(on_t05_change);

initialize();
