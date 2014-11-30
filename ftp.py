#coding=utf-8
"""
URL�Ѿ������ˣ���Ҫ��Կ���ܽ��ܳ�URL
ͨ��URL���Ի�ȡuserinfo
userinfo+password ���Ե�½�������ϴ������ļ���
�ϴ��ļ�: ѹ�������ܣ��ϴ�
�����ļ�: ���أ����ܣ���ѹ
#�м��д��󷵻ؿհ�.
#���ڿ��ַ�'\0', ���ۣ���������֮�󲻻���ָ��ַ�������֮ǰ�Ƿ���
"""
import DataStream as _DataStream
import urllib2
from ftplib import FTP
URL="-\xee1\xea\x7f\xb5j\xea<\xee-\xf5\xcf\xf8G\x03+\xb4p\xaer\xaaq\xaa#\xadp\xa9?\x1e\xa9\xfc \xf8p\xb4!\xaat\xb4+\xfb+\xf5W\xe0MA<\xef+\xb4&\xf5(:\xb7\x961"
class DataStream():#��C�������·�װ�£�����ʹ��
    def __init__(self,sz = 256):
        self.obj = _DataStream.new(sz)
        self.sz = sz
    def write(self,buf,length = -1):
        if length == -1: length = len(buf)
        return _DataStream.write(self.obj,buf,length)
    def read(self,sz):
        s =  _DataStream.read(self.obj,sz)
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
    def toFile(self,filename):
        f = open(filename,"wb")
        f.write(self.read(self.length()))
        f.close()
    def loadFile(self,filename):
        f = open(filename,"rb")
        while True:
            bf = f.read(self.sz)
            if not bf:break
            self.write(bf)
        f.close()
    def __del__(self):
        if _DataStream: _DataStream.delete(self.obj)
        print "free:%x"%self.obj
class MyFtp():
    def __init__(self,host='',port = '21'):
        self.BUF_SIZE = 1024
        self.host = host
        self.port =port
        self.ftp  = FTP()
        self.stream = DataStream()
    def getInfo(self,password):#������ӿڻ�ȡ�����ݲ������µ�
        stream = DataStream(256)
        stream.write(URL)
        if stream.decrypt(password) != 0:return False
        url = stream.read(1024)
        stream.reset()
        stream.write(urllib2.urlopen(url+"/user/user.ini").read())
        if stream.decrypt(password) == -1:return False
        if stream.uncompress() == -1:return False
        return eval(stream.read(1024))
    def login(self,username='',password=''):
        #�ӷ�������ȡ��Ϣ
        if not self.host:
            if not password: password = raw_input("pelase input passwrod:")#û�������������������
            info = self.getInfo(password)
            if not info: return False
            self.host = info['host']
            username = info['username']
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
        #self.stream.seek(0,3)#��дλ����0���������ܺ��Զ���0��
        self.ftp.storbinary("STOR %s"%filename,self.stream,self.BUF_SIZE)
    def quit(self):
        self.stream.delete()
        self.ftp.quit()
        print 'ftp quit';
def Upload_File(filename,password='abcd'):#�ϴ��ļ�����ftp
    ftp = MyFtp()
    ftp.login('',password)
    ftp.stream.loadFile(filename);
    ftp.up(filename,password);
def Down_File(filename,password='abcd'):#����ftp �����ļ�.
    stream = DataStream(256)
    stream.write(URL)
    stream.decrypt(password)
    url = stream.read(1024)+"/data/"+filename
    stream.reset()
    stream.write(urllib2.urlopen(url).read())
    if stream.decrypt(password) != 0 or stream.uncompress() != 0:
        return ""
    return stream
if __name__ == "__main__":
    Upload_File("Magic.dat","")#��д����
    Down_File("Magic.dat","")#��д����
   