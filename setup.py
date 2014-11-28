#coding=utf-8
"""
build datastream module
1. setup.py build
2. setup.py install
3. import DataStream
"""
from distutils.core import setup, Extension
MOD = "DataStream"
setup(name=MOD,ext_modules=[Extension(MOD,sources=["datastream.c"])])