from datetime import datetime
from time import time
import os

start_datetime = datetime.strptime('2011/10/20 11:58:32.623', '%Y/%m/%d %H:%M:%S.%f')

offset=480071.64


file_list={}
for i in range(1,13):
    file_list[i]=((i-1)*400+1,i*400)
file_list[13]=(4801,5230)
file_list[14]=(5231,5630)
file_list[15]=(5631,6030)
file_list[16]=(6031,6430)

datasets_path=sorted(['../'+f+'/' for f in os.listdir('../') if 'dataset' in f])

def get_dataset_num(file_num):
    dataset_num=0
    for i in file_list.keys():
        if file_num>file_list[i][0] and file_num<file_list[i][1]:
            dataset_num=i
    return dataset_num

def get_path(file_num):
    dataset_num=get_dataset_num(file_num)
    path=''
    for p in datasets_path:
        if '%003d'%(dataset_num) in p:
            path=p
            break
    path+='location_001_ivdata_%003d.txt' %file_num if file_num<1000 else 'location_001_ivdata_%0004d.txt'%file_num
    return path
# #events class
# class event:
#     def __init__(self,timestamp,device,phase):
#         self.event_time=(datetime.strptime(timestamp[:23], '%m/%d/%Y %H:%M:%S.%f') - start_datetime).total_seconds()
# #         self.event_time= self.event_time - offset if self.event_time > offset else self.event_time
#         print(self.event_time)
#         self.phase=phase
#         self.device=device
#         self.files=[-1,-1]
#
#     def __str__(self):
#         return 'Dno=%3s datetime=%12s P=%c F=%s'%(self.device,self.event_time,self.phase,str(self.files))
