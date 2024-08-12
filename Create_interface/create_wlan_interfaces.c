#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/nl80211.h>

#define MAX_PAYLOAD 1024

static int create_interface(struct nl_sock *socket, int driver_id, int phy_index, const char *ifname) {
    struct nl_msg *msg;
    int ret;

    msg = nlmsg_alloc();
    if (!msg) {
        fprintf(stderr, "Failed to allocate netlink message.\n");
        return -1;
    }

    printf("Creating interface: %s on phy index: %d\n", ifname, phy_index);

    genlmsg_put(msg, 0, 0, driver_id, 0, 0, NL80211_CMD_NEW_INTERFACE, 0);
    NLA_PUT_STRING(msg, NL80211_ATTR_IFNAME, ifname);
    NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, NL80211_IFTYPE_STATION);
    NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy_index);

    ret = nl_send_auto(socket, msg);
    if (ret < 0) {
        fprintf(stderr, "Failed to send netlink message: %s\n", nl_geterror(ret));
        nlmsg_free(msg);
        return -1;
    }

    printf("Netlink message sent successfully\n");

    struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        fprintf(stderr, "Failed to allocate netlink callbacks\n");
        nlmsg_free(msg);
        return -1;
    }

    int err = nl_recvmsgs_default(socket);
    if (err < 0) {
        fprintf(stderr, "Error receiving netlink message: %s\n", nl_geterror(err));
        nl_cb_put(cb);
        nlmsg_free(msg);
        return -1;
    }

    printf("Netlink message received successfully\n");

    nl_cb_put(cb);
    nlmsg_free(msg);
    return 0;

nla_put_failure:
    fprintf(stderr, "Failed to put netlink attributes\n");
    nlmsg_free(msg);
    return -1;
}

int main() {
    struct nl_sock *socket;
    int driver_id;
   // nl_debug = 1;
   // nl_socket_set_buffer_size(socket, 8192, 8192);
    socket = nl_socket_alloc();
    if (!socket) {
        fprintf(stderr, "Failed to allocate netlink socket.\n");
        return -1;
    }

    if (genl_connect(socket) < 0) {
        fprintf(stderr, "Failed to connect to generic netlink.\n");
        nl_socket_free(socket);
        return -1;
    }

    driver_id = genl_ctrl_resolve(socket, "nl80211");
    if (driver_id < 0) {
        fprintf(stderr, "Failed to resolve nl80211 interface.\n");
        nl_socket_free(socket);
        return -1;
    }
    printf("nl80211 driver ID: %d\n", driver_id);

    if (create_interface(socket, driver_id, 5, "wlan1") < 0) {
    fprintf(stderr, "Failed to create wlan1 interface.\n");
    nl_socket_free(socket);
    return -1;
    }
    
    if (create_interface(socket, driver_id, 5, "wlan2") < 0) {
    fprintf(stderr, "Failed to create wlan1 interface.\n");
    nl_socket_free(socket);
    return -1;
    }
/*
    if (create_interface(socket, driver_id, "phy4", "wlan1") < 0) {
        fprintf(stderr, "Failed to create wlan1 interface.\n");
        nl_socket_free(socket);
        return -1;
    }

    if (create_interface(socket, driver_id, "phy4", "wlan2") < 0) {
        fprintf(stderr, "Failed to create wlan2 interface.\n");
        nl_socket_free(socket);
        return -1;
    }
*/
    printf("Interfaces wlan1 and wlan2 created successfully.\n");

    nl_socket_free(socket);
    return 0;
}
