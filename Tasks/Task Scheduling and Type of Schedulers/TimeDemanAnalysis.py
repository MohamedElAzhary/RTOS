import math

class Task:
    def __init__(self,periodicity,executionTime):
        self.periodicity = periodicity
        self.executionTime = executionTime


e1 = 2.5
p1 = 5

e2 = 4.5
p2 = 15

e3 = 3.5
p3 = 20


def W_T1(t):
    w = e1
    if(w > p1):
        exceededDeadline = True
    else:
        exceededDeadline = False
    return[t,w,exceededDeadline]

def W_T2(t):
    w = e2 + ((math.ceil(float(t)/p1))*e1) 
    if(w > p3):
        exceededDeadline = True
    else:
        exceededDeadline = False
    return [t,w,exceededDeadline]

def W_T3(t):
    w = e3 + ((math.ceil(float(t)/p2))*e2) +((math.ceil(float(t)/p1))*e1) 
    if(w > p3):
        exceededDeadline = True
    else:
        exceededDeadline = False
    return [t,w,exceededDeadline]




if __name__ == "__main__":

    T1_list = list()
    T2_list = list()
    T3_list = list()

    for t in range(1,p1+1):
        T1_list.append(W_T1(t))
    for t in range(1,p2+1):
        T2_list.append(W_T2(t))
    for t in range(1,p3+1):
        T3_list.append(W_T3(t))

    print(T1_list)
    print('\n')
    print(T2_list)
    print('\n')
    print(T3_list)