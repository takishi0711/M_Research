class A:
    def __init__(self, _a):
        self.a = _a


lis = []
lis.append(A(1))
lis.append(A(2))
lis.append(A(3))

lis2 = [x.a for x in lis]

print(lis2)