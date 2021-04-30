# Imports
import serial
import struct
import sys

import subprocess
import os
import time
import signal
import random
import re
from datetime import datetime


# Function that send command in cmd prompt
def sendCommand(input_stream, command):
	print(command.decode("utf-8"))
	input_stream.write(command)
	input_stream.flush()


# Function that reads register value (binary output)
def readRegValue(process, reg):
	sendCommand(process.stdin, ("p/x " + reg + "\n").encode("utf-8"))
	raw_val = process.stdout.readline().decode("utf-8")
	hex_val = reg_val_regex.findall(raw_val)[0]
	bin_str = bin(int(hex_val, 16))[2:].zfill(32)
	return bin_str


# Function that write a value in the register
def writeRegValue(process, reg, val, base=2):
	int_val = int(val, base)
	sendCommand(process.stdin, ("set " + str(reg) + " = " + str(int_val) + "\n").encode("utf-8"))


# Function that wait until prompt is ready 
def waitTillPrompt(process):
	currentLine = ""
	while(True):
		readChar = process.stdout.read(1).decode("utf-8")
		# print(readChar)
		currentLine += str(readChar)
		if(readChar == '\n'):
			print(currentLine, end='')
			currentLine = ""
		elif "(gdb) " in currentLine:
			print(currentLine, end='')
			break

# Funtion that finds a breakpoint
def waitTillPromptSignalIfBreak(process):
	currentLine = ""
	breakpointFound = False
	while(True):
		readChar = process.stdout.read(1).decode("utf-8")
		# print(readChar)
		currentLine += str(readChar)
		if(readChar == '\n'):
			print(currentLine, end='')
			currentLine = ""
		elif "Breakpoint" in currentLine:
			breakpointFound = True
		elif "(gdb) " in currentLine:
			print(currentLine, end='')
			break
	return breakpointFound

# Function that wait until find an Event
def waitTillEvent(process):
	global crash_flag
	crash_flag = 0
	while(True):
		line = process.stdout.readline()
		#print('RAEAD', line.decode("utf-8"))
		if("Breakpoint" in line.decode("utf-8")):
			# print("")
			break

		elif "SIGSEGV" in line.decode("utf-8"):
			crash_flag = 1
			register_log.write("\tSample " + str(j) + " " + str(fault) + ": SegmentationFault\n ")
			register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " SegmentationFault\n")
			sendCommand(process.stdin, b'kill\n')


		elif "SIGBUS" in line.decode("utf-8"):
			crash_flag = 1
			register_log.write("\tSample " + str(j) + " " + str(fault) + ": BusError\n ")
			register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " BusError\n")
			sendCommand(process.stdin, b'kill\n')
		
		#elif "Inferior 1" in line.decode("utf-8"):
		#	crash_flag = 1
		#	register_log.write("\tSample " + str(j) + " " + str(fault) + ": RelocationError\n ")
		#	register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " RelocationError\n")
		#	sendCommand(process.stdin, b'kill\n')
		

		elif "SIGABRT" in line.decode("utf-8"):
			crash_flag = 1
			register_log.write("\tSample " + str(j) + " " + str(fault) + ": Aborted\n ")
			register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " Aborted\n")
			sendCommand(process.stdin, b'kill\n')

		elif "SIGILL" in line.decode("utf-8"):
			crash_flag = 1
			register_log.write("\tSample " + str(j) + " " + str(fault) + ": IlegalInstruction\n ")
			register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " IlegalInstruction\n")
			sendCommand(process.stdin, b'kill\n')

		elif "Traceback (most recent call last)" in line.decode("utf-8"):
			crash_flag = 1
			register_log.write("\tSample " + str(j) + " " + str(fault) + ": MemoryError\n ")
			register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " MemoryError\n")
			sendCommand(process.stdin, b'kill\n')



		
	waitTillPrompt(process)

# Function that close the uC and the txt files (reset)
def signal_handler(signum, frame):
	# print("Here")

	
	global p
	try:
		os.waitpid(p.pid, os.WNOHANG)
	except:
		print("Nevermind...")
	register_log.close()
	register_log_clean.close()

	exit(1)
 
 
 
def find_between( s, first, last ):
	try:
		start = s.index( first ) + len( first )
		end = s.index( last, start )
		return s[start:end]
	except ValueError:
		return ""
	

# Function that classify output
def classify(vals):
	flower = [0, 0, 0]
	setosa = 0
	versi  = 0
	virg   = 0 
	
	if(vals[0] > 0):
		flower[0] += 1
	else:
		flower[1] += 1

	if(vals[1] > 0):
		flower[0] += 1
	else:
		flower[2] += 1

	if(vals[2] > 0):
		flower[1] += 1
	else:
		flower[2] += 1

	return flower.index(max(flower))

# Function that converts float to hexadecimal
def float_to_hex(f):
	return struct.pack("<I" ,struct.unpack('<I', struct.pack('<f', f))[0])

# Extract just the number of hexa (ex: 0x58 -> 58)
reg_val_regex = re.compile(r'0x(.*)')

# Timestamp
timestamp = str(datetime.now())


# Possible registers
regs = ["$eax","$ecx","$edx","$ebx","$esp","$ebp","$esi","$edi","$eip","$eflags","$cs","$ss","$ds","$es","$fs","$gs"]
#regs = ["$r0","$r1","$r2","$r3","$r4","$r5","$r6","$r7","$r8","$r9","$r10","$r11","$r12","$sp","$lr","$pc","$xpsr","$fpscr","$PRIMASK","$BASEPRI","$FAULTMASK","$CONTROL","$MSP","$PSP"]

# Find the used registers in the assembly file
used_regs = []
i = 0
j = 0

with open('main.txt', 'r') as assembly_file:
	assembly_code = assembly_file.read()
	# print(assembly_code)
	for reg in regs:
		if(reg[1:] in assembly_code):
			j+=1
			print(reg, "\t\t--> PRESENT", j)
			used_regs += [reg]
		else:
			i += 1
			print(reg, "\t\t--> NOT PRESENT", i )

# Used registers
print(used_regs)


# Open file IRIS dataset
inputs = open("csv/iris.csv", "r")

lines = inputs.readlines()


#input_data represents the dataset
input_data = []

lines = lines[1:]
for line in lines:
	vals = line.split(",")
	input_data += [(float(vals[3].strip()), float(vals[4].strip()))]

inputs.close()


# Start gdb process
pathToProcess = "gdb"
process = subprocess.Popen([pathToProcess, "main.out"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell = True)
waitTillPrompt(process)

sendCommand(process.stdin, b'set confirm off\n')
waitTillPrompt(process)

# Before call function in main
sendCommand(process.stdin, b'break main.c:215\n')
waitTillPrompt(process)
# Result
#sendCommand(process.stdin, b'break main.c:205\n')
#waitTillPrompt(process)


time.sleep(1)


signal.signal(signal.SIGINT, signal_handler)

#Load data and golden ref
truth = []

# Golden reference (truth)
with open('csv/golden.csv', 'r') as input_file:
	for line in input_file.readlines():
		truth.append([float(x.strip()) for x in line.split(' ')])

# prince
print(truth)
	
print("*********************************************")
print("*                                           *")
print("*                                           *")
print("*        Starting Fault Injection           *")
print("*                                           *")
print("*                                           *")
print("*********************************************")



# Percentage of time
percentages = [x/100.0 for x in range(10, 100, 10)]
#percentages = [10/100]

faults = []
#791 is the number of instructions in the svm_compute (can calculate it with another code (call joao))
total_instructions = 791
for per in percentages:
	for bit_pos in range(0,32):
		faults += [(int(per*total_instructions), bit_pos)]

print('Fault campaign will be simulated with: \n')
print(faults)



# Counter
register_log           = ""
total_faults           = 0
total_masked_faults    = 0
total_critical_faults  = 0
total_tolerable_faults = 0
total_weird_faults     = 0
total_crashes		   = 0

if not os.path.exists('logs'):
    os.makedirs('logs')

for reg in used_regs:
	begin = datetime.now()
	# Counter for each register
	local_faults           = 0
	local_masked_faults    = 0
	local_critical_faults  = 0
	local_tolerable_faults = 0
	local_weird_faults     = 0
	local_crashes		   = 0

	register_log       = open("logs/log_register_" + str(reg[1:]) + ".txt", "a" )
	register_log_clean = open("logs/log_register_" + str(reg[1:]) + "_clean.txt", "a" )
	timestamp = str(datetime.now())
	register_log.write("\n******** Run started at "+ str(timestamp) + "*******\n")
	register_log_clean.write("\n******** Run started at "+ str(timestamp) + "*******\n")

	# Input data[0]
	for j in [0, 2, 5]:

		for fault in faults:
			f = open("sample.txt","w")
			f.write(str(input_data[j][0]) + " " + str(input_data[j][1]))
			f.close()
			
			# Continue until breakpoint
			sendCommand(process.stdin, b'run\n')

			(position, bit_pos) = fault
			print("Running: [", reg, " at ", position, " on bit ", bit_pos, " Sample ", j,  "]" )
			register_log.write("Running: [" + str(reg) + " at " + str(position) + " on bit " + str(bit_pos) + " Sample " + str(j) + "]\n")
			#Send header
		
			print("\tSending input sample")
			register_log.write("\tSending input sample\n")
			# in this version, the input sample is already in the C code
			print((input_data[j][0]), (input_data[j][1]))
			


			#Reaches svm_compute
			waitTillEvent(process)

			if crash_flag == 1:
				crash_flag = 0
				continue
			else:
				pass

			print("\nStepping thorugh")
			register_log.write("\tStepping thorugh\n")

		
			# # Find the position
			si_command = ("si " + str(position) + "\n").encode("utf-8")
			sendCommand(process.stdin, si_command)
			waitTillPrompt(process)

			sendCommand(process.stdin, ("p/x $pc\n").encode("utf-8"))
			waitTillPrompt(process)

			# Inject fault
			val = readRegValue(process, reg)
			waitTillPrompt(process)
			val = list(val)
			bit_to_change = bit_pos
			val[bit_to_change] = '0' if val[bit_to_change] == 1 else '1'
			val = "".join(list(val))
			writeRegValue(process, reg, val)
			waitTillPrompt(process)

			sendCommand(process.stdin, b'continue\n')



			file_output = open("output.txt", 'r')
			result = file_output.readlines()
			file_output.close()
			
			

			line = re.findall("[\+\-]?\d+\.\d+", result[0])
			if len(line) != 3:
				line = line + ['0']

			line = ["{0:.5f}".format(float(x)) for x in line]
				
           
			sendCommand(process.stdin, b'continue\n')


			try:
				# Mismatch result
				result = ([float(x) for x in line])
				
				
				if(not result == truth[j]):
					print("***************** MISMATCH ****************")


					expected_class = classify(truth[j])
					gotten_class = classify(result)
					if gotten_class !=  expected_class:
						print(f'Result: {result} \nTruth: {truth[j]}')
						print('CRITICAL')
						register_log.write("\tSample " + str(j) + " " + str(fault) + ": Crit " + str(result) + " != " + str(truth[j]) + "\n")
						register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " Crit" +  str(result) + " != " + str(truth[j]) + "\n")
						total_critical_faults += 1
						local_critical_faults += 1			
					else:
						print(f'Result: {result} \nTruth: {truth[j]}')
						print('TOLERABLE')
						register_log.write("\tSample " + str(j) + " " + str(fault) + ": Tole " + str(result) + " != " + str(truth[j]) + "\n")
						register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " Tole" +  str(result) + " != " + str(truth[j]) + "\n")
						total_tolerable_faults += 1
						local_tolerable_faults += 1

				else:
					print(f'Result: {result} \nTruth: {truth[j]}')
					print('MASKED')
					
					register_log.write("\tSample " + str(j) + " " + str(fault) + ": Masked\n")
					register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " Masked" + "\n")
					total_masked_faults += 1
					local_masked_faults += 1

			except:
				print('WEIRD ERROR')
				print("Could not properly read the value")
				print("Unexpected error:", sys.exc_info()[0])
				register_log.write("\tSample " + str(j) + " " + str(fault) + ": Weird error\n")
				register_log_clean.write("Sample " + str(j) + " " + str(fault[0]) + " " + str(fault[1])  + " Weird Error" +  str(result) + " != " + str(truth[j]) + "\n")
				total_weird_faults += 1
				local_weird_faults += 1

 
	# Write the counter in the logs
	elapsed_time =  datetime.now() - begin
	register_log_clean.write("Tolerable faults: " + str(local_tolerable_faults) + "\n")
	register_log_clean.write("Crit faults: "      + str(local_critical_faults)  + "\n")
	register_log_clean.write("Masked faults: "    + str(local_masked_faults)    + "\n")
	

	register_log_clean.write("It took: "     + str((elapsed_time.seconds)/3600)  + ' hours '   + "\n")

	register_log.close()
	register_log_clean.close()

