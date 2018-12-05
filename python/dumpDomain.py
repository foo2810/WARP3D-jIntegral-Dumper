# -*- coding: utf-8 -*-

# Domain Dumper
#
# Last modified: 2018/9/25
#

import sys
import datetime
import logging
from pathlib import Path
from jdomain.util import DomainExtracter
from jdomain.exception import DomainFormantError


def usage():
	sys.stderr.write("Usage: python bpfdump.py <input file> <output file>\n")


def printTitle():
	print("**************************************************")
	print("* Domain Dumper                                  *")
	print("*                                                *")
	print("* Last modified: 2018/10/2                       *")
	print("**************************************************")


def printMap(mapping):
	print("Map")
	print("+--------------------------+----------+")
	print("| Domain ID                | step     |")
	print("+--------------------------+----------+")
	for domain in mapping.keys():
		for step in mapping[domain].keys():
			print("| {:<24s} | {:^8d} |".format(domain, step))
			print("+--------------------------+----------+")


def main():
	args = sys.argv
	argc = len(args)
	
	if argc != 3:
		usage()
		sys.exit(1)
	
	logging.basicConfig(filename="log/log.txt", filemode="w", level=logging.INFO)
	logging.info("System starting")
	
	printTitle()
	print("\n")
		

	iFile = Path(args[1])
	oFile = Path(args[2])

	if not iFile.exists():
		sys.stderr.write(">>> [Error] Input file not found\n")
		logging.info("Error: Input file not found - {}".format(str(iFile)))
		logging.info("System exitting (status: 1)")
		sys.exit(1)
	
	reader = None
	try:
		reader = DomainExtracter(str(iFile))
	except BaseException as error:
		logging.warning("Error: Unexpected error ({})".format(error))
		logging.info("System exitting")
		sys.exit(1)
		
	mapping = reader.map

	if len(mapping) == 0:
		sys.stderr.write(">>> [Error] No domain value in {}\n\n".format(str(iFile)))
		logging.info("Error: No domain value in {}".format(str(iFile)))
		logging.info("System exitting (status: 1)")
		sys.exit(1)

	oFile.write_text("Create in {}\n\n".format(str(datetime.datetime.today())))


	try:
		domain = None
		step = None
	
		# Main loop
		while True:
			printMap(mapping)
			print("\n")	

			# Input domain ID
			while True:
				domain = input("> Enter domain ID: ")
				print("")

				if domain in mapping:
					break	

				print(">>> Error: Domain {} not found. Try again".format(domain))
				print("")

			# Input stem number
			while True:
				step = input("> Enter stem number: ")
				print("")
				try:
					step = int(step)
				except:
					print(">>> Error: Your input is not a number. Try again")
					print("")
					continue

				if step in mapping[domain]:
					break

				print(">>> Error: step {} not found".format(step))
				print("")

			reader.printDomainValues(domain, step)
			jVal = reader.getDomainValues(domain, step)
			jVal.dump(str(oFile), "a")
			with oFile.open("a") as f:
				f.write("\n\n\n")

			"""
			with oFile.open("a") as f:
				f.write(str(reader.getDomainValues(domain, step)) + "\n" + "-" * 117 + "\n\n")
			"""

			# Continue or not
			yn = None
			while True:
				yn = input("> Continue? (yes or no): ")
				print("")
				if yn=="yes" or yn=="YES" or yn=="y" or yn=="Y" or yn =="no" or yn=="NO" or yn=="n" or yn=="N":
					break

				print(">>> Error: Try again")
				print("")

			if yn == "no" or yn == "NO" or yn == "n" or yn == "N":
				break

	except KeyboardInterrupt:
		print("Now exitting...")
		logging.info("System exitting by KeyboardInterrupt (status: 0)")
		sys.exit(0)
	
	except DomainFormatError:
		sys.stderr.write(">>> [Error] bpf file may be broken\n")
		logging.info("Error: DomainFormatError")
		logging.info("System exitting (status: 1)")
		sys.exit(1)
	
	except BaseException as error:
		logging.warning("Error!: Unexpected error ({})".format(error))
		logging.info("System exitting (status: 1)")
		sys.exit(1)
	
	logging.info("System exitting (status: 0)")
	


if __name__ == "__main__":
	main()
