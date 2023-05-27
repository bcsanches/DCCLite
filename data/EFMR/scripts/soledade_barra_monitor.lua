print("Hello from monitor")

print(dcclite)

print(dcclite.dcc0)
print(dcclite.terminal)
print(dcclite.dccpp)

dcclite.dcc0.HLX_T02:say_hello()

print(dcclite.dcc0[1852].address)
print(dcclite.dcc0.HLX_T02.address)

collectgarbage("collect") 

print(dcclite.dcc0.HLX_T02)

collectgarbage("collect") 

-- dcclite.dcc0:get_turnout(1300):on_throw()