/************************
管理一段数据流，可以实现加密解密
压缩解压缩
version1.0 功能基本实现，压缩解压缩也实现了。
version1.1加密还是有问题，引起压缩也出问题，加密流程应该没问题才是
version1.2内存分配出问题，导致越界死机，已经修改
bug: read返回空白字符，有空白字符怎么办
*************************/
#pragma comment(lib,"zlib\\zdll.lib")
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <python.h>
#include "datastream.h"
#include "zlib/zconf.h"
#include "zlib/zlib.h"
#define CACHE_SIZE 4

typedef struct _StreamCache StreamCache;
struct _StreamCache{
StreamCache *next;
unsigned short sz;
char buffer[1];
};
struct _DataStream{
int type;
unsigned short cache_size;
unsigned long read_offset;
unsigned long write_offset;
StreamCache *head,*tail;
};
/****加密解密****/
#define Encryption_Decryption(a,b)  ( (~((a)&(b))&(b))|((a)&~(b)) )
#define BLOCK_SIZE 16
static unsigned char Magic_num[]={// 256个，随机排序 每个数据都是唯一
	  0x23, 0xbe, 0x84, 0xe1, 0x6c, 0xd6, 0xae, 0x52, 0x90, 0x49, 0xf1, 0xf1, 0xbb, 0xe9, 0xeb, 0xb3,    
		0xa6, 0xdb, 0x3c, 0x87, 0x0c, 0x3e, 0x99, 0x24, 0x5e, 0x0d, 0x1c, 0x06, 0xb7, 0x47, 0xde, 0xb3,
		0x12, 0x4d, 0xc8, 0x43, 0xbb, 0x8b, 0xa6, 0x1f, 0x03, 0x5a, 0x7d, 0x09, 0x38, 0x25, 0x1f, 0x5d,
		0xd4, 0xcb, 0xfc, 0x96, 0xf5, 0x45, 0x3b, 0x13, 0x0d, 0x89, 0x0a, 0x1c, 0xdb, 0xae, 0x32, 0x20,
		0x9a, 0x50, 0xee, 0x40, 0x78, 0x36, 0xfd, 0x12, 0x49, 0x32, 0xf6, 0x9e, 0x7d, 0x49, 0xdc, 0xad,
		0x4f, 0x14, 0xf2, 0x44, 0x40, 0x66, 0xd0, 0x6b, 0xc4, 0x30, 0xb7, 0x32, 0x3b, 0xa1, 0x22, 0xf6,
		0x22, 0x91, 0x9d, 0xe1, 0x8b, 0x1f, 0xda, 0xb0, 0xca, 0x99, 0x02, 0xb9, 0x72, 0x9d, 0x49, 0x2c,
		0x80, 0x7e, 0xc5, 0x99, 0xd5, 0xe9, 0x80, 0xb2, 0xea, 0xc9, 0xcc, 0x53, 0xbf, 0x67, 0xd6, 0xbf,
		0x14, 0xd6, 0x7e, 0x2d, 0xdc, 0x8e, 0x66, 0x83, 0xef, 0x57, 0x49, 0x61, 0xff, 0x69, 0x8f, 0x61,
		0xcd, 0xd1, 0x1e, 0x9d, 0x9c, 0x16, 0x72, 0x72, 0xe6, 0x1d, 0xf0, 0x84, 0x4f, 0x4a, 0x77, 0x02,
		0xd7, 0xe8, 0x39, 0x2c, 0x53, 0xcb, 0xc9, 0x12, 0x1e, 0x33, 0x74, 0x9e, 0x0c, 0xf4, 0xd5, 0xd4,
		0x9f, 0xd4, 0xa4, 0x59, 0x7e, 0x35, 0xcf, 0x32, 0x22, 0xf4, 0xcc, 0xcf, 0xd3, 0x90, 0x2d, 0x48,
		0xd3, 0x8f, 0x75, 0xe6, 0xd9, 0x1d, 0x2a, 0xe5, 0xc0, 0xf7, 0x2b, 0x78, 0x81, 0x87, 0x44, 0x0e,
		0x5f, 0x50, 0x00, 0xd4, 0x61, 0x8d, 0xbe, 0x7b, 0x05, 0x15, 0x07, 0x3b, 0x33, 0x82, 0x1f, 0x18,
		0x70, 0x92, 0xda, 0x64, 0x54, 0xce, 0xb1, 0x85, 0x3e, 0x69, 0x15, 0xf8, 0x46, 0x6a, 0x04, 0x96,
    0x73, 0x0e, 0xd9, 0x16, 0x2f, 0x67, 0x68, 0xd4, 0xf7, 0x4a, 0x4a, 0xd0, 0x57, 0x68, 0x76, 0x29
};
/******************************end**********************************************/
StreamCache *StreamCache_add(DataStream*me,StreamCache*prev)
{
	StreamCache *cache;
	if(me == NULL)return NULL;
	cache = (StreamCache*)malloc(sizeof(StreamCache)+me->cache_size);
	if(!cache){
		int sz = sizeof(StreamCache)+me->cache_size;
		cache = (StreamCache*)malloc(256);
	}
	if(cache != NULL){
		memset(cache,0,sizeof(StreamCache));
		if(prev != NULL && prev != me->tail){
			cache->next = prev->next;
			prev->next = cache;
		}
		else{
			if(me->tail == NULL)me->head = me->tail = cache;
			else {
				me->tail->next =cache;
				me->tail = cache;
			}
			cache->next =NULL;
		}
	}
	return cache;
}
DataStream *DataStream_new(int size)
{
	DataStream *me;
	if(size < CACHE_SIZE)size=CACHE_SIZE;
	else size = (size+CACHE_SIZE-1)/CACHE_SIZE*CACHE_SIZE;
	me = (DataStream*)malloc(sizeof(DataStream));
	if(me){
		me->cache_size = size;
		me->read_offset = me->write_offset = 0;
		me->head = me->tail = NULL;
	}
	return me;
}
void DataStream_empty(DataStream*me)
{
	if(me){
		StreamCache*cache= me->head;
		while(cache){
			cache = cache->next;
			free(me->head);
			me->head = cache;
		}
		me->head = me->tail = NULL;
		me->write_offset = me->read_offset = 0;
	}
}
void DataStream_delete(DataStream*me)
{
	StreamCache *cache;
	if(me){
		cache = me->head;
		while(cache){
			StreamCache *temp = cache->next;
			free(cache);
			cache = temp;
		}
		free(me);
	}
}
int DataStream_read(DataStream*me,char *buffer,int len)
{
	StreamCache *cache;
	int nDone = 0;
	unsigned long offset = 0;//整个数据偏移量
	if(len <= 0)return 0;
	if(!me||!me->head||!buffer)return -1;
	for(cache = me->head;cache;offset+=cache->sz,cache=cache->next){
		unsigned long shift = me->read_offset - offset;//当前cache偏移量
		if(shift < cache->sz){
			int nbytes = cache->sz - shift;
			if(nbytes+nDone > len)nbytes = len - nDone;
			memcpy(buffer+nDone,cache->buffer+shift,nbytes);
			nDone += nbytes;
			me->read_offset += nbytes;
			if(nDone>=len)break;
		}
	}
	return nDone;
}

int DataStream_write(DataStream *me, char *buffer,int len)
{
	StreamCache *cache;
	int nDone = 0;
	unsigned long offset = 0;
	if(len <= 0)return 0;
	if(!me||!buffer)return -1;
	for(cache = me->head;;offset+=cache->sz,cache=cache->next){
		int shift = me->write_offset - offset;
		if(NULL == cache) {
			cache = StreamCache_add(me,NULL);
			//cache->sz = me->cache_size;
		}
		if(NULL == cache)break;
		if(shift < me->cache_size){
			int nbytes = me->cache_size - shift;
			if(nbytes+nDone > len)nbytes = len - nDone;
		 if(me->write_offset+nbytes > me->read_offset ){//对同一个datastream覆盖写，要求不可覆盖未读数据
			//	cache = StreamCache_add(me,cache);
			//	me->read_offset += me->cache_size;
			//	if(cache == NULL)break;
			// printf("Error..\n");
			}
			memcpy(cache->buffer+shift,buffer+nDone,nbytes);
			nDone += nbytes;
			me->write_offset +=nbytes;
			cache->sz = shift + nbytes;
			if(nDone>=len)break;
		}
		else
			cache->sz = me->cache_size;
	}
	return nDone;
}
int DataStream_collect(DataStream*me)
{//有时候覆盖的写，后面的数据不要了，就把它们回收掉，返回回收的大小
	StreamCache *cache;
	unsigned long offset;
	int sz = 0;
	for(cache = me->head,offset = 0;cache;offset+=cache->sz,cache=cache->next){
		int shift = me->write_offset - offset;
		if(shift < me->cache_size){
			sz = cache->sz - shift;
			cache->sz = shift;
			cache = cache->next;
			break;
		}
	}
	while(cache){
		StreamCache *temp = cache->next;
		sz += cache->sz;
		free(cache);
		cache = temp;
	}
	return sz;
}
int DataStream_merge(DataStream*a,DataStream*b)
{//把b merge到a并删除b,且删除a原来的内容
	if(a&&b){
		DataStream_empty(a);
		*a = *b;
		a->read_offset = 0;
		a->write_offset = 0;
		free(b);
	}
	return 0;
}
unsigned long DataStream_length(DataStream*me)
{
	unsigned long len = 0;
	StreamCache *cache;
	for(cache = me->head;cache;cache = cache->next)len+=cache->sz;
	return len;
}
unsigned long DataStream_seek(DataStream *me,unsigned long offset, int mode)
{
	unsigned long old=0;
	switch(mode)
	{
	case SEEK_READ:
		old = me->read_offset;
		me->read_offset = offset;
		break;
	case SEEK_WRITE:
		old = me->write_offset;
		me->write_offset = offset;
		break;
	case SEEK_READ|SEEK_WRITE:
		old = me->write_offset;
		me->write_offset = offset;
		me->read_offset = offset;
		break;
	}
	return old;
}

int DataStream_compress(DataStream *me)
{
	int ret;
	z_stream strm = {0};	
	int buf_size = 4096*10;
	char *buf = NULL;
	StreamCache *cache;
	DataStream *temp = NULL;
	do{
		buf = (char*)malloc(buf_size);
		if(buf == NULL)buf_size>>=1;
		else break;
	}while(buf_size>256);
	if(buf == NULL)return -1;
	temp = DataStream_new(me->cache_size);
	if(temp == NULL)goto ERROR;
	ret = deflateInit(&strm,Z_DEFAULT_COMPRESSION);
	if(ret != Z_OK)goto ERROR;
	strm.next_out = buf;
	strm.avail_out = buf_size;
	for(cache=me->head;cache&&cache->sz>0;cache=cache->next){	
		strm.avail_in = cache->sz;
		strm.next_in = cache->buffer;
		do{
			ret = deflate(&strm,cache->next?Z_NO_FLUSH:Z_FINISH);
			if(strm.total_out > 0){
				if(DataStream_write(temp,buf,strm.total_out) != strm.total_out)goto ERROR;
				strm.next_out = buf;
				strm.avail_out = buf_size;
				strm.total_out = 0;
			}
		}while(ret == Z_BUF_ERROR);
		if(ret != Z_OK && ret != Z_STREAM_END)goto ERROR;
	}
	deflateEnd(&strm);
	DataStream_merge(me,temp);	
	free(buf);
	return 0;
ERROR:
	deflateEnd(&strm);
	if(buf)free(buf);
	if(temp)DataStream_delete(temp);
	return -1;
}
int DataStream_uncompress(DataStream *me)
{
	int ret;
	z_stream strm = {0};	
	int buf_size = 4096;
	char *buf = NULL;
	StreamCache *cache;
	DataStream *temp = NULL;
	do{
		buf = (char*)malloc(buf_size);
		if(buf == NULL)buf_size>>=1;
		else break;
	}while(buf_size>256);
	if(buf == NULL)return -1;
	temp = DataStream_new(me->cache_size);
	if(NULL == temp)goto ERROR;
	ret = inflateInit(&strm);
	strm.next_out = buf;
	strm.avail_out = buf_size;
	for(cache=me->head;cache&&cache->sz >0;cache=cache->next){
		strm.avail_in = cache->sz;
		strm.next_in =cache->buffer;
		do{
			ret = inflate(&strm,cache->next?Z_NO_FLUSH:Z_FINISH);
			if(strm.total_out > 0){
				DataStream_write(temp,buf,strm.total_out);
				strm.next_out = buf;
				strm.avail_out = buf_size;
				strm.total_out = 0;
			}
		
		}while(ret == Z_BUF_ERROR && strm.avail_in>0);
		if(ret != Z_OK && ret != Z_STREAM_END)goto ERROR;
	}
	inflateEnd(&strm);
	free(buf);
	DataStream_merge(me,temp);	
	return 0;
ERROR:
	deflateEnd(&strm);
	if(buf)free(buf);
	if(temp)DataStream_delete(temp);
	return -1;
}
int DataStream_encrypt(DataStream *me,const char *password)
{
	DataStream *temp;
	char *buf;
	int sz;
	unsigned int crc = 0;
	int crc_sz = sizeof(unsigned int);
	unsigned int ps=0;
	if(!me|| !me->head || !password || !password[0])return -1;
    temp = DataStream_new(me->cache_size);
    buf = (char*)malloc(BLOCK_SIZE);
	if(buf == NULL || temp == NULL)goto ERROR;
	DataStream_seek(me,0,SEEK_READ);
	while((sz = DataStream_read(me,buf,BLOCK_SIZE-crc_sz)) > 0){
		int i = 0;
		crc = crc32(crc,buf,sz);
		for(i = 0; i < sz; ++i){   
			if(ps>=strlen(password))ps = 0;
			buf[i] = Encryption_Decryption(buf[i],Magic_num[password[ps++]]);
		}
		memcpy(buf+sz,&crc,crc_sz);
		DataStream_write(temp,buf,sz+crc_sz);
	}
	DataStream_merge(me,temp);
	free(buf);
	return 0;
ERROR:
	if(temp)DataStream_delete(temp);
	if(buf)free(buf);
	return -1;
}
int DataStream_decrypt(DataStream *me,const char *password)
{
	DataStream *temp;
	char *buf;
	int sz;
	unsigned int crc=0;
	int crc_sz = sizeof(unsigned int);
	unsigned int ps=0;
	if(!me|| !me->head || !password || !password[0])return 0;
    temp = DataStream_new(me->cache_size);
    buf = (char*)malloc(BLOCK_SIZE);
	if(buf == NULL || temp == NULL)goto ERROR;
	DataStream_seek(me,0,SEEK_READ);
	while((sz = DataStream_read(me,buf,BLOCK_SIZE)) > 0){
		unsigned int crc1 = 0;
		int i = 0;
		if(sz <= crc_sz)goto ERROR;
		memcpy(&crc1,buf+(sz-crc_sz),crc_sz);
		for(i = 0; i < sz-crc_sz; ++i){   
			if(ps>=strlen(password))ps = 0;
			buf[i] = Encryption_Decryption(buf[i],Magic_num[password[ps++]]);
		}
		crc = crc32(crc,buf,sz-crc_sz);
		if(crc != crc1)goto ERROR;
		DataStream_write(temp,buf,sz-crc_sz);
	}
	DataStream_merge(me,temp);
	free(buf);
	return 0;
ERROR:
	if(temp)DataStream_delete(temp);
	if(buf)free(buf);
	return -1;
}


void main()
{
	char *s = "aaabbbcccdddeeefff111222333444555";
	char ptr_arr[1024]={0};
	char *ptr = ptr_arr;
	int sz;
	DataStream*me = DataStream_new(256);
	DataStream_write(me,s,strlen(s));
	printf("length:%d\n",strlen(s));
		sz = DataStream_compress(me);
	DataStream_encrypt(me,"abc");

	printf("ret:%d\n",sz);
	sz = DataStream_read(me,ptr,1024);
	printf("sz:%d\n",sz);
}
PyObject *Extest_write(PyObject *self,PyObject*args)
{
	DataStream*me;
	char *buf = NULL;
	int sz = 0;
	int res ;
	res = PyArg_ParseTuple(args,"isi",&me,&buf,&sz);
	if(!res)(PyObject*)Py_BuildValue("i",-2);
	res = DataStream_write(me,buf,sz);
	return (PyObject*)Py_BuildValue("i",res);	
}
PyObject *Extest_read(PyObject *self,PyObject*args)
{
	DataStream*me;
	int ret = -1;
	char *buf = NULL;
	int sz = 0;
	int len =0;
	PyObject *out;
  ret = PyArg_ParseTuple(args,"ii",&me,&sz);
	if(!ret)goto END;
	if(sz<=0)goto END;
	len = DataStream_length(me);
	if(sz > len) sz = len;
	buf = (char*)calloc(1,sz+1);
	if(buf == NULL)goto END;
	ret = DataStream_read(me,buf,sz);
END:
	if(ret > 0)
	  out =  (PyObject*)Py_BuildValue("s",buf);
	else
		out =  (PyObject*)Py_BuildValue("s","");
	if(buf)free(buf);
	return out;
}
PyObject *Extest_compress(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	res = PyArg_ParseTuple(args,"i",&me);
	if(!res)return(PyObject*)Py_BuildValue("i",-1);
	res = DataStream_compress(me);
	return (PyObject*)Py_BuildValue("i",res);	
}
PyObject *Extest_uncompress(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	res = PyArg_ParseTuple(args,"i",&me);
	if(!res)return (PyObject*)Py_BuildValue("i",-1);
	res = DataStream_uncompress(me);
	return (PyObject*)Py_BuildValue("i",res);	
}
PyObject *Extest_encrypt(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	char *password = NULL;
	res = PyArg_ParseTuple(args,"is",&me,&password);
	if(!res)return (PyObject*)Py_BuildValue("i",-1);		
	res = DataStream_encrypt(me,password);
	return (PyObject*)Py_BuildValue("i",res);		
}
PyObject *Extest_decrypt(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	char *password =NULL;
	res = PyArg_ParseTuple(args,"is",&me,&password);
	if(!res)return (PyObject*)Py_BuildValue("i",-1);	
	res = DataStream_decrypt(me,password);
	return (PyObject*)Py_BuildValue("i",res);	
}
PyObject *Extest_new(PyObject *self,PyObject*args)
{
	int sz = 0;
	void *ds;
	PyObject *out = NULL;
	int res = PyArg_ParseTuple(args,"i",&sz);
	if(!res)return (PyObject*)Py_BuildValue("i",0);	
	ds = DataStream_new(sz);
	if(!ds)return NULL;
	return (PyObject*)Py_BuildValue("i",ds);
}
PyObject *Extest_seek(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	int offset;
	int mode;
	res = PyArg_ParseTuple(args,"iii",&me,&offset,&mode);
	if(!res)return (PyObject*)Py_BuildValue("i",-1);
	res = DataStream_seek(me,offset,mode);
	return  (PyObject*)Py_BuildValue("i",res);
}
PyObject *Extest_delete(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	res = PyArg_ParseTuple(args,"i",&me);
	if(!res)return (PyObject*)Py_BuildValue("i",0);	
	DataStream_delete(me);	
	return (PyObject*)Py_BuildValue("i",1);
}
PyObject *Extest_length(PyObject *self,PyObject*args)
{
	DataStream*me;
	int res ;
	res = PyArg_ParseTuple(args,"i",&me);
	if(!res)return (PyObject*)Py_BuildValue("i",0);	
	res = DataStream_length(me);
	return  (PyObject*)Py_BuildValue("i",res);
}
static PyMethodDef DataStreamMethods[]={
	{"delete",Extest_delete,METH_VARARGS},
	{"new",Extest_new,METH_VARARGS},
	{"write",Extest_write,METH_VARARGS},
	{"read",Extest_read,METH_VARARGS},
	{"compress",Extest_compress,METH_VARARGS},
	{"uncompress",Extest_uncompress,METH_VARARGS},
	{"encrypt",Extest_encrypt,METH_VARARGS},
	{"decrypt",Extest_decrypt,METH_VARARGS},
	{"seek",Extest_seek,METH_VARARGS},
	{"length",Extest_length,METH_VARARGS},
	{NULL,NULL}
};

void initDataStream()
{
	Py_InitModule("DataStream",DataStreamMethods);
}