import csv
import numpy as np
import os

list_of_files = ["_MISTO_burst", "_MISTO_cbr","_TCP_cbr", "_TCP_burst", "_UDP_cbr", "_UDP_burst", "_ML_MISTO_burst", "_ML_MISTO_cbr","_ML_TCP_cbr", "_ML_TCP_burst", "_ML_UDP_cbr", "_ML_UDP_burst", "_MP_MISTO_burst", "_MP_MISTO_cbr","_MP_TCP_cbr", "_MP_TCP_burst", "_MP_UDP_cbr", "_MP_UDP_burst"]

for f in range(len(list_of_files)):

	filename = list_of_files[f] + ".csv"
	
	results = "CONSOLIDADO" + filename
	with open(results, "w") as f_output:
		for i in range(20):
			numberedFilename = 	str(i) + filename
			with open(numberedFilename, "r") as f_input:
				csv_r = csv.reader(f_input, lineterminator = "\n")
				csv.writer(f_output, lineterminator = "\n").writerows(csv_r)