NAME ?= ns3
NS3_DOCKER_TAG ?= 3.0

all: start

.PHONY: start
start:
	docker run --rm -it \
				--privileged \
				--network=host \
				-v $(PWD)/data:/usr/ns3/ns-3.30/scratch \
				-p 5060:5060 \
				--name $(NAME) hygorjardim/ns3:$(NS3_DOCKER_TAG)
