#!/usr/bin/env python3

# Usage: From `firmware` directory execute:
# ./resources/pixelart/png2code.py > ./src/function_pixelart_data.h

from PIL import Image
import os

imgcount = 0
indextable = "static const uint32_t pixelart_indices[] = {\n"
indextable += "\t// format: offset, width, height\n"
datatable = "static const uint8_t pixelart_data[] ICACHE_RODATA_ATTR = {"
offset = 0

imgpath = os.path.dirname(os.path.realpath(__file__))
for filename in os.listdir(imgpath):
	if filename.endswith(".png"):
		im = Image.open(os.path.join(imgpath, filename))
		imgcount += 1
		pix = im.load()
		width = im.size[0]
		height = im.size[1]

		indextable += "\t// filename: %s, size: %d x %d\n" % (filename, width, height)
		indextable += "\t%d, %d, %d,\n" % (offset, width, height)
		offset += width * height * 3

		datatable += "\n"

		for y in range(0, height):
			datatable += "\t"
			for x in range(0, width):
				red = format(pix[x, y][0], "02x")
				green = format(pix[x, y][1], "02x")
				blue = format(pix[x, y][2], "02x")

				datatable += "0x%s, 0x%s, 0x%s" % (red, green, blue)
				if (x != width - 1):
					datatable += ", "
				else:
					datatable += ","
			datatable += "\n"

indextable = indextable[:-2] + "\n};"
datatable = datatable[:-2] + "\n};"
print("#include <os_type.h>\n")
print("#define PIXELART_IMGCOUNT %d\n" % imgcount)
print(indextable)
print()
print(datatable)
