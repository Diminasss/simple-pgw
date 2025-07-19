#include "bearer.h"
#include "pdn_connection.h"

bearer::bearer(uint32_t dp_teid, pdn_connection &pdn) : _dp_teid(dp_teid), _pdn(pdn) {}

uint32_t bearer::get_sgw_dp_teid() const { return _sgw_dp_teid; }
void bearer::set_sgw_dp_teid(uint32_t sgw_dp_teid) { _sgw_dp_teid = sgw_dp_teid; }

uint32_t bearer::get_dp_teid() const { return _dp_teid; }

std::shared_ptr<pdn_connection> bearer::get_pdn_connection() const {
    return _pdn.shared_from_this();
}