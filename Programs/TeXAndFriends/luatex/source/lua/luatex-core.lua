-- luatex-core security and io overloads ...........

-- if not modules then modules = { } end modules ['luatex-core'] = {
--     version   = 1.080,
--     comment   = 'companion to luatex',
--     author    = 'Hans Hagen & Luigi Scarso',
--     copyright = 'LuaTeX Development Team',
-- }

LUATEXCOREVERSION = 1.080 -- we reflect the luatex version where changes happened

-- This file overloads some Lua functions. The readline variants provide the same
-- functionality as LuaTeX <= 1.04 and doing it this way permits us to keep the
-- original io libraries clean. Performance is probably even a bit better now.

-- We test for functions already being defined so that we don't overload ones that
-- are provided in the startup script.

local type, next, getmetatable, require = type, next, getmetatable, require
local find, gsub, format = string.find, string.gsub, string.format

local io_open             = io.open
local io_popen            = io.popen
local io_lines            = io.lines

local fio_readline        = fio.readline
local fio_checkpermission = fio.checkpermission
local fio_recordfilename  = fio.recordfilename

local mt                  = getmetatable(io.stderr)
local mt_lines            = mt.lines
local saferoption         = status.safer_option
local shellescape         = status.shell_escape -- 0 (disabled) 1 (anything) 2 (restricted)
local kpseused            = status.kpse_used    -- 0 1

local write_nl            = texio.write_nl

io.saved_lines            = io_lines -- always readonly
mt.saved_lines            = mt_lines -- always readonly

local function luatex_io_open(name,how)
    if not how then
        how = 'r'
    end
    local f = io_open(name,how)
    if f then
        if type(how) == 'string' and find(how,'w') then
            fio_recordfilename(name,'w')
        else
            fio_recordfilename(name,'r')
        end
    end
    return f
end

local function luatex_io_open_readonly(name,how)
    if not how then
        how = 'r'
    else
        how = gsub(how,'[^rb]','')
        if how == '' then
            how = 'r'
        end
    end
    local f = io_open(name,how)
    if f then
        fio_recordfilename(name,'r')
    end
    return f
end

local function luatex_io_popen(name,...)
    local okay, found = fio_checkpermission(name)
    if okay and found then
        return io_popen(found,...)
    end
end

-- local function luatex_io_lines(name,how)
--     if name then
--         local f = io_open(name,how or 'r')
--         if f then
--             return function()
--                 return fio_readline(f)
--             end
--         end
--     else
--         return io_lines()
--     end
-- end

-- For some reason the gc doesn't kick in so we need to close explitly
-- so that the handle is flushed.

local error, type = error, type

local function luatex_io_lines(name,how)
    if type(name) == "string" then
        local f = io_open(name,how or 'r')
        if f then
            return function()
                local l = fio_readline(f)
                if not l then
                    f:close()
                end
                return l
            end
        else
            -- for those who like it this way:
            error("patched 'io.lines' can't open '" .. name .. "'")
        end
    else
        return io_lines()
    end
end

local function luatex_io_readline(f)
    return function()
        return fio_readline(f)
    end
end

io.lines = luatex_io_lines
mt.lines = luatex_io_readline

-- We assume management to be provided by the replacement of kpse. This is the
-- case in ConTeXt.

if kpseused == 1 then

    io.open  = luatex_io_open
    io.popen = luatex_io_popen

end

if saferoption == 1 then

    local function installdummy(str,f)
        local reported = false
        return function(...)
            if not reported then
                write_nl(format("safer option set, function %q is %s",
                    str,f and "limited" or "disabled"))
                reported = true
            end
            if f then
                return f(...)
            end
        end
    end

    local function installlimit(str,f)
        local reported = false
    end
    
    debug = nil  
    os.execute = installdummy("os.execute")
    os.spawn   = installdummy("os.spawn")
    os.exec    = installdummy("os.exec")
    os.setenv  = installdummy("os.setenv")
    os.tempdir = installdummy("os.tempdir")

    io.popen   = installdummy("io.popen")
    io.open    = installdummy("io.open",luatex_io_open_readonly)

    os.rename  = installdummy("os.rename")
    os.remove  = installdummy("os.remove")

    io.tmpfile = installdummy("io.tmpfile")
    io.output  = installdummy("io.output")

    lfs.chdir  = installdummy("lfs.chdir")
    lfs.lock   = installdummy("lfs.lock")
    lfs.touch  = installdummy("lfs.touch")
    lfs.rmdir  = installdummy("lfs.rmdir")
    lfs.mkdir  = installdummy("lfs.mkdir")

end

if saferoption == 1 or shellescape ~= 1 then

    ffi = require('ffi')
    for k, v in next, ffi do
        if k ~= 'gc' then
            ffi[k] = nil
        end
    end
    ffi = nil

end

-- os.[execute|os.spawn|os.exec] already are shellescape aware)


if md5 then

    local sum    = md5.sum
    local gsub   = string.gsub
    local format = string.format
    local byte   = string.byte

    if not md5.sumhexa then
        function md5.sumhexa(k)
            return (gsub(sum(k), ".", function(c)
                return format("%02x",byte(c))
            end))
        end
    end

    if not md5.sumHEXA then
        function md5.sumHEXA(k)
            return (gsub(sum(k), ".", function(c)
                return format("%02X",byte(c))
            end))
        end
    end

end

-- compatibility: this might go away

if not unpack then
    unpack = table.unpack
end

if not package.loaders then
    package.loaders = package.searchers
end

if not loadstring then
    loadstring = load
end

-- compatibility: this might stay

if bit32 then

    -- lua 5.2: we're okay

elseif utf8 then

    -- lua 5.3:  bitwise.lua, v 1.24 2014/12/26 17:20:53 roberto

    bit32 = load ( [[
local select = select -- instead of: arg = { ... }

bit32 = {
  bnot = function (a)
    return ~a & 0xFFFFFFFF
  end,
  band = function (x, y, z, ...)
    if not z then
      return ((x or -1) & (y or -1)) & 0xFFFFFFFF
    else
      local res = x & y & z
      for i=1,select("#",...) do
        res = res & select(i,...)
      end
      return res & 0xFFFFFFFF
    end
  end,
  bor = function (x, y, z, ...)
    if not z then
      return ((x or 0) | (y or 0)) & 0xFFFFFFFF
    else
      local res = x | y | z
      for i=1,select("#",...) do
        res = res | select(i,...)
      end
      return res & 0xFFFFFFFF
    end
  end,
  bxor = function (x, y, z, ...)
    if not z then
      return ((x or 0) ~ (y or 0)) & 0xFFFFFFFF
    else
      local res = x ~ y ~ z
      for i=1,select("#",...) do
        res = res ~ select(i,...)
      end
      return res & 0xFFFFFFFF
    end
  end,
  btest = function (x, y, z, ...)
    if not z then
      return (((x or -1) & (y or -1)) & 0xFFFFFFFF) ~= 0
    else
      local res = x & y & z
      for i=1,select("#",...) do
          res = res & select(i,...)
      end
      return (res & 0xFFFFFFFF) ~= 0
    end
  end,
  lshift = function (a, b)
    return ((a & 0xFFFFFFFF) << b) & 0xFFFFFFFF
  end,
  rshift = function (a, b)
    return ((a & 0xFFFFFFFF) >> b) & 0xFFFFFFFF
  end,
  arshift = function (a, b)
    a = a & 0xFFFFFFFF
    if b <= 0 or (a & 0x80000000) == 0 then
      return (a >> b) & 0xFFFFFFFF
    else
      return ((a >> b) | ~(0xFFFFFFFF >> b)) & 0xFFFFFFFF
    end
  end,
  lrotate = function (a ,b)
    b = b & 31
    a = a & 0xFFFFFFFF
    a = (a << b) | (a >> (32 - b))
    return a & 0xFFFFFFFF
  end,
  rrotate = function (a, b)
    b = -b & 31
    a = a & 0xFFFFFFFF
    a = (a << b) | (a >> (32 - b))
    return a & 0xFFFFFFFF
  end,
  extract = function (a, f, w)
    return (a >> f) & ~(-1 << (w or 1))
  end,
  replace = function (a, v, f, w)
    local mask = ~(-1 << (w or 1))
    return ((a & ~(mask << f)) | ((v & mask) << f)) & 0xFFFFFFFF
  end,
}
        ]] )

elseif bit then

    -- luajit (for now)

    bit32 = load ( [[
local band, bnot, rshift, lshift = bit.band, bit.bnot, bit.rshift, bit.lshift

bit32 = {
  arshift = bit.arshift,
  band    = band,
  bnot    = bnot,
  bor     = bit.bor,
  bxor    = bit.bxor,
  btest   = function(...)
    return band(...) ~= 0
  end,
  extract = function(a,f,w)
    return band(rshift(a,f),2^(w or 1)-1)
  end,
  lrotate = bit.rol,
  lshift  = lshift,
  replace = function(a,v,f,w)
    local mask = 2^(w or 1)-1
    return band(a,bnot(lshift(mask,f)))+lshift(band(v,mask),f)
  end,
  rrotate = bit.ror,
  rshift  = rshift,
}
        ]] )

else

    -- hope for the best or fail

    bit32 = require("bit32")

end

-- this is needed for getting require("socket") right

do

    local loaded = package.loaded

    if not loaded.socket then loaded.socket = loaded["socket.core"] end
    if not loaded.mime   then loaded.mime   = loaded["mime.core"]   end

end

do

    local lfsattributes     = lfs.attributes
    local symlinkattributes = lfs.symlinkattributes

    -- these can now be done using lfs (was dead slow before)

    if not lfs.isfile then
        function lfs.isfile(name)
            local m = lfsattributes(name,"mode")
            return m == "file" or m == "link"
        end
    end

    if not lfs.isdir then
        function lfs.isdir(name)
            local m = lfsattributes(name,"mode")
            return m == "directory"
        end
    end

    -- shortnames have also be sort of dropped from kpse

    if not lfs.shortname then
        function lfs.shortname(name)
            return name
        end
    end

    -- now there is a target field, so ...

    if not lfs.readlink then
        function lfs.readlink(name)
            return symlinkattributes(name,"target") or nil
        end
    end

end

-- so far

if utilities and utilities.merger and utilities.merger.compact then

    local byte, format, gmatch = string.byte, string.format, string.gmatch
    local concat = table.concat

    local data = gsub(io.loaddata('luatex-core.lua'),'if%s+utilities.*','')

    local t = { }
    local r = { }
    local n = 0
    local d = gsub(data,'\r\n','\n')      -- be nice for unix
    local s = utilities.merger.compact(d) -- no comments and less spaces

    t[#t+1] = '/* generated from and by luatex-core.lua */'
    t[#t+1] = ''
 -- t[#t+1] = format('/*\n\n%s\n\n*/',d)
 -- t[#t+1] = ''
    t[#t+1] = '#include "lua.h"'
    t[#t+1] = '#include "lauxlib.h"'
    t[#t+1] = ''
    t[#t+1] = 'int load_luatex_core_lua (lua_State * L);'
    t[#t+1] = ''
    t[#t+1] = 'int load_luatex_core_lua (lua_State * L)'
    t[#t+1] = '{'
    t[#t+1] = '  static unsigned char luatex_core_lua[] = {'
    for c in gmatch(d,'.') do
        if n == 16 then
            n = 1
            t[#t+1] = '    ' .. concat(r,', ') .. ','
        else
            n = n + 1
        end
        r[n] = format('0x%02x',byte(c))
    end
    n = n + 1
    r[n] = '0x00'
    t[#t+1] = '    ' .. concat(r,', ',1,n)
    t[#t+1] = '  };'
 -- t[#t+1] = format('unsigned int luatex_core_lua_len = 0x%x;',#d+1)
    t[#t+1] = '  return luaL_dostring(L, (const char*) luatex_core_lua);'
    t[#t+1] = '}'

    io.savedata('luatex-core.c',concat(t,'\n'))
    io.savedata('luatex-core-stripped.lua',s)

end
