CC=g++
CXXFLAGS=-g -std=c++17 -I/usr/include/x86_64-linux-gnu/dpdk -I/usr/include/dpdk
LDFLAGS=-L/usr/lib/x86_64-linux-gnu -lrte_eal -lrte_ring -pthread -ldl -lm

SPDK_INC=/root/spdk/include
SPDK_LIB=/root/spdk/build/lib
SPDK_LDFLAGS=-L$(SPDK_LIB) -lspdk_env_dpdk -lspdk_nvme

DOCKER_CONTAINER?=hft_demo
WORKDIR?=/root/project

.PHONY: all clean md-gen oms-sor matcher logger
all: md-gen oms-sor matcher logger

md-gen:
	docker exec $(DOCKER_CONTAINER) bash -lc "cd $(WORKDIR) && \
	$(CC) md-gen/main.cpp $(CXXFLAGS) $(LDFLAGS) -o md-gen/md-gen -Wl,-rpath,/usr/lib/x86_64-linux-gnu"

oms-sor:
	docker exec $(DOCKER_CONTAINER) bash -lc "cd $(WORKDIR) && \
	$(CC) oms-sor/main.cpp $(CXXFLAGS) $(LDFLAGS) -o oms-sor/oms-sor -Wl,-rpath,/usr/lib/x86_64-linux-gnu"

matcher:
	docker exec $(DOCKER_CONTAINER) bash -lc "cd $(WORKDIR) && \
	$(CC) matcher/main.cpp $(CXXFLAGS) $(LDFLAGS) -o matcher/matcher -Wl,-rpath,/usr/lib/x86_64-linux-gnu"

logger:
	docker exec $(DOCKER_CONTAINER) bash -lc "cd $(WORKDIR) && \
	$(CC) logger/main.cpp -I$(SPDK_INC) $(SPDK_LDFLAGS) -o logger/logger"

clean:
	docker exec $(DOCKER_CONTAINER) bash -lc "cd $(WORKDIR) && rm -f md-gen/md-gen oms-sor/oms-sor matcher/matcher logger/logger"
