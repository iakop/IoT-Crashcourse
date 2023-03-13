#!/bin/python

import sys, getopt

usagetext = "Usage:\n\tbeamercode.py -t \"Title\" -a <true/false> [inputfile] [outputfile]"
helptext = ("Options:\n\t"
			"-h, --help: Prints this help text\n\t"
			"-t, --title: Title for the frames\n\t"
			"-a, --annotate: Output the frames with tikz formatted comments\n\t"
			)

def main(argv):
	global usagetext
	global helptext
	annotate = False

	inputfile = ''
	outputfile = ''
	title = ''
	linethresh = 20 # Threshhold for when to change frame
	lineslack = 5 # Slack for when either keeping or losing lines on frame

	# Define options and args:
	try:
		opts, args = getopt.getopt(argv,"ht:a",["help","title","annotate"])
	except getopt.GetoptError:
		print(usagetext)
		sys.exit(2)

	# Get options:
	for opt, arg in opts:
		if opt in ("-h", "--help"):
			print(usagetext)
			print(helptext)
			sys.exit(0)
		elif opt in ("-t", "--title"):
			title = arg
		elif opt in ("-a", "--annotate"):
			annotate = True

	# Get input and output files:
	try:
		inputfile = args[0]
		outputfile = args[1]
	except:
		print(usagetext)
		sys.exit(2)

	infile = open(inputfile, "r")
	outfile = open(outputfile, "w")

	# To get the best legibility, any code "blocks" are favored
	# Block is defined with any consecutive lines
	# Whitespace is removed from the equation
	blocks = []
	blocks.append(block())
	block_idx = 0
	for linecount, line in enumerate(infile):
		if line.strip(): # If not blank line
			blocks[block_idx].content.append(line);
		else:
			if blocks[block_idx].content: # If the current working block isn't empty:
				block_idx += 1
				blocks.append(block())

	# If tikz annotation is enabled, the frame will have half for code, half for annotation
	# Comments are moved to annotation
	###if cmt = line.find("//"):
	# Consecutive comments become part of the same annotation.

	# Generate frames:
	frames = []
	frames.append(frame())
	frame_idx = 0
	prevLines = 0
	lines = 0
	totalLines = 0
	frames[frame_idx].startln = totalLines
	prevWhitespace = 0
	whitespace = 0
	for b in blocks:
		lines += len(b.content)

		if lines + whitespace < linethresh:
			frames[frame_idx].blocks.append(b)
			prevLines = lines
			prevWhitespace = whitespace
			whitespace += 1
		elif (lines + whitespace > linethresh) and (lines + whitespace < (linethresh + lineslack)):
			frames[frame_idx].blocks.append(b)
			frame_idx += 1
			frames.append(frame())
			totalLines += lines + whitespace
			frames[frame_idx].startln = totalLines
			lines = 0
			prevLines = 0
			prevWhitespace = 0
			whitespace = 0
		elif (lines + whitespace > linethresh) and (prevLines + prevWhitespace > (linethresh - lineslack)):
			frame_idx += 1
			frames.append(frame())
			lines = len(b.content)
			lines = 0
			totalLines += prevLines + prevWhitespace
			prevLines = 0
			prevWhitespace = 0
			whitespace = 0
			frames[frame_idx].blocks.append(b)
			frames[frame_idx].startln = totalLines
		else: # Split the block lines into more blocks
			# Find needed number of lines left
			linesToUse = (linethresh - prevLines)
			# Create a new block from just the needed lines:
			tmpBlock = block()
			tmpBlock.content.append(b.content[linesToUse:])
			# Insert them into the blocks list:
			blocks.insert(blocks.index(b)+1, tmpBlock)
			# Shorten current block:
			b.content = b.content[0:linesToUse]
			# Append block to frame and increment index
			frames[frame_idx].blocks.append(b)
			frame_idx += 1
			frames.append(frame())
			totalLines += prevLines + linesToUse + whitespace
			frames[frame_idx].startln = totalLines
			lines = 0
			prevLines = 0
			prevWhitespace = 0
			whitespace = 0

	# Thus pages are made up of blocks, with whitespaces inserted again between them
	for f in frames:
		outfile.write("\\begin{frame}[fragile]{" + title + "}\n")
		outfile.write("\\begin{mdframed}\n")
		outfile.write("\\begin{lstlisting}[firstnumber=" + str(f.startln) + "]\n")
		for b in f.blocks:
			for line in b.content:
				outfile.write(line)
			if (f.blocks.index(b) != 0) and (f.blocks.index(b) != (len(f.blocks)-1)):
				outfile.write("\n")
		outfile.write("\\end{lstlisting}\n")
		outfile.write("\\end{mdframed}\n")
		outfile.write("\\end{frame}\n")

class frame:
	def __init__(self):
		self.blocks = [] # Code blocks in the frame
		self.startln = []
		self.endln = []

	#def generate(self):

class block:
	def __init__(self):
		self.content = [] # Lines in the code block
		self.comments = [] # List of comments on the block

	def extractComments(self):
		pass # For now does nothing

class comment:
	def __init__(self):
		# Start and end lines WITHIN THE BLOCK
		self.lines
		self.startln
		self.endln

if __name__ == "__main__":
	main(sys.argv[1:])

#f = open("demofile.txt", "rt")