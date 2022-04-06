NAME ?= ns3
NS3_DOCKER_TAG ?= 3.0

all: build start

.PHONY: build start
build:
	docker build \
		--tag="hygorjardim/ns3:$(NS3_DOCKER_TAG)" \
		ns3-docker

start:
	docker run --rm -ti --privileged --network=host -v $(PWD)/data:/usr/ns3/ns-3.30/scratch -p 5060:5060 --name $(NAME) hygorjardim/ns3:$(NS3_DOCKER_TAG)
