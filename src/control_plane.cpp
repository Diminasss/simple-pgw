#include "control_plane.h"
#include <stdexcept>

std::shared_ptr<pdn_connection> control_plane::find_pdn_by_cp_teid(uint32_t cp_teid) const {
    auto it = _pdns.find(cp_teid);
    if (it != _pdns.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<pdn_connection> control_plane::find_pdn_by_ip_address(const boost::asio::ip::address_v4 &ip) const {
    auto it = _pdns_by_ue_ip_addr.find(ip);
    if (it != _pdns_by_ue_ip_addr.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<bearer> control_plane::find_bearer_by_dp_teid(uint32_t dp_teid) const {
    auto it = _bearers.find(dp_teid);
    if (it != _bearers.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<pdn_connection> control_plane::create_pdn_connection(const std::string &apn, boost::asio::ip::address_v4 sgw_addr, uint32_t sgw_cp_teid) {
    auto apn_it = _apns.find(apn);
    if (apn_it == _apns.end()) {
        throw std::runtime_error("APN not found");
    }
    boost::asio::ip::address_v4 apn_gw = apn_it->second;

    // Используем sgw_cp_teid как cp_teid
    uint32_t cp_teid = sgw_cp_teid;
    // Генерируем UE IP-адрес из фиксированного диапазона
    static uint32_t ip_counter = 1;
    boost::asio::ip::address_v4 ue_ip = boost::asio::ip::make_address_v4("10.0.0." + std::to_string(ip_counter++));

    auto pdn = pdn_connection::create(cp_teid, apn_gw, ue_ip);
    pdn->set_sgw_addr(sgw_addr);
    pdn->set_sgw_cp_teid(sgw_cp_teid);

    _pdns[cp_teid] = pdn;
    _pdns_by_ue_ip_addr[ue_ip] = pdn;

    return pdn;
}

void control_plane::delete_pdn_connection(uint32_t cp_teid) {
    auto pdn_it = _pdns.find(cp_teid);
    if (pdn_it != _pdns.end()) {
        auto pdn = pdn_it->second;
        _pdns_by_ue_ip_addr.erase(pdn->get_ue_ip_addr());
        for (const auto &bearer_pair : pdn->_bearers) {
            _bearers.erase(bearer_pair.first);
        }
        _pdns.erase(pdn_it);
    }
}

std::shared_ptr<bearer> control_plane::create_bearer(const std::shared_ptr<pdn_connection> &pdn, uint32_t sgw_teid) {
    // Используем sgw_teid как dp_teid
    uint32_t dp_teid = sgw_teid;
    std::shared_ptr<bearer> bearer(new ::bearer(dp_teid, *pdn));
    bearer->set_sgw_dp_teid(sgw_teid);
    pdn->add_bearer(bearer);
    _bearers[dp_teid] = bearer;
    return bearer;
}

void control_plane::delete_bearer(uint32_t dp_teid) {
    auto bearer_it = _bearers.find(dp_teid);
    if (bearer_it != _bearers.end()) {
        auto bearer = bearer_it->second;
        auto pdn = bearer->get_pdn_connection();
        if (pdn) {
            pdn->remove_bearer(dp_teid);
        }
        _bearers.erase(bearer_it);
    }
}

void control_plane::add_apn(std::string apn_name, boost::asio::ip::address_v4 apn_gateway) {
    _apns[apn_name] = apn_gateway;
}