all: smsh1.c splitline.c execute.c
	gcc -o smsh1 smsh1.c splitline.c execute.c -std=c99
	./smsh1


part1: 
	gcc -o smsh2 smsh2.c splitline.c execute.c -std=c99
	
part2: 
	gcc -o smsh3 smsh3.c splitline.c execute.c -std=c99


part3: 
	gcc -o smsh4 smsh4.c splitline.c execute.c -std=c99
	
clean:
	rm -f part1 part2 part all
