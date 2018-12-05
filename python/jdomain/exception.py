# Exception about JDomain

class DomainFormantError(Exception):
	def __init__(self):
		super().__init__("Format of domain values is invalid")

