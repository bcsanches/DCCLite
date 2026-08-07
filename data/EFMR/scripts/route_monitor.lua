log_info("[RouteMonitor] Init started")

TURNOUT_STATES = {
	closed = "closed",
	thrown = "thrown"
}

RouteMonitor = {}
RouteMonitor.__index = RouteMonitor

function RouteMonitor:new(r)
	
	if not r then
		error("[RouteMonitor:new] parameters required")
	end

	if not r.turnouts then
		error("[RouteMonitor:new] turnouts required")
	end

	for _, turnout_info in ipairs(self.turnouts) do
		if not turnout_info.turnout then
			error("[RouteMonitor:new] turnout required")
		end

		if (not turnout_info.state == TURNOUT_STATES.closed) and (not turnout_info.state == TURNOUT_STATES.thrown) then
			error("[RouteMonitor:new] turnout state must be 'closed' or 'thrown'")
		end
	end

	setmetatable(r, self)
    self.__index = self

end

function RouteMonitor:is_route_set()
	for _, turnout_info in ipairs(self.turnouts) do
		if not turnout_info.turnout[turnout_info.state] then
			return false
		end
	end
	
	return true
end

log_info("[RouteMonitor] Init finished")

