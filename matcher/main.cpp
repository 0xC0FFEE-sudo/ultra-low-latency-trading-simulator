#include <iostream>
#include <rte_eal.h>
#include <rte_ring.h>
#include <rte_lcore.h>
#include <cstring>
#include <cstdlib>
#include <getopt.h>

struct match_ctx { struct rte_ring* in; struct rte_ring* out; };

static int match_loop(void *arg) {
    auto ctx = (match_ctx*)arg;
    void *pkt;
    while (true) {
        if (rte_ring_dequeue(ctx->in, &pkt) == 0) {
            if (rte_ring_enqueue(ctx->out, pkt) < 0) {
                std::cerr << "matcher: drop fill" << std::endl;
            }
        }
    }
    return 0;
}

static void usage() {
    std::cerr << "Usage: matcher [-i in_ring] [-o out_ring] [-t threads]\n";
    exit(1);
}

int main(int argc, char** argv) {
    const char* in_name = "ORDER_RING";
    const char* out_name = "FILL_RING";
    int threads = 1;
    int opt;
    while ((opt = getopt(argc, argv, "i:o:t:")) != -1) {
        switch (opt) {
            case 'i': in_name = optarg; break;
            case 'o': out_name = optarg; break;
            case 't': threads = atoi(optarg); break;
            default: usage();
        }
    }
    if (rte_eal_init(argc, argv) < 0) {
        std::cerr << "matcher: EAL init failed" << std::endl;
        return -1;
    }
    struct rte_ring* in_ring = rte_ring_lookup(in_name);
    if (!in_ring) {
        std::cerr << "matcher: input ring not found" << std::endl;
        return -1;
    }
    struct rte_ring* out_ring = rte_ring_create(out_name, 1024, rte_socket_id(), 0);
    if (!out_ring) {
        std::cerr << "matcher: failed to create output ring" << std::endl;
        return -1;
    }
    match_ctx ctx{in_ring, out_ring};
    unsigned next_lcore = -1;
    for (int i = 0; i < threads; ++i) {
        next_lcore = rte_get_next_lcore(next_lcore, 1, 0);
        if (next_lcore >= RTE_MAX_LCORE) break;
        rte_eal_remote_launch(match_loop, &ctx, next_lcore);
    }
    rte_eal_mp_wait_lcore();
    rte_eal_cleanup();
    return 0;
}
