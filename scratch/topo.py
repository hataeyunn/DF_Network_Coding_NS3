f = open("topo.txt", 'w')
for i in range(0,24):
    check = i % 3
    if check == 0:
        for j in range(24,36):
            if j % 2 == 0:
                f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(j)+")->GetId());\n")
        if i-3 >= 0:
            f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i-3)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i+1)+")->GetId());\n")
        if i+3 <=23:
            f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i+3)+")->GetId());\n")
        
    elif check == 1:
        if i-3 >= 0:
            f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i-3)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i+1)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i-1)+")->GetId());\n")
        if i+3 <=23:
            f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i+3)+")->GetId());\n")
    else:
        for j in range(24,36):
            if j % 2 == 1:
                f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(j)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i-1)+")->GetId());\n")
        if i-3 >= 0:
            f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i-3)+")->GetId());\n")
        if i+3 <=23:
            f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(i+3)+")->GetId());\n")
    f.write("\n")

for i in range(24,36):
    if i % 2 == 0:
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(0)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(3)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(6)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(9)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(12)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(15)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(18)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(21)+")->GetId());\n")
    else:
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(2)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(5)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(8)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(11)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(14)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(17)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(20)+")->GetId());\n")
        f.write("nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(23)+")->GetId());\n")
    f.write("\n")

f.close()