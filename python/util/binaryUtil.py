# Utilities for byte type data 

import struct

# tool
def byteToIntLE(bData):
	return int.from_bytes(bData, "little")

def byteToDoubleLE(bData):
	return struct.unpack("<d", bData)[0]

def byteToFloatLE(bData):
	return struct.unpack("<f", bData)[0]

class EOBException(Exception):
	def __init__(self):
		super().__init__("End of Byte Data")


class BinaryFileReader:
	def __init__(self, bFile):
		self.rawData = None
		self.cPtr = 0

		with open(bFile, "rb") as f:
			self.rawData = f.read()
		
	def readBytes(self, size):
		if self.cPtr + size > len(self.rawData):
			raise EOBException

		readData = self.rawData[self.cPtr:self.cPtr+size]
		self.cPtr += size
		return readData
	
	def moveTo(self, ptr):
		if ptr > len(self.rawData):
			self.cPtr = len(self.rawData)
		else:
			self.cPtr = ptr
		
	def shiftPtr(self, size):
		if self.cPtr > len(self.rawData):
			raise EOBException
		
		self.cPtr += size
	
	def getCurPos(self):
		return self.cPtr
		
	def extreactData(self, startPos, size):
		save = self.getCurPos()

		self.moveTo(startPos)
		data = self.readBytes(size)

		self.moveTo(save)

		return data
	
	def eos(self):
		return self.cPtr >= len(self.rawData)
	
