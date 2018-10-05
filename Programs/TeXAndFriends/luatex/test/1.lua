local x = 3
local f = io.open("1.out", "w")
io.output(f)
if (x * 2 > 6) then
   io.write(x * 4)
else
   io.write(x + 4)
end
