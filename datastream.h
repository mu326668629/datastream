#ifndef __DATASTREAM__
typedef struct _DataStream DataStream;

enum{
	SEEK_WRITE = 1,
	SEEK_READ = 2
};
enum{
	DATASTREAM_TYPE_A = 1,//read_offset>= write_offset
};
#endif