local base = _G
local coroutine = require("coroutine")
local string = require("string")
local math = require("math")
local os = require("os")
local socket = socket or require("socket") ; 
local tp = socket.tp or require("socket.tp"); 
local ltn12 = ltn12 or require("ltn12");
local headers = socket.headers or require("socket.headers"); 
local mime = mime or require("mime"); 
socket.smtp = {}
local _M = socket.smtp
module("socket.smtp")
_M.TIMEOUT = 60
_M.SERVER = "localhost"
_M.PORT = 25
_M.DOMAIN = os.getenv("SERVER_NAME") or "localhost"
_M.ZONE = "-0000"
local metat = { __index = {} }
function metat.__index:greet(domain)
    self.try(self.tp:check("2.."))
    self.try(self.tp:command("EHLO", domain or _M.DOMAIN))
    return socket.skip(1, self.try(self.tp:check("2..")))
end
function metat.__index:mail(from)
    self.try(self.tp:command("MAIL", "FROM:" .. from))
    return self.try(self.tp:check("2.."))
end
function metat.__index:rcpt(to)
    self.try(self.tp:command("RCPT", "TO:" .. to))
    return self.try(self.tp:check("2.."))
end
function metat.__index:data(src, step)
    self.try(self.tp:command("DATA"))
    self.try(self.tp:check("3.."))
    self.try(self.tp:source(src, step))
    self.try(self.tp:send("\r\n.\r\n"))
    return self.try(self.tp:check("2.."))
end
function metat.__index:quit()
    self.try(self.tp:command("QUIT"))
    return self.try(self.tp:check("2.."))
end
function metat.__index:close()
    return self.tp:close()
end
function metat.__index:login(user, password)
    self.try(self.tp:command("AUTH", "LOGIN"))
    self.try(self.tp:check("3.."))
    self.try(self.tp:send(mime.b64(user) .. "\r\n"))
    self.try(self.tp:check("3.."))
    self.try(self.tp:send(mime.b64(password) .. "\r\n"))
    return self.try(self.tp:check("2.."))
end
function metat.__index:plain(user, password)
    local auth = "PLAIN " .. mime.b64("\0" .. user .. "\0" .. password)
    self.try(self.tp:command("AUTH", auth))
    return self.try(self.tp:check("2.."))
end
function metat.__index:auth(user, password, ext)
    if not user or not password then return 1 end
    if string.find(ext, "AUTH[^\n]+LOGIN") then
        return self:login(user, password)
    elseif string.find(ext, "AUTH[^\n]+PLAIN") then
        return self:plain(user, password)
    else
        self.try(nil, "authentication not supported")
    end
end
function metat.__index:send(mailt)
    self:mail(mailt.from)
    if base.type(mailt.rcpt) == "table" then
        for i,v in base.ipairs(mailt.rcpt) do
            self:rcpt(v)
        end
    else
        self:rcpt(mailt.rcpt)
    end
    self:data(ltn12.source.chain(mailt.source, mime.stuff()), mailt.step)
end
function _M.open(server, port, create)
    local tp = socket.try(tp.connect(server or _M.SERVER, port or _M.PORT,
        _M.TIMEOUT, create))
    local s = base.setmetatable({tp = tp}, metat)
    s.try = socket.newtry(function()
        s:close()
    end)
    return s
end
local function lower_headers(headers)
    local lower = {}
    for i,v in base.pairs(headers or lower) do
        lower[string.lower(i)] = v
    end
    return lower
end
local seqno = 0
local function newboundary()
    seqno = seqno + 1
    return string.format('%s%05d==%05u', os.date('%d%m%Y%H%M%S'),
        math.random(0, 99999), seqno)
end
local send_message
local function send_headers(tosend)
    local canonic = headers.canonic
    local h = "\r\n"
    for f,v in base.pairs(tosend) do
        h = (canonic[f] or f) .. ': ' .. v .. "\r\n" .. h
    end
    coroutine.yield(h)
end
local function send_multipart(mesgt)
    local bd = newboundary()
    local headers = lower_headers(mesgt.headers or {})
    headers['content-type'] = headers['content-type'] or 'multipart/mixed'
    headers['content-type'] = headers['content-type'] ..
        '; boundary="' ..  bd .. '"'
    send_headers(headers)
    if mesgt.body.preamble then
        coroutine.yield(mesgt.body.preamble)
        coroutine.yield("\r\n")
    end
    for i, m in base.ipairs(mesgt.body) do
        coroutine.yield("\r\n--" .. bd .. "\r\n")
        send_message(m)
    end
    coroutine.yield("\r\n--" .. bd .. "--\r\n\r\n")
    if mesgt.body.epilogue then
        coroutine.yield(mesgt.body.epilogue)
        coroutine.yield("\r\n")
    end
end
local function send_source(mesgt)
    local headers = lower_headers(mesgt.headers or {})
    headers['content-type'] = headers['content-type'] or
        'text/plain; charset="iso-8859-1"'
    send_headers(headers)
    while true do
        local chunk, err = mesgt.body()
        if err then coroutine.yield(nil, err)
        elseif chunk then coroutine.yield(chunk)
        else break end
    end
end
local function send_string(mesgt)
    local headers = lower_headers(mesgt.headers or {})
    headers['content-type'] = headers['content-type'] or
        'text/plain; charset="iso-8859-1"'
    send_headers(headers)
    coroutine.yield(mesgt.body)
end
function send_message(mesgt)
    if base.type(mesgt.body) == "table" then send_multipart(mesgt)
    elseif base.type(mesgt.body) == "function" then send_source(mesgt)
    else send_string(mesgt) end
end
local function adjust_headers(mesgt)
    local lower = lower_headers(mesgt.headers)
    lower["date"] = lower["date"] or
        os.date("!%a, %d %b %Y %H:%M:%S ") .. (mesgt.zone or _M.ZONE)
    lower["x-mailer"] = lower["x-mailer"] or socket._VERSION
    lower["mime-version"] = "1.0"
    return lower
end
function _M.message(mesgt)
    mesgt.headers = adjust_headers(mesgt)
    local co = coroutine.create(function() send_message(mesgt) end)
    return function()
        local ret, a, b = coroutine.resume(co)
        if ret then return a, b
        else return nil, a end
    end
end
_M.send = socket.protect(function(mailt)
    local s = _M.open(mailt.server, mailt.port, mailt.create)
    local ext = s:greet(mailt.domain)
    s:auth(mailt.user, mailt.password, ext)
    s:send(mailt)
    s:quit()
    return s:close()
end)
socket.smtp = _M
return _M
