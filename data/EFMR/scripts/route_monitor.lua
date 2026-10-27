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

	for _, turnout_info in ipairs(r.turnouts) do
		if not turnout_info.turnout then
			error("[RouteMonitor:new] turnout required")
		end

		if (turnout_info.state ~= TURNOUT_STATES.closed) and (turnout_info.state ~= TURNOUT_STATES.thrown) then
			error("[RouteMonitor:new] turnout state must be 'closed' or 'thrown'")
		end
	end

	setmetatable(r, self)
    self.__index = self

	return r
end

function RouteMonitor:is_route_set()
	for _, turnout_info in ipairs(self.turnouts) do
		if not turnout_info.turnout[turnout_info.state] then
			return false
		end
	end
	
	return true
end

function RouteMonitor:lock(reason)
	log_info("[RouteMonitor] lock reason = " .. tostring(reason))

	for _, turnout_info in ipairs(self.turnouts) do
		log_info("[RouteMonitor] calling turnout lock reason = " .. tostring(reason))

		turnout_info.turnout:lock(reason)
	end
end

function RouteMonitor:unlock()
	for _, turnout_info in ipairs(self.turnouts) do
		turnout_info.turnout:unlock()
	end
end

log_info("[RouteMonitor] Init finished")

