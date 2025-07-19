#include "pdn_connection.h"

std::shared_ptr<pdn_connection> pdn_connection::create(uint32_t cp_teid, boost::asio::ip::address_v4 apn_gw, boost::asio::ip::address_v4 ue_ip_addr) {
    return std::shared_ptr<pdn_connection>(new pdn_connection(cp_teid, apn_gw, ue_ip_addr));
}

pdn_connection::pdn_connection(uint32_t cp_teid, boost::asio::ip::address_v4 apn_gw, boost::asio::ip::address_v4 ue_ip_addr)
    : _apn_gateway(apn_gw),
    _ue_ip_addr(ue_ip_addr),
    _cp_teid(cp_teid),
    _sgw_cp_teid(0),
    _sgw_address(),
    _bearers(),
    _default_bearer(nullptr) {}

uint32_t pdn_connection::get_cp_teid() const { return _cp_teid; }
boost::asio::ip::address_v4 pdn_connection::get_apn_gw() const { return _apn_gateway; }
boost::asio::ip::address_v4 pdn_connection::get_ue_ip_addr() const { return _ue_ip_addr; }

uint32_t pdn_connection::get_sgw_cp_teid() const { return _sgw_cp_teid; }
void pdn_connection::set_sgw_cp_teid(uint32_t sgw_cp_teid) { _sgw_cp_teid = sgw_cp_teid; }

std::shared_ptr<bearer> pdn_connection::get_default_bearer() const { return _default_bearer; }
void pdn_connection::set_default_bearer(std::shared_ptr<bearer> bearer) { _default_bearer = bearer; }

boost::asio::ip::address_v4 pdn_connection::get_sgw_address() const { return _sgw_address; }
void pdn_connection::set_sgw_addr(boost::asio::ip::address_v4 sgw_addr) { _sgw_address = sgw_addr; }

void pdn_connection::add_bearer(std::shared_ptr<bearer> bearer) {
    _bearers[bearer->get_dp_teid()] = bearer;
}

void pdn_connection::remove_bearer(uint32_t dp_teid) {
    _bearers.erase(dp_teid);
    if (_default_bearer && _default_bearer->get_dp_teid() == dp_teid) {
        _default_bearer = nullptr;
    }
}