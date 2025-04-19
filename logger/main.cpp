#include <iostream>
#include <cstring>
#include <cstdint>
#include <spdk/env.h>
#include <spdk/nvme.h>

static struct spdk_nvme_ctrlr *g_ctrlr = nullptr;
static struct spdk_nvme_ns *g_ns = nullptr;
static struct spdk_nvme_qpair *g_qpair = nullptr;

static bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                     struct spdk_nvme_ctrlr_opts *opts) {
    return true;
}

static void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                      struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts) {
    g_ctrlr = ctrlr;
    g_ns = spdk_nvme_ctrlr_get_ns(ctrlr, 1);
}

static void write_complete(void *cb_ctx, const struct spdk_nvme_cpl *completion) {
    // write completion callback
}

int main(int argc, char **argv) {
    struct spdk_env_opts opts;
    spdk_env_opts_init(&opts);
    opts.name = "logger";
    if (spdk_env_init(&opts) < 0) {
        std::cerr << "logger: SPDK env init failed" << std::endl;
        return -1;
    }
    if (spdk_nvme_probe(NULL, NULL, probe_cb, attach_cb, NULL) != 0 || !g_ctrlr) {
        std::cerr << "logger: SPDK probe failed" << std::endl;
        return -1;
    }
    g_qpair = spdk_nvme_ctrlr_alloc_io_qpair(g_ctrlr, NULL, 0);
    if (!g_qpair) {
        std::cerr << "logger: qpair alloc failed" << std::endl;
        return -1;
    }
    const size_t buf_size = 4096;
    void *buf = spdk_dma_malloc(buf_size, buf_size, NULL);
    if (!buf) {
        std::cerr << "logger: DMA malloc failed" << std::endl;
        return -1;
    }
    memset(buf, 0xAB, buf_size);
    uint64_t lba = 0;
    uint32_t lba_count = buf_size / 512;
    while (true) {
        int rc = spdk_nvme_ns_cmd_write(g_ns, g_qpair, buf, lba, lba_count, write_complete, NULL, 0);
        if (rc) {
            std::cerr << "logger: write cmd submit failed: " << rc << std::endl;
        }
        spdk_nvme_qpair_process_completions(g_qpair, 0);
    }
    spdk_nvme_ctrlr_free_io_qpair(g_qpair);
    spdk_env_fini();
    return 0;
}
