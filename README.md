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

### Usage for C
```
#include <arpa/inet.h>
#include "bbip.h"

int main(int argc, const char **argv)
{
    bbip_t bbip;
    int bbip_errno;
    bbip_result_t *result;
    struct in_addr addr;
    FILE *fpData;

    if (argc < 2) {
        printf("usage:\n");
        printf("  bbip_demo <ip>\n");
        return 1;
    }
    fpData = fopen("bbip.dat", "rb");
    if (fpData == NULL) {
       fprintf(stderr, "Invalid data file\n");
       return 1;
    }
    if (bbip_init(&bbip, fpData, &bbip_errno) == NULL) {
       fprintf(stderr, "init bbip failed: %d\n", bbip_errno);
       return 1;
    }
    bbip_preload(&bbip);  // increase performance

    inet_aton(argv[1], &addr);
    result = bbip_search(&bbip, ntohl(addr.s_addr));
    if (result != NULL) {
        printf("country: %.*s\n", result->country_len, result->country);
        printf("province: %.*s\n", result->province_len, result->province);
        printf("city: %.*s\n", result->city_len, result->city);
        printf("isp: %.*s\n", result->isp_len, result->isp);
    }
}
```
gcc -o demo demo.c bbip.c
