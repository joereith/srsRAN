/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2020 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#ifndef SRSLTE_RRC_BEARER_CFG_H
#define SRSLTE_RRC_BEARER_CFG_H

#include "srsenb/hdr/stack/rrc/rrc_config.h"
#include "srslte/asn1/s1ap.h"
#include "srslte/common/logmap.h"
#include "srslte/interfaces/enb_interfaces.h"
#include "srslte/interfaces/enb_rrc_interface_types.h"
#include "srslte/srslog/srslog.h"

namespace srsenb {

class security_cfg_handler
{
public:
  explicit security_cfg_handler(const rrc_cfg_t& cfg_) : cfg(&cfg_), logger(srslog::fetch_basic_logger("RRC")) {}
  security_cfg_handler& operator=(const security_cfg_handler& other)
  {
    cfg                   = other.cfg;
    k_enb_present         = other.k_enb_present;
    security_capabilities = other.security_capabilities;
    std::copy(other.k_enb, other.k_enb + 32, k_enb);
    sec_cfg = other.sec_cfg;
    ncc     = other.ncc;
    return *this;
  }

  bool set_security_capabilities(const asn1::s1ap::ue_security_cap_s& caps);
  void set_security_key(const asn1::fixed_bitstring<256, false, true>& key);
  void set_ncc(uint8_t ncc_) { ncc = ncc_; }

  asn1::rrc::security_algorithm_cfg_s get_security_algorithm_cfg();
  const srslte::as_security_config_t& get_as_sec_cfg() const { return sec_cfg; }
  uint8_t                             get_ncc() const { return ncc; }
  bool                                is_as_sec_cfg_valid() const { return k_enb_present; }

  void regenerate_keys_handover(uint32_t new_pci, uint32_t new_dl_earfcn);

private:
  void generate_as_keys();

  srslog::basic_logger&         logger;
  const rrc_cfg_t*              cfg                   = nullptr;
  bool                          k_enb_present         = false;
  asn1::s1ap::ue_security_cap_s security_capabilities = {};
  uint8_t                       k_enb[32]             = {}; // Provided by MME
  srslte::as_security_config_t  sec_cfg               = {};
  uint8_t                       ncc                   = 0;
};

class bearer_cfg_handler
{
public:
  struct erab_t {
    struct gtpu_tunnel {
      uint32_t teid_out = 0;
      uint32_t teid_in  = 0;
      uint32_t addr     = 0;
    };
    uint8_t                                     id = 0;
    asn1::s1ap::erab_level_qos_params_s         qos_params;
    asn1::bounded_bitstring<1, 160, true, true> address;
    uint32_t                                    teid_out = 0;
    uint32_t                                    teid_in  = 0;
    std::vector<gtpu_tunnel>                    tunnels;
  };

  bearer_cfg_handler(uint16_t rnti_, const rrc_cfg_t& cfg_, gtpu_interface_rrc* gtpu_);

  int  add_erab(uint8_t                                            erab_id,
                const asn1::s1ap::erab_level_qos_params_s&         qos,
                const asn1::bounded_bitstring<1, 160, true, true>& addr,
                uint32_t                                           teid_out,
                const asn1::unbounded_octstring<true>*             nas_pdu);
  bool release_erab(uint8_t erab_id);
  void release_erabs();
  bool modify_erab(uint8_t                                    erab_id,
                   const asn1::s1ap::erab_level_qos_params_s& qos,
                   const asn1::unbounded_octstring<true>*     nas_pdu);

  // Methods to apply bearer updates
  void     add_gtpu_bearer(uint32_t erab_id);
  uint32_t add_gtpu_bearer(uint32_t                                erab_id,
                           uint32_t                                teid_out,
                           uint32_t                                addr,
                           const gtpu_interface_rrc::bearer_props* props = nullptr);
  void     fill_pending_nas_info(asn1::rrc::rrc_conn_recfg_r8_ies_s* msg);

  const std::map<uint8_t, erab_t>&        get_erabs() const { return erabs; }
  const asn1::rrc::drb_to_add_mod_list_l& get_established_drbs() const { return current_drbs; }

  std::map<uint8_t, std::vector<uint8_t> > erab_info_list;
  std::map<uint8_t, erab_t>                erabs;

private:
  srslog::basic_logger& logger;
  uint16_t              rnti = 0;
  const rrc_cfg_t*      cfg  = nullptr;
  gtpu_interface_rrc*   gtpu = nullptr;

  // last cfg
  asn1::rrc::drb_to_add_mod_list_l current_drbs;
};

} // namespace srsenb

#endif // SRSLTE_RRC_BEARER_CFG_H
