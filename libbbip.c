#include <stdio.h>
#include <stdlib.h>
#include "libbbip.h"
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <arpa/inet.h>

bbip_t *bbip_init(bbip_t *ps, FILE *fp, int *bbip_errno)
{
	if (fp == NULL) {
		*bbip_errno = 1;
		return NULL;
	}

	ps->fp = fp;

	fseek(ps->fp,1,SEEK_SET);
	if (fread(&ps->nCount,sizeof(int),1,ps->fp)!=1){
		fclose(ps->fp);
		*bbip_errno = 2;
		// php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't load sinaip library, read error");
		return NULL;
	}
	ps->nCount = htonl(ps->nCount);
	
	if (!ps->nCount)
	{
		fclose(ps->fp);
		// php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can't load sinaip library");
		*bbip_errno = 3;
		return NULL;
	}
	
	ps->index_start = (unsigned int *)calloc(ps->nCount+1, sizeof(int));
	ps->index_end = (unsigned int *)calloc(ps->nCount+1, sizeof(int));
	ps->index_ptr = (unsigned int *)calloc(ps->nCount+1, sizeof(int));
	
	fseek(ps->fp,0,SEEK_END);
	ps->size = ftell(ps->fp);
	ps->mmap = (char *)mmap(0, ps->size, PROT_READ, MAP_SHARED, fileno(ps->fp), 0);
	
	return ps;
}

long bbip_query(bbip_t *ps, unsigned int ip)
{
	int i_s = 1;
	int i_e = ps->nCount;
	int run = 0;
	while (i_s <= i_e)
	{
		int i_mid = (i_s+i_e)/2;
		
		if (ps->index_start[i_mid-1] == 0)
		{
			ps->ptr = BASE_PTR+(i_mid-2)*sizeof(int);
			ps->index_start[i_mid-1] = htonl(*(int *)(ps->mmap+ps->ptr));
			/*
			fseek(ps->fp,BASE_PTR+(i_mid-2)*sizeof(int),SEEK_SET);
			if (1 != fread(&ps->index_start[i_mid-1],sizeof(int),1,ps->fp)) return 0;
			ps->index_start[i_mid-1] = htonl(ps->index_start[i_mid-1]);*/
		}
		
		if (ip == ps->index_start[i_mid-1]){
			return i_mid-1;
		}else if (ps->index_start[i_mid-1] > ip)
		{
			i_e = i_mid-1;
		}else
		{
			i_s = i_mid + 1;
		}
		run++;
	}
	if (i_e > 1 && ps->index_start[i_e-1] == 0)
	{
		ps->ptr = BASE_PTR+(i_e-2)*sizeof(int);
		ps->index_start[i_e-1] = htonl(*(int *)(ps->mmap+ps->ptr));
		/*
		fseek(ps->fp,BASE_PTR+(i_e-2)*sizeof(int),SEEK_SET);
		if (1 != fread(&ps->index_start[i_e-1],sizeof(int),1,ps->fp)) return 0;
		ps->index_start[i_e-1] = htonl(ps->index_start[i_e-1]);*/
	}
	if (i_e > 1 && ps->index_end[i_e-1] == 0)
	{
		ps->ptr = BASE_PTR+(ps->nCount+i_e-2)*sizeof(int);
		ps->index_end[i_e-1] = htonl(*(int *)(ps->mmap+ps->ptr));
		/*
		fseek(ps->fp,BASE_PTR+(ps->nCount+i_e-2)*sizeof(int),SEEK_SET);
		if (1 != fread(&ps->index_end[i_e-1],sizeof(int),1,ps->fp)) return 0;
		ps->index_end[i_e-1] = htonl(ps->index_end[i_e-1]);*/
	}
	if (i_e >= 1 && ps->index_start[i_e] && ip>ps->index_start[i_e-1] && ip<=ps->index_end[i_e-1])
	{
		return i_e-1;
	}else
	{
		return 0;
	}
}

int bbip_preload(bbip_t *ps)
{
	fseek(ps->fp, BASE_PTR, SEEK_SET);
	
	if (fread((ps->index_start+1), sizeof(int), ps->nCount, ps->fp) != ps->nCount) return -1;
	if (fread((ps->index_end+1), sizeof(int), ps->nCount, ps->fp) != ps->nCount) return -1;
	if (fread((ps->index_ptr+1), sizeof(int), ps->nCount, ps->fp) != ps->nCount) return -1;
	
	int i;
	for (i=1;i<=ps->nCount;i++)
	{
		ps->index_start[i] = htonl(ps->index_start[i]);
		ps->index_end[i] = htonl(ps->index_end[i]);
		ps->index_ptr[i] = htonl(ps->index_ptr[i]);
	}
	return 0;
}


char *bbip_getstr(bbip_t *ps, unsigned char *len)
{
	*len = ps->mmap[ps->ptr++];
	if (!*len) return NULL;
	char *data;// = (char *)emalloc(*len+1);
	data = ps->mmap+ps->ptr;
	//memcpy(data,ps->mmap+ps->ptr,*len);
	ps->ptr+=*len;
	return data;
	/*
	if (fread(len,sizeof(char),1,ps->fp)!=1) return NULL;
	if (!*len) return NULL;
	char *data = (char *)emalloc(*len+1);
	if (fread(data, sizeof(char), *len, ps->fp)!=*len) return NULL;
	data[*len] = 0;
	return data;*/
}


/*******************************************************************************
*
* ipToBin - convert an ipv4 address to binary representation 
*		   and pads zeros to the beginning of the string if 
*		   the length is not 32 
*		   (Important for ranges like 10.10.0.1 - 20.20.20.20 )
*
* ip   - ipv4 address on host order
* pOut - Buffer to store binary.
*
* RETURNS: OK or ERROR 
*/
static int ipToBin(uint32_t value , char * pOut)
{
	char *ptr;

	ptr = pOut + IP_BINARY_LENGTH - 1;
	*ptr = '\0';

	do {
		*--ptr = 48+value % 2;
		value /= 2;
	} while (ptr > pOut && value);
	
	while (ptr > pOut)
	{
		*--ptr = 48;
	}
	return 0;
}

/*******************************************************************************
*
* rangeToCidrSize - convert an ip Range to CIDRSize for edns-client-subnet
*
* scanIP - client IP address
* from	 - IP Range start address
* to	   - IP Range end address
* RETURNS: OK or ERROR 
*/

int rangeToCidrSize(uint32_t scanIP, uint32_t from ,uint32_t to)
{
	int	 cidrStart = 0;
	int	 cidrEnd = MAX_CIDR_MASK - 1;
	long	newfrom;
	long	mask;
	char	fromIp[IP_BINARY_LENGTH];
	char	toIp[IP_BINARY_LENGTH];

	if (to-from>=4294967296) {
		return 0;
	}else if (to-from>=2147483648) {
		return 1;
	}else if (to-from>=1073741824) {
		return 2;
	}else if (to-from>=536870912) {
		return 3;
	}else if (to-from>=268435456) {
		return 4;
	}else if (to-from>=134217728) {
		return 5;
	}else if (to-from>=67108864) {
		return 6;
	}else if (to-from>=33554432) {
		return 7;
	}else if (to-from>=16777216) {
		return 8;
	}else if (to-from>=8388608) {
		return 9;
	}else if (to-from>=4194304) {
		return 10;
	}else if (to-from>=2097152) {
		return 11;
	}else if (to-from>=1048576) {
		return 12;
	}else if (to-from>=524288) {
		return 13;
	}else if (to-from>=262144) {
		return 14;
	}else if (to-from>=131072) {
		return 15;
	}else if (to-from>=65536) {
		return 16;
	}else if (to-from>=32768) {
		return 17;
	}else if (to-from>=16384) {
		return 18;
	}else if (to-from>=8192) {
		return 19;
	}else if (to-from>=4096) {
		return 20;
	}else if (to-from>=2048) {
		return 21;
	}else if (to-from>=1024) {
		return 22;
	}else if (to-from>=512) {
		return 23;
	}else if (to-from>=256) {
		return 24;
	}else if (to-from>=128) {
		return 25;
	}else if (to-from>=64) {
		return 26;
	}else if (to-from>=32) {
		return 27;
	}else if (to-from>=16) {
		return 28;
	}else if (to-from>=8) {
		return 29;
	}else if (to-from>=4) {
		return 30;
	}else if (to-from>=2) {
		return 31;
	}else {
		return 32;
	}
	memset (fromIp,0x0,sizeof(fromIp));
	memset (toIp,0x0,sizeof(toIp));

	if ( ipToBin(from,fromIp) != 0 ) 
		return -1;
	if ( ipToBin(to,toIp) != 0 )
		return -1;

	if(from < to )
	{

		/* Compare the from and to address ranges to get the first
		 * point of difference
		 */

		while(fromIp[cidrStart]==toIp[cidrStart])
			cidrStart ++;
		cidrStart = 32 - cidrStart -1 ;

		/* Starting from the found point of difference make all bits on the 
		 * right side zero 
		 */

		newfrom = from >> (cidrStart +1)  << (cidrStart +1) ;		

		/* Starting from the end iterate reverse direction to find 
		 * cidrEnd
		 */ 
		while( fromIp[cidrEnd] == '0' && toIp[cidrEnd] == '1')
			cidrEnd --;

		cidrEnd = MAX_CIDR_MASK - 1 - cidrEnd;

		if(cidrEnd <= cidrStart)
		{
			/* 
			 * Make all the bit-shifted bits equal to 1, for
			 * iteration # 1.
			 */
			
			int rc ;
			mask = pow (2, cidrStart ) - 1;
			rc = rangeToCidrSize (scanIP, from , newfrom | mask);
			if (rc != -1) return rc;
			rangeToCidrSize (scanIP, newfrom | 1 <<  cidrStart ,to);
			if (rc != -1) return rc;
		}
		else
		{
			int mask = (-1) << (32 - MAX_CIDR_MASK-cidrEnd);
			
			// Step 3. Find lowest IP address
			uint32_t lowest = newfrom & mask;
			 
			// Step 4. Find highest IP address
			uint32_t highest = newfrom + (~mask);
			
			if (scanIP >= lowest && scanIP <= highest)
			{
				return MAX_CIDR_MASK-cidrEnd;
			}
		}
	}
	else
	{
		int mask = (-1) << (32 - MAX_CIDR_MASK-cidrEnd);
		
		// Step 3. Find lowest IP address
		uint32_t lowest = from & mask;
		 
		// Step 4. Find highest IP address
		uint32_t highest = from + (~mask);
		
		if (scanIP >= lowest && scanIP <= highest)
		{
			return MAX_CIDR_MASK-cidrEnd;
		}
	}
	return -1;
}
