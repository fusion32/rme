# Convert all .png files in the current working directory to byte arrays for embedding
# into a C++ project.

import glob, os, re

header_file = open("pngfiles.h", "wb")
source_file = open("pngfiles.cpp", "wb")

header_file.write(b"#ifndef RME_PNGFILES_H_\n")
header_file.write(b"#define RME_PNGFILES_H_\n")

r = re.compile("^([a-zA-Z_][a-zA-Z_0-9]*)$")
for path in sorted(glob.glob("*.png")):
	# Only allow filenames that can be used as C variables.
	filename = os.path.basename(path)
	basename, ext = os.path.splitext(os.path.basename(path))
	if not r.match(basename):
		print(f"Skipped file (unsuitable filename): {filename}")
		continue

	# Load PNG file as byte array
	data = open(path, "rb").read()

	# Output to header
	header_file.write(f"extern unsigned char {basename}_png[{len(data)}];\n".encode("utf-8"))

	# Output to source
	tmp = []
	tmp.append(f"/* {filename} - {len(data)} bytes */\n")
	tmp.append(f"unsigned char {basename}_png[{len(data)}] = {{")
	for i, b in enumerate(data):
		if (i % 8) == 0:
			tmp.append(f"\n  0x{b:02x},")
		else:
			tmp.append(f" 0x{b:02x},")
	tmp.append("\n")
	tmp.append("};\n")
	source_file.write("".join(tmp).encode("utf-8"))

header_file.write(b"#endif //RME_PNGFILES_H_")
source_file.write(b"\n")

