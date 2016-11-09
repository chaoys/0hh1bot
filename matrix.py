#!/usr/bin/env python  
import pygtk  
import gtk
import subprocess
import time
 
def get_color(pointer_x, pointer_y):  
    """Returns an (R, G, B) tuple at the current pointer location."""  
      
    root_window = gtk.gdk.get_default_root_window()  
    #pointer_x, pointer_y = gtk.gdk.Display(None).get_pointer()[1:3]  
    #print pointer_x, pointer_y  
    pixbuf = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB, False, 8, 1, 1)  
    pixbuf = pixbuf.get_from_drawable(root_window,  
       root_window.get_colormap(), pointer_x, pointer_y, 0, 0, 1, 1)  
    return tuple(map(ord, pixbuf.get_pixels()[:3]))  

minx = 699
miny = 305
maxx = 1287
maxy = 889
# background
bg = (34, 34, 34)
grey = (42, 42, 42)
# there are different RED & BLUE colors
#blue = (53, 184, 213)
#red = (213, 83, 54)
grey_code = 0
red_code = 1
blue_code = 2
bad_code = 3
# bricks list
bricks = []
seeds = []
row = 0

def is_bg(c):
	return c == bg
def is_grey(c):
	return c == grey
# there are different RED & BLUE colors
def is_red(c):
	return c[0] > 190
def is_blue(c):
	return c[2] > 190
def color_code(c):
	if is_grey(c):
		return grey_code
	elif is_red(c):
		return red_code
	elif is_blue(c):
		return blue_code
	else:
		return bad_code
def color_click(c):
	if c == red_code:
		return 1
	elif c == blue_code:
		return 3
	else:
		print("bad code")


def matrix_scan():
	global row
	global bricks
	global seeds
	y = miny
	while y < maxy:
		last_row = -1
		last_col = -1
		in_bg = 0
		col = 0
		for x in range(minx, maxx):
			c = get_color(x, y)
			if is_bg(c):
				if in_bg == 0:
#					print("new col(%d %d)" % (x, y))
					col = col + 1
					in_bg = 1
				continue
			in_bg = 0
			if last_row == row and last_col == col:
#				print("same brick")
				continue
			# ignore noisy color
			if is_grey(c) or is_red(c) or is_blue(c):
#				print("new brick(%d %d)" % (x, y))
				brick = (x, y, color_code(c))
				bricks.append(brick)
				last_row = row
				last_col = col
				if is_grey(c):
					pass
#					print("(%d,%d,%d)" % (row, col, grey_code))
				elif is_red(c):
					seeds.append((row, col, red_code))
#					print("(%d,%d,%d)" % (row, col, red_code))
				elif is_blue(c):
					seeds.append((row, col, blue_code))
#					print("(%d,%d,%d)" % (row, col, blue_code))

		# find next row
		y = y + 1
		while y < maxy:
			c = get_color(minx, y)
			if c != bg:
				y = y + 1
				continue
#			print("new row(%d %d)" % (minx, y))
			row = row + 1
			break
		y = y + 1
		# skip row seperator
		while y < maxy:
			c = get_color(minx, y)
			if is_grey(c) or is_red(c) or is_blue(c):
#				print("new row at(%d %d)" % (minx, y))
				break
			y = y + 1
		# to avoid corner case
		y = y + 1

def change_color(x, y, c, n):
	print("%d %d %d %d" % (n, n/row, n%row, c))
	cmd = "xdotool mousemove %d %d click %d" % (x, y, color_click(c))
	print(cmd)
	time.sleep(0.05)
	pp = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
	
matrix_scan()

row = row + 1

seeds_str = str(seeds).replace("[","(").replace("]",")").replace(" ","").replace("),",")")
#print(seeds_str)

cmd = "./a.out %d %s" % (row, seeds_str.replace("(","\(").replace(")","\)"))
pp = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
res = pp.stdout.read().strip()
print(res)
#print('========')

num = 0
for c in map(int, res):
	brick = bricks.pop(0)
	if c != brick[2]:
		change_color(brick[0], brick[1], c, num)
	num = num + 1
