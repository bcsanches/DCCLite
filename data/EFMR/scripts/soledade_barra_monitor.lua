
local hlx_t04 = dcclite.dcc0.HLX_T04;
local hlx_t05 = dcclite.dcc0.HLX_T05;

function on_state_change(decoder)
    print("Hello from 'on_state_change' ");

    if(hlx_t04.thrown and hlx_t05.thrown) then
        print("hlx_t04 and hlx_t05 closed - set route to normal");

        return;
    end
        

    if(hlx_t04.closed or hlx_t05.closed) then
        print("hlx_t04 or hlx_t05 closed - set route to reverse");

        return;
    end

end


print("Hello from monitor");

hlx_t04:on_state_change(on_state_change);
hlx_t05:on_state_change(on_state_change);