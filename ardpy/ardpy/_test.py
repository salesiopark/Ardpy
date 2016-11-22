#22/Nov/2016
import ardpy
a = ardpy.Ledm(0x21)
l = ardpy.Tlcd(0x11)
l.clear()
st = "0123456789"
lcnt=0
acnt=0
while True:
    a.write(st[acnt])
    l.setCursor(0,0)
    l.puts(str(lcnt))
    lcnt +=1
    acnt +=1
    if acnt>9:
        acnt = 0
