import os
import time
import random
import win32pipe, win32file

def receiveRSAfromA():
    
    fileHandle = win32file.CreateFile("\\\\.\\pipe\\SendRSA2Bpipe",
                                win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                                0, None,
                                win32file.OPEN_EXISTING,
                                0, None)
    res = win32file.ReadFile(fileHandle, 4096)
    win32file.CloseHandle(fileHandle)
    res = str(res[1]).lstrip('b').lstrip("'").rstrip("'")
    print(res, "Received from A")
    return res

def send_des_message(key, IV):
    time.sleep(2)
    f=open(r'\\.\Pipe\Innerpipe', 'w')
    print("Send DES Key", hex(key), "to DES.cpp")
    f.write(hex(key))
    f.close()
    time.sleep(2)
    f=open(r'\\.\Pipe\Innerpipe', 'w')
    print("Send DES IV", hex(IV), "to DES.cpp")
    f.write(hex(IV))
    f.close()

def fast_power(base, power, n):
    result = 1
    tmp = base
    while power > 0:
        if power&1 == 1:
            result = (result * tmp) % n
        tmp = (tmp * tmp) % n 
        power = power>>1
    return result

def MillerRabin(n, iter_num):
    # 2 is prime
    if n == 2:
        return True
    # if n is even or less than 2, then n is not a prime
    if n&1 == 0 or n<2:
        return False
    # n-1 = (2^s)m
    m, s = n - 1, 0
    while m & 1 == 0:
        m = m >> 1
        s += 1
    # M-R test
    for _ in range(iter_num):
        b = fast_power(random.randint(2, n - 1), m, n)
        if b == 1 or b == n-1:
            continue
        for __ in range(s - 1):
            b = fast_power(b, 2, n)
            if b == n-1:
                break
        else:
            return False
    return True

def rand64int():
    num = 1
    for i in range(0, 62):
        num = num * 2 + random.randint(0, 1)
    num = num * 2 + 1
    return num

def generate_large_Prime():
    while True:
        x = rand64int()
        if MillerRabin(x, 10):
            return x

def extendgcd(a, b):
    if b == 0:  
        return 1, 0, a
    else:
        x, y, gcd = extendgcd(b, a % b)  
        x, y = y, (x - (a // b) * y) 
        return x, y, gcd

def SendPublicKey2A(s):
    p = win32pipe.CreateNamedPipe(r'\\.\pipe\SendRSA2Apipe',
        win32pipe.PIPE_ACCESS_DUPLEX,
        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
        1, 65536, 65536, 300, None)

    win32pipe.ConnectNamedPipe(p, None)
    data = bytes(s.encode('utf-8'))  
    win32file.WriteFile(p, data)
    print(s, "sent to A")

def generate_RSA():
    p = generate_large_Prime()
    q = generate_large_Prime()
    n = p * q
    phi_n = (p - 1) * (q - 1)
    d = int(0)
    e = int(0)
    while True:
        d = random.randint(1, phi_n - 1)
        x, y, gcd = extendgcd(d, phi_n)
        # Test if gcd(d, phi_n) = 1
        if gcd == 1:
            e = (x % phi_n+ phi_n) % phi_n
            break

    # print("p =", p)
    # print("q =", q)
    # print("n =", n)
    # print("phi_n =", phi_n)
    # print("d =", d)
    # print("e =", e)
    print("Send nB to A")
    SendPublicKey2A(str(n))
    print("Send eB to A")
    SendPublicKey2A(str(e))
    return e, n, d

def DRSA(c, d, n):
    m = fast_power(c, d, n)
    return m


e, n, d = generate_RSA()
print("Receive RSA Key from A")
time.sleep(2)
Key_RSA = int(receiveRSAfromA())
print("Receive RSA IV from A")
time.sleep(2)
IV_RSA = int(receiveRSAfromA())

print("DRSA for Key")
Key = DRSA(Key_RSA, d, n)
print("DRSA for IV")
IV = DRSA(IV_RSA, d, n)

send_des_message(Key, IV)



