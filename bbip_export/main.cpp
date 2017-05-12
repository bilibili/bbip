#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <arpa/inet.h>

MYSQL *myData = NULL;
int IsMySQLOK = 0;

struct s_items
{
	unsigned int start;
	unsigned int end;
	unsigned int datalen;
	int ptr;
	char *data;
};

void db_close()
{
	if (myData!=NULL) mysql_close( myData ) ;
	myData=NULL;
}

int db_connect(const char *szHost,const char *szUser,const char *szPass,const char *database)
{
	db_close();
	fprintf(stdout,"connecting to database...");
	if ( (myData = mysql_init((MYSQL*) 0)) && 
       mysql_real_connect( myData, szHost, szUser, szPass, NULL, MYSQL_PORT, NULL, 0 ) )
    {
		fprintf(stdout,"\e[32mOK\e[0m\n");
		if (mysql_select_db(myData,database) > 0)
		{
			fprintf(stdout,"\e[31mselect database failure\e[0m\n");
			db_close();
		}
		return 0;
	}else
	{
		fprintf(stdout,"\e[31mFailure\e[0m\n");
		return -1;
	}
}
bool db_query(const char *szSQL)
{
	if (!mysql_query( myData, szSQL ))
	{
		IsMySQLOK = 1;
		return true;
	}else
	{
		IsMySQLOK = 0;
		fprintf(stdout,"Database: Query Error\nDatabase: %s\nQuery: %s\n",mysql_error(myData),szSQL);
		return false;
	}
}

void pushToList(s_items **d_l, int *size, int *cur_num, unsigned int start, unsigned int end, char *data, int d_size)
{
	if ((*cur_num)+1 > *size)
	{
		fprintf(stderr, "oversize, current: %d, new: %d\n", *size, (*size) * 2);
		s_items *newd_l = new s_items[(*size) * 2];
		memcpy(newd_l, *d_l, sizeof(s_items)*(*size));
		(*size)*=2;
		delete *d_l;
		*d_l = newd_l;
	}
	(*d_l)[*cur_num].start = start;
	(*d_l)[*cur_num].end = end;
	(*d_l)[*cur_num].data = data;
	(*d_l)[*cur_num].datalen= d_size;
	(*cur_num)++;
}

int compare (const void * a, const void * b)
{
	s_items *v1 = (s_items *)a;
	s_items *v2 = (s_items *)b;
	if (v1->start < v2->start) return -1;
	if (v1->start == v2->start) return 0;
	return 1;
}


int main(int argc, char **argv)
{
	if (argc < 5)
	{
		printf("Usage:\n  %s <db host> <user> <pass> <db> <file>\n\n", argv[0]);
		return -1;
	}
	if (db_connect(argv[1],argv[2],argv[3],argv[4]))
	{
		return -1;
	}
	char *ip_hash = (char *)malloc(536870912);
	memset(ip_hash, 0, 536870912);
	
	MYSQL_RES * res ;
	MYSQL_ROW row ;
	
	db_query("SET NAMES utf8");
	int dump_list_num = 0;
	s_items *dump_list;
	int dump_list_size = 0;
	int t_dup_block = 0;
	char h_sp_table[8] = {1,2,4,8,16,32,64,128};
	if (db_query("SELECT * FROM ip_geo_records ORDER BY end-start ASC"))
	{
		printf("storing results\n");
		res = mysql_store_result( myData ) ;
		printf("calcuating result numbers\n");
		int rowNum = (int) mysql_num_rows( res ) ;
		int num_fields = mysql_num_fields( res );
		
		dump_list = new s_items[rowNum*4];
		dump_list_size = rowNum*4;

		fprintf(stdout, "\033[32mTotal records: %d  fields: %d\033[0m\n", rowNum, num_fields);
		for (int i=0;i<rowNum;i++)
		{
			if (i%5000==0){
				fprintf(stdout,"processing %d   DUP: %d\r", i, t_dup_block);
				fflush(stdout);
			}
			if (row=mysql_fetch_row(res))
			{
				unsigned long *lengths;
				lengths = mysql_fetch_lengths(res);
				
				if (lengths == NULL) continue;
				
				int d_size = 9;
				int f_i;
				d_size += lengths[2]; // country
				d_size += lengths[3]; // province
				d_size += lengths[4]; // city
				d_size += lengths[5]; // district
				d_size += lengths[6]; // isp
				d_size += lengths[7]; // type
				d_size += lengths[8]; // desc
				d_size += lengths[10]; // lat
				d_size += lengths[11]; // lng
				
				char *d_data = (char *)malloc(d_size);
				char *last = d_data;
				*last = lengths[2];	last++; // country
				if (lengths[2]) last = (char *)memcpy(last, row[2], lengths[2])+lengths[2];	// country
				*last = lengths[3];	last++; // province
				if (lengths[3]) last = (char *)memcpy(last, row[3], lengths[3])+lengths[3];	// province
				*last = lengths[4];	last++; // city
				if (lengths[4]) last = (char *)memcpy(last, row[4], lengths[4])+lengths[4];	// city
				*last = lengths[5];	last++; // district
				if (lengths[5]) last = (char *)memcpy(last, row[5], lengths[5])+lengths[5];	// district
				*last = lengths[6];	last++; // isp
				if (lengths[6]) last = (char *)memcpy(last, row[6], lengths[6])+lengths[6];	// isp
				*last = lengths[7];	last++; // type
				if (lengths[7]) last = (char *)memcpy(last, row[7], lengths[7])+lengths[7];	// type
				*last = lengths[8];	last++; // desc
				if (lengths[8]) last = (char *)memcpy(last, row[8], lengths[8])+lengths[8];	// desc
				*last = lengths[10]; last++; // lat
				if (lengths[10]) last = (char *)memcpy(last, row[10], lengths[10])+lengths[10];	// lat
				*last = lengths[11]; last++; // lng
				if (lengths[11]) last = (char *)memcpy(last, row[11], lengths[11])+lengths[11];	// lng
				
				unsigned int start = atoi(row[0]);
				unsigned int end = atoi(row[1]);
				long c_s = -1;
				long c_e = end;
				int last_ip_item = 1;
				
				unsigned int ip_hash_p = start/8;
				unsigned int h_s_ptr = 7-start%8;
				unsigned char hash_p;
				
				for (unsigned int h_s = start;h_s<=end;h_s++)
				{
					hash_p = (ip_hash[ip_hash_p] >> h_s_ptr) & 1;
					
					if (last_ip_item != hash_p)
					{
						last_ip_item = hash_p;
						if (h_s != c_s)
						{
							if (hash_p == 1)	// from free to dup block
							{
								pushToList(&dump_list, &dump_list_size, &dump_list_num, c_s, h_s - 1, d_data, d_size);
								c_s = -1;
								t_dup_block++;
							}else	// from dup block to free
							{
								c_s = h_s;
							}
						}
					}
					ip_hash[ip_hash_p] |= h_sp_table[h_s_ptr];
					
					if (h_s_ptr-- == 0){
						//ip_hash_p++;
						if (last_ip_item == 0)
						{
							while (ip_hash[++ip_hash_p] == 0 && h_s+8<=end)
							{
								ip_hash[ip_hash_p] = 0xff;
								h_s+=8;
							}
						}else
						{
							ip_hash_p++;
						}
						h_s_ptr = 7;
					}
				}
				if (c_s != -1)
				{
					pushToList(&dump_list, &dump_list_size, &dump_list_num, c_s, end, d_data, d_size);
				}
			}
		}
		printf("free resulting...\n");
		mysql_free_result( res ) ;
	}
	unsigned int i;
	free(ip_hash);
	s_items tmp;
	qsort (dump_list, dump_list_num, sizeof(s_items), compare);
	char ver = 1;
	FILE *fp = fopen(argv[5], "wb");
	if (fp == NULL)
	{
		fprintf(stderr, "Open file for output error\n");
		return -2;
	}
	fwrite(&ver, sizeof(char), 1, fp);
	dump_list_num = htonl(dump_list_num);
	fwrite(&dump_list_num, sizeof(int), 1, fp);
	dump_list_num = htonl(dump_list_num);
	int ptr = dump_list_num * 12 + 5;
	fseek(fp, ptr,SEEK_SET);
	for (i = 0; i< dump_list_num;i++)
	{
		if (i%1000==0){
			fprintf(stdout,"exporting %d              \r");
			fflush(stdout);
		}
		int datalen = htonl(dump_list[i].datalen);
		fwrite(&datalen, sizeof(int), 1, fp);
		fwrite(dump_list[i].data, sizeof(char), dump_list[i].datalen, fp);
		
		dump_list[i].ptr = ptr;
		ptr+=dump_list[i].datalen + 4;
	}
	fseek(fp, 5, SEEK_SET);
	unsigned int wtmp = 0;
	for (i = 0; i< dump_list_num;i++)
	{
		wtmp = htonl(dump_list[i].start);
		fwrite(&wtmp, sizeof(int), 1, fp);
	}
	for (i = 0; i< dump_list_num;i++)
	{
		wtmp = htonl(dump_list[i].end);
		fwrite(&wtmp, sizeof(int), 1, fp);
	}
	for (i = 0; i< dump_list_num;i++)
	{
		wtmp = htonl(dump_list[i].ptr);
		fwrite(&wtmp, sizeof(int), 1, fp);
	}
	fclose(fp);
	printf("END td: %d   dup: %d int size: %d\n", dump_list_num, t_dup_block,  sizeof(int));
	return 0;
}
