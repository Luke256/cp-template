from math import isqrt

t = int(input())

for _ in range(t):
    n = int(input())

    print((m:=n<<1)-1-isqrt((m<<1)-1))