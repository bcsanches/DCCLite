
local SECTION_STATES = {
    clear = {},
    up_start = {},
    up = {},
    down_start = {},
    down = {}
}

function get_section_state_name(state)
    lgo_trace("trying to find state name")

    if state == SECTION_STATES.clear then
        return "clear"
    elseif state == SECTION_STATES.up_start then
        return "up_start"
    elseif state == SECTION_STATES.up then
        return "up"
    elseif state == SECTION_STATES.down_start then
        return "down_start"
    else
        return "down"
    end
end

--[[

    UP -> going from start_sensor to end_sensor
    DOWN -> going from end_sensor to start_sensor

--]]

local MiniBlock = {};
Section = {};

function MiniBlock:new(o)

    if not o then
        error("[MiniBlock:new]  parameters required");
    end
    
    setmetatable(o, self);
    self.__index = self;

    if not o.start_sensor then
        error("[MiniBlock:new] start sensor is required");
    end

    if not o.end_sensor then
        error("[MiniBlock:new] end sensor is required");
    end

    if not o.state_table then
        error("[MiniBlock:new] state_table is required");
    end

    if not o.owner then
        error("[MiniBlock:new] owner is required");
    end

    --[[
    o.start_sensor = start_sensor;
    o.end_sensor = end_sensor;

    o.state_table = state_table;    

    o.owner = owner;
    --]]

    return o;
end

function MiniBlock:on_finished()
    self.owner.state = SECTION_STATES.clear;

    self.owner.callback(self);

    self.mini_block = nil;
end

function MiniBlock:on_start_sensor_change(sensor)
    -- we simple ignore the start sensor oscilatting    
end

function MiniBlock:on_end_sensor_change(sensor)

    if sensor.active then
        if self.owner.state == state_table.complete then
            log_warn("[MiniBlock:on_end_sensor_change] miniblock end sensor activated again, did start sensor was active when deactivated?")

            return
        end

        log_trace("[MiniBlock:on_end_sensor_change] end sensor active - block is complete - train touched second sensor")
        self.owner.state = state_table.complete;

        self.owner.callback(owner)
    else
        if self.start_sensor.active then
            -- on this case, we ignore and expect end sensor to be re activated
            log_warn("[MiniBlock:on_end_sensor_change] miniblock end sensor deactivated, but start sensor is active!!!");

            return;
        end

        log_trace("[MiniBlock:on_end_sensor_change] miniblock finished - train left the block");
        self:on_finished();
    end

end

function dispatch_mini_block_sensor_event(sensor, mini_block)
    if mini_block.start_sensor == sensor then
        mini_block.on_start_sensor_change(sensor);
    else
        mini_block.on_end_sensor_change(sensor);
    end
end

function Section:on_start_sensor_change(sensor)
    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block);
    elseif sensor.active then

        log_trace("[Section:on_start_sensor_change] start sensor active - train entered the block and is going up")

        --local a = SECTION_STATES.up_start

        --log_trace("test " .. get_section_state_name(SECTION_STATES.up_start))

        log_trace("[Section:on_start_sensor_change] sgt" .. type(self.state))

        -- train entered the block
        self.state = SECTION_STATES.up_start;

        log_trace("[Section:on_start_sensor_change] state changed to: " .. self.state)

        self.mini_block = MiniBlock.new({
            start_sensor = self.start_sensor,
            end_sensor = self.end_sensor,
            owner = self,
            state_table = {
                complete = SECTION_STATES.up
            }
        });

        log_trace("[Section:on_start_sensor_change] created mini bloco: " .. type(self.mini_block))

        self.callback(self);
    else
        -- sensor disabled... but no miniblock active... invalid state
        log_error("[Section:on_start_sensor_change] start sensor inactive event... but block was not active...");

    end
end

function Section:on_end_sensor_change(sensor)
    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block);
    elseif sensor.active then

        log_trace("[Section:on_end_sensor_change] end sensor active - train entered the block and is going down")

        -- train entered the block
        self.state = SECTION_STATES.down_start;

        self.mini_block = MiniBlock.new({
            start_sensor = self.end_sensor,
            end_sensor = self.start_sensor,
            owner = self,
            state_table = {
                complete = SECTION_STATES.down
            }
        });
    else    
        -- sensor disabled... but no miniblock active... invalid state
        log_error("[Section:on_start_sensor_change] end sensor inactive event... but block was not active...");

    end
end

function Section:is_going_up()
    return self.state == SECTION_STATES.going_up or self.state == SECTION_STATES.up
end

function Section:is_going_down()
    return self.state == SECTION_STATES.going_down or self.state == SECTION_STATES.down
end

function Section:new(o)

    if not o then
        error("[Section:new]  parameters required");
    end
    
    setmetatable(o, self);
    self.__index = self;

    if not o.start_sensor or not o.end_sensor then
        error("[Section:new] start_sensor and end_sensor must be provided");
    end    

    if not o.callback and type(o.callback) ~= "function" then
        error("[Section:new] callback is nil or not a function]")
    end

    log_trace("[Section:new] Registering callback for start sensor")
    o.start_sensor:on_state_change(
        function(sensor) 
            o:on_start_sensor_change(sensor);
        end
    );

    log_trace("[Section:new] Registering callback for end sensor")
    o.end_sensor:on_state_change(
        function(sensor) 
            o:on_end_sensor_change(sensor);
        end
    );

    -- how are the sensors?
    if o.start_sensor.active and o.end_sensor.active then
        log_error("[Section:new] both sensors active, state will be undefined")        
    else
        if o.start_sensor.active then
            o:on_start_sensor_change(o.start_sensor);            
        elseif o.end_sensor.active then
            o:on_end_sensor_change(o.end_sensor);            
        else
            o.state = SECTION_STATES.clear;
        end
    end    
    
    return o;
end



