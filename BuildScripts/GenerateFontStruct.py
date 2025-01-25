import sys
from PIL import Image

# GenerateFontStruct.py: generate the C struct from a given bitmap image
# The input bitmap should be in the same format as this example
# https://github.com/epto/epto-fonts/blob/master/font-images/img/ISO88591-8x16.png

BIMAP_FILEPATH = "roman-8x16.bmp"

# Rows and columns of characters
NCHAR_WIDTH = 16
NCHAR_HEIGHT = 16
# Characters dimensions (pixels)
CHAR_WIDTH = 8
CHAR_HEIGHT = 16
# Border size between the characters
BORDER = 1

if __name__=="__main__":
	fontImage = Image.open(BIMAP_FILEPATH)
	bitmap = fontImage.load()
	width, height = fontImage.size

	if NCHAR_WIDTH*(CHAR_WIDTH+BORDER) != width or NCHAR_HEIGHT*(CHAR_HEIGHT+BORDER) != height:
		print("Dimensions of the image do not match parameters. Please set the correct ones.", file=sys.stderr)
		exit(1)

	# Iterate over characters
	# offsets are for the borders
	x_offset = 0
	y_offset = 0
	print("{")
	for y in range(NCHAR_HEIGHT):
		for x in range(NCHAR_WIDTH):
			# Iterate over each char's pixels
			print("\t{ ", end="")
			for j in range(CHAR_HEIGHT):
				curInt = 0
				for i in range(CHAR_WIDTH):
					if bitmap[x*(CHAR_WIDTH+1)+i, y*(CHAR_HEIGHT+1)+j] != (0,0,0):
						# Not black, add the corresponding bit
						curInt += 2**(7-i)
					# else:
				print(format(curInt, "#04x"), end=", " if (j!=15) else " ")
					# 	print(" ", end="")
					# print(" ", end="")
			print("}," if ((y!=NCHAR_HEIGHT-1) or (x!=NCHAR_WIDTH-1)) else "}")
			x_offset += 1
		y_offset += 1
		x_offset = 0
	print("};")
