import csv
import sys

MPI_Time = []
Aggregate_Time = []
Aggregate_Sent_Message_Size = []
Aggregate_Collective_Time = []
Aggregate_Point_To_Point = []

from os import walk

pwd = sys.argv[1]
f = []
for (dirpath, dirnames, filenames) in walk(pwd):
    f.extend(filenames)
    break

latest_report = max(f)
print(latest_report)

with open(pwd+latest_report, 'r') as in_file:
    lines = in_file.read().splitlines()
    n = len(lines)
    #MPI Time (seconds)
    start = 0
    end = 0
    found = 0
    for i in range(n):
        if (lines[i].find("---------------------------------------------------------------------------")!=-1):
            end = i
            if (found==2):
                break
            if (found==1):
                found = found + 1
            continue
        if (lines[i].find("@--- MPI Time (seconds)") != -1):
            print(lines[i])
            start = i+2
            found = 1
    for i in range(start, end):
        line = lines[i]
        MPI_Time.append(line.split("\t"))

    with open(pwd+'MPI_Time.csv', 'w') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(MPI_Time)
    
    #Aggregate Time (top twenty, descending, milliseconds)
    start = 0
    end = 0
    found = 0
    for i in range(n):
        if (lines[i].find("---------------------------------------------------------------------------")!=-1):
            end = i
            if (found==2):
                break
            if (found==1):
                found = found + 1
            continue
        if (lines[i].find("Aggregate Time (top twenty, descending, milliseconds)") != -1):
            print(lines[i])
            start = i+2
            found = 1
    for i in range(start, end):
        line = lines[i]
        Aggregate_Time.append(line.split("\t"))

    with open(pwd+'Aggregate_Time.csv', 'w') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(Aggregate_Time)

    #Aggregate Sent Message Size (top twenty, descending, bytes)
    start = 0
    end = 0
    found = 0
    for i in range(n):
        if (lines[i].find("---------------------------------------------------------------------------")!=-1):
            end = i
            if (found==2):
                break
            if (found==1):
                found = found + 1
            continue
        if (lines[i].find("Aggregate Sent Message Size (top twenty, descending, bytes)") != -1):
            print(lines[i])
            start = i+2
            found = 1
    for i in range(start, end):
        line = lines[i]
        Aggregate_Sent_Message_Size.append(line.split("\t"))

    with open(pwd+'Aggregate_Sent_Message_Size.csv', 'w') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(Aggregate_Sent_Message_Size)
    
    #Aggregate Collective Time (top twenty, descending)
    start = 0
    end = 0
    found = 0
    for i in range(n):
        if (lines[i].find("---------------------------------------------------------------------------")!=-1):
            end = i
            if (found==2):
                break
            if (found==1):
                found = found + 1
            continue
        if (lines[i].find("Aggregate Collective Time (top twenty, descending)") != -1):
            print(lines[i])
            start = i+2
            found = 1
    for i in range(start, end):
        line = lines[i]
        Aggregate_Collective_Time.append(line.split("\t"))

    with open(pwd+'Aggregate_Collective_Time.csv', 'w') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(Aggregate_Collective_Time)
    
    #Aggregate Point-To-Point Sent (top twenty, descending)
    start = 0
    end = 0
    found = 0
    for i in range(n):
        if (lines[i].find("---------------------------------------------------------------------------")!=-1):
            end = i
            if (found==2):
                break
            if (found==1):
                found = found + 1
            continue
        if (lines[i].find("Aggregate Point-To-Point Sent (top twenty, descending)") != -1):
            print(lines[i])
            start = i+2
            found = 1
    for i in range(start, end):
        line = lines[i]
        Aggregate_Point_To_Point.append(line.split("\t"))

    with open(pwd+'Aggregate_Point_To_Point.csv', 'w') as out_file:
        writer = csv.writer(out_file)
        writer.writerows(Aggregate_Point_To_Point)
