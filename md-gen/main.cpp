#include <iostream>
#include <rte_eal.h>
#include <rte_ring.h>
#include <rte_lcore.h>
#include <getopt.h>

struct feed_ctx { struct rte_ring* ring; int interval_us; };

static int feed_loop(void *arg) {
    auto ctx = (feed_ctx*)arg;
    const char* feed = "QUOTE,ABC,100,200";
    while (true) {
        if (rte_ring_enqueue(ctx->ring, (void*)feed) < 0) {
            std::cerr << "md-gen: dropped feed" << std::endl;
        }
        rte_delay_us_sleep(ctx->interval_us);
    }
    return 0;
}

static void usage() {
    std::cerr<<"Usage: md-gen [-r ring_name] [-i interval_us]\n";
    exit(1);
}

int main(int argc, char** argv) {
    const char* ring_name = "MD_RING";
    int interval_us = 1000;
    int opt;
    while ((opt = getopt(argc, argv, "r:i:")) != -1) {
        switch(opt) {
        case 'r': ring_name = optarg; break;
        case 'i': interval_us = atoi(optarg); break;
        default: usage();
        }
    }
    if (rte_eal_init(argc, argv) < 0) {
        std::cerr << "md-gen: EAL init failed" << std::endl;
        return -1;
    }
    struct rte_ring* md_ring = rte_ring_create(ring_name, 1024, rte_socket_id(), 0);
    if (!md_ring) {
        std::cerr << "md-gen: failed to create ring" << std::endl;
        return -1;
    }
    feed_ctx ctx{md_ring, interval_us};
    unsigned lcore_id = rte_get_next_lcore(-1, 1, 0);
    rte_eal_remote_launch(feed_loop, &ctx, lcore_id);
    rte_eal_mp_wait_lcore();
    rte_eal_cleanup();
    return 0;
}
