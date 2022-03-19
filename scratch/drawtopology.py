f = open("topology.txt","w")
sen = ""
odd_list = [0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45,48]
even_list = [2,5,8,11,14,17,20,23,26,29,32,35,38,41,44,47,50]
for i in range(59,71):
    if i%2==0:
        #even
        for j in even_list:
            sen = "nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(j)+")->GetId());"
            f.write(sen)
            f.write("\n")
            sen = "nodeConnections[allNodes.Get("+str(j)+")->GetId()].push_back(allNodes.Get("+str(i)+")->GetId());"
            f.write(sen)
            f.write("\n")

            
    else :
        #odd
        for j in odd_list:
            sen = "nodeConnections[allNodes.Get("+str(i)+")->GetId()].push_back(allNodes.Get("+str(j)+")->GetId());"
            f.write(sen)
            f.write("\n")
            sen = "nodeConnections[allNodes.Get("+str(j)+")->GetId()].push_back(allNodes.Get("+str(i)+")->GetId());"
            f.write(sen)
            f.write("\n")

    f.write("\n")

f.close()