import numpy as np
import os
import sys
import pandas as pd
from pprint import pprint
from multiprocessing import Pool, cpu_count
directory = 'events'


def save_derived(f):
    print(f)
    datafiles = [directory+'/'+f+'/'+x for x in os.listdir(directory+'/'+f)]
    for d in datafiles:
        data = np.loadtxt(d, delimiter=',')
        derived = compute_parameters(data)
        if type(derived)!=type(False):
            np.savetxt('events/derived/{}/{}'.format(f,d.split('/')[2]),derived,delimiter=',')


def compute_i_rms(data):
    return np.sqrt(np.mean(np.square(data)))

def compute_v_rms(data):
    return np.sqrt(np.mean(np.square(data)))

def compute_p_real(v,i):
    return abs(np.mean(np.multiply(v,i)))

def compute_p_apparent(vr,ir):
    return vr*ir

def compute_parameters(data):
    feature_vector_len = int(len(data)/200)
    i_rms = np.zeros(feature_vector_len)
    v_rms = np.zeros(feature_vector_len)
    p_app = np.zeros(feature_vector_len)
    p_real = np.zeros(feature_vector_len)
    j,k=0,0
    x=False
    try:
        while not(data[j, 2] < 0.0 and data[j+1, 2] > 0.0):
            j+=1
        j+=1
        while j+200<len(data):
            i=data[j:j+200, 1]
            v=data[j:j+200, 2]
            i_rms[k] = compute_i_rms(i)
            v_rms[k] = compute_v_rms(v)
            p_real[k] = compute_p_real(v,i)
            p_app[k] = compute_p_apparent(v_rms[k],i_rms[k])
            j+=200
            k+=1
        p_reactive=np.sqrt(np.multiply(p_app,p_app) - np.multiply(p_real,p_real))

        i_rms, v_rms = np.transpose(np.matrix(i_rms)), np.transpose(np.matrix(v_rms))
        p_real, p_reactive = np.transpose(np.matrix(p_real)), np.transpose(np.matrix(p_reactive))
        p_app = np.transpose(np.matrix(p_app))
        x = np.hstack((i_rms, v_rms, p_real, p_reactive, p_app))
    print(x)
    input()
    except:
        print("Some error")
    return x

def main():
    pool = Pool(cpu_count())
    files=os.listdir(directory)
    if not os.path.exists('events/derived'):
            print('Creating directory')
            os.makedirs('events/derived')
    for f in files:
        if 'dataset' not in f:
            files.remove(f)
    pool.map(save_derived,files)

main()
