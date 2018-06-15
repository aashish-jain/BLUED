import numpy as np
import os

path = 'events/dataset'

def check(data):
    power_initial = np.mean(np.multiply(data[200:400,1], data[200:400,2]))
    power_final   = np.mean(np.multiply(data[59800:60000,1], data[59800:60000,2]))
    if (power_final - power_initial) > 0:
        return 'ON'
    else:
        return 'OFF'

for i in range(1,16):

    list_of_files = os.listdir(path+str(i)+'/')
    print(list_of_files)
    for File in list_of_files:
        data = np.loadtxt(path+str(i)+'/'+File,delimiter = ',')
        status = check(data)
        os.rename(path+str(i)+'/'+File, str((path+str(i)+'/'+File).split('_')[0]) + '_' + status + '_' + str([f for f in (path+str(i)+'/'+File).split('_')][1]) + '_' + str([f for f in File.split('_')][2]))
