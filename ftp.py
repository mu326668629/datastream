#coding=utf-8
import DataStream
obj  =DataStream.new(4)
s = "1234567890"
sz = len(s);
DataStream.write(obj,s,sz)
DataStream.write(obj,s,sz)
ret = DataStream.encrypt(obj,"abde12345")

ret = DataStream.decrypt(obj,"abde12345")
ret = DataStream.compress(obj)
ret = DataStream.uncompress(obj)
print ret
ret = DataStream.read(obj,1024)
print "read",ret
ret = DataStream.read(obj,1024)
print "read2",ret
