import math

# Read the coded message from file
file = open("../assets/messagecode.txt", "r")
hex_code = file.read().strip()
code = bin(int(hex_code, 16))[2:].zfill(4*len(hex_code))

# Separate each block
blocks = [code[i:i+40] for i in range(0, len(code), 40)]

# number of parity bits
n_p = math.floor(math.log(len(blocks[0]), 2))

flip_bits = []

# iterate over every bloack
for block in blocks:
	data = block[1:-1]
	parity_idx = []

	# get the indexes of bits corresponding each parity bit
	for i in range(n_p+1):
		r_i = []
		max_range = 2**i
		curr_range = 0
		off = max_range+1
		idx = 2**i
		while idx<len(data)+1:
			if curr_range < max_range:
				r_i.append(idx)
				idx += 1
				curr_range += 1
			else:
				curr_range = 0
				idx+=off-1
		# store the bit indexes
		parity_idx.append(r_i)

	parity = ''

	# calculate the parity bits
	for i in range(n_p+1):
		p_bit = False
		for j in parity_idx[i]:
			if data[j-1] == '1':
				p_bit = not p_bit

		if p_bit:
			parity = '1' + parity
		else:
			parity = '0' + parity

	# get the flip bits
	flip_bits.append(int(parity,2))


# flip the flip bits
for i in range(len(blocks)):
	if flip_bits[i] > 0:
		new = list(blocks[i])
		if new[flip_bits[i]] == '1':
			new[flip_bits[i]] = '0'
		else:
			new[flip_bits[i]] = '1'
		blocks[i] = ''.join(new)

block_texts = []

# get indexes of parity bits
R = [2**p for p in range(n_p+1)]

# get the data by removing parity bits
for i in range(len(blocks)):
	block_texts.append("".join([char for idx, char in enumerate(blocks[i][1:-1]) if idx+1 not in set(R)]))

text = ''

# convert the given binary string to ascii string
for block_text in block_texts:
	text_dec = int(block_text, 2)
	text += text_dec.to_bytes((text_dec.bit_length() + 7 // 8), "big").decode()

# OUTPUT
print("Text:", text.lower())
print("Code Word:", hex_code)
print("Bit flip idx:", flip_bits)
print("Num Blocks:", len(blocks))