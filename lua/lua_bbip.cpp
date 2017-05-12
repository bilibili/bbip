#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "luajit.h"

#include "lualib.h"
#include "lauxlib.h"

#ifdef __cplusplus
}
#endif
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include "libbbip.h"

int lua_bbip_init(lua_State *L) {
	if (!lua_isstring(L, 1)) {
		luaL_error(L, "Invalid args");
		return 0;
	}
	const char *filename = lua_tostring(L, 1);
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		luaL_error(L, "Can't load bbip library");
		return 0;
	}
	int errno = 0;
	
	bbip_t *ps = (bbip_t *)lua_newuserdata(L, sizeof(bbip_t));
	bbip_t *rc = NULL;
	rc = bbip_init(ps, fp, &errno);
	if (rc == NULL) {
		switch (errno) {
			case 1:
				luaL_error(L, "Can't load bbip library");
				return 0;
			case 2:
				luaL_error(L, "Can't load bbip library, read error");
				return 0;
			case 3:
				luaL_error(L, "Can't load bbip library");
				return 0;
			default:
				luaL_error(L, "Unknow bpbp library error: %d", errno);
				return 0;
		}
	} else {
	    //关联metatable
	    luaL_getmetatable(L, "BBIP");
	    lua_setmetatable(L, -2);

		return 1;
	}
}

int lua_bbip_preload(lua_State *L) {
	bbip_t *ps = (bbip_t *)luaL_checkudata(L, 1, "BBIP");
	if (bbip_preload(ps) != 0) {
		luaL_error(L, "Can't load bbip library");
		return 0;
	}
	lua_pushnumber(L, ps->nCount);
	return 1;
}

int lua_bbip_count(lua_State *L) {
	bbip_t *ps = (bbip_t *)luaL_checkudata(L, 1, "BBIP");
	lua_pushnumber(L, ps->nCount);
	return 1;
}

double atof_length(char *line, size_t n)
{
    double   value;
    int      dot;
    int      neg = 0;
    int      point = 0;

    if (n == 0) {
        return 0;
    }

    dot = 0;

    for (value = 0; n--; line++) {

        if (*line == '.') {
            if (dot) {
                return 0;
            }

            dot = 1;
            continue;
        }

        if (*line == '-') {
        	if (neg) {
        		return 0;
        	}

        	neg = 1;

        	value = -value;
        	continue;
        }

        if (*line < '0' || *line > '9') {
            return 0;
        }

        value = value * 10 + (*line - '0') * (neg ? -1 : 1);
        point += dot;
    }

    while (point--) {
        value = value / 10;
    }

    return value;
}

int lua_bbip_query_all(lua_State *L, int type) {
	bbip_t *ps = (bbip_t *)luaL_checkudata(L, 1, "BBIP");
	int len;
	unsigned long ip;
	const char *ip_addr;
	switch (lua_type(L, 2)) {
		case LUA_TSTRING:
			ip_addr = lua_tostring(L, 2);
			struct in_addr addr;
			if (!inet_aton(ip_addr, &addr))
			{
				luaL_error(L, "Argument 1 must be a IP. Failed to resolve IP.");
				return 0;
			}
			ip = ntohl(addr.s_addr);
			break;
		case LUA_TNUMBER:
			ip = lua_tonumber(L, 2);
			break;
		default:
			luaL_error(L, "Argument 1 must be a IP (Integer / String). Failed to resolve IP.");
			return 0;
	}

	long key = bbip_query(ps,ip);
	if (key == 0)
	{
		lua_pushnil(L);
		return 1;
	}else
	{
		if (ps->index_ptr[key] == 0)
		{
			ps->ptr = BASE_PTR+(ps->nCount*2+key-1)*sizeof(int);
			ps->index_ptr[key] = htonl(*(int *)(ps->mmap+ps->ptr));
			//fseek(ps->fp,BASE_PTR+(ps->nCount*2+key-1)*sizeof(int),SEEK_SET);
			//if (1 != fread(&ps->index_ptr[key],sizeof(int),1,ps->fp)) RETVAL_FALSE;
			//ps->index_ptr[key] = htonl(ps->index_ptr[key]);
		}
		ps->ptr = ps->index_ptr[key]+4;

		int bit = rangeToCidrSize(ip, ps->index_start[key],ps->index_end[key]); 
	
		unsigned char ilen;
		char *data;

		if (type == 0) {
			// by multiple result
			// start end  country province  city district isp type desc lat lng cidr
			// range_start, range_end, country, province, city, district, isp, type, desc, lat, lng, cidr
		    lua_pushnumber(L, ps->index_start[key]);
		    lua_pushnumber(L, ps->index_end[key]);

		    //country
		    data=bbip_getstr(ps,&ilen);
	    	lua_pushlstring(L, data, ilen);
			
			//province
		    data=bbip_getstr(ps,&ilen);
		    lua_pushlstring(L, data, ilen);

		    //city
		    data=bbip_getstr(ps,&ilen);
		    lua_pushlstring(L, data, ilen);
		    
		    //district
		    data=bbip_getstr(ps,&ilen);
		    lua_pushlstring(L, data, ilen);
		    
		    //isp
		    data=bbip_getstr(ps,&ilen);
		    lua_pushlstring(L, data, ilen);
		    
		    //type
		    data=bbip_getstr(ps,&ilen);
		    lua_pushlstring(L, data, ilen);
		    
		    //desc
		    data=bbip_getstr(ps,&ilen);
		    lua_pushlstring(L, data, ilen);
		    
		    //lat
		    data=bbip_getstr(ps,&ilen);
		    if (ilen > 0) {
		    	double d = atof_length(data, ilen);
		    	lua_pushnumber(L, d);
		    } else {
		    	lua_pushnil(L);
		    }
		    // lua_pushlstring(L, data, ilen);
		    
		    //lng
		    data=bbip_getstr(ps,&ilen);
		    if (ilen > 0) {
		    	double d = atof_length(data, ilen);
		    	lua_pushnumber(L, d);
		    } else {
		    	lua_pushnil(L);
		    }
		    // lua_pushlstring(L, data, ilen);
		    
		    //cidr
		    lua_pushnumber(L, bit);
		    
			return 12;
		} else {
			// by table
			lua_newtable(L);

		    lua_pushstring(L, "start");
		    lua_pushnumber(L, ps->index_start[key]);
		    lua_settable(L, -3);

		    lua_pushstring(L, "end");
		    lua_pushnumber(L, ps->index_end[key]);
		    lua_settable(L, -3);

		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "country");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "province");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "city");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "district");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "isp");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "type");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "desc");
		    lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "lat");
		    //lat
		    if (ilen > 0) {
		    	double d = atof_length(data, ilen);
		    	lua_pushnumber(L, d);
		    } else {
		    	lua_pushnil(L);
		    }
		    // lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    data=bbip_getstr(ps,&ilen);
		    lua_pushstring(L, "lng");
		    if (ilen > 0) {
		    	double d = atof_length(data, ilen);
		    	lua_pushnumber(L, d);
		    } else {
		    	lua_pushnil(L);
		    }
		    // lua_pushlstring(L, data, ilen);
		    lua_settable(L, -3);
		    
		    lua_pushstring(L, "cidr");
		    lua_pushnumber(L, bit);
		    lua_settable(L, -3);
		    
		    return 1;
		}
		return 1;
	}
}

int lua_bbip_query(lua_State *L) {
	return lua_bbip_query_all(L, 0);
}

int lua_bbip_query_table(lua_State *L) {
	return lua_bbip_query_all(L, 1);
}

static const struct luaL_reg lua_bbiplib[] = {
	{"new", lua_bbip_init},
	{NULL, NULL}
};

extern "C"
int luaopen_bbip (lua_State *L) {
    //建立metatable

    luaL_newmetatable(L, "BBIP");
 
    //查找索引，把它指向metatable自身（因为稍后我们会在metatable里加入一些成员）
    lua_pushvalue(L, -1);
    lua_setfield(L,-2, "__index");
 
    // preload方法
    lua_pushcfunction(L, lua_bbip_preload);
    lua_setfield(L, -2, "preload");
 	
    // count方法
    lua_pushcfunction(L, lua_bbip_count);
    lua_setfield(L, -2, "count");
 	
    // query方法
    lua_pushcfunction(L, lua_bbip_query);
    lua_setfield(L, -2, "query");
 	
    // query_table方法
    lua_pushcfunction(L, lua_bbip_query_table);
    lua_setfield(L, -2, "query_table");
 	
	luaL_register(L, "bbip", lua_bbiplib);
	return 1;
}
