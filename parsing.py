result = []
temp = []
f = open("ys1214.txt","r")
check = 0
while True:
    line = f.readline()
    if not line: break
    if line.find("--------- total result ---------"):
        check = 1
    if check == 1 and line.find("Throughput"):
        temp.append(line)
    if check == 1 and line.find("Latency"):
        temp.append(line)
    if check == 1 and line.find("Coding"):
        temp.append(line)
        check = 0
        result.append(temp)
    
for i in result:
    print(i)
f.close()