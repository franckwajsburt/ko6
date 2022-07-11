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

#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../../../include/soclib/framebuffer.h"
#include "../include/vci_framebuffer.h"

namespace soclib {
namespace caba {

using namespace soclib;

#define tmpl(t) template<typename vci_param> t VciFrameBuffer<vci_param>

//////////////////////////////////////
tmpl(void)::write_fb(size_t     index,
                     vci_data_t wdata, 
                     vci_be_t   be)
{
    if ( vci_param::B == 4 ) // VCI DATA == 32 bits 
    {
        uint32_t   mask  = (uint32_t)vci_param::be2mask(be);
        uint32_t*  tab   = (uint32_t*)m_fb_controller.surface();
        tab[index] = (wdata & mask) | (tab[index] & ~mask);
    }
    else                     // VCI DATA == 64 bits
    {
        uint64_t   mask  = (uint64_t)vci_param::be2mask(be);
        uint64_t*  tab   = (uint64_t*)m_fb_controller.surface();
        tab[index] = (wdata & mask) | (tab[index] & ~mask);
    }
}

////////////////////////
tmpl(void)::transition()
{
	if ( not p_resetn.read() ) 
    {
		r_fsm_state  = IDLE;
		return;
	}

    // buffer display
	switch ( m_defered_timeout ) 
    {
	    case 0:   break;
	    case 1:
	    {
		struct timeval tv, diff_tv;
		gettimeofday(&tv, NULL);

		/* compute time since last update */
		if (tv.tv_usec < m_last_update.tv_usec)
        {
			diff_tv.tv_usec =
			    1000000 + tv.tv_usec - m_last_update.tv_usec;
			diff_tv.tv_sec = tv.tv_sec - m_last_update.tv_sec - 1;
		}
        else
        {
			diff_tv.tv_usec = tv.tv_usec - m_last_update.tv_usec;
			diff_tv.tv_sec = tv.tv_sec - m_last_update.tv_sec;
		}
		/* 25 updates per second is enough */
		if (diff_tv.tv_sec > 1 ||
		    (diff_tv.tv_sec == 0 && diff_tv.tv_usec > 40000))
        {
			m_fb_controller.update();
			m_defered_timeout = 0;
			m_last_update = tv;
		}
        else 
        {
			m_defered_timeout = 30;
		}
		break;
	    }
	    default:  --m_defered_timeout;
	}
	
    // VCI target FSM
    switch ( r_fsm_state.read() )
    {
        //////////
        case IDLE:   // consume one VCI CMD flit / no response in this state
        {
            if ( not p_vci.cmdval.read() ) break;

            vci_addr_t address    = p_vci.address.read();
            bool       error      = true;
            bool       config     = false;

            std::list<soclib::common::Segment>::iterator seg;
            for ( seg = m_seglist.begin() ; seg != m_seglist.end() ; seg++ )
            {
                if( seg->contains( address ) )
                {
                    vci_addr_t regs_base = m_seg_base + FBF_REGS_BASE;
                    if( address >= regs_base ) config = true;
                    error = false;
                    break;
                }
            }

            r_srcid    = p_vci.srcid.read();
            r_trdid    = p_vci.trdid.read();
            r_pktid    = p_vci.pktid.read();

            /////////////////////////////////////////////////////////////////////////
            if( config )                                    // register access
            {
                size_t index = ((size_t)(address >> 2)) & 0x3;
                r_index      = index;
  
                ////////////////////// legal read register
                if ( (p_vci.cmd.read() == vci_param::CMD_READ) and
                     (p_vci.eop.read() == true ) )
                {
                    if ( index == FBF_WIDTH_REG  )
                    {
                        r_data      = m_width;
                        r_fsm_state = READ_REG_RSP; 
                    }
                    else if ( index == FBF_HEIGHT_REG ) 
                    {
                        r_data      = m_height;
                        r_fsm_state = READ_REG_RSP; 
                    }
                    else if ( index == FBF_TYPE_REG ) 
                    {
                        r_data      = m_subsampling;
                        r_fsm_state = READ_REG_RSP; 
                    }
                    else
                    {
                        std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                                  << " : undefined register in config segment" 
                                  << std::endl; 

                        r_fsm_state = ERROR_RSP;
                    }
                }
                ////////////////////// legal write register
                else if ( (p_vci.cmd.read() == vci_param::CMD_WRITE) and
                          (p_vci.eop.read() == true ) )
                {
                    std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                              << " : no writable register in config segment" 
                              << std::endl; 

                    r_fsm_state = WRITE_REG_RSP;
                }
                ///////////////////// illegal one flit register access
                else if ( p_vci.eop.read() )
                {
                    std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                              << " : one flit illegal command in config segment" 
                              << std::endl; 
                    r_fsm_state = ERROR_RSP;
                }
                //////////////////// illegal multi flit register access
                else
                {
                    std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                              << " : multi-flit illegal command in config segment"
                              << std::endl; 
                    r_fsm_state = ERROR_CMD;
                }
            }
            /////////////////////////////////////////////////////////////////////////
            else                                          // frame buffer access
            {
                size_t index; 
                if ( vci_param::B == 4 )            // VCI DATA == 32 bits
                {
                    index = (size_t)((address - m_seg_base)>>2);
                }
                else                                // VCI DATA == 64 bits
                {
                    index = (size_t)((address - m_seg_base)>>3);
                }

                ///////////////////// legal write FBF
                if ( not error and (p_vci.cmd.read() == vci_param::CMD_WRITE) )
                {
                    write_fb( index, 
                              p_vci.wdata.read(),
                              p_vci.be.read() );

                    r_index = index + 1;

                    if ( not p_vci.eop.read() )
                    {
			            r_fsm_state = WRITE_BUF_CMD;
                    }
                    else
                    {
			            r_fsm_state = WRITE_BUF_RSP;
			            m_defered_timeout  = 30;
		            }
                }
                ///////////////////// legal read FBF
                else if ( not error and (p_vci.cmd.read() == vci_param::CMD_READ) )
                {
                    if ( vci_param::B == 4 )        // VCI DATA == 32 bits
                    {
	                    r_flit_count = p_vci.plen.read()>>2;
                    }
                    else                            // VCI DATA == 64 bits
                    {
                        r_flit_count = p_vci.plen.read()>>3;
                    }

                    r_index     = index;
                    r_fsm_state = READ_BUF_RSP;
                }
                ////////////////////// single flit out of segment access
                else if ( error and p_vci.eop.read() )
                {
                    std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                              << " : one flit out of segment access"
                              << std::endl; 
                    r_fsm_state = ERROR_RSP;
                }
                ////////////////////// multi flit out of segment access
                else
                {
                    std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                              << " : multi flit out of segment access"
                              << std::endl; 
                    r_fsm_state = ERROR_CMD;
                }
            }
            break;
        }
        //////////////////
        case READ_BUF_RSP:  // return a flit from buffer in case of read
        { 
            if ( not p_vci.rspack.read() ) break;

            r_flit_count = r_flit_count.read() - 1;
            r_index      = r_index.read() + 1;
            if ( r_flit_count.read() == 1 )  r_fsm_state = IDLE;

            break;
        }
        ///////////////////
        case WRITE_BUF_CMD:  // write another flit in buffer
        {
            if ( not p_vci.cmdval.read() ) break;

            vci_addr_t address = p_vci.address.read();
            size_t index;
            if ( vci_param::B == 4 )        // VCI DATA == 32 bits
            {
                index = (size_t)((address - m_seg_base)>>2);
            }
            else                            // VCI DATA == 64 bits
            {
                index = (size_t)((address - m_seg_base)>>3);
            }

            if ( r_index.read() != index )
            {
                std::cout << "VCI_FRAMEBUFFER ERROR " << name()
                          << " : addresses must be contiguous "
                          << " in a write burst" << std::endl;

                if ( p_vci.eop.read() ) r_fsm_state = ERROR_RSP;
                else                    r_fsm_state = ERROR_CMD;
                break;
            }

            write_fb( index,
                      p_vci.wdata.read(),
                      p_vci.be.read() );

            r_index = index + 1;

            if ( p_vci.eop.read() )
            {
		        r_fsm_state = WRITE_BUF_RSP;
	            m_defered_timeout = 30;
	        }
            break;
        }
        ///////////////////
        case WRITE_BUF_RSP:   // returns single flit write response 
        {
            if ( p_vci.rspack.read() ) r_fsm_state = IDLE;
            break;
        }
        //////////////////
        case READ_REG_RSP:
        {
            if ( p_vci.rspack.read() ) r_fsm_state = IDLE;
            break;
        }
        ///////////////////
        case WRITE_REG_RSP:   // write data to relevant register
        {
            if ( p_vci.rspack.read() ) r_fsm_state = IDLE;
            break;
        }
        ///////////////
        case ERROR_CMD:   // wait last flit for erroneous command packet
        {
            if( p_vci.cmdval.read() && p_vci.eop.read() )
            {
                r_fsm_state = ERROR_RSP;
            }
            break;
        }
        ///////////////
        case ERROR_RSP:   // returns single flit error response 
        {
            if( p_vci.rspack.read() ) r_fsm_state = IDLE;
            break;
        }
    }
}  // end transition()

//////////////////////
tmpl(void)::genMoore()
{
    if ( (r_fsm_state.read() == IDLE)      or
         (r_fsm_state.read() == WRITE_BUF_CMD) )
    {
        p_vci.cmdack  = true;
        p_vci.rspval  = false;
        p_vci.rdata   = 0;
        p_vci.rsrcid  = 0;
        p_vci.rtrdid  = 0;
        p_vci.rpktid  = 0;
        p_vci.rerror  = vci_param::ERR_NORMAL;
        p_vci.reop    = true;
    }
    else if ( (r_fsm_state.read() == WRITE_BUF_RSP ) or
              (r_fsm_state.read() == WRITE_REG_RSP ) )
    {
        p_vci.cmdack  = false;
        p_vci.rspval  = true;
        p_vci.rdata   = 0;
        p_vci.rsrcid  = r_srcid.read();
        p_vci.rtrdid  = r_trdid.read();
        p_vci.rpktid  = r_pktid.read();
        p_vci.rerror  = vci_param::ERR_NORMAL;
        p_vci.reop    = true;
    }
    else if ( r_fsm_state.read() == READ_BUF_RSP )
    {
        p_vci.cmdack  = false;
        p_vci.rspval  = true;
        p_vci.rsrcid  = r_srcid.read();
        p_vci.rtrdid  = r_trdid.read();
        p_vci.rpktid  = r_pktid.read();
        p_vci.rerror  = vci_param::ERR_NORMAL;
        p_vci.reop    = (r_flit_count.read() == 1);

        if (vci_param::B == 4)
        {
            uint32_t* tab = (uint32_t*)m_fb_controller.surface();
            p_vci.rdata   = tab[r_index.read()];
        }
        else
        {
            uint64_t* tab = (uint64_t*)m_fb_controller.surface();
            p_vci.rdata   = tab[r_index.read()];
        }
    }
    else if ( r_fsm_state.read() == READ_REG_RSP )
    {
        p_vci.cmdack  = false;
        p_vci.rspval  = true;
        p_vci.rsrcid  = r_srcid.read();
        p_vci.rtrdid  = r_trdid.read();
        p_vci.rpktid  = r_pktid.read();
        p_vci.rerror  = vci_param::ERR_NORMAL;
        p_vci.reop    = true;
        p_vci.rdata   = r_data.read();
    }
    else if ( r_fsm_state.read() == ERROR_CMD )
    {
        p_vci.cmdack  = true;
        p_vci.rspval  = false;
        p_vci.rdata   = 0;
        p_vci.rsrcid  = 0;
        p_vci.rtrdid  = 0;
        p_vci.rpktid  = 0;
        p_vci.rerror  = vci_param::ERR_NORMAL;
        p_vci.reop    = true;
    }
    else if ( r_fsm_state.read() == ERROR_RSP )
    {
        p_vci.cmdack  = false;
        p_vci.rspval  = true;
        p_vci.rdata   = 0;
        p_vci.rsrcid  = r_srcid.read();
        p_vci.rtrdid  = r_trdid.read();
        p_vci.rpktid  = r_pktid.read();
        p_vci.rerror  = vci_param::ERR_GENERAL_DATA_ERROR;
        p_vci.reop    = true;
    }
    else
    {
        assert(false);
    }
}

///////////////////////////
tmpl(/**/)::VciFrameBuffer(
    sc_module_name       name,
    const IntTab         &index,
    const MappingTable   &mt,
    unsigned long        width,
    unsigned long        height,
    int                  subsampling )
	: caba::BaseModule(name),
     
      p_clk("clk"),
      p_resetn("resetn"),
      p_vci("vci"),

      r_fsm_state( "r_fsm_state" ),
      r_flit_count( "r_flit_count" ),
      r_index( "r_index" ),
      r_srcid( "r_srcid" ),
      r_trdid( "r_trdid" ),
      r_pktid( "r_pktid" ),
      r_data( "r_data" ),

      m_seglist( mt.getSegmentList(index) ),
      m_width( width ),
      m_height( height ),
      m_subsampling( subsampling ),
	  m_defered_timeout( 0 ),
      m_fb_controller( (const char *)name, width, height, subsampling )
{
    std::cout << "  - Building VciFramebuffer : " << name << std::endl
              << "    => width  = " << std::dec << width << std::endl
              << "    => heigth = " << std::dec << height << std::endl;

    assert( (m_seglist.empty() == false) and
    "VCI_FRAMEBUFFER error : no segment allocated");

    size_t fbf_size = m_fb_controller.surface_size();

    assert( (fbf_size <= FBF_REGS_BASE) and
    "VCI_FRAMEBUFFER error : frame buffer size too large");
    
    assert( ((vci_param::B == 4) or (vci_param::B == 8)) and
    "VCI_FRAMEBUFFER error : VCI DATA width must be 32 or 64 bits");

    // get segment
    std::list<soclib::common::Segment>::iterator seg;
    for (seg = m_seglist.begin(); seg != m_seglist.end(); seg++)
    {
        m_seg_base = seg->baseAddress();

        std::cout << "    => segment " << seg->name()
                  << " / base = " << std::hex << seg->baseAddress()
                  << " / size = " << seg->size() << std::endl;

        assert( (seg->size() >= (FBF_REGS_BASE + 0x1000)) and
        "VCI_FRAMEBUFFER error : segment size too small");
    }

	SC_METHOD(transition);
	dont_initialize();
	sensitive << p_clk.pos();
	
	SC_METHOD(genMoore);
	dont_initialize();
	sensitive << p_clk.neg();
}

/////////////////////////////
tmpl(/**/)::~VciFrameBuffer()
{
}

/////////////////////////
tmpl(void)::print_trace()
{
    const char* state_str[] = { "IDLE",
                                "READ_BUF_RSP",
                                "WRITE_BUF_CMD",
                                "WRITE_BUF_RSP",
                                "READ_REG_RSP",
                                "WRITE_REG_RSP",
                                "ERROR_CMD",
                                "ERROR_RSP" };

    std::cout << "FRAMEBUFFER " << name()
              << " : state = " << state_str[r_fsm_state.read()] 
              << " / index = " << r_index.read()
              << " / count = " << r_flit_count.read() << std::endl; 
}

}}

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

