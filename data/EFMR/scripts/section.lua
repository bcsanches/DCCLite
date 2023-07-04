
local section_states = {
    clear = {},
    up_start = {},
    up = {},
    down_start = {},
    down = {}
}

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
    
    setmetable(o, self);
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

function MiniBlock:finished()
    self.owner.state = section_states.clear;

    self.mini_block = nil;
end

function MiniBlock:on_start_sensor_change(sensor)

    -- makes no sense... block already started sensor is oscillating... ignore
    if sensor.active then
        return;
    end
    

    -- special case... start sensor turned off AFTER end_sensor turned off
    if self.end_sensor.inactive then
        log_warn("[MiniBlock:on_start_sensor_change] miniblock finished, but with START_SENSOR!!!");

        self:on_miniblock_finished();
    end    
end

function MiniBlock:on_end_sensor_change(sensor)

    if sensor.active then
        self.owner.state = state_table.complete;
    else
        if self.start_sensor.active then

            log_warn("[MiniBlock:on_start_sensor_change] miniblock end sensor deactivated, but start sensor is active!!!");

            return;
        end

        log_trace("[MiniBlock:on_start_sensor_change] miniblock finished");
        self:on_miniblock_finished();
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

        -- train entered the block
        self.state = section_states.up_start;

        self.mini_block = MiniBlock.new({
            start_sensor = self.start_sensor,
            end_sensor = self.end_sensor,
            owner = self,
            state_table = {
                complete = section_states.up
            }
        });
    else
        -- sensor disabled... but no miniblock active... invalid state
        log_error("[Section:on_start_sensor_change] start sensor inactive event... but block was not active...");

    end
end

function Section:on_end_sensor_change(sensor)
    if self.mini_block then
        dispatch_mini_block_sensor_event(sensor, self.mini_block);
    elseif sensor.active then

        -- train entered the block
        self.state = section_states.down_start;

        self.mini_block = MiniBlock.new({
            start_sensor = self.end_sensor,
            end_sensor = self.start_sensor,
            owner = self,
            state_table = {
                complete = section_states.down
            }
        });
    else    
        -- sensor disabled... but no miniblock active... invalid state
        log_error("[Section:on_start_sensor_change] end sensor inactive event... but block was not active...");

    end
end

function Section:new(o)

    if not o then
        error("[Section:new]  parameters required");
    end
    
    setmetable(o, self);
    self.__index = self;

    if not o.start_sensor or not o.end_sensor then
        error("[Section:new] start_sensor and end_sensor must be provided");
    end    

    o.start_sensor.on_state_change(
        function(sensor) 
            o:on_start_sensor_change(sensor);
        end
    );

    o.end_sensor.on_state_change(
        function(sensor) 
            o:on_end_sensor_change(sensor);
        end
    );

    -- how are the sensors?
    if o.start_sensor.active and o.end_sensor.active then
        log_error("[Section:new] both sensors active, state will be undefined")        
    else
        if o.start_sensor.active then
            o.on_start_sensor_change(o.start_sensor);            
        elseif end_sensor.active then
            o.on_end_sensor_change(o.end_sensor);            
        else
            o.state = section_states.clear;
        end
    end    
    
    return o;
end



