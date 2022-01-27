image 	:= "blang/latex:ubuntu"
pwd 	:= $(shell pwd)
uid 	:= $(shell id -u)
gid 	:= $(shell id -g)

all: clean
	docker run --rm -i --user="$(uid):$(gid)" --net=none -v $(pwd):/data $(image) xelatex main.tex

clean:
	rm -f main.aux main.log main.pdf main.out
