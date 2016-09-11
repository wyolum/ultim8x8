n_row = 16
n_col = 8

def getPixelNum(row, col):
    out = row * n_col
    if row % 2:
        out += n_col - 1 - col
    else:
        out += col
    return out
    return row * n_col + ((row % 2) * n_col + ((row % 2) * -1) * col)
    return row * n_col + (col % 2) * (n_col - col)

for r in range(n_row):
    for c in range(n_col):
        print '%02x' % getPixelNum(r, c),
    print
