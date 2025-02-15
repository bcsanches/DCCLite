log_info("[Section] Init started")

SECTION_STATES = {
    clear = 0,
    up_start = 1,
    up = 2,
    down_start = 3,
    down = 4
}

function get_section_state_name(state)
    --log_trace("trying to find state name")

    if state == SECTION_STATES.clear then
        return "clear"
    elseif state == SECTION_STATES.up_start then
        return "up_start"
    elseif state == SECTION_STATES.up then
        return "up"
    elseif state == SECTION_STATES.down_start then
        return "down_start"
    elseif state == SECTION_STATES.down then
        return "down"
    else
        return nil
    end
end

--[[

    UP -> going from start_sensor to end_sensor
    DOWN -> going from end_sensor to start_sensor

--]]

local MiniBlock = {}
Section = {}
Section.__index = Section

TSection = {}
TSection.__index = TSection
setmetatable(TSection, Section)

function MiniBlock:new(o)

    if not o then
        error("[MiniBlock:new]  parameters required")
    end
    
    setmetatable(o, self)
    self.__index = self

    if not o.start_sensor then
        error("[MiniBlock:new] start sensor is required")
    end

    if not o.end_sensor then
        error("[MiniBlock:new] end sensor is required")
    end

    if not o.state_table then
        error("[MiniBlock:new] state_table is required")
    end

    if not o.owner then
        error("[MiniBlock:new] owner is required")
    end

    --[[
    o.start_sensor = start_sensor
    o.end_sensor = end_sensor

    o.state_table = state_table    

    o.owner = owner
    --]]

    return o
end

function MiniBlock:on_finished()
    self.owner:_on_mini_block_finished()
end

function MiniBlock:on_start_sensor_change(sensor)
    -- we simple ignore the start sensor oscilatting    
end

function MiniBlock:on_end_sensor_change(sensor)    

    log_trace("[MiniBlock:on_end_sensor_change] starting it")    

    if sensor.active then
        if self.owner.state == self.state_table.complete then
            log_warn("[MiniBlock:on_end_sensor_change] miniblock end sensor activated again, did start sensor was active when deactivated?")

            return
        end

        log_trace("[MiniBlock:on_end_sensor_change] end sensor active - block is complete - train touched second sensor")
        self.owner:_update_state(self.state_table.complete)        

        self.owner.callback(self.owner)
    else
        if self.start_sensor.active then
            -- on this case, we ignore and expect end sensor to be re activated
            log_warn("[MiniBlock:on_end_sensor_change] miniblock end sensor deactivated, but start sensor is active!!!")

            return
        end

        log_trace("[MiniBlock:on_end_sensor_change] miniblock finished - train left the block")
        self:on_finished()
    end

end

function dispatch_mini_block_sensor_event(sensor, mini_block)    
    if mini_block.start_sensor == sensor then
        mini_block:on_start_sensor_change(sensor)
    else
        mini_block:on_end_sensor_change(sensor)
    end
end

-------------------------------------------------------------------------------
--
-- Section
--
-------------------------------------------------------------------------------

function Section:_on_mini_block_finished()    
    self:_update_state(SECTION_STATES.clear)    
    self.mini_block = nil

    self.callback(self)    
end

function Section:reset()
    self:_on_mini_block_finished()
end

function Section:handle_sensor_change(sensor, start_sensor, end_sensor, new_state, complete_state)

    if sensor.active then

        log_trace("[Section:handle_sensor_change] train entered the block and is " .. get_section_state_name(new_state))        
        
        self:_update_state(new_state)        

        log_trace("[Section:handle_sensor_change] state changed to: " .. get_section_state_name(self.state))

        self.mini_block = MiniBlock:new({
            start_sensor = start_sensor,
            end_sensor = end_sensor,
            owner = self,
            state_table = {
                complete = complete_state
            }
        })

        log_trace("[Section:handle_sensor_change] created mini block")

        self.callback(self)
    else
        -- sensor disabled... but no miniblock active... invalid state
        log_error("[Section:handle_sensor_change] start sensor inactive event... but block was not active...")

    end
end


function Section:on_start_sensor_change(sensor)
    log_trace("[Section:" .. self.name .. "] on_start_sensor_change")

	if self.inactive then
		return
	end

    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block)
    else
        self:handle_sensor_change(sensor, self.start_sensor, self.end_sensor, SECTION_STATES.up_start, SECTION_STATES.up)
    end
end

function Section:on_end_sensor_change(sensor)    
    log_trace("[Section:" .. self.name .. "] on_end_sensor_change")

	if self.inactive then
		return
	end

    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block)
    else
        self:handle_sensor_change(sensor, self.end_sensor, self.start_sensor, SECTION_STATES.down_start, SECTION_STATES.down)        
    end
end

function Section:is_going_up()
    return self.state == SECTION_STATES.going_up or self.state == SECTION_STATES.up
end

function Section:is_going_down()
    return self.state == SECTION_STATES.going_down or self.state == SECTION_STATES.down
end

function Section:is_clear()
    return self.state == SECTION_STATES.clear
end

function Section:get_state_name()
    return get_section_state_name(self.state)
end

function Section:deactivate()
	o.inactive = true
	
	if self.state ~= SECTION_STATES.clear then
		self:_on_mini_block_finished()
	end
end

function Section:activate()
	if not self.inactive then
		return
	end

	self.inactive = false
	if self.start_sensor.active and self.end_sensor.active then
        log_error("[Section:activate] both sensors active, state will be undefined")        
    else
        if self.start_sensor.active then
            self:on_start_sensor_change(self.start_sensor)            
        elseif self.end_sensor.active then
            self:on_end_sensor_change(self.end_sensor)            
        else
            self:_update_state(SECTION_STATES.clear)            
        end
    end
end

function Section:is_active()
	return not self.inactive
end


function Section:_update_state(newState)    
    log_trace("[Section] _update_state")

    self.state = newState
    self.dispatcher:on_section_state_change(self, newState)
end


function Section:new(o)

    if not o then
        error("[Section:new]  parameters required")
    end
    
    setmetatable(o, self)

    if not o.name then
        error("[Section:new] name is required")
    end

    if not o.address then
        error("[Section:new] address is required")
    end

    if not o.start_sensor or not o.end_sensor then
        error("[Section:new] start_sensor and end_sensor must be provided")
    end    

    if not o.callback and type(o.callback) ~= "function" then
        error("[Section:new] callback is nil or not a function]")
    end

    log_trace("[Section:new] Registering callback for start sensor")
    o.start_sensor:on_state_change(
        function(sensor)            
            o:on_start_sensor_change(sensor)
        end
    )

    log_trace("[Section:new] Registering callback for end sensor")
    o.end_sensor:on_state_change(
        function(sensor)                        
            o:on_end_sensor_change(sensor)
        end
    )

    log_trace("[Section:new] acessing dispatcher and registering")

    o.dispatcher = dcclite.dispatcher
    o.dispatcher:register_section(o.name, o)

    log_trace("[Section:new] configuring initial state")

    -- Activate it?
	if not o:inactive then
    	o:activate()
	end
    
    log_trace("[Section:new] OK")
    
    return o
end

-------------------------------------------------------------------------------
--
-- TSECTION
--
-------------------------------------------------------------------------------

function TSection:on_start_sensor_change(sensor)
    log_trace("[TSection:on_start_sensor_change] on_start_sensor_change")

    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block)
    else
        self:handle_sensor_change(sensor, self.start_sensor, self.end_sensor, SECTION_STATES.up_start, SECTION_STATES.up)
    end
end

function TSection:on_closed_sensor_change(sensor)

    log_trace("[TSection:on_closed_sensor_change] on_closed_sensor_change")

    if not self.turnout.closed then

        if sensor.active then
            self.dispatcher:panic(self, "[TSection::on_closed_sensor_change] Closed sensor activated, but turnout is not closed")        
        else 
            -- if sensor deactivated, we ignore, train may have just left the block and turnout changed before sensor went off
            log_trace("[TSection:on_closed_sensor_change] Closed sensor deactivated with a thrown turnout")            
        end
        
        return
    end

    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block)
    else
        self:handle_sensor_change(sensor, self.end_sensor, self.start_sensor, SECTION_STATES.down_start, SECTION_STATES.down)        
    end
end

function TSection:on_thrown_sensor_change(sensor)

    log_trace("[TSection:on_thrown_sensor_change] on_thrown_sensor_change")

    if self.turnout.closed then

        if sensor.active then
            self.dispatcher:panic(self, "[TSection::on_thrown_sensor_change] Thrown sensor activated, but turnout is closed")        
        else 
            -- if sensor deactivated, we ignore, train may have just left the block 
            log_trace("[TSection:on_closed_sensor_change] Throw sensor deactivated with a closed turnout")            
        end
        
        return
    end

    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block)
    else
        self:handle_sensor_change(sensor, self.end_sensor, self.start_sensor, SECTION_STATES.down_start, SECTION_STATES.down)        
    end
end

function TSection:on_turnout_state_change(turnout)    

    log_trace("[TSection:on_turnout_state_change] Turnout state changed")

    if self.state ~= SECTION_STATES.clear then
        self.dispatcher:panic(self, "turnout changed state while section is ACTIVE")        
    end

    if self.turnout.closed then
        self.end_sensor = self.closed_sensor
    else
        self.end_sensor = self.thrown_sensor
    end
end

function TSection:new(o)

    if not o then
        error("[TSection:new]  parameters required")
    end
    
    setmetatable(o, self)    

    if not o.name then
        error("[TSection:new] name is required")
    end

    if not o.start_sensor or not o.closed_sensor or not o.thrown_sensor then
        error("[TSection:new] start_sensor, end_sensor and thrown_sensor must be provided")
    end    

    if not o.callback and type(o.callback) ~= "function" then
        error("[TSection:new] callback is nil or not a function]")
    end

    if not o.turnout then
        error("[TSection:new] turnout is required")
    end

    if not o.address then
        error("[TSection:new] address is required")
    end

    log_trace("[TSection:new] Registering callback for start sensor")
    o.start_sensor:on_state_change(
        function(sensor)            
            o:on_start_sensor_change(sensor)
        end
    )

    log_trace("[TSection:new] Registering callback for closed sensor")
    o.closed_sensor:on_state_change(
        function(sensor)                        
            o:on_closed_sensor_change(sensor)
        end
    )

    log_trace("[TSection:new] Registering callback for thrown sensor")
    o.thrown_sensor:on_state_change(
        function(sensor)                        
            o:on_thrown_sensor_change(sensor)
        end
    )

    log_trace("[TSection:new] Registering callback for turnout")
    o.turnout:on_state_change(
        function(turnout)
            o:on_turnout_state_change(turnout)
        end
    )

    o.state = SECTION_STATES.clear
    o.dispatcher = dcclite.dispatcher
    o.dispatcher:register_tsection(o.name, o)
        
    log_trace("[TSection:new] Calling turnout state change for initializing")
    TSection.on_turnout_state_change(o, o.turnout)

    -- how are the sensors?
    if o.start_sensor.active and o.end_sensor.active then
        log_error("[TSection:new] both sensors active, state will be undefined")        
    else
        if o.start_sensor.active then
            o:on_start_sensor_change(o.start_sensor)            
        elseif o.end_sensor.active then
            o:on_end_sensor_change(o.end_sensor)            
        else
            o:_update_state(SECTION_STATES.clear)            
        end
    end        
    
    return o
end

log_info("[Section] Init finished")