# bbip

A library to resolve IPv4 address to geographic position. Support to Language: Thrift protocol, PHP, Lua.

### Directory Struct
- lua    - Library for Lua
- php    - Libarry for PHP
- thrift - Library for Thrift

### Build LUA Module
```
cd lua
make && make install
```

### Usage for LUA
```
require("bbip");
bip = bbip.new("BBIP.dat");
bip:preload()
for i = 1, 10000000 do
    -- return as multiple variables
    range_start, range_end, country, province, city, district, isp, type, desc, lat, lng, cidr = bip:query("223.5.5.5")
    print(range_start, range_end, country, province, city, district, isp, type, desc, lat, lng, cid)
    -- return as a Lua Table
    rc = bip:query_table("223.5.5.5")
end
```
