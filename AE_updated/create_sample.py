import os

acc = open("sensors_data/accelerometer.txt", "r").readlines()
mag = open("sensors_data/magnetometer.txt", "r").readlines()
gyros = open("sensors_data/gyros.txt", "r").readlines()
gold = open("GOLD/Xtotal.txt","r").readlines();

for i in range(5):

    
    samples = open("sample.txt", "w")
   
    data = str(mag[i]) + str(acc[i]) + str(gyros[i]) + "\n"
    samples.write(data)

    samples.close()
    
    if i==0:
        os.system("g++ -o main main.cpp functions.cpp functions.h")
    
    starting_AE_Algorithm = "./main" + " " + str(i)
    os.system(starting_AE_Algorithm) 
    
    X = open("X.txt","r").readline()
    #comparision:
    #print(gold[i+1])
    #print(X)
    if (gold[i+1] == X):
        print("Match")
    else:
        print("Mismatch")
    
    