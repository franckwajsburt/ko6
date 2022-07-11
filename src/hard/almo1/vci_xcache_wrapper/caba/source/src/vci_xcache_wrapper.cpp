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
 * Copyright (c) UPMC, Lip6, SoC
 *         Alain Greiner <alain.greiner@lip6.fr>
 *         Nicolas Pouillon <nipo@ssji.net>
 *
 * Maintainers: alain eric.guthmuller@polytechnique.edu nipo
 */

// #define SOCLIB_MODULE_DEBUG

///////////////////////////////////////////////////////////////////////////////////
// History
// - 01/06/2008
//   The VCI_XCACHE and the ISS_WRAPPER components have been merged, in order
//   to increase the simulation speed: this VCI_XCACHE_WRAPPER component
//   is directly wrapping the processsor ISS, allowing a direct communication
//   between the processor and the cache.
//   The number of associativity levels is now a parameter for both the data
//   and the instruction cache.
// - 15/07/2008
//   The data & instruction caches implementation have been modified, in order
//   to use the GenericCache object. Similarly, the write buffer implementation
//   has been modified to use the WriteBuffer object.
//   A VCI write burst is constructed when two conditions are satisfied :
//   The processor make strictly successive write requests, and they are
//   in the same cache line. The write buffer performs re-ordering, to
//   respect the contiguous addresses VCI constraint. In case of several
//   WRITE_WORD requests in the same word, only the last request is conserved.
//   In case of several WRITE_HALF or WRITE_BYTE requests in the same word,
//   the requests are merged in the same word. In case of uncached write
//   requests, each request is transmited as a single VCI transaction.
//   Both the data & instruction caches can be flushed in one single cycle.
//   Finally, new activity counters have been introduced for instrumentation.
// - 19/08/2008
//   The VCI CMD FSM has been modified to send single cell packet in case of MISS.
//   The uncached mode (using the mapping table) has been introduced  in the
//   ICACHE FSM.
// - 12/09/2010
//   The VCI RSP FSM has been modified to accept single flit response packets
//   in case of read bus error.
// - 02/03/2012
//   The r_vci_resp_fifo_ins & r_vci_rsp_fifo_data FIFOs have been introduced,
//   replacing the r_dcache_miss_buf[] & r_icache_miss_buf[], in order to comply 
//   with the single word update() method of the generic_cache.
//   The DCACHE_ERROR and ICACHE_ERROR states have been removed.
// - 20/03/2012
//   The ICACHE_MISS_INVAL and DCACHE_MISS_INVAL states have been introduced,
//   in order to comply with the two cycles policy implemented by the generic
//   cache : one cycle for victim select / one cycle for victim inval.
/////////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <limits>
#include "iss2.h"
#include "arithmetics.h"
#include "../include/vci_xcache_wrapper.h"

namespace soclib {
namespace caba {

namespace {
const char *dcache_fsm_state_str[] = {
        "DCACHE_IDLE",
        "DCACHE_WRITE_UPDT",
        "DCACHE_WRITE_REQ",
        "DCACHE_MISS_SELECT",
        "DCACHE_MISS_INVAL",
        "DCACHE_MISS_WAIT",
        "DCACHE_UNC_WAIT",
        "DCACHE_XTN_HIT",
        "DCACHE_XTN_INVAL",
    };
const char *icache_fsm_state_str[] = {
        "ICACHE_IDLE",
        "ICACHE_MISS_SELECT",
        "ICACHE_MISS_INVAL",
        "ICACHE_MISS_WAIT",
        "ICACHE_UNC_WAIT",
    };
const char *cmd_fsm_state_str[] = {
        "CMD_IDLE",
        "CMD_INS_MISS",
        "CMD_INS_UNC",
        "CMD_DATA_MISS",
        "CMD_DATA_UNC",
        "CMD_DATA_WRITE",
    };
const char *rsp_fsm_state_str[] = {
        "RSP_IDLE",
        "RSP_INS_MISS",
        "RSP_INS_UNC",
        "RSP_DATA_MISS",
        "RSP_DATA_UNC",
        "RSP_DATA_WRITE",
    };
}

#define tmpl(...)  template<typename vci_param, typename iss_t> __VA_ARGS__ VciXcacheWrapper<vci_param, iss_t>

using soclib::common::uint32_log2;

/////////////////////////////////
tmpl(/**/)::VciXcacheWrapper(
/////////////////////////////////
    sc_module_name name,
    int proc_id,
    const soclib::common::MappingTable &mt,
    const soclib::common::IntTab &index,
    size_t icache_ways,
    size_t icache_sets,
    size_t icache_words,
    size_t dcache_ways,
    size_t dcache_sets,
    size_t dcache_words )
    :
      soclib::caba::BaseModule(name),

      p_clk("clk"),
      p_resetn("resetn"),
      p_vci("vci"),

      m_cacheability_table(mt.getCacheabilityTable()),
      m_iss(this->name(), proc_id),
      m_srcid(mt.indexForId(index)),

      m_dcache_ways(dcache_ways),
      m_dcache_words(dcache_words),
      m_dcache_yzmask((~0)<<(uint32_log2(dcache_words) + 2)),
      m_icache_ways(icache_ways),
      m_icache_words(icache_words),
      m_icache_yzmask((~0)<<(uint32_log2(icache_words) + 2)),

      r_dcache_fsm("r_dcache_fsm"),
      r_dcache_addr_save("r_dcache_addr_save"),
      r_dcache_wdata_save("r_dcache_wdata_save"),
      r_dcache_type_save("r_dcache_type_save"),
      r_dcache_be_save("r_dcache_be_save"),
      r_dcache_cacheable_save("r_dcache_cacheable_save"),
      r_dcache_way_save("r_dcache_way_save"),
      r_dcache_set_save("r_dcache_set_save"),
      r_dcache_word_save("r_dcache_word_save"),
      r_dcache_miss_req("r_dcache_miss_req"),
      r_dcache_unc_req("r_dcache_unc_req"),
      r_dcache_write_req("r_dcache_write_req"),

      r_icache_fsm("r_icache_fsm"),
      r_icache_addr_save("r_icache_addr_save"),
      r_icache_miss_req("r_icache_miss_req"),
      r_icache_unc_req("r_icache_unc_req"),

      r_vci_cmd_fsm("r_vci_cmd_fsm"),
      r_vci_cmd_min("r_vci_cmd_min"),
      r_vci_cmd_max("r_vci_cmd_max"),
      r_vci_cmd_cpt("r_vci_cmd_cpt"),

      r_vci_rsp_fsm("r_vci_rsp_fsm"),
      r_vci_rsp_ins_error("r_vci_rsp_ins_error"),
      r_vci_rsp_data_error("r_vci_rsp_data_error"),
      r_vci_rsp_cpt("r_vci_rsp_cpt"),
      r_vci_rsp_fifo_ins("r_vci_rsp_fifo_ins", 2),	    // 2 words depth
      r_vci_rsp_fifo_data("r_vci_rsp_fifo_data", 2),	// 2 words depth

      r_icache_updated("r_icache_updated"),
      r_dcache_updated("r_dcache_updated"),

      r_wbuf("wbuf", dcache_words ),
      r_icache("icache", icache_ways, icache_sets, icache_words),
      r_dcache("dcache", dcache_ways, dcache_sets, dcache_words)
{
    std::cout << "  - Building VciXcacheWrapper " << name << std::endl;

    assert((icache_words*vci_param::B) < (1<<vci_param::K) and "I need more PLEN bits");

    SC_METHOD(transition);
    dont_initialize();
    sensitive << p_clk.pos();

    SC_METHOD(genMoore);
    dont_initialize();
    sensitive << p_clk.neg();

    typename iss_t::CacheInfo cache_info;
    cache_info.has_mmu = false;
    cache_info.icache_line_size = icache_words*sizeof(data_t);
    cache_info.icache_assoc = icache_ways;
    cache_info.icache_n_lines = icache_sets;
    cache_info.dcache_line_size = dcache_words*sizeof(data_t);
    cache_info.dcache_assoc = dcache_ways;
    cache_info.dcache_n_lines = dcache_sets;
    m_iss.setCacheInfo(cache_info);
}

/////////////////////////////////
tmpl(/**/)::~VciXcacheWrapper()
/////////////////////////////////
{
}

//////////////////////////////////////////
tmpl(void)::cache_monitor( addr_t addr )
//////////////////////////////////////////
{
    bool        cache_hit;
    size_t	    cache_way = 0;
    size_t	    cache_set = 0;
    size_t	    cache_word = 0;
    uint32_t	cache_rdata = 0;

    cache_hit = r_dcache.read_neutral( addr,
                                       &cache_rdata,
                                       &cache_way,
                                       &cache_set,
                                       &cache_word );

    if ( cache_hit != m_debug_previous_d_hit )
    {
        std::cout << "Monitor PROC " << name()
                  << " DCACHE at cycle " << std::dec << m_cpt_total_cycles
                  << " / HIT = " << cache_hit 
                  << " / PADDR = " << std::hex << addr
                  << " / DATA = " << cache_rdata 
                  << " / WAY = " << cache_way << std::endl;
	    m_debug_previous_d_hit = cache_hit;
    }

    cache_hit = r_icache.read_neutral( addr,
                                       &cache_rdata,
                                       &cache_way,
                                       &cache_set,
                                       &cache_word );

    if ( cache_hit != m_debug_previous_i_hit )
    {
        std::cout << "Monitor PROC " << name()
                  << " ICACHE at cycle " << std::dec << m_cpt_total_cycles
                  << " / HIT = " << cache_hit 
                  << " / PADDR = " << std::hex << addr
                  << " / DATA = " << cache_rdata 
                  << " / WAY = " << cache_way << std::endl;
	    m_debug_previous_i_hit = cache_hit;
    }
}

//////////////////////////////////
tmpl(void)::file_trace(FILE* file)
//////////////////////////////////
{
    if(r_dcache_updated) 
    {
        fprintf(file, "*****************************  cycle %d       DATA \n", m_cpt_total_cycles);
        r_dcache.fileTrace(file);
    }
    if(r_icache_updated) 
    {
        fprintf(file, "*****************************  cycle %d       INSTRUCTION\n", m_cpt_total_cycles);
        r_icache.fileTrace(file);
    }
}
//////////////////////////////////
tmpl(void)::file_stats(FILE* file)
//////////////////////////////////
{
    float imiss_rate 	= (float)m_cpt_ins_miss / (float)(m_cpt_exec_ins);
    float dmiss_rate 	= (float)m_cpt_data_miss / (float)(m_cpt_read);
    float cpi		= (float)m_cpt_total_cycles / (float)(m_cpt_exec_ins);

    fprintf(file,"%8d %8d %8d %8d %8d    %f    %f    %f \n", 
            m_cpt_total_cycles, 
            m_cpt_exec_ins,
            m_cpt_ins_miss,
            m_cpt_read,
            m_cpt_data_miss,
            imiss_rate, 
            dmiss_rate,
            cpi);
}
////////////////////////
tmpl(void)::print_cpi()
////////////////////////
{
    std::cout << name() << " CPU " << m_srcid << " : CPI = "

              << (float)m_cpt_total_cycles/(float)(m_cpt_exec_ins) << std::endl;
}
////////////////////////
tmpl(void)::print_stats()
////////////////////////
{
    std::cout << "------------------------------------" << std:: dec << std::endl
    << name() << " / Time = " << m_cpt_total_cycles << std::endl
    << "- CPI               = " << (float)m_cpt_total_cycles/m_cpt_exec_ins << std::endl
    << "- READ RATE         = " << (float)m_cpt_read/m_cpt_exec_ins << std::endl 
    << "- WRITE RATE        = " << (float)m_cpt_write/m_cpt_exec_ins << std::endl
    << "- UNC RATE          = " << (float)m_cpt_data_unc/m_cpt_exec_ins << std::endl 
    << "- CACHED WRITE RATE = " << (float)m_cpt_write_cached/m_cpt_write << std::endl
    << "- IMISS_RATE        = " << (float)m_cpt_ins_miss/m_cpt_exec_ins << std::endl
    << "- DMISS RATE        = " << (float)m_cpt_data_miss/m_cpt_read << std::endl 
    << "- INS MISS COST     = " << (float)m_cost_ins_miss_frz/m_cpt_ins_miss << std::endl
    << "- DMISS COST        = " << (float)m_cost_data_miss_frz/m_cpt_data_miss << std::endl
    << "- UNC COST          = " << (float)m_cost_data_unc_frz/m_cpt_data_unc << std::endl
    << "- WRITE COST        = " << (float)m_cost_write_frz/m_cpt_write << std::endl
    << "- WRITE LENGTH      = " << (float)m_length_write_transaction/m_cpt_write_transaction 
    << std::endl;
}
////////////////////////////////////
tmpl(void)::print_trace(size_t mode)
////////////////////////////////////
{
    std::cout << std::dec << "PROC " << name() << std::endl;

    std::cout << "  " << m_ireq << std::endl;
    std::cout << "  " << m_irsp << std::endl;
    std::cout << "  " << m_dreq << std::endl;
    std::cout << "  " << m_drsp << std::endl;

    std::cout << "  " << icache_fsm_state_str[r_icache_fsm.read()]
              << " | " << dcache_fsm_state_str[r_dcache_fsm.read()]
              << " | " << cmd_fsm_state_str[r_vci_cmd_fsm.read()]
              << " | " << rsp_fsm_state_str[r_vci_rsp_fsm.read()] << std::endl;

    if(mode & 0x1)
    {
        r_wbuf.printTrace();
    }

    if(((mode & 0x2) != 0) and r_dcache_updated)
    {
        std::cout << "  Data cache" << std::endl;
        r_dcache.printTrace();
    }

    if(((mode & 0x4) != 0) and r_icache_updated)
    {
        std::cout << "  Instruction cache" << std::endl;
        r_icache.printTrace();
    }
}
//////////////////////////
tmpl(void)::transition()
//////////////////////////
{
    if ( ! p_resetn.read() ) 
    {
        m_iss.reset();

        // FSM states
        r_dcache_fsm  = DCACHE_IDLE;
        r_icache_fsm  = ICACHE_IDLE;
        r_vci_cmd_fsm = CMD_IDLE;
        r_vci_rsp_fsm = RSP_IDLE;

        // write buffer, caches & FIFOS
        r_wbuf.reset();
        r_icache.reset();
        r_dcache.reset();
        r_vci_rsp_fifo_ins.init();
        r_vci_rsp_fifo_data.init();

        // synchronisation flip-flops from ICACHE & DCACHE FSMs to VCI  FSMs
        r_icache_miss_req    = false;
        r_icache_unc_req     = false;
        r_dcache_miss_req    = false;
        r_dcache_unc_req     = false;
        r_dcache_write_req   = false;

        // signals from the VCI RSP FSM to the ICACHE or DCACHE FSMs
        r_vci_rsp_data_error   = false;
        r_vci_rsp_ins_error    = false;

        // instrumentation registers
        r_icache_updated	= false;
        r_dcache_updated	= false;

        // Debug variables
        m_debug_previous_i_hit = false;
        m_debug_previous_d_hit = false;

        // activity counters
        m_cpt_dcache_read  = 0;
        m_cpt_dcache_write = 0;
        m_cpt_icache_read  = 0;
        m_cpt_icache_write = 0;

	    m_cpt_exec_ins = 0;
        m_cpt_total_cycles = 0;

        m_cpt_read = 0;
        m_cpt_write = 0;
        m_cpt_data_miss = 0;
        m_cpt_ins_miss = 0;
        m_cpt_data_unc = 0;
        m_cpt_ins_unc = 0;
        m_cpt_write_cached = 0;

        m_cost_write_frz = 0;
        m_cost_data_miss_frz = 0;
        m_cost_data_unc_frz = 0;
        m_cost_ins_miss_frz = 0;
        m_cost_ins_unc_frz = 0;

        m_cpt_write_transaction = 0;
        m_length_write_transaction = 0;

        return;
    }

    // Response FIFOs default values
    bool       vci_rsp_fifo_ins_get       = false;
    bool       vci_rsp_fifo_ins_put       = false;
    data_t     vci_rsp_fifo_ins_data      = 0;

    bool       vci_rsp_fifo_data_get       = false;
    bool       vci_rsp_fifo_data_put       = false;
    data_t     vci_rsp_fifo_data_data      = 0;

#ifdef SOCLIB_MODULE_DEBUG_2
    std::cout
        << name()
        << " dcache fsm: " << dcache_fsm_state_str[r_data_fsm]
        << " icache fsm: " << icache_fsm_state_str[r_icache_fsm]
        << " cmd fsm: " << cmd_fsm_state_str[r_vci_cmd_fsm]
        << " rsp fsm: " << rsp_fsm_state_str[r_vci_rsp_fsm] << std::endl;
#endif

    m_cpt_total_cycles++;

    r_icache_updated = (r_icache_fsm.read() == ICACHE_MISS_WAIT) and
                       (r_icache_word_save.read() == m_icache_words-1);

    r_dcache_updated = ( (r_dcache_fsm.read() == DCACHE_MISS_WAIT) and
                         (r_dcache_word_save.read() == m_dcache_words-1) ) or
                         (r_dcache_fsm.read() == DCACHE_WRITE_UPDT);

    m_ireq.valid = m_dreq.valid = false;

    m_iss.getRequests( m_ireq, m_dreq );

#ifdef SOCLIB_MODULE_DEBUG
    std::cout << name() << " Instruction Request: " << m_ireq << std::endl;
    std::cout << name() << " Data        Request: " << m_dreq << std::endl;
#endif

    /////////////////////////////////////////////////////////////////////
    // The ICACHE FSM controls the intruction cache
    //
    // Processor requests are taken into account only in the IDLE state.
    // In case of MISS, or in case of uncached instruction, the FSM 
    // writes the missing address line in the  r_icache_addr_save register 
    // and sets the r_icache_miss_req (or the r_icache_unc_req) flip-flop.
    // The r_icache_buf_unc_valid is set in case of uncached access.
    // In case of bus error, the VCI_RSP FSM sets the r_vci_rsp_ins_error
    // flip-flop. It is reset by the ICACHE FSM.
    ///////////////////////////////////////////////////////////////////////

    // default value for m_irsp
    m_irsp.valid       = false;
    m_irsp.error       = false;
    m_irsp.instruction = 0;

    switch(r_icache_fsm) 
    {
    /////////////////
    case ICACHE_IDLE:
    {
        if ( m_ireq.valid ) 
        {
            data_t  icache_ins;
            bool    icache_hit;

            m_cpt_icache_read++;

            bool    icache_cacheable = m_cacheability_table[(uint64_t)m_ireq.addr];

            if ( icache_cacheable )    // cacheable access
            {
                icache_hit = r_icache.read(m_ireq.addr, &icache_ins);
                if ( not icache_hit )   // miss
                {
                    m_cpt_ins_miss++;
                    m_cost_ins_miss_frz++;

                    r_icache_addr_save = m_ireq.addr;
                    r_icache_word_save = 0;
                    r_icache_fsm       = ICACHE_MISS_SELECT;
                    r_icache_miss_req  = true;
                } 
                else                    // hit
                {
                    m_irsp.valid          = true;
                    m_irsp.instruction    = icache_ins;
                } 
            }
            else                        // non cacheable access 
            {
                m_cpt_ins_unc++;
                m_cost_ins_unc_frz++;

                r_icache_addr_save = m_ireq.addr;
                r_icache_fsm       = ICACHE_UNC_WAIT;
                r_icache_unc_req   = true;
            }
        }
        break;
    }
    ////////////////////////    
    case ICACHE_MISS_SELECT:    // selects a victim line
    {
        m_cost_ins_miss_frz++;

        bool    valid;
        size_t  way;
        size_t  set;
        addr_t  victim;     // unused

        valid = r_icache.victim_select( r_icache_addr_save.read(),
                                        &victim,
                                        &way,
                                        &set );
        r_icache_way_save = way;
        r_icache_set_save = set;
        if ( valid )    r_icache_fsm = ICACHE_MISS_INVAL;
        else            r_icache_fsm = ICACHE_MISS_WAIT;
        break;
    }
    ///////////////////////
    case ICACHE_MISS_INVAL:     // inval the victim line
    {
        m_cost_ins_miss_frz++;

        addr_t  nline;       // unused

        r_icache.inval( r_icache_way_save.read(),
                        r_icache_set_save.read(),
                        &nline );

        r_icache_fsm = ICACHE_MISS_WAIT;
        break;
    }
    //////////////////////
    case ICACHE_MISS_WAIT:
    {
        m_cost_ins_miss_frz++;

        if ( r_vci_rsp_ins_error.read() )      // error reported
        {
            m_irsp.valid          = true;
            m_irsp.error          = true;
            r_vci_rsp_ins_error   = false;
            r_icache_fsm          = ICACHE_IDLE;
        }
        else if ( r_vci_rsp_fifo_ins.rok() )   // available instruction 
        {
            m_cpt_icache_write++;

            vci_rsp_fifo_ins_get = true;
            r_icache_word_save   = r_icache_word_save.read()+1;
            r_icache.write( r_icache_way_save.read(),
                            r_icache_set_save.read(),
                            r_icache_word_save.read(),
                            r_vci_rsp_fifo_ins.read() );

            if ( r_icache_word_save.read() == m_icache_words-1 ) // last word
            {
                r_icache.victim_update_tag( r_icache_addr_save.read(),
                                            r_icache_way_save.read(),
                                            r_icache_set_save.read() );
                r_icache_fsm = ICACHE_IDLE;
            }
        }
        break;
    }
    /////////////////////
    case ICACHE_UNC_WAIT:
    {
        m_cost_ins_miss_frz++;

        if ( r_vci_rsp_ins_error.read() )   // error reported
        {
            r_vci_rsp_ins_error  = false;
            m_irsp.valid           = true;
            m_irsp.error           = true;
            r_icache_fsm         = ICACHE_IDLE;
        }
        else if ( r_vci_rsp_fifo_ins.rok() ) // instruction available
        {
            vci_rsp_fifo_ins_get = true;
            if ( m_ireq.valid and 
                 (m_ireq.addr == r_icache_addr_save.read()) ) // unmodified
            {
                m_irsp.valid       = true;
                m_irsp.instruction = r_vci_rsp_fifo_ins.read();
            }
            r_icache_fsm         = ICACHE_IDLE;
        }
        break;
    }

    } // end switch r_icache_fsm

#ifdef SOCLIB_MODULE_DEBUG
    std::cout << name() << " Instruction Response: " << m_irsp << std::endl;
#endif

    ///////////////////////////////////////////////////////////////////////////////////////
    // The DCACHE FSM controls the data cache
    //
    // In order to support VCI write burst, the processor requests are taken into account
    // in the WRITE_REQ state as well as in the IDLE state. It uses a very simple
    // write buffer, that contains at most one cache line. It tries to build write bursts 
    // when there is several consecutive write requests in the same cache line.
    // - In the IDLE state, the processor request cannot be satisfied if
    //   there is a cached read miss, or an uncached read.
    // - In the WRITE_REQ state, the request cannot be satisfied if
    //   there is a cached read miss, or an uncached read,
    //   or when the write buffer is full.
    // - In all other states, the processor request is not satisfied.
    //
    // The cache access takes into account the cacheability_table.
    // All LL or SC requests are handled as uncacheable.
    // 
    // Error handling :  Read Bus Errors are synchronous events, but
    // Write Bus Errors are asynchronous events (processor is not frozen).
    // - If a Read Bus Error is detected, the VCI_RSP FSM sets the
    //   r_vci_rsp_data_error flip-flop, and the synchronous error is signaled
    //   by the DCACHE FSM.
    // - If a Write Bus Error is detected, the VCI_RSP FSM  signals directly
    //   the asynchronous error using the setWriteBerr() method.
    //////////////////////////////////////////////////////////////////////////////////////

    // default value for m_drsp 
    m_drsp.valid = false;
    m_drsp.error = false;
    m_drsp.rdata = 0;

    switch ( r_dcache_fsm ) 
    {
    //////////////////////
    case DCACHE_WRITE_REQ:
    {
        // post the write request in the write buffer if no pending request
        if ( not r_dcache_write_req ) 
        {
            if ( r_wbuf.wok( r_dcache_addr_save ) )     // write request accepted
            {
                r_wbuf.write(r_dcache_addr_save, r_dcache_be_save, r_dcache_wdata_save);
                // close the write burst if current request uncacheable
                if (  not r_dcache_cacheable_save ) r_dcache_write_req = true ;
            } 
            else                                        // write request not accepted
            {
                // close the write burts and stay in DCACHE_WRITE_REQ
                r_dcache_write_req = true;
                m_cost_write_frz++;
                break;
            }
        } 
        else    //  previous write transaction not completed
        {
            m_cost_write_frz++;
            break;
        }

        // close the write packet if the next processor request is not a write
        if ( not m_dreq.valid || m_dreq.type != iss_t::DATA_WRITE )
            r_dcache_write_req = true ;

        // The next state and the processor request parameters are computed
        // as in the DCACHE_IDLE state (see below ...)
    }
    /////////////////
    case DCACHE_IDLE:
    {
        if ( m_dreq.valid ) 
        {
            bool        dcache_cacheable;

            m_cpt_dcache_read++;

            // dcache_cacheable evaluation
            if ( (m_dreq.type == iss_t::DATA_LL) or (m_dreq.type == iss_t::DATA_SC) or
                 (m_dreq.type == iss_t::XTN_READ) or (m_dreq.type == iss_t::XTN_WRITE) )
                      dcache_cacheable = false;
            else
                      dcache_cacheable = m_cacheability_table[m_dreq.addr];

            r_dcache_addr_save      = m_dreq.addr;
            r_dcache_type_save      = m_dreq.type;
            r_dcache_wdata_save     = m_dreq.wdata;
            r_dcache_be_save        = m_dreq.be;
            r_dcache_cacheable_save = dcache_cacheable;

            if ( dcache_cacheable ) // cacheable read or write
            {
                bool        dcache_hit;
                data_t      dcache_rdata = 0;
                size_t      dcache_way   = 0;
                size_t      dcache_set   = 0;
                size_t      dcache_word  = 0;

                dcache_hit = r_dcache.read( m_dreq.addr, 
                                            &dcache_rdata,
                                            &dcache_way,
                                            &dcache_set,
                                            &dcache_word );

                if ( m_dreq.type == iss_t::DATA_READ )        // cacheable read
                {
                    m_cpt_read++;

                    if ( not dcache_hit )   // read miss
                    {
                        m_cpt_data_miss++;
                        m_cost_data_miss_frz++;

                        r_dcache_way_save   = dcache_way;
                        r_dcache_set_save   = dcache_set;
                        r_dcache_word_save  = 0;
                        r_dcache_fsm        = DCACHE_MISS_SELECT;
                        r_dcache_miss_req   = true;
                    } 
                    else                    // read hit
                    {
                        m_drsp.valid        = true;
                        m_drsp.rdata        = dcache_rdata;
                        r_dcache_fsm        = DCACHE_IDLE;
                    } 
                }
                else if ( m_dreq.type == iss_t::DATA_WRITE )  // cacheable write
                {
                    m_cpt_write++;

                    if ( not dcache_hit )   // write miss
                    {
                        m_drsp.valid        = true;
                        r_dcache_fsm        = DCACHE_WRITE_REQ;
                    }
                    else                    // write hit
                    {
                        m_cpt_write_cached++;

                        m_drsp.valid        = true;
                        r_dcache_fsm        = DCACHE_WRITE_UPDT;
                        r_dcache_way_save   = dcache_way;
                        r_dcache_set_save   = dcache_set;
                        r_dcache_word_save  = dcache_word; 
                    }
                }
                else
                {
                    std::cout << "Error in processor" << name()
                              << "Only read or write requests can be cacheable"
                              << std::endl;
                    exit(0);
                }
            } 
            else                    // uncacheable request
            {
                switch( m_dreq.type ) 
                {
                // we expect a single word rdata for these 3 requests
                case iss_t::DATA_READ:
                case iss_t::DATA_LL:
                case iss_t::DATA_SC:
                {
                    m_cpt_data_unc++;
                    m_cost_data_unc_frz++;

                    r_dcache_unc_req   = true;
                    r_dcache_addr_save = m_dreq.addr;
                    r_dcache_fsm       = DCACHE_UNC_WAIT;
                    break;
                }
                case iss_t::XTN_READ:
                {
                    std::cout << "Error in processor" << name()
                              << "XTN read requests are not supported"
                              << std::endl;
                    exit(0);
                }
                case iss_t::XTN_WRITE:  // only SYNC and INVAL requests are supported
                {
                    if ( m_dreq.addr/4 == iss_t::XTN_DCACHE_INVAL )
                    {
                        // two cycles 
                        r_dcache_fsm = DCACHE_XTN_HIT;
                    }
                    else if ( m_dreq.addr/4 == iss_t::XTN_SYNC )
                    {
                        // nothing to do
                        r_dcache_fsm = DCACHE_IDLE;
                        m_drsp.valid = true;
                        m_drsp.rdata = 0;
                    }
                    else
                    {
                        std::cout << "Error in processor" << name()
                                  << "Only XTN_SYNC & XTN_DCACHE_INVAL are supported"
                                  << std::endl;
                        exit(0);
                    }
                    break;
                }
                case iss_t::DATA_WRITE:
                {
                    m_cpt_data_unc++;

                    r_dcache_fsm = DCACHE_WRITE_REQ;
                    m_drsp.valid = true;
                    m_drsp.rdata = 0;
                    break;
                }
                } // end switch m_dreq.type
            } // end non cacheable request
        } 
        else    // no dcache_req 
        {    
            r_dcache_fsm = DCACHE_IDLE;
        }
        break;
    }
    ///////////////////////
    case DCACHE_WRITE_UPDT:
    {
        m_cpt_dcache_write++;
        r_dcache.write( r_dcache_way_save.read(), 
                        r_dcache_set_save.read(),
                        r_dcache_word_save.read(),
                        r_dcache_wdata_save.read(),
                        r_dcache_be_save.read() );
        r_dcache_fsm = DCACHE_WRITE_REQ;
        break;
    }
    ////////////////////////
    case DCACHE_MISS_SELECT:    // select the victim line
    {
        m_cost_data_miss_frz++;

        bool    valid;
        size_t  way;
        size_t  set;
        addr_t  victim;     // unused

        valid = r_dcache.victim_select( r_dcache_addr_save.read(),
                                        &victim,
                                        &way,
                                        &set );
        r_dcache_way_save = way;
        r_dcache_set_save = set;
        if ( valid ) r_dcache_fsm = DCACHE_MISS_INVAL;
        else         r_dcache_fsm = DCACHE_MISS_WAIT;
        break;
    }
    ///////////////////////
    case DCACHE_MISS_INVAL:     // inval the victim line
    {
        m_cost_ins_miss_frz++;

        addr_t  nline;       // unused

        r_dcache.inval( r_dcache_way_save.read(),
                        r_dcache_set_save.read(),
                        &nline );

        r_dcache_fsm = DCACHE_MISS_WAIT;
        break;
    }
    //////////////////////
    case DCACHE_MISS_WAIT:
    {
        m_cost_data_miss_frz++;

        if ( r_vci_rsp_data_error.read() )       // error reported
        {
            m_drsp.valid         = true;
            m_drsp.error         = true;
            r_vci_rsp_data_error = false;
            r_dcache_fsm         = DCACHE_IDLE;
        }
        else if ( r_vci_rsp_fifo_data.rok() )    // available data 
        {
            m_cpt_dcache_write++;

            vci_rsp_fifo_data_get = true;
            r_dcache_word_save    = r_dcache_word_save.read()+1;
            r_dcache.write( r_dcache_way_save.read(),
                            r_dcache_set_save.read(),
                            r_dcache_word_save.read(),
                            r_vci_rsp_fifo_data.read() );

            if ( r_dcache_word_save.read() == m_dcache_words-1 ) // last word
            {
                r_dcache.victim_update_tag( r_dcache_addr_save.read(),
                                            r_dcache_way_save.read(),
                                            r_dcache_set_save.read() );
                r_dcache_fsm = DCACHE_IDLE;
            }
        }
        break;
    }
    /////////////////////
    case DCACHE_UNC_WAIT:   // wait rdata for LL, SC, or uncacheable read
    {
        m_cost_data_unc_frz++;

        if ( r_vci_rsp_data_error.read() )      // error reported
        {
            m_drsp.valid          = true;
            m_drsp.error          = true;
            r_vci_rsp_data_error  = false;
            r_dcache_fsm          = DCACHE_IDLE;
        }
        else if ( r_vci_rsp_fifo_data.rok() )   // available data
        {
            vci_rsp_fifo_data_get = true;
            if ( m_dreq.valid and 
                 (m_dreq.addr == r_dcache_addr_save.read()) ) // request unmodified
            {
                m_drsp.valid = true;
                m_drsp.rdata = r_vci_rsp_fifo_data.read();
            }
            r_dcache_fsm = DCACHE_IDLE;
        }
        break;
    }
    ////////////////////
    case DCACHE_XTN_HIT:    // test hit in case of XTN_INVAL
    {
        uint32_t	data;   // unused
        size_t		word;   // unused
        size_t		way;
        size_t		set;
        bool		hit = r_dcache.read( r_dcache_wdata_save.read(),
                                         &data,
                                         &way,
                                         &set,
                                         &word );
        if ( hit )	// inval to be done
        {
            r_dcache_way_save = way;
            r_dcache_set_save = set;
            r_dcache_fsm      = DCACHE_XTN_INVAL;
        }
        else		// miss : nothing to do
        {
            r_dcache_fsm      = DCACHE_IDLE;
            m_drsp.valid      = true;
        }
        break;
    }
    //////////////////////
    case DCACHE_XTN_INVAL:
    {
        addr_t  nline;  // unused
        r_dcache.inval( r_dcache_way_save.read(),
                        r_dcache_set_save.read(),
                        &nline );
        r_dcache_fsm = DCACHE_IDLE;
        m_drsp.valid = true;
        break;
    }
    } // end switch r_dcache_fsm

#ifdef SOCLIB_MODULE_DEBUG
    std::cout << name() << " Data Response: " << m_drsp << std::endl;
#endif

    ////////// Compute number of executed instructions ////////////////
    if ( (m_ireq.valid and m_irsp.valid) and 
         (!m_dreq.valid or m_drsp.valid) and 
         (m_ireq.addr != m_pc_previous) )
    {
        m_cpt_exec_ins++;
        m_pc_previous = m_ireq.addr;
    }

    /////////// execute one iss cycle /////////////////////////////////
    uint32_t it = 0;
    for (size_t i=0; i<(size_t)iss_t::n_irq; i++)
    {
            if(p_irq[i].read()) it |= (1<<i);
    }
    m_iss.executeNCycles(1, m_irsp, m_drsp, it);

    ////////////////////////////////////////////////////////////////////////////
    // This FSM handles requests from both the DCACHE FSM & the ICACHE FSM.
    // There is 5 request types, with the following priorities :
    // 1 - Instruction Miss     : r_icache_miss_req
    // 2 - Instruction Uncached : r_icache_unc_req
    // 3 - Data Write           : r_dcache_write_req
    // 4 - Data Read Miss       : r_dcache_miss_req
    // 5 - Data Read Uncached   : r_dcache_unc_req
    // These request flip-flops are reset by the CMD_FSM.
    // There is at most one (CMD/RSP) VCI transaction, as both CMD_FSM
    // and RSP_FSM exit simultaneously the IDLE state.
    //
    // VCI formats:
    // According to the VCI advanced specification, all read command packets
    // (Uncached, Miss data, Miss instruction) are one word packets.
    // For write burst packets, all words must be in the same cache line,
    // and addresses must be contiguous (the BE field is 0 in case of "holes").
    // The PLEN VCI field is always documented.
    //////////////////////////////////////////////////////////////////////////////

    switch (r_vci_cmd_fsm) 
    {
    //////////////
    case CMD_IDLE:
    {
        if (r_vci_rsp_fsm != RSP_IDLE) break;

        r_vci_cmd_cpt = 0;
        if ( r_icache_miss_req ) 
        {
            r_vci_cmd_fsm     = CMD_INS_MISS;
            r_icache_miss_req = false;
        } 
        else if ( r_icache_unc_req ) 
        {
            r_vci_cmd_fsm    = CMD_INS_UNC;
            r_icache_unc_req = false;
        } 
        else if ( r_dcache_write_req ) 
        {
            r_vci_cmd_fsm = CMD_DATA_WRITE;
            r_vci_cmd_cpt = r_wbuf.getMin();
            r_vci_cmd_min = r_wbuf.getMin();
            r_vci_cmd_max = r_wbuf.getMax();
            m_cpt_write_transaction++;
            m_length_write_transaction += (r_wbuf.getMax() - r_wbuf.getMin() + 1);
        } 
        else if ( r_dcache_miss_req ) 
        {
            r_vci_cmd_fsm     = CMD_DATA_MISS;
            r_dcache_miss_req = false;
        } 
        else if ( r_dcache_unc_req ) 
        {
            r_vci_cmd_fsm    = CMD_DATA_UNC;
            r_dcache_unc_req = false;
        }
        break;
    }
    ////////////////////
    case CMD_DATA_WRITE:
    {
        if ( p_vci.cmdack.read() ) 
        {
            r_vci_cmd_cpt = r_vci_cmd_cpt + 1;
            if (r_vci_cmd_cpt == r_vci_cmd_max) 
            {
                r_dcache_write_req = false;
                r_vci_cmd_fsm      = CMD_IDLE ;
                r_wbuf.reset() ;
            }
        }
        break;
    }
    ///////////////////
    case CMD_DATA_MISS:
    case CMD_DATA_UNC:
    case CMD_INS_MISS:
    case CMD_INS_UNC:
    {
        if ( p_vci.cmdack.read() ) 
        {
            r_vci_cmd_fsm = CMD_IDLE;
        }
        break;
    }
    } // end  switch r_vci_cmd_fsm

    //////////////////////////////////////////////////////////////////////////
    // The VCI_RSP FSM controls the VCI response, and writes the received
    // data in the r_vci_rsp_fifo_data and r_vci_rsp_fifo_ins FIFOs.
    // 
    // In order to have only one active VCI transaction, this VCI_RSP_FSM
    // is synchronized with the VCI_CMD FSM, and both FSMs exit the
    // IDLE state simultaneously.
    //
    // VCI formats:
    // This component accepts single word or multi-word response packets for
    // write response packets.
    //
    // Error handling:
    // This FSM analyzes the VCI error code and signals directly the
    // Write Bus Error.
    // In case of Read Data Error, the VCI_RSP FSM sets the r_vci_rsp_data_error
    // flip_flop and the error is signaled by the DCACHE FSM.
    // In case of Instruction Error, the VCI_RSP FSM sets the r_vci_rsp_ins_error
    // flip_flop and the error is signaled by the DCACHE FSM.
    // In case of Cleanup Error, the simulation stops with an error message...
    //////////////////////////////////////////////////////////////////////////

    switch (r_vci_rsp_fsm) 
    {
    //////////////
    case RSP_IDLE:
    {
        assert( ! p_vci.rspval.read() and "Unexpected response" );
        if (r_vci_cmd_fsm != CMD_IDLE) break;

        r_vci_rsp_cpt = 0;
        if      ( r_icache_miss_req )       r_vci_rsp_fsm = RSP_INS_MISS;
        else if ( r_icache_unc_req )        r_vci_rsp_fsm = RSP_INS_UNC;
        else if ( r_dcache_write_req )      r_vci_rsp_fsm = RSP_DATA_WRITE;
        else if ( r_dcache_miss_req )       r_vci_rsp_fsm = RSP_DATA_MISS;
        else if ( r_dcache_unc_req )        r_vci_rsp_fsm = RSP_DATA_UNC;
        break;
    }
    //////////////////
    case RSP_INS_MISS:
    {
        if ( p_vci.rspval.read() )
        {
            if ( (p_vci.rerror.read()&0x1) != 0 )       // error reported
            {
                r_vci_rsp_ins_error = true;
                if ( p_vci.reop.read() ) r_vci_rsp_fsm = RSP_IDLE;
            }
            else                                        // no error reported
            {
                if ( r_vci_rsp_fifo_ins.wok() )
                {
                    assert( (r_vci_rsp_cpt.read() < m_icache_words) and
                    "The VCI response packet for instruction miss is too long");
                    r_vci_rsp_cpt              = r_vci_rsp_cpt.read() + 1;
                    vci_rsp_fifo_ins_put       = true,
                    vci_rsp_fifo_ins_data      = p_vci.rdata.read();
                    if ( p_vci.reop.read() )
                    {
                        assert( (r_vci_rsp_cpt.read() == (m_icache_words - 1)) and
                        "The VCI response packet for instruction miss is too short");
                        r_vci_rsp_fsm     = RSP_IDLE;
                    }
                }
            }
        }
        break;
    }
    /////////////////
    case RSP_INS_UNC:
    {
        if ( p_vci.rspval.read() )
        {
            assert(p_vci.reop.read() and
            "illegal VCI response packet for uncached instruction");
            if ( (p_vci.rerror.read()&0x1) != 0 )       // error reported
            {
                r_vci_rsp_ins_error = true;
                r_vci_rsp_fsm = RSP_IDLE;
            }                                           // no error reported
            else
            {
                if ( r_vci_rsp_fifo_ins.wok() )
                {
                    vci_rsp_fifo_ins_put  = true;
                    vci_rsp_fifo_ins_data = p_vci.rdata.read();
                    r_vci_rsp_fsm         = RSP_IDLE;
                }
            }
        }
        break;
    }
    ///////////////////
    case RSP_DATA_MISS:
    {
        if ( p_vci.rspval.read() )
        {
            if ( (p_vci.rerror.read()&0x1) != 0 )       // error reported
            {
                r_vci_rsp_data_error = true;
                if ( p_vci.reop.read() ) r_vci_rsp_fsm = RSP_IDLE;
            }
            else                                        // no error reported
            {
                if ( r_vci_rsp_fifo_data.wok() )
                {
                    assert( (r_vci_rsp_cpt.read() < m_dcache_words) and
                    "The VCI response packet for data miss is too long");
                    r_vci_rsp_cpt                 = r_vci_rsp_cpt.read() + 1;
                    vci_rsp_fifo_data_put         = true;
                    vci_rsp_fifo_data_data        = p_vci.rdata.read();
                    if ( p_vci.reop.read() )
                    {
                        assert( (r_vci_rsp_cpt.read() == m_dcache_words - 1) and
                        "The VCI response packet for data miss is too short");
                        r_vci_rsp_fsm     = RSP_IDLE;
                    }
                }
            }
        }
        break;
    }
    ////////////////////
    case RSP_DATA_WRITE:
    {
        if (  p_vci.rspval.read() )
        {
            if ( (p_vci.rerror.read() & 0x1) == 0x1 ) m_iss.setWriteBerr(); 
            if ( p_vci.reop.read() ) r_vci_rsp_fsm = RSP_IDLE;
        }
        break;
    }
    //////////////////
    case RSP_DATA_UNC:
    {
        if ( p_vci.rspval.read() )
        {
            assert(p_vci.reop.read() and
            "illegal VCI response packet for uncached data");
            if ( (p_vci.rerror.read()&0x1) != 0 )       // error reported
            {
                r_vci_rsp_data_error = true;
                r_vci_rsp_fsm = RSP_IDLE;
            }                                           // no error reported
            else
            {
                if ( r_vci_rsp_fifo_data.wok() )
                {
                    vci_rsp_fifo_data_put  = true;
                    vci_rsp_fifo_data_data = p_vci.rdata.read();
                    r_vci_rsp_fsm          = RSP_IDLE;
                }
            }
        }
        break;
    }
    } // end switch r_vci_rsp_fsm

    ///////////////// Response FIFOs update  //////////////////////
    r_vci_rsp_fifo_ins.update(vci_rsp_fifo_ins_get,
                              vci_rsp_fifo_ins_put,
                              vci_rsp_fifo_ins_data);

    r_vci_rsp_fifo_data.update(vci_rsp_fifo_data_get,
                               vci_rsp_fifo_data_put,
                               vci_rsp_fifo_data_data);

} // end transition()

//////////////////////////////////////////////////////////////////////////////////
tmpl(void)::genMoore()
//////////////////////////////////////////////////////////////////////////////////
{
    // VCI initiator response

    p_vci.rspack = true;

    // VCI initiator command

    switch (r_vci_cmd_fsm) {

    case CMD_IDLE:
        p_vci.cmdval  = false;
        p_vci.address = 0;
        p_vci.wdata   = 0;
        p_vci.be      = 0;
        p_vci.plen    = 0;
        p_vci.cmd     = vci_param::CMD_WRITE;
        p_vci.trdid   = 0;
        p_vci.pktid   = 0;
        p_vci.srcid   = 0;
        p_vci.cons    = false;
        p_vci.wrap    = false;
        p_vci.contig  = false;
        p_vci.clen    = 0;
        p_vci.cfixed  = false;
        p_vci.eop     = false;
        break;

    case CMD_DATA_UNC:
        p_vci.cmdval = true;
        p_vci.address = r_dcache_addr_save & ~0x3;
        switch( r_dcache_type_save ) 
        {
        case iss_t::DATA_READ:
            p_vci.wdata = 0;
            p_vci.be  = r_dcache_be_save.read();
            p_vci.cmd = vci_param::CMD_READ;
            p_vci.plen = soclib::common::fls(r_dcache_be_save.read())
                         - ffs(r_dcache_be_save.read()) + 1;
            break;
        case iss_t::DATA_LL:
            p_vci.wdata = 0;
            p_vci.be  = 0xF;
            p_vci.cmd = vci_param::CMD_LOCKED_READ;
            p_vci.plen = 4;
            break;
        case iss_t::DATA_SC:
            p_vci.wdata = r_dcache_wdata_save.read();
            p_vci.be  = 0xF;
            p_vci.cmd = vci_param::CMD_STORE_COND;
            p_vci.plen = 4;
            break;
        default:
            assert("this should not happen");
        }
        p_vci.trdid  = 0;
        p_vci.pktid  = 0;
        p_vci.srcid  = m_srcid;
        p_vci.cons   = false;
        p_vci.wrap   = false;
        p_vci.contig = true;
        p_vci.clen   = 0;
        p_vci.cfixed = false;
        p_vci.eop    = true;
        break;

    case CMD_DATA_WRITE:
        p_vci.cmdval  = true;
        p_vci.address = r_wbuf.getAddress(r_vci_cmd_cpt);
        p_vci.wdata   = r_wbuf.getData(r_vci_cmd_cpt);
        p_vci.be      = r_wbuf.getBe(r_vci_cmd_cpt);
        p_vci.plen    = soclib::common::fls(r_wbuf.getBe(r_vci_cmd_max))
                        - ffs(r_wbuf.getBe(r_vci_cmd_min))
                        + (r_vci_cmd_max - r_vci_cmd_min) * vci_param::B + 1;
        p_vci.cmd     = vci_param::CMD_WRITE;
        p_vci.trdid   = 0;
        p_vci.pktid   = 0;
        p_vci.srcid   = m_srcid;
        p_vci.cons    = false;
        p_vci.wrap    = false;
        p_vci.contig  = true;
        p_vci.clen    = 0;
        p_vci.cfixed  = false;
        p_vci.eop     = (r_vci_cmd_cpt == r_vci_cmd_max);
        break;

    case CMD_DATA_MISS:
        p_vci.cmdval = true;
        p_vci.address = r_dcache_addr_save & m_dcache_yzmask;
        p_vci.be     = 0xF;
        p_vci.plen   = m_dcache_words << 2;
        p_vci.cmd    = vci_param::CMD_READ;
        p_vci.trdid  = 0;
        p_vci.pktid  = 0;
        p_vci.srcid  = m_srcid;
        p_vci.cons   = false;
        p_vci.wrap   = false;
        p_vci.contig = true;
        p_vci.clen   = 0;
        p_vci.cfixed = false;
        p_vci.eop = true;
        break;

    case CMD_INS_MISS:
        p_vci.cmdval = true;
        p_vci.address = r_icache_addr_save & m_icache_yzmask;
        p_vci.be     = 0xF;
        p_vci.plen   = m_icache_words << 2;
        p_vci.cmd    = vci_param::CMD_READ;
        p_vci.trdid  = 0;
        p_vci.pktid  = 0;
        p_vci.srcid  = m_srcid;
        p_vci.cons   = false;
        p_vci.wrap   = false;
        p_vci.contig = true;
        p_vci.clen   = 0;
        p_vci.cfixed = false;
        p_vci.eop = true;
        break;

    case CMD_INS_UNC:
        p_vci.cmdval = true;
        p_vci.address = r_icache_addr_save & ~0x3;
        p_vci.be     = 0xF;
        p_vci.plen   = 4;
        p_vci.cmd    = vci_param::CMD_READ;
        p_vci.trdid  = 0;
        p_vci.pktid  = 0;
        p_vci.srcid  = m_srcid;
        p_vci.cons   = false;
        p_vci.wrap   = false;
        p_vci.contig = true;
        p_vci.clen   = 0;
        p_vci.cfixed = false;
        p_vci.eop = true;
        break;

    } // end switch r_vci_cmd_fsm

} // end genMoore()

}} // end namespace

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4




