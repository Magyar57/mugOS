import sys
from PIL import Image

# GenerateFontStruct.py: generate the C struct from a given input image
#
# The input should be in the same format as this example
# https://github.com/epto/epto-fonts/blob/master/font-images/img/ISO88591-8x16.png
# This is the default mugOS font (the image has been processed)
# http://www.mediafire.com/view/23hpqqhkhr22x2x/IBM_VGA_8x16_Font_(PS2_Model_30_BIOS_Original_09-02-86).png

BITMAP_FILEPATH = "IBM-VGA-8x16.bmp"

# Rows and columns of characters
NCHAR_WIDTH = 16
NCHAR_HEIGHT = 16
# Characters dimensions (pixels)
CHAR_WIDTH = 8
CHAR_HEIGHT = 16
# Border size between the characters
BORDER = 1

if __name__=="__main__":

	if len(sys.argv)>1:
		BITMAP_FILEPATH = sys.argv[1]

	print("Treating image", BITMAP_FILEPATH, file=sys.stderr)
	fontImage = Image.open(BITMAP_FILEPATH)
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
					pixel = bitmap[x*(CHAR_WIDTH+BORDER)+i, y*(CHAR_HEIGHT+BORDER)+j]
					pixel = pixel[:3] # keep only colors(ignore transparency value)
					if pixel != (0,0,0):
						# Not black, add the corresponding bit
						curInt += 2**(7-i)
				print(format(curInt, "#04x"), end=", " if (j!=15) else " ")
			print("}," if ((y!=NCHAR_HEIGHT-1) or (x!=NCHAR_WIDTH-1)) else "}")
			x_offset += 1
		y_offset += 1
		x_offset = 0
	print("};")
