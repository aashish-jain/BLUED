import csv
import matplotlib.pyplot as plt
import numpy as np
from os import listdir
import sys

def plot_event(f,data):
    i, v, P, Q, S = data[:,0],data[:,1],data[:,2],data[:,3],data[:,4]
    _f, axarr = plt.subplots(5, sharex=True)
    axarr[0].plot(i)
    axarr[0].set_title('I')
    axarr[1].plot(v)
    axarr[1].set_title('V')
    axarr[2].plot(P)
    axarr[2].set_title('P')
    axarr[3].plot(Q)
    axarr[3].set_title('Q')
    axarr[4].plot(S)
    axarr[4].set_title('S')
    _f.subplots_adjust(hspace=0.15,wspace=0.15,top=0.96,bottom=0.03,right=0.96,left=0.05)
    plt.draw()
    plt.show()

def __main__():

    directory='events/derived/'
    directory_files=listdir(directory)
    for f in directory_files:
        files = listdir(directory+f)
        count=0
        for f1 in files:
            print (f1)
            plt.close('all')
            data = np.loadtxt(directory+f+'/'+f1, delimiter=',')
            count+=1
            plot_event(f1,data)
            input("Hlo")
            #_inp=input("Press Enter to continue any character to stop")
            #print (_inp)
            #break;
            #if _inp!='\n':
            #    break
__main__()
