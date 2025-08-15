#include <cassert>
#include <cstdio>
#include <format>
#include <SystemConfiguration/SystemConfiguration.h>
#include "eth.h"

namespace eth {

Eth::Eth(const char *name, const char *iface) {
    // adapted from https://github.com/wireshark/wireshark/blob/master/capture/capture-pcap-util.c
    std::array<char, 256> iface_buf, name_buf;
    if (iface == nullptr) {
        auto interfaces = SCNetworkInterfaceCopyAll();
        if (interfaces == nullptr) {
            throw std::runtime_error("SCNetworkInterfaceCopyAll() error");
        }
        auto interfaces_num = CFArrayGetCount(interfaces);
        for (auto i = 0; i < interfaces_num; i++) {
            auto interface = static_cast<SCNetworkInterfaceRef>(CFArrayGetValueAtIndex(interfaces, i));
            auto bsdname = SCNetworkInterfaceGetBSDName(interface);
            auto friendlyname = SCNetworkInterfaceGetLocalizedDisplayName(interface);
            if ((bsdname == nullptr) || (friendlyname == nullptr)) {
                continue;
            }
            if (!CFStringGetCString(bsdname, iface_buf.data(), iface_buf.size(), kCFStringEncodingUTF8) ||
                !CFStringGetCString(friendlyname, name_buf.data(), name_buf.size(), kCFStringEncodingUTF8)) {
                continue;
            }
            if (std::strstr(name_buf.data(), name) != nullptr) {
                iface = iface_buf.data();
                break;
            }
        }
    }
    if (iface == nullptr) {
        std::printf("no matching hardware port, check \"networksetup -listallhardwareports\"\r\n");
        throw std::runtime_error("no interfaces match");
    }    

    // match correct device
    pcap_if_t *all_devs;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&all_devs, errbuf) != 0) {
        throw std::runtime_error(std::format("pcap_findalldevs() error: {}", errbuf));
    }
    pcap_t *found_dev = nullptr, *found_tx_dev = nullptr;
    for (pcap_if_t *d = all_devs; d; d = d->next) {
        if (std::strcmp(d->name, iface) == 0) {
            found_dev = pcap_create(d->name, errbuf);
            if (found_dev == nullptr) {
                throw std::runtime_error(std::format("pcap_create() error: {}", errbuf));
            }
            if ((pcap_set_snaplen(found_dev, 65535) != 0) ||
                (pcap_set_timeout(found_dev, 100) != 0) ||
                (pcap_set_immediate_mode(found_dev, 1) != 0) ||
                (pcap_activate(found_dev) != 0)) {
                throw std::runtime_error("pcap_set_*() or pcap_activate() error");
            }
            if (pcap_setdirection(found_dev, PCAP_D_IN) != 0) {
                throw std::runtime_error(std::format("pcap_setdirection() error"));
            }
            found_tx_dev = pcap_open_live(d->name, 65535, 0, 0, errbuf);
            if (found_tx_dev == nullptr) {
                throw std::runtime_error(std::format("pcap_open_live() for tx error: {}", errbuf));
            }
            break;
        }
    }
    pcap_freealldevs(all_devs);
    if ((found_dev == nullptr) || (found_tx_dev == nullptr)) {
        throw std::runtime_error("no interfaces match");
    }

    // add rx thread
    _dev = found_dev;
    _tx_dev = found_tx_dev;
    _rx.run = true;
    _rx.thread = std::thread([this]() {
        while (_rx.run) {
            struct pcap_pkthdr hdr;
            const uint8_t *raw = pcap_next(_dev, &hdr);
            if ((raw != nullptr) && (hdr.caplen == hdr.len) &&
                (hdr.len >= Packet::HDR_LEN) && (hdr.len <= (Packet::HDR_LEN + Packet::MTU))) {
                auto pkt = pkt_alloc();
                std::memcpy(pkt->raw().data(), raw, hdr.len);
                pkt->set_len(hdr.len - Packet::HDR_LEN);

                bool taken = false;
                _rx.cb_lock.lock();
                auto &[cb, arg] = _rx.cb;
                if (cb && cb(pkt, arg)) {
                    taken = true;
                }
                _rx.cb_lock.unlock();
                if (!taken) {
                    pkt_free(pkt);
                }
            }
        }
    });
}

Eth::~Eth() {
    _rx.run = false;
    _rx.thread.join();
    pcap_close(_dev);
    pcap_close(_tx_dev);
}

Packet *Eth::pkt_alloc(bool wait) {
    assert(wait);
    return new Packet();
}

void Eth::pkt_free(Packet *pkt) {
    delete pkt;
}

void Eth::set_tx_cb(tx_callback cb) {
    std::scoped_lock lock(_tx.lock);
    _tx.cb = cb;
}

void Eth::set_rx_cb(rx_callback cb, void *arg) {
    std::scoped_lock lock(_rx.cb_lock);
    _rx.cb = {cb, arg};
}

bool Eth::send(Packet *pkt, bool wait) {
    assert(wait);
    std::scoped_lock lock(_tx.lock);
    auto raw = pkt->raw();
    pcap_sendpacket(_tx_dev, raw.data(), raw.size() - 4); // excludes FCS
    pkt_free(pkt);
    if (_tx.cb) {
        _tx.cb();
    }
    return true;
}

};
