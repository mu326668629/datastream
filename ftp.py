#coding=utf-8
import DataStream as _DataStream
from ftplib import FTP
class DataStream():#对C对象重新封装下，方便使用
    def __init__(self,sz = 256):
        self.obj = _DataStream.new(sz)
        self.sz = sz
    def write(self,buf,length = -1):
        if length == -1: length = len(buf)
        return _DataStream.write(self.obj,buf,length)
    def read(self,sz):
        s =  _DataStream.read(self.obj,sz)
       # print "read:%d => %d"%(sz,len(s))
        return s
    def seek(self,offset,mode):
        return _DataStream.seek(self.obj,offset,mode)
    def compress(self):
        return _DataStream.compress(self.obj)
    def uncompress(self):
        return _DataStream.uncompress(self.obj)
    def encrypt(self,password):
        return _DataStream.encrypt(self.obj,password)
    def decrypt(self,password):
        return _DataStream.decrypt(self.obj,password)
    def length(self):
        return _DataStream.length(self.obj)
    def delete(self):
        _DataStream.delete(self.obj)
    def reset(self):
        _DataStream.delete(self.obj)
        self.obj = _DataStream.new(self.sz)
    #def __del__(self):#最后调用析构函数，这个竟然被删掉了
        #_DataStream.delete(self.obj)
        #print "free:%x"%self.obj
class MyFtp():
    def __init__(self,host,port = '21'):
        self.BUF_SIZE = 1024
        self.ftp  = FTP()
        self.host = host
        self.port =port
        self.stream = DataStream()
    def login(self,username,password):
        self.ftp.connect(self.host,self.port)
        self.ftp.login(username,password)
        self.ftp.cwd("data")
    def down(self,filename,password='abcd'):
        self.stream.reset()
        self.ftp.retrbinary("RETR %s"%filename,self.stream.write,self.BUF_SIZE) 
        self.stream.decrypt(password)
        return self.stream.uncompress()
    def up(self,filename,password='abcd'):
        self.stream.compress()
        self.stream.encrypt(password)
        #self.stream.seek(0,3)#读写位置置0，不过加密后自动置0了
        self.ftp.storbinary("STOR %s"%filename,self.stream,self.BUF_SIZE)
    def quit(self):
        self.ftp.quit()
        print 'ftp quit';
def up(ftp,filename):
    ftp.up(filename)
if __name__ == "__main__":
    ftp = MyFtp("hsot")
    ftp.login("name","password")
    ftp.stream.write("aaabbbcccdddeeefff111222333444555")
    ftp.up("test.dat")
    ftp.down("test.dat")
    print "lenght:",ftp.stream.length()
    print ftp.stream.read(1024)
    
   