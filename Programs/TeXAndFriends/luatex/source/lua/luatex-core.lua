-- luatex-core security and io overloads ..

-- if not modules then modules = { } end modules ['luatex-core'] = {
--     version   = 1.112,
--     comment   = 'companion to luatex',
--     author    = 'Hans Hagen & Luigi Scarso',
--     copyright = 'LuaTeX Development Team',
-- }

LUATEXCOREVERSION = 1.180 -- we reflect the luatex version where changes happened

-- This file overloads some Lua functions. The readline variants provide the same
-- functionality as LuaTeX <= 1.04 and doing it this way permits us to keep the
-- original io libraries clean. Performance is probably even a bit better now.

-- We test for functions already being defined so that we don't overload ones that
-- are provided in the startup script.



local saferoption    = status.safer_option
local luadebugoption = status.luadebug_option
local shellescape    = status.shell_escape -- 0 (disabled) 1 (anything) 2 (restricted)
local kpseused       = status.kpse_used    -- 0 1
local gmatch         = string.gmatch

--
-- Useful extension of lfs.mkdir,
-- lfs.mkdirp(path) make parent directories as needed
-- (from  luatex-fonts-merged.lua)
--
function lfs.mkdirp(path)
 local full=""
 local r1,r2,r3
 for sub in gmatch(path,"(/*[^\\/]+)") do 
  full=full..sub
  r1,r2,r3 = lfs.mkdir(full)
 end
 return r1,r2,r3 
end



if kpseused == 1 and (status.shell_escape ~=1)  then

    local type = type
    local gsub = string.gsub
    local find = string.find

    local mt                    = getmetatable(io.stderr)
    local mt_lines              = mt.lines

    local kpse_checkpermission  = kpse.check_permission
    local kpse_recordinputfile  = kpse.record_input_file
    local kpse_recordoutputfile = kpse.record_output_file

    local kpse_in_name_ok_silent_extended = kpse.in_name_ok_silent_extended
    local kpse_out_name_ok_silent_extended = kpse.out_name_ok_silent_extended

    local io_open               = io.open
    -- local io_popen              = io.popen -- not need, we  use os.kpsepopen
    local io_lines              = io.lines

    local fio_readline          = fio.readline

    local write_nl              = texio.write_nl
    local format   		= string.format

    io.saved_lines              = io_lines -- always readonly
    mt.saved_lines              = mt_lines -- always readonly
 
    local os_rename = os.rename
    local os_remove = os.remove

    local lfs_attributes = lfs.attributes 
    local lfs_chdir = lfs.chdir
    local lfs_lock_dir =  lfs.lock_dir
    local lfs_dir = lfs.dir 
    local lfs_link = lfs.link 
    local lfs_mkdir = lfs.mkdir
    local lfs_mkdirp = lfs.mkdirp
    local lfs_rmdir = lfs.rmdir
    local lfs_symlinkattributes = lfs.symlinkattributes
    local lfs_touch = lfs.touch



    local LUATEX_EPERM = -1 
    local LUATEX_EPERM_MSG = "LuaTeX: operation not permitted"

    local function luatex_io_open(name,how)
        if not how then
            how = 'r'
        end
        local check = true 
        if how == 'r' or how == 'rb' or how == '' then 
           check = kpse_in_name_ok_silent_extended(name)  
        else 
           check = kpse_out_name_ok_silent_extended(name)
        end  
	local f = nil 
	if check then
	 f = io_open(name,how)
         if f then
            if type(how) == 'string' and find(how,'w') then
                kpse_recordoutputfile(name,'w')
            else
                kpse_recordinputfile(name,'r')
            end
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
        local check = false 
        if how == 'r' or how == 'rb' or how == '' then 
           check = kpse_in_name_ok_silent_extended(name)  
        end  
	local f = nil 
	if check then
	  f = io_open(name,how)
          if f then
            fio_recordfilename(name,'r')
          end
	end  
        return f
    end

    --  not need, we  use os.kpsepopen 
    --local function luatex_io_popen(name,...)
    --    local okay, found = kpse_checkpermission(name)
    --    if okay and found then
    --        return io_popen(found,...)
    --    end
    --end

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

    -- For some reason the gc doesn't kick in so we need to close explicitly
    -- so that the handle is flushed.

    local error, type = error, type

    local function luatex_io_lines(name,how)
        if type(name) == "string" then
            local check = kpse_in_name_ok_silent_extended(name)  
            local f = check and io_open(name,how or 'r')
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

    --
    --
    -- These functions must pass
    --  kpse.in_name_ok_silent_extended
    -- and kpse.out_name_ok_silent_extended
    --

    local function luatex_os_rename(oldname,newname)
      local check1 = kpse_in_name_ok_silent_extended(oldname)  
      local check2 = kpse_out_name_ok_silent_extended(newname)  
      if check1 and check2 then 
        return os_rename(oldname,newname)
      else
        return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
      end
    end

    local function luatex_os_remove(filename)
      local check1 = kpse_in_name_ok_silent_extended(filename)  
      local check2 = kpse_out_name_ok_silent_extended(filename)  
      if check1 and check2 then 
       return os_remove(filename)
      else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
      end
    end

    local function luatex_lfs_attributes(filepath, opt)
      local check1 = kpse_in_name_ok_silent_extended(filepath)
      if check1 then 
       return lfs_attributes(filepath,opt)
      else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
      end
    end

    local function luatex_lfs_chdir(name)
     local check1 = kpse_in_name_ok_silent_extended(name) and kpse_out_name_ok_silent_extended(name)  
     if check1 then 
       return lfs_chdir(name)
      else
       return nil, LUATEX_EPERM_MSG
     end
    end

    local function luatex_lfs_lock_dir(name,second_stale)
     local check1 = kpse_in_name_ok_silent_extended(name) and kpse_out_name_ok_silent_extended(name)  
     if check1 then 
       return lfs_lock_dir(name,second_stale)
      else
       return nil, LUATEX_EPERM_MSG
     end
    end

    local function luatex_lfs_dir(name)
     local check1 = kpse_in_name_ok_silent_extended(name)
     if check1 then 
       return lfs_dir(name)
      else
       error(LUATEX_EPERM_MSG)
     end
    end

    local function luatex_lfs_link(oldf,newf,symlink) 
     local check1 = kpse_in_name_ok_silent_extended(newf) and kpse_out_name_ok_silent_extended(newf)
     if check1 then
	check1 = kpse_in_name_ok_silent_extended(oldf) and kpse_out_name_ok_silent_extended(oldf)
	if check1 then
	  return lfs_link(oldf,newf,symlink) 
	 else
	  return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
	end 
      else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
     end
    end

    local function luatex_lfs_mkdir(name)
     local check1 = kpse_in_name_ok_silent_extended(name) and kpse_out_name_ok_silent_extended(name)  
     if check1 then 
       return lfs_mkdir(name)
      else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
     end
    end

    local function luatex_lfs_mkdirp(name)
     local check1 = kpse_in_name_ok_silent_extended(name) and kpse_out_name_ok_silent_extended(name)  
     if check1 then 
        local full=""
        local r1,r2,r3 
        for sub in gmatch(name,"(/*[^\\/]+)") do 
	  full=full..sub
  	  r1,r2,r3 = lfs_mkdir(full)
        end
        return r1,r2,r3
     else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
     end
    end

    local function luatex_lfs_rmdir(name)
     local check1 = kpse_in_name_ok_silent_extended(name) and kpse_out_name_ok_silent_extended(name)  
     if check1 then 
       return lfs_rmdir(name)
      else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
     end
    end

    local function luatex_lfs_symlinkattributes(filepath,aname)
     local check1 = kpse_in_name_ok_silent_extended(filepath) 
     if check1 then 
       return lfs_symlinkattributes(filepath,aname)
      else
       return nil, LUATEX_EPERM_MSG,LUATEX_EPERM
     end
    end

    local function luatex_lfs_touch(name,atime,mtime)
     local check1 = kpse_in_name_ok_silent_extended(name) and kpse_out_name_ok_silent_extended(name)  
     if check1 then 
       return lfs_touch(name,atime,mtime)
      else
       return nil, LUATEX_EPERM_MSG
     end
    end




    io.lines = luatex_io_lines
    mt.lines = luatex_io_readline

    io.open  = luatex_io_open
    --io.popen = luatex_io_popen -- not need, we  use os.kpsepopen
    io.popen = os.kpsepopen

    os.rename = luatex_os_rename
    os.remove = luatex_os_remove

    lfs.attributes        = luatex_lfs_attributes
    lfs.chdir             = luatex_lfs_chdir
    lfs.lock_dir          = luatex_lfs_lock_dir
    lfs.dir               = luatex_lfs_dir
    lfs.link              = luatex_lfs_link
    lfs.mkdir             = luatex_lfs_mkdir
    lfs.mkdirp            = luatex_lfs_mkdirp
    lfs.rmdir             = luatex_lfs_rmdir
    lfs.symlinkattributes = luatex_lfs_symlinkattributes
    lfs.touch             = luatex_lfs_touch                


else

    -- we assume management elsewhere

end

if luadebugoption == 0  then

  for k,_  in pairs(package.loaded.debug) do 
   if not(k=='traceback') then package.loaded.debug[k] = nil; end 
  end

  --[==[ Not really necessary ]==]
  for k,v in pairs(debug) do 
   if not(k=='traceback') then debug[k] = nil; end 
  end

end


-- maybe also only when in kpse mode

if saferoption == 1 then

    local write_nl = texio.write_nl
    local format   = string.format

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

    os.execute = installdummy("os.execute")
    os.spawn   = installdummy("os.spawn")
    os.exec    = installdummy("os.exec")
    os.setenv  = installdummy("os.setenv")
    os.tempdir = installdummy("os.tempdir")

    os.kpsepopen = installdummy("os.kpsepopen")

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
    lfs.mkdirp  = installdummy("lfs.mkdirp")


    package.loaded.debug = nil
    debug = nil

    -- os.[execute|os.spawn|os.exec] already are shellescape aware)

end

-- maybe also only when in kpse mode

if saferoption == 1 or shellescape ~= 1 then

    package.loadlib      = function() end
    package.searchers[4] = nil
    package.searchers[3] = nil

    if os.setenv then
        os.setenv = function(...) end
    end
    ffi = require('ffi')

    if ffi then
        for k, v in next, ffi do
            if k ~= 'gc' then
                ffi[k] = nil
            end
        end
    end

    ffi = nil
    package.loaded.ffi = nil
    package.preload.ffi = error

end

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

    if not loaded.lfs then loaded.lfs = lfs end

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

-- start omit

if utilities and utilities.merger and utilities.merger.compact then

    local byte, format, gmatch, gsub = string.byte, string.format, string.gmatch, string.gsub
    local concat = table.concat

    local data = io.loaddata('luatex-core.lua')

    data = gsub(data,'%-%-%s*start%s*omit.-%-%-%s*stop%s*omit%s*','')
    data = gsub(data,'\r\n','\n')

    local t = { }
    local r = { }
    local n = 0
    local s = utilities.merger.compact(data) -- no comments and less spaces

    t[#t+1] = '/* generated from and by luatex-core.lua */'
    t[#t+1] = ''
    t[#t+1] = '#include "lua.h"'
    t[#t+1] = '#include "lauxlib.h"'
    t[#t+1] = ''
    t[#t+1] = 'int load_luatex_core_lua (lua_State * L);'
    t[#t+1] = ''
    t[#t+1] = 'int load_luatex_core_lua (lua_State * L)'
    t[#t+1] = '{'
    t[#t+1] = '  static unsigned char luatex_core_lua[] = {'
    for c in gmatch(data,'.') do
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

-- stop omit
