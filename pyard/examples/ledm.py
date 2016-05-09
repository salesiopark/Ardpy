from pyard import Ledm
from time import sleep

a=Ledm(16)
while True:
    for key in ['0', '1', '2', '3', '4', '5', '6','7','8','9']:
        a.write(str(key))
        sleep(0.5)
