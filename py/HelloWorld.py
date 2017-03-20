#!/usr/bin/python
# -*- coding: UTF-8 -*-


import sys;
x = 'runoob'; sys.stdout.write(x + '\n')

print "Hello, World中国!";
print "你好中国";

if True:
    print "anserr"
    print "True"
else:
    print "anstere"
    #木有缩进
    print "False"

days = ['Monday', 'Tuesday', 'Wednesday',
        'Thursday', 'Friday']
a = '44444' + \
    '666666' + \
    '888888'
print days
print a

word = 'word'
sentence = "这是一个句子。"
paragraph = """这是一个段落
可以换行，包含多个语句"""  # 我是注释
# 输出变量的值
print word
print sentence
print paragraph

raw_input("\n\n Press the enter key to exit.")

x = "a"
y = "b"

print x
print y
print x,
print y,

if  1 == 2 :
    print "1==3"
elif 1 != 1:
    print "world"
else :
    print "你好啊"
