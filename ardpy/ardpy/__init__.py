from .ardpy import Ardpy
from .arduno import Arduno # device id : 0
from .ledm import Ledm # device id : 101
from .tlcd import Tlcd # device id : 102

# 모듈이 추가되면 아래 리스트에 추가한다.
# 각 클래스는 _VERSION 문자열에 버전을 a.b.c 형식으로 명기해야 한다.
subVers = [
    Arduno._VERSION,
    Ledm._VERSION,
    Tlcd._VERSION,
]

#calculate ardpy package version
def readVer(strVer):
    lst = strVer.split('.')
    return ( int(lst[0]),int(lst[1]),int(lst[2]) )

def readSubVer(strVer):
    lst = strVer.split('.')
    return ( int(lst[0]), int(lst[1])*10+int(lst[2]) )

verA, verB, verC = readVer(Ardpy._VERSION)

for ver in subVers:
    a, bc = readSubVer(ver)
    verB += a
    verC += bc
    
_VERSION = '%d.%d.%d'%(verA, verB, verC)
print('ardpy ver',_VERSION)

#불필요한 변수들은 삭제한다.
del(subVers, readVer, readSubVer, verA, verB, verC, a, bc, ver)
del(ardpy, arduno, ledm, tlcd) # 맨 마지막에 와야한다(왜?)
