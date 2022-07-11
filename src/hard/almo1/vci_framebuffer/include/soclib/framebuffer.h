/*
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
 *         Nicolas Pouillon <nipo@ssji.net>, 2006
 *
 * Maintainers: nipo
 */

#ifndef FBF_REGS_H
#define FBF_REGS_H

/******************************************************************************************
 * The segment attached to the framebuffer peripheral contains two parts:
 * - A 4 Mbytes subsegment containing the frame buffer itself.
 * - a 4 Kbytes subsegment containing the addresable registers.
 *****************************************************************************************/

#define FBF_REGS_BASE  0x400000    // 4 Mbytes

enum SoclibFbfRegisters 
{
    FBF_WIDTH_REG  = 0,      // number of pixels per line (read-only)
    FBF_HEIGHT_REG = 1,      // number of lines           (read-only)
    FBF_TYPE_REG   = 2,      // subsampling type
};

enum SoclibFbfTypes
{
    FBF_TYPE_YUV420 = 420,   // 1,5 bytes per pixel
    FBF_TYPE_YUV422 = 422,   // 2   bytes per pixel
    FBF_TYPE_RGB    = 0,     // 3   bytes per pixel
    FBF_TYPE_RGB16  = 16,    // 2   bytes per pixel
    FBF_TYPE_RGB32  = 32,    // 4   bytes per pixel
    FBF_TYPE_RGB256 = 256,   // 1   byte  per pixel
    FBF_TYPE_BW     = 1,     // 1/8 byte  per pixel
};

#endif /* FBF_REGS_H */

// Local Variables:
// tab-width: 4
// c-basic-offset: 4
// c-file-offsets:((innamespace . 0)(inline-open . 0))
// indent-tabs-mode: nil
// End:

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

