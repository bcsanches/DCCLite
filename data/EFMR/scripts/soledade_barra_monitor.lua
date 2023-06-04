
local hlx_t04 = dcclite.dcc0.HLX_T04;
local hlx_t05 = dcclite.dcc0.HLX_T05;
local hlx_t06 = dcclite.dcc0.HLX_T06;

function on_t04_change(decoder)
    local t04_throw = hlx_t04.thown;

    -- Ponta do triangulo esta sentido subida, então não importa a posição do t04
    if(hlx_t06.thrown) then
        return;
    end

    -- saida para soledade esta sentido helix (subida)
    if(hxl_t05.thrown) then
        return;
    end
    
end

function on_t05_change(decoder)
    
end

function initialize()
    log_trace("Hello from 'on_state_change' ");

    local t04_throw = hlx_t04.thown;
    local t05_throw = hlx_t05.thown;

    if(t04_throw and t05_throw) then
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
