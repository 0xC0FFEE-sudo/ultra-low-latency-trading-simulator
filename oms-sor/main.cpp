#include <iostream>
#include <cstring>
#include <cstdlib>
#include <getopt.h>
#include <rte_eal.h>
#include <rte_ring.h>
#include <rte_lcore.h>

struct oms_ctx { struct rte_ring* md_ring; struct rte_ring* ord_ring; double risk_limit; };
static double g_pnl = 0.0;

static int process_loop(void *arg) {
    auto ctx = (oms_ctx*)arg;
    void *feed;
    while (true) {
        if (rte_ring_dequeue(ctx->md_ring, &feed) == 0) {
            int price = 0, qty = 0;
            char sym[16];
            sscanf((char*)feed, "QUOTE,%15[^,],%d,%d", sym, &price, &qty);
            double order_val = price * qty;
            if (g_pnl + order_val > ctx->risk_limit) {
                std::cerr << "oms-sor: risk limit breached, PnL=" << g_pnl << std::endl;
                rte_eal_mp_wait_lcore();
                exit(1);
            }
            g_pnl += order_val;
            if (rte_ring_enqueue(ctx->ord_ring, feed) < 0) {
                std::cerr << "oms-sor: drop order" << std::endl;
            }
        }
    }
    return 0;
}

static void usage() {
    std::cerr << "Usage: oms-sor [-m md_ring] [-o ord_ring] [-l risk_limit]\n";
    exit(1);
}

int main(int argc, char** argv) {
    const char* md_name = "MD_RING";
    const char* ord_name = "ORDER_RING";
    double risk_limit = 100000.0;
    int opt;
    while ((opt = getopt(argc, argv, "m:o:l:")) != -1) {
        switch (opt) {
            case 'm': md_name = optarg; break;
            case 'o': ord_name = optarg; break;
            case 'l': risk_limit = atof(optarg); break;
            default: usage();
        }
    }
    if (rte_eal_init(argc, argv) < 0) {
        std::cerr << "oms-sor: EAL init failed" << std::endl;
        return -1;
    }
    struct rte_ring* md_ring = rte_ring_lookup(md_name);
    if (!md_ring) {
        std::cerr << "oms-sor: MD_RING not found" << std::endl;
        return -1;
    }
    struct rte_ring* ord_ring = rte_ring_create(ord_name, 1024, rte_socket_id(), 0);
    if (!ord_ring) {
        std::cerr << "oms-sor: failed to create ORDER_RING" << std::endl;
        return -1;
    }
    oms_ctx ctx{md_ring, ord_ring, risk_limit};
    unsigned lcore_id = rte_get_next_lcore(-1, 1, 0);
    rte_eal_remote_launch(process_loop, &ctx, lcore_id);
    rte_eal_mp_wait_lcore();
    rte_eal_cleanup();
    return 0;
}
