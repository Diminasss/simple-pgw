#pragma once

#include <pdn_connection.h>

#include <boost/asio/ip/address.hpp>

#include <memory>

class control_plane {
public:
    std::shared_ptr<pdn_connection> find_pdn_by_cp_teid(uint32_t cp_teid) const;

    std::shared_ptr<pdn_connection> find_pdn_by_ip_address(const boost::asio::ip::address_v4 &ip) const;

    std::shared_ptr<bearer> find_bearer_by_dp_teid(uint32_t dp_teid) const;

    std::shared_ptr<pdn_connection> create_pdn_connection(const std::string &apn, boost::asio::ip::address_v4 sgw_addr,
                                                          uint32_t sgw_cp_teid);
    void delete_pdn_connection(uint32_t cp_teid);

    std::shared_ptr<bearer> create_bearer(const std::shared_ptr<pdn_connection> &pdn, uint32_t sgw_teid);

    void delete_bearer(uint32_t dp_teid);

    void add_apn(std::string apn_name, boost::asio::ip::address_v4 apn_gateway);

private:
    std::unordered_map<uint32_t, std::shared_ptr<pdn_connection>> _pdns;
    std::unordered_map<boost::asio::ip::address_v4, std::shared_ptr<pdn_connection>> _pdns_by_ue_ip_addr;
    std::unordered_map<uint32_t, std::shared_ptr<bearer>> _bearers;
    std::unordered_map<std::string, boost::asio::ip::address_v4> _apns;
    uint32_t _next_cp_teid = 1;
    uint32_t _next_dp_teid = 1;
    boost::asio::ip::address_v4 _next_ue_ip = boost::asio::ip::make_address_v4("10.0.0.1");
};