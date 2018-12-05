# Domain Utility

import sys
import struct
from collections import OrderedDict
from jdomain.exception import DomainFormantError
from util.binaryUtil import *


def _normStr(string):
	ret = ""

	length = len(string)

	for i in range(-1, -length, -1):
		if string[i] != " ":
			j = length + i + 1
			ret = string[:j]
			break
	
	return ret


class DomainValues:
	def __init__(self):
		self.numDomains = None
		self.domainID = None
		self.stname = None
		self.lsldnm = None
		self.ltmstp = None

		self.nList = None
		self.domains = None
		self.skList = None

		self.diAvg = None
		self.diMin = None
		self.diMax = None

		self.nList2 = None
		self.domains2 = None
	
	def dump(self, csvName, mode):
		with open(csvName, mode) as csv:

			csv.write("domain:,{:<s},structure:,{:<8s},loading:,{:<8s}," \
					  "loading step number:,{:>12d}\n".format(self.domainID, self.stname, self.lsldnm, self.ltmstp))

			csv.write("domain,dm1,dm2,dm3,dm4,dm5,dm6,dm7,dm8,total J,killed ele\n")

			for i in range(len(self.nList)):
				csv.write("{:3d},".format(self.nList[i]))
				csv.write("{0[0]:11.4E}," \
						  "{0[1]:11.4E}," \
						  "{0[2]:11.4E}," \
						  "{0[3]:11.4E}," \
						  "{0[4]:11.4E}," \
						  "{0[5]:11.4E}," \
						  "{0[6]:11.4E}," \
						  "{0[7]:11.4E}," \
						  "{0[8]:11.4E},".format(self.domains[i]))
				csv.write("{:3d}\n".format(self.skList[i]))

			csv.write("domain average,domain min,domain max\n")
			csv.write("{:11.4E},{:11.4E},{:11.4E}\n\n".format(self.diAvg, self.diMin, self.diMax))
			csv.write("stress intensity factors from J (single-mode loading):\n" \
					  "domain,KI pstrs,KI pstrn,KII pstrs,KII pstrn,KIII\n")

			for i in range(len(self.nList2)):
				csv.write("{:3d},".format(self.nList[i]))
				csv.write("{0[0]:11.4E}," \
						  "{0[1]:11.4E}," \
						  "{0[2]:11.4E}," \
						  "{0[3]:11.4E}," \
						  "{0[4]:11.4E}\n".format(self.domains2[i]))

			csv.flush()
	
	def __repr__(self):
		ret = "domain: {:<24s}  structure: {:<8s}  loading: {:<8s}  " \
		      "loading step number: {:>12d}\n".format(self.domainID, self.stname, self.lsldnm, self.ltmstp)
		ret = ret + " domain     dm1         dm2         dm3         dm4         " \
		            "dm5         dm6         dm7         dm8        total J  killed ele\n"
		

		for i in range(len(self.nList)):
			ret = ret + " {:3d}    ".format(self.nList[i])
			ret = ret + " {0[0]:11.4E}" \
			            " {0[1]:11.4E}" \
			            " {0[2]:11.4E}" \
			            " {0[3]:11.4E}" \
			            " {0[4]:11.4E}" \
			            " {0[5]:11.4E}" \
			            " {0[6]:11.4E}" \
			            " {0[7]:11.4E}" \
			            " {0[8]:11.4E}".format(self.domains[i])
			ret = ret + "   {:3d}\n".format(self.skList[i])
			   
		ret = ret + " domain average   domain min   domain max\n"
		ret = ret + " {:11.4E}     {:11.4E}  {:11.4E}\n\n".format(self.diAvg, self.diMin, self.diMax)
		ret = ret + "stress intensity factors from J (single-mode loading):\n" \
		            " domain  KI pstrs    KI pstrn    KII pstrs   KII pstrn   KIII\n"
					
		for i in range(len(self.nList2)):
			ret = ret + " {:3d}   ".format(self.nList2[i])
			ret = ret + " {0[0]:11.4E}" \
			            " {0[1]:11.4E}" \
			            " {0[2]:11.4E}" \
			            " {0[3]:11.4E}" \
			            " {0[4]:11.4E}\n".format(self.domains2[i])

		return ret
			   

## ************
## * Caution! *
## ************
##
## bpf file is wrote using "Sequential access" of fortran.
##
## + Fortran binary file
## How to access disk files in fortran depends on parameter, "access" in opening disk files.
## [Sequential access]
##   <header><record><footer><header><record><footer>...
## 	   header: data size (4bytes ?)
##     record: data
##     footer: data size. the value is same with header (4bytes ?)
## 
## [Direct access]
##   <record><record><record>...
##   record: data (all records are same size!)
## 
## [Stream access]
##   Same with high levele I/O system of C lang

class DomainExtracter(BinaryFileReader):
	def __init__(self, bFile):
		super().__init__(bFile)

		self.map = OrderedDict()

		self._createMap()
	
	
	def printDomainValues(self, domainID, step):
		pos = self.map[domainID][step]

		print(self._readDomainVal(pos))
	
	def getDomainValues(self, domainID, step):
		pos = self.map[domainID][step]

		return self._readDomainVal(pos)

	def _createMap(self):
		typeNum = None
		numLines = None
		numStep = None
		numIter = None

		while True:
			try:
				lenRecord = byteToIntLE(super().readBytes(4))

				# Read thunk header
				typeNum = byteToIntLE(super().readBytes(4))
				numLines = byteToIntLE(super().readBytes(4))
				numStep = byteToIntLE(super().readBytes(4))
				numIter = byteToIntLE(super().readBytes(4))

				super().shiftPtr(4)	# skip footer
				#footer = byteToIntLE(super().readBytes(4))


				# Domain Values: packet type = 17
				if typeNum != 17:
					#print("SKIP: ", typeNum)
					self._skipPackContent(numLines)
				else:
					meta = self._getDomainMeta()
					domain = _normStr(meta[1])
					step = meta[4]
					
					if not domain in self.map:
						self.map[domain] = dict()

					self.map[domain][step] = super().getCurPos()

					self._skipPackContent(numLines)

			except EOBException:
				break

	def _getDomainMeta(self):
		savepos = super().getCurPos()

		meta = list()

		try:
			lenRec = byteToIntLE(super().readBytes(4))

			numDomains = byteToIntLE(super().readBytes(4))
			domainID = super().readBytes(24).decode()
			stname = super().readBytes(8).decode()
			lsldnm = super().readBytes(8).decode()
			ltmstp = byteToIntLE(super().readBytes(4))

			super().shiftPtr(4)	# skip footer

		except EOBException:
			raise DomainFormatError

		super().moveTo(savepos)

		meta.append(numDomains)
		meta.append(domainID)
		meta.append(stname)
		meta.append(lsldnm)
		meta.append(ltmstp)

		return meta

	def _readDomainVal(self, pos):
		savePos = super().getCurPos()
		super().moveTo(pos)

		jVal = DomainValues() 

		## Read Meta Data
		try:
			lenRec = byteToIntLE(super().readBytes(4))

			numDomains = byteToIntLE(super().readBytes(4))
			domainID = _normStr(super().readBytes(24).decode())
			stname = _normStr(super().readBytes(8).decode())
			lsldnm = _normStr(super().readBytes(8).decode())
			ltmstp = byteToIntLE(super().readBytes(4))

			super().shiftPtr(4)	# skip footer

		except EOBException:
			raise DomainFormatError

		jVal.numDomains = numDomains
		jVal.domainID = domainID
		jVal.stname = stname
		jVal.lsldnm = lsldnm
		jVal.ltmstp = ltmstp


		## Read J-integral values for domain(s)
		nList = list()
		domains = list()
		skList = list()
		try:
			for i in range(1, numDomains+1):
				lenRec = byteToIntLE(super().readBytes(4))

				nowring = byteToIntLE(super().readBytes(4))
				diterms = struct.unpack("<ddddddddd", super().readBytes(72))

				skippedKilled = byteToIntLE(super().readBytes(4))

				nList.append(nowring)
				domains.append(diterms)
				skList.append(skippedKilled)

				super().shiftPtr(4)	# skip footer

		except EOBException:
			raise DomainFormatError

		jVal.nList = nList
		jVal.domains = domains
		jVal.skList = skList


		## Read max, min and average values
		if numDomains > 1:
			try:
				lenRec = byteToIntLE(super().readBytes(4))

				diAvg = byteToDoubleLE(super().readBytes(8))
				diMin = byteToDoubleLE(super().readBytes(8))
				diMax = byteToDoubleLE(super().readBytes(8))

				super().shiftPtr(4)	# skip footer

			except EOBException:
				raise DomainFormatError

			jVal.diAvg = diAvg
			jVal.diMin = diMin
			jVal.diMax = diMax



		## Read stress intensity factor values computed form J-values
		nList2 = list()
		domains2 = list()
		try:
			for i in range(1, numDomains+1):
				lenRec = byteToIntLE(super().readBytes(4))
				
				nowring2 = byteToIntLE(super().readBytes(4))
				diterms2 = struct.unpack("<ddddd", super().readBytes(40))

				nList2.append(nowring2)
				domains2.append(diterms2)

				super().shiftPtr(4)	# skip footer

		except EOBException:
			raise DomainFormatError

		jVal.nList2 = nList2
		jVal.domains2 = domains2



		
		super().moveTo(savePos)
			
		
		return jVal

		
	
	def _skipPackContent(self, numLines):
		for i in range(numLines):
			lenRec = byteToIntLE(super().readBytes(4))
			super().shiftPtr(lenRec)
			super().shiftPtr(4)	# skip footer

