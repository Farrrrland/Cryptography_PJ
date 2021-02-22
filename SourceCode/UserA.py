import random
import time
import win32pipe, win32file

def fast_power(base, power, n):
    result = 1
    tmp = base
    while power > 0:
        if power&1 == 1:
            result = (result * tmp) % n
        tmp = (tmp * tmp) % n 
        power = power>>1
    return result

def receivePublicKeyfromB():
    
    fileHandle = win32file.CreateFile("\\\\.\\pipe\\SendRSA2Apipe",
                                win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                                0, None,
                                win32file.OPEN_EXISTING,
                                0, None)
    res = win32file.ReadFile(fileHandle, 4096)
    win32file.CloseHandle(fileHandle)
    res = str(res[1]).lstrip('b').lstrip("'").rstrip("'")
    print(res, "received from B")
    return res

def SendRSA2B(s):
    p = win32pipe.CreateNamedPipe(r'\\.\pipe\SendRSA2Bpipe',
        win32pipe.PIPE_ACCESS_DUPLEX,
        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
        1, 65536, 65536, 300, None)

    win32pipe.ConnectNamedPipe(p, None)
    data = bytes(s.encode('utf-8'))  
    win32file.WriteFile(p, data)
    print(data, "sent to B")

def RSA(m, n, e):
    c = fast_power(m, e, n)
    print("Cypher text =", c, "ready to be sent")
    SendRSA2B(str(c))


mKey = int(0x38290fca2b38dc1f)
mIV = int(0x38273cad27b29367)

print("Receive nB from B")
time.sleep(2)
n = int(receivePublicKeyfromB())
print("Receive eB from B")
time.sleep(2)
e = int(receivePublicKeyfromB())

print("RSA encrypt Key")
RSA(mKey, n, e)
print("RSA encrypt IV")
RSA(mIV, n, e)