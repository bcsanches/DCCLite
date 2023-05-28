print("Hello from monitor")

print(dcclite)

print(dcclite.dcc0)
print(dcclite.terminal)
print(dcclite.dccpp)

print(dcclite.dcc0[1852].address)
print(dcclite.dcc0.HLX_T02.address)

print(dcclite.dcc0[dcclite.dcc0.HLX_T02.address].address)

collectgarbage("collect") 

print(dcclite.dcc0.HLX_T02)

collectgarbage("collect") 

-- dcclite.dcc0:get_turnout(1300):on_throw()