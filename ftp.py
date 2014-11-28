#coding=utf-8
import DataStream
from ftplib import FTP

class MyFtp():
    def __init__(self,host,port = '21'):
        self.BUF_SIZE = 1024
        self.ftp  = FTP()
        self.host = host
        self.port =port
    def login(self,username,password):
        self.ftp.connect(self.host,self.port)
        self.ftp.login(username,password)
        self.ftp.cwd("data")
    def down(self,filename):
        fp = open(filename,"wb")
        self.ftp.retrbinary("RETR %s"%filename,fp.write,self.BUF_SIZE)
        fp.close()
    def up(self,filename):
        fp = open(filename,"rb")
        self.ftp.storbinary("STOR %s"%filename,fp,self.BUF_SIZE)
        fp.close()
    def quit(self):
        self.ftp.quit()
        print 'ftp quit';
if __name__ == "__main__":
   