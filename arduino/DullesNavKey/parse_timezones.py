import arrow

fn = "timezones.csv"
lines = open(fn).readlines()
for line in lines[1:]:
    line = line.strip()
    line = line.split(',')
    if line[4].strip() != "Deprecated":
        print "%30s" % line[2], arrow.get().to(line[2])
