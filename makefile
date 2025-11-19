all:
	make clean
	make build
	make run

build:
	gcc chash.c -o "chash"

run:
	./chash

clean:
	-rm chash


debug:
	make clean
	-rm -r chash.dSYM
	gcc -g chash.c -o chash
	lldb chash
	run

