from turtle import color
from unicodedata import name
import matplotlib.pyplot as plt
import numpy as np

# For Question 1
f1 = open("out1.txt", "r")
out1=[float(x.rstrip()) for x in f1]
f1.close()
out1=np.array(out1)[::-1]
out1_range = np.array(range(1,out1.size+1))

plt.scatter(out1_range,out1,color='green',label="Time(sec)",marker="1")
plt.legend()
plt.xlabel('File Sizes, in KB')
plt.ylabel('Time in Sec')
plt.title("Block wise varying size(KB) read times. 4KB Clusters are found")
plt.savefig("out1.png")
plt.clf()

# For Question 2
f2 = open("out2.txt", "r")
f2_range = open("out2_range.txt", "r")
out2=[float(x.rstrip()) for x in f2]
out2_range=[float(x.rstrip()) for x in f2_range]
f2.close()
f2_range.close()
out2=np.array(out2)[::-1]
out2_range = np.array(out2_range)[::-1]

plt.scatter(out2_range,out2,color='green',label="Time(sec)",marker=".")
plt.legend()
plt.xlabel('Offset from the file(in bytes)')
plt.ylabel('Time in Sec')
plt.title("Prefetch detection in reading large file.(10GB)")
plt.savefig("out2.png")
plt.clf()

# For Question 3

f3 = open("out3.txt", "r")
f3_1 = open("out3_1.txt", "r")
f3_range = open("out3_range.txt", "r")
f3_range_1 = open("out3_range_1.txt", "r")
out3=[float(x.rstrip()) for x in f3]
out3_1=[float(x.rstrip()) for x in f3_1]
out3_range=[float(x.rstrip()) for x in f3_range]
out3_range_1=[float(x.rstrip()) for x in f3_range_1]
f3.close()
f3_range.close()
f3_range_1.close()
out3=np.array(out3)[::-1]
out3_1=np.array(out3_1)[::-1]
out3_range = np.array(out3_range)[::-1]
out3_range_1 = np.array(out3_range_1)[::-1]
out3_mean = [np.mean(out3)]*len(out3_range)
out3_mean_1 = [np.mean(out3_1)]*len(out3_range_1)

fig,ax = plt.subplots()
data_line_out3 = ax.scatter(out3_range, out3, label="Time(sec)", marker="o")
mean_line_out3 = ax.plot(out3_range, out3_mean, label="Time(sec)", linestyle='--')
data_line_out3_1 = ax.scatter(out3_range_1, out3_1, label="Time(sec)", marker=".", color="red")
mean_line_out3_1 = ax.plot(out3_range_1, out3_mean_1, label="Time(sec)", linestyle='--', color="red")
plt.legend()
plt.xlabel('Iteration No.')
plt.ylabel('Time in Sec')
plt.title("Read same file cache block vs seperate ones")
plt.savefig("out3.png")
plt.clf()

# plt.plot(out3_range,out3,color='green', label="Time(sec)",marker=".")
# plt.plot(out3_range_1,out3_1,color='blue', label="Time(sec)",marker="*")
