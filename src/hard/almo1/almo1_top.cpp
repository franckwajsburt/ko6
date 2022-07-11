/*
 * Implementation note:
 * This generic architecture supports both multi-tasking, and multi-cpuessing.
 * - The number of cpu cannot be larger than 8.
 * - An IO controler and a Frame buffer can be optionally activated.
 */

#include <systemc>
#include <limits>
#include <cstdlib>

#include "vci_signals.h"
#include "vci_param.h"
#include "mapping_table.h"
#include "gdbserver.h"

#include "mips32.h"
#include "vci_vgsb.h"
#include "vci_xcache_wrapper.h"
#include "vci_multi_tty.h"
#include "vci_timer.h"
#include "vci_multi_icu.h"
#include "vci_multi_dma.h"
#include "vci_block_device.h"
#include "vci_framebuffer.h"
#include "vci_simple_ram.h"
#include "alloc_elems.h"

#include "almo1_config.h"

#define ROM_TGTID   0
#define RAM_TGTID   1
#define TIMER_TGTID 2
#define FBF_TGTID   3
#define BD_TGTID    4
#define DMA_TGTID   5
#define TTY_TGTID   6
#define ICU_TGTID   7

#define DMA_SRCID   n_cpus
#define BD_SRCID   n_cpus+1

int _main(int argc, char *argv[])
{
    using namespace sc_core;
    using namespace soclib::caba;
    using namespace soclib::common;

    // VCI fields width definition
    //  cell_size   = 4;
    //  plen_size   = 8;
    //  addr_size   = 32;
    //  rerror_size = 1;
    //  clen_size   = 1;
    //  rflag_size  = 1;
    //  srcid_size  = 12;
    //  pktid_size  = 1;
    //  trdid_size  = 4;
    //  wrplen_size = 1;

    typedef VciParams<4,8,32,1,1,1,12,1,4,1> vci_param;

    size_t  n_cycles        = 1000000000;   // simulated cycles
    bool    icached         = true;         // Icache activated
    size_t  icache_sets     = 256;          // Icache parameters
    size_t  icache_words    = 4;
    size_t  icache_ways     = 1;
    bool    dcached         = true;         // Dcache activated
    size_t  dcache_sets     = 256;          // Dcache parameters
    size_t  dcache_words    = 4;
    size_t  dcache_ways     = 1;
    size_t  ram_latency     = 0;            // Ram latency (L2 MISS emulation)
    size_t  nb_irq_in       = 0;            // number of IRQ for ICU
    size_t  n_cpus          = 1;            // Number of cpu
    size_t  n_ttys          = 1;            // Number of terminals per cpuessor
    char    sys_name[256]   = "";           // pathname to the system binary
    char    app_name[256]   = "";           // pathname to the application binary
    bool    bd_ok           = false;        // Block Device activated
    char    bd_filename[256]= "";           // pathname for the bd file
    bool    fbf_ok          = false;        // FBF acctivated
    size_t  fbf_size        = 0;            // number of lines = number of pixels
    bool    debug_ok        = false;        // debug activated
    size_t  from_cycle      = 0;            // debug start cycle
    size_t  to_cycle        = 1000000000;   // debug end cycle
    bool    trace_ok        = false;        // cache trace activated
    char    trace_filename[256];
    FILE*   trace_file      = NULL;
    bool    stats_ok        = false;        // statistics activated
    char    stats_filename[256];
    FILE*   stats_file      = NULL;
    bool    wrong_option    = false;

    std::cerr << "**********************" << std::endl;
    std::cerr << "* almo1 202109071854 *" << std::endl;
    std::cerr << "**********************" << std::endl;

if (argc > 1)
{
    for( int n=1 ; n<argc ; n=n+2 )
    {
        if( (strcmp(argv[n],"-NCYCLES") == 0) && (n+1<argc) )
        {
            n_cycles = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-NCPUS") == 0) && (n+1<argc) )
        {
            n_cpus = atoi(argv[n+1]);
            if( (n_cpus > 8) || (n_cpus < 1) )
            {
                std::cerr << "The NCPUS argument cannot be greater than 8 and lesser than 1" << std::endl;
                exit(0);
            }
        }
        else if( (strcmp(argv[n],"-NTTYS") == 0) && (n+1<argc) )
        {
            n_ttys = atoi(argv[n+1]);
            if( (n_ttys > TTY_MAX_NR) || (n_ttys < 1) )
            {
                std::cerr << "The NTTYS argument cannot be greater than " << TTY_MAX_NR << " and lesser than 1" << std::endl;
                wrong_option = true;
                break;
            }
        }
        else if( (strcmp(argv[n],"-NICACHESET") == 0) && (n+1<argc) )
        {
            icache_sets = atoi(argv[n+1]);
            if(icache_sets == 0)
            {
                icached = false;
                icache_sets = 1;
            }
        }
        else if( (strcmp(argv[n],"-NICACHEWAY") == 0) && (n+1<argc) )
        {
            icache_ways = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-NICACHELEN") == 0) && (n+1<argc) )
        {
            icache_words = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-NDCACHESET") == 0) && (n+1<argc) )
        {
            dcache_sets = atoi(argv[n+1]);
            if(dcache_sets == 0)
            {
                dcached = false;
                dcache_sets = 1;
            }
        }
        else if( (strcmp(argv[n],"-NDCACHEWAY") == 0) && (n+1<argc) )
        {
            dcache_ways = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-NDCACHELEN") == 0) && (n+1<argc) )
        {
            dcache_words = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-TRACE") == 0) && (n+1<argc) )
        {
            trace_ok = true;
            if(n_cycles > 10000) n_cycles = 10000;
            strcpy(trace_filename, argv[n+1]);
            trace_file = fopen(trace_filename,"w+");
        }
        else if( (strcmp(argv[n],"-STATS") == 0) && (n+1<argc) )
        {
            stats_ok = true;
            strcpy(stats_filename, argv[n+1]);
            stats_file = fopen(stats_filename,"w+");
        }
        else if( (strcmp(argv[n],"-TOCYCLE") == 0) && (n+1<argc) )
        {
            debug_ok = true;
            to_cycle = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-DEBUG") == 0) && (n+1<argc) )
        {
            debug_ok = true;
            from_cycle = atoi(argv[n+1]);
        }
        else if( (strcmp(argv[n],"-KERNEL") == 0) && (n+1<argc) )
        {
            strcpy(sys_name, argv[n+1]) ;
        }
        else if( (strcmp(argv[n],"-APP") == 0) && (n+1<argc) )
        {
            strcpy(app_name, argv[n+1]) ;
        }
        else if( (strcmp(argv[n],"-BDFILE") == 0) && (n+1<argc) )
        {
            bd_ok = true;
            strcpy(bd_filename, argv[n+1]) ;
        }
        else if( (strcmp(argv[n],"-FBFSIZE") == 0) && (n+1<argc) )
        {
            fbf_ok = true;
            fbf_size = atoi(argv[n+1]) ;
        }
        else if( (strcmp(argv[n],"-RAMLATENCY") == 0) && (n+1<argc) )
        {
            ram_latency = atoi(argv[n+1]);
        }
        else
            wrong_option = true;
    }
}

nb_irq_in = 10 + n_ttys; // 8 timers (at most) + bd + dma + ttys

std::cerr << std::endl;
std::cerr << "Current Parameters: " << std::endl;
std::cerr << "    -KERNEL      = " << sys_name << std::endl;
std::cerr << "    -APP         = " << app_name << std::endl;
std::cerr << "    -NCYCLES     = " << n_cycles << std::endl;
std::cerr << "    -NCPUS       = " << n_cpus << std::endl;
std::cerr << "    -NTTYS       = " << n_ttys << std::endl;
std::cerr << "    -NICACHESET  = " << icache_sets << std::endl;
std::cerr << "    -NICACHEWAY  = " << icache_ways << std::endl;
std::cerr << "    -NICACHELEN  = " << icache_words << std::endl;
std::cerr << "    -NDCACHESET  = " << dcache_sets << std::endl;
std::cerr << "    -NDCACHEWAY  = " << dcache_ways << std::endl;
std::cerr << "    -NDCACHELEN  = " << dcache_words << std::endl;
std::cerr << "    -RAMLATENCY  = " << ram_latency << std::endl;
std::cerr << "    -BDFILE      = " << bd_filename << std::endl;
std::cerr << "    -FBFSIZE     = " << fbf_size << std::endl;
std::cerr << "    -TRACE       = " << trace_filename << std::endl;
std::cerr << "    -STATS       = " << stats_filename << std::endl;
if (debug_ok) 
{
  std::cerr << "    -DEBUG       = " << from_cycle << " to ";
  std::cerr << ((to_cycle < n_cycles) ? to_cycle : n_cycles) << std::endl;
}
else
{
  std::cerr << "    -DEBUG       = " << "not used" << std::endl;
}

/* parameters checking */
if ((wrong_option) || (strcmp(sys_name,"") == 0))
{
    if (strcmp(sys_name,"") == 0)
    std::cerr << std::endl << "Error: missing kernel filename" << std::endl ;

    std::cerr << std::endl << "Usage: ";
    std::cerr << argv[0] << " -KERNEL filename [OPTIONS]" << std::endl << std::endl;
    std::cerr << "   OPTIONS (the order does not matter):"   << std::endl << std::endl;
    std::cerr << "   -KERNEL      <filename> of the kernel in elf format (mandatory parameter) " << std::endl;
    std::cerr << "   -APP         <filename> of the user application in elf format" << std::endl;
    std::cerr << "   -NCYCLES     <number> of simulated cycles (eg. 10000)" << std::endl;
    std::cerr << "   -NCPUS       <number> of cpu (1 to 8)" << std::endl;
    std::cerr << "   -NTTYS       <number> of ttys (1 to "<< TTY_MAX_NR << ")" << std::endl;
    std::cerr << "   -NICACHESET  <number> of sets of the instruction cache (power of 2, eg. 512)" << std::endl;
    std::cerr << "   -NICACHEWAY  <number> of ways of the instruction cache (1=direct mapped to 4)" << std::endl;
    std::cerr << "   -NICACHELEN  <number> of words of the instruction cache line (2,4,8,16)" << std::endl;
    std::cerr << "   -NDCACHESET  <number> of sets of the data cache (power of 2, eg.512)" << std::endl;
    std::cerr << "   -NDCACHEWAY  <number> of ways of the data cache (1=direct mapped to 4)" << std::endl;
    std::cerr << "   -NDCACHELEN  <number> of words of the data cache line" << std::endl;
    std::cerr << "   -RAMLATENCY  <number> of cycles" << std::endl;
    std::cerr << "   -BDFILE      <filename> of disk image" << std::endl;
    std::cerr << "   -FBFSIZE     <number> of pixels per side (square window) (eg. 512)" << std::endl;
    std::cerr << "   -TRACE       <filename> where cache history is writteni (eg. trace)" << std::endl;
    std::cerr << "   -STATS       <Filename> where the runtime statistics are written (eg. stats)" << std::endl;
    std::cerr << "   -DEBUG       <start> and <last> cycle for execution trace (eg. 1000 6000)" << std::endl;
    std::cerr << std::endl;
    exit(0);
}

//////////////////////////////////////////////////////////////////////////
// Mapping Table
//////////////////////////////////////////////////////////////////////////

#define BASE(seg) SEG_##seg##_BASE
#define SIZE(seg) SEG_##seg##_SIZE

MappingTable maptab(32, IntTab(12), IntTab(12), 0xFFF00000);

maptab.add(Segment(".boot"  , BASE(RESET) , SIZE(RESET) , IntTab(ROM_TGTID),  icached));
maptab.add(Segment(".ktext" , BASE(KERNEL), SIZE(KERNEL), IntTab(RAM_TGTID),  icached));
maptab.add(Segment(".kdata" , BASE(KDATA) , SIZE(KDATA) , IntTab(RAM_TGTID),  dcached));
maptab.add(Segment(".kunc"  , BASE(KUNC)  , SIZE(KUNC)  , IntTab(RAM_TGTID) , false));
maptab.add(Segment(".text"  , BASE(CODE)  , SIZE(CODE)  , IntTab(RAM_TGTID),  icached));
maptab.add(Segment(".data"  , BASE(DATA)  , SIZE(DATA)  , IntTab(RAM_TGTID),  dcached));

maptab.add(Segment(".tty"   , BASE(TTY)   , SIZE(TTY)   , IntTab(TTY_TGTID),  false));
maptab.add(Segment(".dma"   , BASE(DMA)   , SIZE(DMA)   , IntTab(DMA_TGTID),  false));
maptab.add(Segment(".icu"   , BASE(ICU)   , SIZE(ICU)   , IntTab(ICU_TGTID),  false));
maptab.add(Segment(".timer" , BASE(TIMER) , SIZE(TIMER) , IntTab(TIMER_TGTID),false));
maptab.add(Segment(".bd"    , BASE(BD)    , SIZE(BD)    , IntTab(BD_TGTID),   false));
maptab.add(Segment(".fbf"   , BASE(FBF)   , SIZE(FBF)   , IntTab(FBF_TGTID),  false));

std::cout << std::endl << maptab << std::endl;

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
sc_clock        signal_clk("signal_clk", sc_time( 1, SC_NS ), 0.5 );
sc_signal<bool> signal_resetn("signal_resetn");

VciSignals<vci_param> *signal_vci_init_cpu =
    alloc_elems<VciSignals<vci_param> >("signal_vci_init_cpu", n_cpus);

VciSignals<vci_param> signal_vci_init_dma("signal_vci_init_dma");
VciSignals<vci_param> signal_vci_init_bd("signal_vci_init_bd");

VciSignals<vci_param> signal_vci_tgt_rom("signal_vci_tgt_rom");
VciSignals<vci_param> signal_vci_tgt_ram("signal_vci_tgt_ram");
VciSignals<vci_param> signal_vci_tgt_tim("signal_vci_tgt_tim");
VciSignals<vci_param> signal_vci_tgt_fbf("signal_vci_tgt_fbf");
VciSignals<vci_param> signal_vci_tgt_bd("signal_vci_tgt_bd");
VciSignals<vci_param> signal_vci_tgt_dma("signal_vci_tgt_dma");
VciSignals<vci_param> signal_vci_tgt_icu("signal_vci_tgt_icu");
VciSignals<vci_param> signal_vci_tgt_tty("signal_vci_tgt_tty");

sc_signal<bool> signal_false("signal_false");

sc_signal<bool> *signal_irq_cpu =
    alloc_elems<sc_signal<bool> >("signal_irq_cpu", n_cpus);

sc_signal<bool> *signal_irq_tim =
    alloc_elems<sc_signal<bool> >("signal_irq_tim", n_cpus);

sc_signal<bool> *signal_irq_dma =
    alloc_elems<sc_signal<bool> >("signal_irq_dma", 1 /* FW n_cpus */);

sc_signal<bool> **signal_irq_tty =
    alloc_elems<sc_signal<bool> >("signal_irq_tty", 1 /* FW n_cpus */, n_ttys);

sc_signal<bool> signal_irq_bd("signal_irq_bd");

////////////////////////////////////////////////////////////////////
// VCI Components : (n_cpus+2) initiators / (8) targets
// The BD & DMA components are both initiator & target.
// The BD and FBF components are optionnal.
////////////////////////////////////////////////////////////////////
// - srcid cpu : pid
// - srcid dma  : n_cpus   : dma
// - srcid bd  : n_cpus+1 : bd
/////////////////////////////////////////////////////////////////
// - tgtid rom
// - tgtid ram
// - tgtid timer
// - tgtid dma
// - tgtid fbf
// - tgtid bd
// - tgtid tty
// - tgtid icu
/////////////////////////////////////////////////////////////////
// The ICU controls at most 32 input IRQs:
//
// then, for cpu_i, the base INT number is (1 + i * irq_span)
// knowing that irq_span = 2 + n_ttys
//
// - IRQ[0]  : timer 0
// - ...(depending on n_cpus
// - IRQ[7]  : timer 7 
// - IRQ[8]  : bd   
// - IRQ[9]  : dma   
// - IRQ[10] : TTY0 
// - ...(depending on n_ttys)
// - IRQ[13] : TTY3 
/////////////////////////////////////////////////////////////////

Loader loader(sys_name, app_name);
GdbServer<Mips32ElIss>::set_loader(&loader);

VciXcacheWrapper<vci_param, GdbServer<Mips32ElIss> >* cpu[n_cpus];
for( size_t p = 0 ; p < n_cpus ; p++ )
{
    std::ostringstream cpu_name;
    cpu_name << "cpu_" << p;
    cpu[p] = new VciXcacheWrapper< vci_param, GdbServer<Mips32ElIss> >(
            cpu_name.str().c_str(),
            p,
            maptab,
            IntTab(p),
            icache_ways, icache_sets, icache_words,
            dcache_ways, dcache_sets, dcache_words);
}

VciSimpleRam<vci_param>* rom;
rom = new VciSimpleRam<vci_param>("rom",
        IntTab(ROM_TGTID),
        maptab,
        loader);

VciSimpleRam<vci_param>* ram;
ram = new VciSimpleRam<vci_param>("ram",
        IntTab(RAM_TGTID),
        maptab,
        loader,
        ram_latency);

std::vector<std::string> vect_names;
for( size_t p = 0 ; p < 1 /* FW n_cpus */ ; p++ )
{
    for (size_t t = 0; t < n_ttys; t++)
    {
        std::ostringstream term_name;
        // FW term_name << "cpu" << p << "_term" << t;
        term_name << "xterm" << t;
        vect_names.push_back(term_name.str().c_str());
    }
}
VciMultiTty<vci_param> *tty;
tty = new VciMultiTty<vci_param>("tty",
        IntTab(TTY_TGTID),
        maptab,
        vect_names);

VciMultiIcu<vci_param> *icu;
icu = new VciMultiIcu<vci_param>("icu",
        IntTab(ICU_TGTID),
        maptab,
        nb_irq_in,
        n_cpus);


VciTimer<vci_param>* timer;
timer = new VciTimer<vci_param>("timer",
        IntTab(TIMER_TGTID),
        maptab,
        n_cpus);

VciMultiDma<vci_param>* dma;
dma = new VciMultiDma<vci_param>("dma",
        maptab,
        IntTab(DMA_SRCID),
        IntTab(DMA_TGTID),
        64,
        1 /* FW n_cpus*/);

VciFrameBuffer<vci_param>* fbf;
if( fbf_ok ) fbf = new VciFrameBuffer<vci_param>("fbf",
        IntTab(FBF_TGTID),
        maptab,
        fbf_size, fbf_size);

VciBlockDevice<vci_param>* bd;
if( bd_ok ) bd = new VciBlockDevice<vci_param>("bd",
        maptab,
        IntTab(BD_SRCID),
        IntTab(BD_TGTID),
        bd_filename,
        512,
        0);

VciVgsb<vci_param>* bus;
bus = new VciVgsb<vci_param>("bus",
        maptab,
        n_cpus+2,
        8);

//////////////////////////////////////////////////////////////////////////
// Net-List
//////////////////////////////////////////////////////////////////////////
for ( size_t p = 0 ; p < n_cpus ; p++)
{
    cpu[p]->p_clk      (signal_clk);
    cpu[p]->p_resetn   (signal_resetn);
    cpu[p]->p_vci      (signal_vci_init_cpu[p]);
    cpu[p]->p_irq[0]   (signal_irq_cpu[p]);
    cpu[p]->p_irq[1]   (signal_false);
    cpu[p]->p_irq[2]   (signal_false);
    cpu[p]->p_irq[3]   (signal_false);
    cpu[p]->p_irq[4]   (signal_false);
    cpu[p]->p_irq[5]   (signal_false);
}

ram->p_clk      (signal_clk);
ram->p_resetn   (signal_resetn);
ram->p_vci      (signal_vci_tgt_ram);

rom->p_clk      (signal_clk);
rom->p_resetn   (signal_resetn);
rom->p_vci      (signal_vci_tgt_rom);

tty->p_clk      (signal_clk);
tty->p_resetn   (signal_resetn);
tty->p_vci      (signal_vci_tgt_tty);
for (size_t p = 0 ; p < 1 /* FW n_cpus */ ; p++)
    for (size_t t = 0 ; t < n_ttys ; t++)
        tty->p_irq[p * n_ttys + t] (signal_irq_tty[p][t]);

icu->p_clk      (signal_clk);
icu->p_resetn   (signal_resetn);
icu->p_vci      (signal_vci_tgt_icu);

for (size_t p = 0 ; p < n_cpus ; p++)
{
    icu->p_irq_out[p] (signal_irq_cpu[p]);
}
std::cout << "  - IRQ connection\n";
for (size_t p = 0 ; p < 8 ; p++)
{
    if (p < n_cpus) {
        icu->p_irq_in[p]     (signal_irq_tim[p]);
        std::cout << "    => icu.irq  " << p << " <-- timer " << p << std::endl; 
    } else
        icu->p_irq_in[p]     (signal_false);
    
    // FW size_t base = 1 + p * (2 + n_ttys);
    // FW icu->p_irq_in[base + 1] (signal_irq_dma[p]);


    /* FW
    for (size_t t = 0; t < n_ttys; t++) {
        icu->p_irq_in[base + 2 + t] (signal_irq_tty[p][t]);
        std::cout << "irq " << base + 2 + t << "tty " << p << " " << t; 
    }
    */
}

if( bd_ok ) {
    icu->p_irq_in[8] (signal_irq_bd);
    std::cout << "    => icu.irq  " << 8 << " <-- bd " << std::endl; 
} else
    icu->p_irq_in[8] (signal_false);

icu->p_irq_in[9] (signal_irq_dma[0]); // FW
std::cout << "    => icu.irq  " << 9 << " <-- dma " << std::endl; 

// FW
for (size_t t = 0; t < n_ttys; t++) {
    icu->p_irq_in[10 + t] (signal_irq_tty[0][t]);
    std::cout << "    => icu.irq " << 10 + t << " <-- tty " << t << std::endl; 
}

timer->p_clk    (signal_clk);
timer->p_resetn (signal_resetn);
timer->p_vci    (signal_vci_tgt_tim);
for (size_t p = 0 ; p < n_cpus ; p++)
    timer->p_irq[p] (signal_irq_tim[p]);

dma->p_clk          (signal_clk);
dma->p_resetn       (signal_resetn);
dma->p_vci_initiator(signal_vci_init_dma);
dma->p_vci_target   (signal_vci_tgt_dma);
for (size_t p = 0 ; p < 1 /* FW n_cpus */ ; p++)
    dma->p_irq[p] (signal_irq_dma[p]);

if( fbf_ok )
{
    fbf->p_clk      (signal_clk);
    fbf->p_resetn   (signal_resetn);
    fbf->p_vci      (signal_vci_tgt_fbf);
}

if( bd_ok )
{
    bd->p_clk          (signal_clk);
    bd->p_resetn       (signal_resetn);
    bd->p_vci_initiator(signal_vci_init_bd);
    bd->p_vci_target   (signal_vci_tgt_bd);
    bd->p_irq          (signal_irq_bd);
}

bus->p_clk      (signal_clk);
bus->p_resetn   (signal_resetn);
for ( size_t p = 0 ; p < n_cpus ; p++)
{
    bus->p_to_initiator[p]  (signal_vci_init_cpu[p]);
}
bus->p_to_initiator[DMA_SRCID] (signal_vci_init_dma);
bus->p_to_initiator[BD_SRCID]  (signal_vci_init_bd);
bus->p_to_target[ROM_TGTID]    (signal_vci_tgt_rom);
bus->p_to_target[RAM_TGTID]    (signal_vci_tgt_ram);
bus->p_to_target[TIMER_TGTID]  (signal_vci_tgt_tim);
bus->p_to_target[DMA_TGTID]    (signal_vci_tgt_dma);
bus->p_to_target[FBF_TGTID]    (signal_vci_tgt_fbf);
bus->p_to_target[BD_TGTID]     (signal_vci_tgt_bd);
bus->p_to_target[TTY_TGTID]    (signal_vci_tgt_tty);
bus->p_to_target[ICU_TGTID]    (signal_vci_tgt_icu);

//////////////////////////////////////////////////////////////////////////
// simulation
//////////////////////////////////////////////////////////////////////////
std::cerr << std::endl;

struct timeval t1,t2;
gettimeofday(&t1, NULL);

if( !bd_ok ) signal_vci_init_bd.cmdval = false;
if( !bd_ok ) signal_vci_tgt_bd.rspval = false;
if( !fbf_ok ) signal_vci_tgt_fbf.rspval = false;

signal_resetn = false;
sc_start( sc_time( 1, SC_NS ) ) ;

std::cout << "\n>>> Type <ctrl-C> to terminate the simulation! <<<\n\n";

signal_resetn = true;
for ( size_t n=1 ; n<n_cycles ; n++ )
{
    sc_start( sc_time( 1 , SC_NS ) ) ;

    if( (n % 1000000) == 0)
    {
        gettimeofday(&t2, NULL);
        uint64_t ms1 = (uint64_t) t1.tv_sec  * 1000ULL +
                       (uint64_t) t1.tv_usec / 1000;
        uint64_t ms2 = (uint64_t) t2.tv_sec  * 1000ULL +
                       (uint64_t) t2.tv_usec / 1000;
        std::cerr << "\r"
                  << "### cycle = " << std::dec << n
                  << " / frequency = "
                  << (double) 1000000 / (double) (ms2 - ms1) << "Khz";
                  //<< std::endl;

        gettimeofday(&t1, NULL);
    }

    if( stats_ok && (n%10 == 0) )
        cpu[0]->file_stats(stats_file);
    if( trace_ok )
        cpu[0]->file_trace(trace_file);

    if( debug_ok && (n > from_cycle) && (n < to_cycle) )
    {
        std::cout << "***************** cycle " << std::dec << n
            << " ***********************" << std::endl;
        for( size_t p = 0 ; p < n_cpus ; p++)
            cpu[p]->print_trace();
        //bus->print_trace();
        //timer->print_trace();
        //rom->print_trace();
        //ram->print_trace();
    }
}

/* forcing xterm to quit too */
//kill(0, SIGINT);
system("killall xterm soclib-fb");
return 0;
}

void quit(int)
{
sc_core::sc_stop();
}

int sc_main (int argc, char *argv[])
{
signal(SIGINT, quit);
signal(SIGPIPE, quit);

try {
    return _main(argc, argv);
} catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
} catch (...) {
    std::cerr << "Unknown exception occured" << std::endl;
    throw;
}
return 0;
}

// Local Variables:
// tab-width: 4;
// c-basic-offset: 4;
// c-file-offsets:((innamespace . 0)(inline-open . 0));
// indent-tabs-mode: nil;
// End:
//
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=4:softtabstop=4

