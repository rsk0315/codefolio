from decimal import Decimal, getcontext

def sind(x: Decimal):
    res = x
    xpow = x
    fact = 1
    i = 3
    while True:
        xpow *= -x * x
        fact *= i * (i-1)
        next_ = res + xpow / fact
        if res == next_: break
        res = next_
        i += 2

    return res

def cosd(x: Decimal):
    res = 1
    xpow = 1
    fact = 1
    i = 2
    while True:
        xpow *= -x * x
        fact *= i * (i-1)
        next_ = res + xpow / fact
        if res == next_: break
        res = next_
        i += 2

    return res

def pi():
    lb, ub = Decimal('3.14'), Decimal('3.15')
    while True:
        mid = (lb + ub) / 2
        if mid in (lb, ub): break
        if sind(mid) < 0:
            ub = mid
        else:
            lb = mid

    return lb

PI = pi()

def acosd(x: Decimal):
    lb, ub = Decimal(0), PI
    while True:
        mid = (lb + ub) / 2
        if mid in (lb, ub): break
        if cosd(mid) < x: ub = mid
        else: lb = mid

    return lb

def asind(x: Decimal):
    lb, ub = -PI/2, PI/2
    while True:
        mid = (lb + ub) / 2
        if mid in (lb, ub): break
        if sind(mid) < x: lb = mid
        else: ub = mid

    return lb
