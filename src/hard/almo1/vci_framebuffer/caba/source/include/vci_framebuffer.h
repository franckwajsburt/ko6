/* -*- c++ -*-
 *
 * SOCLIB_LGPL_HEADER_BEGIN
 * 
 * This file is part of SoCLib, GNU LGPLv2.1.
 * 
 * SoCLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 of the License.
 * 
 * SoCLib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with SoCLib; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 * 
 * SOCLIB_LGPL_HEADER_END
 *
 * Copyright (c) UPMC, Lip6, Asim
 *         Nicolas Pouillon <nipo@ssji.net>, 2007
 *
 * Maintainers: alain
 */

/////////////////////////////////////////////////////////////////////////////////////
// This component implements a Frame Buffer, supporting various pixel encoding.
// It can handle only 32 bits or 64 bits VCI DATA interface.
//
// For 32 bits VCI DATA width:
// - in case of READ burst, both the VCI ADDRESS and PLEN must be multiple of 4.
// - in case of WRITE burst, the VCI ADDRESS must be multiple of 4. Successive
//   addresses must be contiguous and the BE is taken into account.
//
// For 64 bits VCI DATA width:
// - in case of READ burst, both the VCI ADDRESS and PLEN must be multiple of 8.
// - in case or WRITE burst, the VCI ADDRESS must be multiple of 8. Successive
//   addresses must be contiguous, and the BE is taken into account.
/////////////////////////////////////////////////////////////////////////////////////

#ifndef SOCLIB_VCI_FRAMEBUFFER_H
#define SOCLIB_VCI_FRAMEBUFFER_H

#include <systemc>
#include <sys/time.h>
#include "vci_target_fsm.h"
#include "caba_base_module.h"
#include "mapping_table.h"
#include "process_wrapper.h"
#include "fb_controller.h"

namespace soclib {
namespace caba {

using namespace sc_core;

/////////////////////////////////////////
template<typename vci_param>
class VciFrameBuffer
/////////////////////////////////////////
	: public caba::BaseModule
{
public:
     
    typedef typename vci_param::fast_addr_t  vci_addr_t; 
    typedef typename vci_param::fast_data_t  vci_data_t; 
    typedef typename vci_param::be_t         vci_be_t; 
    typedef typename vci_param::srcid_t      vci_srcid_t;
    typedef typename vci_param::trdid_t      vci_trdid_t;
    typedef typename vci_param::pktid_t      vci_pktid_t;

    enum fsm_state_e
    {
        IDLE,
        READ_BUF_RSP,
        WRITE_BUF_CMD,
        WRITE_BUF_RSP,
        READ_REG_RSP,
        WRITE_REG_RSP,
        ERROR_CMD,
        ERROR_RSP,
    };

    // Ports
    sc_in<bool>                          p_clk;
    sc_in<bool>                          p_resetn;
    soclib::caba::VciTarget<vci_param>   p_vci;

	VciFrameBuffer(
		sc_module_name                   name,
		const IntTab                     &index,
		const MappingTable               &mt,
		unsigned long                    width,
		unsigned long                    height,
        int                              subsampling = 422);

    ~VciFrameBuffer();

    void print_trace();
 
private:

    // Registers
    sc_signal<int>                       r_fsm_state;
    sc_signal<size_t>                    r_flit_count;
    sc_signal<size_t>                    r_index;              // pixel or register index
    sc_signal<vci_srcid_t>               r_srcid;
    sc_signal<vci_trdid_t>               r_trdid;
    sc_signal<vci_pktid_t>               r_pktid;
    sc_signal<uint32_t>                  r_data;

    // component attributes      
    std::list<soclib::common::Segment>   m_seglist;            // allocated segments
    unsigned long                        m_width;              // pixels per line
    unsigned long                        m_height;             // number of lines
    int                                  m_subsampling;        // pixel encoding
	int                                  m_defered_timeout;
    soclib::common::FbController         m_fb_controller;
	struct timeval                       m_last_update;
    vci_addr_t                           m_seg_base;           // segment base address

    // Methods
    void transition();
    void genMoore();

    void write_fb(size_t index, vci_data_t wdata, vci_be_t be);

protected:
    SC_HAS_PROCESS(VciFrameBuffer);

};

}}

#endif /* SOCLIB_VCI_FRAMEBUFFER_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

