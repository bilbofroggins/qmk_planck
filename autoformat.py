from itertools import zip_longest

def reconfigure(final_rows):
	# [[0, 1, 2], [s0, s1, s2], [f0, f1]]
	last_row_map = {
		0: [0],
		1: [1],
		2: [2],
		3: [3],
		4: [4,5],
		5: [6,7],
		6: [8],
		7: [9],
		8: [10],
		9: [11]
	}
	last_row_map_r = {
		0: 0,
		1: 1,
		2: 2,
		3: 3,
		4: 4,
		5: None,
		6: 5,
		7: None,
		8: 6,
		9: 7,
		10: 8,
		11: 9
	}

	paddings = []
	cols = list([list(c) for c in zip_longest(*final_rows)])
	for c in range(len(cols)):
		lasti = last_row_map_r[c]
		lastone = cols[lasti][4] if lasti is not None else ''

		paddings.append(max(len(lastone), *[len(x) for x in cols[c][:4]] ))

		for i in range(4):
			cols[c][i] = cols[c][i].ljust(paddings[c])

	ret = list([list(r) for r in zip_longest(*cols)])
	del ret[4][-1]
	del ret[4][-1]
	for i in range(len(ret[4])):
		keys = last_row_map[i]
		pad = 0
		for key in keys:
			pad += paddings[key]
			pad += 2
		pad -= 2

		ret[4][i] = ret[4][i].ljust(pad)

	return ret


def autoformat():
	filename = 'keymap.c'
	with open(filename, 'r') as file:
		lines = file.readlines()

	new_lines = []
	keymaps_seen = False
	i = 0
	while i < len(lines):
		line = lines[i].strip('\n')
		if not keymaps_seen:
			new_lines.append(line + '\n')
			if 'LAYOUT_planck_2x2u' in line:
				keymaps_seen = True
			i += 1
			continue

		keymaps_seen = False
		final_rows = []
		auto_rows = lines[i: i + 5]
		for r in range(len(auto_rows)):
			values = auto_rows[r].split(',')
			auto_rows[r] = values
			for c in range(len(values)):
				auto_rows[r][c] = auto_rows[r][c].strip()
			auto_rows[r] = ','.join(auto_rows[r])

		for row in auto_rows:
			values = row.split(',')
			stack = []
			stack_ci = 0
			for c in range(len(values)):
				for char_i in range(stack_ci, len(values[c])):
					char = values[c][char_i]
					if char == '(':
						stack.append('(')
					elif char == ')':
						stack.pop()
				if stack:
					stack_ci = len(values[c])
					values[c + 1] = values[c] + ',' + values[c + 1]
					values[c] = ''
				else:
					stack_ci = 0

			new_values = filter(None, values)
			final_rows.append(new_values)

		final_rows = reconfigure(final_rows)
		for row in range(len(final_rows)):
			new_lines.append(', '.join(final_rows[row]) + ',\n')
			i += 1
		new_lines[-1] = new_lines[-1][:-2] + '\n'


	with open(filename, 'w') as file:
		file.writelines(new_lines)


if __name__ == '__main__':
	autoformat()