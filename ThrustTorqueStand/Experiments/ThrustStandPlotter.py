#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Script for representing thrust stand plots!"""

""" Imports """

import os
from os import path

import argparse

import matplotlib.pyplot as plt

import scipy.io as sio

""" Code """

parser = argparse.ArgumentParser(description='Show thrust stand plots')
parser.add_argument('dirPlots', metavar='dir', type=str,
                    help='directory for processing')
parser.add_argument('--mat', action='store_true',
                    help='save data to .mat file')

args = parser.parse_args()
plotsDirectory = args.dirPlots
saveDataMat = args.mat

print 'Checking', plotsDirectory

if saveDataMat:
	print 'Saving data to ' + plotsDirectory + '.mat file'

# Obtain files from directory

current_files 	= []
thrust_files	= []
speed_files		= []

files = os.listdir(plotsDirectory)
for file in files:
	# print file
	if file.startswith("Current"):
		current_files.append( file )
	if file.startswith("Thrust"):
		thrust_files.append( file )
	if file.startswith("Speed"):
		speed_files.append( file )

# Sort files

current_files.sort()
thrust_files.sort()
speed_files.sort()

# Obtain dta from each file

data_source_dict = {'Thrust': thrust_files, 'Current': current_files, 'Speed': speed_files}
data_to_mat_file = {'Thrust': [], 'Current': [], 'Speed': []}
data_index = 310


for data_source in data_source_dict.values():
	data_index = data_index + 1
	
	plt.subplot( data_index )
	plt.grid()
	data_source_title = data_source_dict.keys()[data_source_dict.values().index(data_source)]
	plt.title( data_source_title )
	
	print 'Processing', data_source_title
	file_index = 0
	for file in data_source:
		fileRelPath = plotsDirectory + '/' + file
		with open(plotsDirectory + '/' + file) as f:
			contentLines = f.readlines()
			time_vector = []
			value_vector = []
			for line in contentLines:
				values = line.split('\t')
				time_moment 	= int(values[0])
				value 			= float(values[1])

				time_vector.append( time_moment )
				value_vector.append( value )
				plt.plot( time_vector, value_vector )

			if saveDataMat:
				data_to_mat_file[data_source_title].append( value_vector )

plt.show()

if saveDataMat:
	# print data_to_mat_file
	sio.savemat( plotsDirectory, data_to_mat_file, appendmat = True )
