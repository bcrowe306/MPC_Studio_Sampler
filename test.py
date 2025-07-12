
pitchRange = 5
minHeight = 8

gridHeight = 96-35

cellHeight = int(gridHeight / pitchRange) if gridHeight / pitchRange > minHeight else minHeight
print(cellHeight)
