#ifndef _BBIP_TYPEDEF_H
#define _BBIP_TYPEDEF_H

#include <stdio.h>
#include <stdint.h>

#define BASE_PTR 5
#define IP_BINARY_LENGTH 32+1  /* 32 bits ipv4 address +1 for null */
#define IP_HEX_LENGTH	10   
#define MAX_CIDR_MASK	32

// typedef long unsigned int uint32_t;
typedef struct 
{
	FILE *fp;
	char *mmap;
	size_t ptr;
	size_t size;
	size_t nCount;
	unsigned int *index_start;
	unsigned int *index_end;
	unsigned int *index_ptr;
	int rsrc_id;
} bbip_t;

typedef struct
{
	unsigned long start;
	unsigned long end;
	unsigned char cidr_bit;
	char *country;
	unsigned char country_len;
	char *province;
	unsigned char province_len;
	char *city;
	unsigned char city_len;
	char *district;
	unsigned char district_len;
	char *isp;
	unsigned char isp_len;
	char *type;
	unsigned char type_len;
	char *desc;
	unsigned char desc_len;
	char *lat;
	unsigned char lat_len;
	char *lng;
	unsigned char lng_len;
} bbip_result_t;

bbip_t *bbip_init(bbip_t *ps, FILE *fp, int *bbip_errno);
long bbip_query(bbip_t *ps, unsigned int ip);
bbip_result_t *bbip_search(bbip_t *ps, unsigned int ip);
int bbip_preload(bbip_t *ps);
char *bbip_getstr(bbip_t *ps, unsigned char *len);
int rangeToCidrSize(uint32_t scanIP, uint32_t from ,uint32_t to);

#endif
