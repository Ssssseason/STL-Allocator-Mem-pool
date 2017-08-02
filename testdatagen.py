import random
maxnum = 1000
maxsize = 200
with open('test.txt', 'w') as f:
    f.write(str(maxnum))
    for i in range(1000):
        f.write(str(random.randint(0, maxsize)))
