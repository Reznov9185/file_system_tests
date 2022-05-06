#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <random>
#include <errno.h>
#include <string.h>
#include <cpuid.h>

using namespace std;

#define KB(x) ((size_t)(x) << 10)
#define MB(x) ((size_t)(x) << 20)
#define GB(x) ((size_t)(x) << 30)

#define INTEL_FAM6_CORE_YONAH 0x0E

#define INTEL_FAM6_CORE2_MEROM 0x0F
#define INTEL_FAM6_CORE2_MEROM_L 0x16
#define INTEL_FAM6_CORE2_PENRYN 0x17
#define INTEL_FAM6_CORE2_DUNNINGTON 0x1D

#define INTEL_FAM6_NEHALEM 0x1E
#define INTEL_FAM6_NEHALEM_G 0x1F /* Auburndale / Havendale */
#define INTEL_FAM6_NEHALEM_EP 0x1A
#define INTEL_FAM6_NEHALEM_EX 0x2E

#define INTEL_FAM6_WESTMERE 0x25
#define INTEL_FAM6_WESTMERE_EP 0x2C
#define INTEL_FAM6_WESTMERE_EX 0x2F

#define INTEL_FAM6_SANDYBRIDGE 0x2A
#define INTEL_FAM6_SANDYBRIDGE_X 0x2D
#define INTEL_FAM6_IVYBRIDGE 0x3A
#define INTEL_FAM6_IVYBRIDGE_X 0x3E

#define INTEL_FAM6_HASWELL 0x3C
#define INTEL_FAM6_HASWELL_X 0x3F
#define INTEL_FAM6_HASWELL_L 0x45
#define INTEL_FAM6_HASWELL_G 0x46

#define INTEL_FAM6_BROADWELL 0x3D
#define INTEL_FAM6_BROADWELL_G 0x47
#define INTEL_FAM6_BROADWELL_X 0x4F
#define INTEL_FAM6_BROADWELL_D 0x56

#define INTEL_FAM6_SKYLAKE_L 0x4E
#define INTEL_FAM6_SKYLAKE 0x5E
#define INTEL_FAM6_SKYLAKE_X 0x55
#define INTEL_FAM6_KABYLAKE_L 0x8E
#define INTEL_FAM6_KABYLAKE 0x9E

#define INTEL_FAM6_CANNONLAKE_L 0x66

#define INTEL_FAM6_ICELAKE_X 0x6A
#define INTEL_FAM6_ICELAKE_D 0x6C
#define INTEL_FAM6_ICELAKE 0x7D
#define INTEL_FAM6_ICELAKE_L 0x7E
#define INTEL_FAM6_ICELAKE_NNPI 0x9D

#define INTEL_FAM6_TIGERLAKE_L 0x8C
#define INTEL_FAM6_TIGERLAKE 0x8D

#define INTEL_FAM6_COMETLAKE 0xA5
#define INTEL_FAM6_COMETLAKE_L 0xA6

/* "Small Core" Processors (Atom) */

#define INTEL_FAM6_ATOM_BONNELL 0x1C     /* Diamondville, Pineview */
#define INTEL_FAM6_ATOM_BONNELL_MID 0x26 /* Silverthorne, Lincroft */

#define INTEL_FAM6_ATOM_SALTWELL 0x36        /* Cedarview */
#define INTEL_FAM6_ATOM_SALTWELL_MID 0x27    /* Penwell */
#define INTEL_FAM6_ATOM_SALTWELL_TABLET 0x35 /* Cloverview */

#define INTEL_FAM6_ATOM_SILVERMONT 0x37     /* Bay Trail, Valleyview */
#define INTEL_FAM6_ATOM_SILVERMONT_D 0x4D   /* Avaton, Rangely */
#define INTEL_FAM6_ATOM_SILVERMONT_MID 0x4A /* Merriefield */

#define INTEL_FAM6_ATOM_AIRMONT 0x4C     /* Cherry Trail, Braswell */
#define INTEL_FAM6_ATOM_AIRMONT_MID 0x5A /* Moorefield */
#define INTEL_FAM6_ATOM_AIRMONT_NP 0x75  /* Lightning Mountain */

#define INTEL_FAM6_ATOM_GOLDMONT 0x5C   /* Apollo Lake */
#define INTEL_FAM6_ATOM_GOLDMONT_D 0x5F /* Denverton */

/* Note: the micro-architecture is "Goldmont Plus" */
#define INTEL_FAM6_ATOM_GOLDMONT_PLUS 0x7A /* Gemini Lake */

#define INTEL_FAM6_ATOM_TREMONT_D 0x86 /* Jacobsville */
#define INTEL_FAM6_ATOM_TREMONT 0x96   /* Elkhart Lake */
#define INTEL_FAM6_ATOM_TREMONT_L 0x9C /* Jasper Lake */

/* Xeon Phi */

#define INTEL_FAM6_XEON_PHI_KNL 0x57 /* Knights Landing */
#define INTEL_FAM6_XEON_PHI_KNM 0x85 /* Knights Mill */

#ifndef BLKGETSIZE
#define BLKGETSIZE _IO(0x12, 96) /* return device size */
#endif

#ifndef BLKPBSZGET
#define BLKSSZGET _IO(0x12, 104) /* get block device sector size */
#endif

// From https://github.com/torvalds/linux/blob/b95fffb9b4afa8b9aa4a389ec7a0c578811eaf42/tools/power/x86/turbostat/turbostat.c
unsigned int intel_model_duplicates(unsigned int model)
{
    switch (model)
    {
    case INTEL_FAM6_NEHALEM_EP:  /* Core i7, Xeon 5500 series - Bloomfield, Gainstown NHM-EP */
    case INTEL_FAM6_NEHALEM:     /* Core i7 and i5 Processor - Clarksfield, Lynnfield, Jasper Forest */
    case 0x1F:                   /* Core i7 and i5 Processor - Nehalem */
    case INTEL_FAM6_WESTMERE:    /* Westmere Client - Clarkdale, Arrandale */
    case INTEL_FAM6_WESTMERE_EP: /* Westmere EP - Gulftown */
        return INTEL_FAM6_NEHALEM;

    case INTEL_FAM6_NEHALEM_EX:  /* Nehalem-EX Xeon - Beckton */
    case INTEL_FAM6_WESTMERE_EX: /* Westmere-EX Xeon - Eagleton */
        return INTEL_FAM6_NEHALEM_EX;

    case INTEL_FAM6_XEON_PHI_KNM:
        return INTEL_FAM6_XEON_PHI_KNL;

    case INTEL_FAM6_BROADWELL_X:
    case INTEL_FAM6_BROADWELL_D: /* BDX-DE */
        return INTEL_FAM6_BROADWELL_X;

    case INTEL_FAM6_SKYLAKE_L:
    case INTEL_FAM6_SKYLAKE:
    case INTEL_FAM6_KABYLAKE_L:
    case INTEL_FAM6_KABYLAKE:
    case INTEL_FAM6_COMETLAKE_L:
    case INTEL_FAM6_COMETLAKE:
        return INTEL_FAM6_SKYLAKE_L;

    case INTEL_FAM6_ICELAKE_L:
    case INTEL_FAM6_ICELAKE_NNPI:
    case INTEL_FAM6_TIGERLAKE_L:
    case INTEL_FAM6_TIGERLAKE:
        return INTEL_FAM6_CANNONLAKE_L;

    case INTEL_FAM6_ATOM_TREMONT_D:
        return INTEL_FAM6_ATOM_GOLDMONT_D;

    case INTEL_FAM6_ATOM_TREMONT_L:
        return INTEL_FAM6_ATOM_TREMONT;

    case INTEL_FAM6_ICELAKE_X:
        return INTEL_FAM6_SKYLAKE_X;
    }
    return model;
}

// From https://github.com/torvalds/linux/blob/b95fffb9b4afa8b9aa4a389ec7a0c578811eaf42/tools/power/x86/turbostat/turbostat.c
static unsigned long long get_tsc_hz()
{
    unsigned int eax_crystal = 0;
    unsigned int ebx_tsc = 0;
    unsigned int crystal_hz = 0;
    unsigned int edx = 0;
    __cpuid(0x15, eax_crystal, ebx_tsc, crystal_hz, edx);
    if (ebx_tsc == 0)
    {
        return 0;
    }
    unsigned int fms, family, model, ebx, ecx;
    __cpuid(1, fms, ebx, ecx, edx);
    family = (fms >> 8) & 0xf;
    model = (fms >> 4) & 0xf;
    if (family == 0xf)
        family += (fms >> 20) & 0xff;
    if (family >= 6)
        model += ((fms >> 16) & 0xf) << 4;

    model = intel_model_duplicates(model);
    if (crystal_hz == 0)
    {
        switch (model)
        {
        case INTEL_FAM6_SKYLAKE_L: /* SKL */
            crystal_hz = 24000000; /* 24.0 MHz */
            break;
        case INTEL_FAM6_ATOM_GOLDMONT_D: /* DNV */
            crystal_hz = 25000000;       /* 25.0 MHz */
            break;
        case INTEL_FAM6_ATOM_GOLDMONT: /* BXT */
        case INTEL_FAM6_ATOM_GOLDMONT_PLUS:
            crystal_hz = 19200000; /* 19.2 MHz */
            break;
        default:
            crystal_hz = 0;
        }
    }
    return (unsigned long long)crystal_hz * ebx_tsc / eax_crystal;
}

uint64_t rdtsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc"
                         : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

int readTest()
{
    struct timeval start_time;
    struct timeval end_time;
    uint64_t time_rdtsc;
    double clock_speed;
    double time_elapsed, sec, ms;

    unsigned long long tsc_hz = get_tsc_hz();

    cout << "Test starts..." << endl;

    // Test block starts
    ofstream output_file1("out1.txt");
    unsigned int avg_cycle_counter = 0;
    unsigned int last_cycle_count = 0;
    unsigned int avg_variance_counter = 0;
    unsigned int avg_variance_counter_per_4kb_block = 0;
    cout << endl
         << "=================================================================" << endl;
    for (int i = 1; i <= 32; i++)
    {
        string file_name = "fileOf" + to_string(i) + "kb";
        int fd = open(file_name.c_str(), O_CREAT|O_RDWR);
        fallocate(fd, O_RDONLY, 0, i*1024);
        int size;
        char *c = (char *)calloc(KB(i), i * 1024);
        lseek(fd, 0, SEEK_SET);
        time_rdtsc = rdtsc();
        gettimeofday(&start_time, NULL);
        size = read(fd, c, i * 1024);
        if(size < 0){
            cout << size << "" << fd << strerror(errno) << endl;
        }
        gettimeofday(&end_time, NULL);
        time_rdtsc = rdtsc() - time_rdtsc;
        fsync(fd);
        close(fd);
        struct stat buffer;
        int status;
        status = stat(file_name.c_str(), &buffer);
        sec = (end_time.tv_sec - start_time.tv_sec);
        time_elapsed = sec;
        ms = (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        time_elapsed += ms / 1000;
        cout << "File of size: " << i << "KB. "
             << "ST Blocks " << buffer.st_blocks << ". BLKsize " << buffer.st_blksize;
        cout << ". Cycle elapsed(using rdtsc()): " << time_rdtsc << " cycle(s)" << endl;
        cout << "Time elapsed(using gettimeofday()): " << time_elapsed << " second(s)" << endl;
        cout << "Time elapsed(using clockspeed): " << (double)time_rdtsc / tsc_hz << " second(s)" << endl;
        avg_cycle_counter += time_rdtsc;
        output_file1 << (double)time_rdtsc / tsc_hz << endl;
        system(("rm -f " + file_name).c_str());
        if (i % 4 == 0)
        {
            avg_cycle_counter = avg_cycle_counter / 4;
            avg_variance_counter = avg_variance_counter / 4;
            avg_variance_counter_per_4kb_block = -1 * (avg_variance_counter_per_4kb_block - time_rdtsc);
            cout << "Report: For " << i << "KB block; Avg Cycle Count: "
                 << avg_cycle_counter << "; Avg Cycle Variance: "
                 << avg_variance_counter
                 << " ; 4kb block cycle Variance: " << avg_variance_counter_per_4kb_block
                 << endl;
            avg_variance_counter_per_4kb_block = 0;
            avg_cycle_counter = 0;
            cout << endl
                 << "=================================================================" << endl;
        }
        if (i % 4 == 1)
        {
            avg_variance_counter += time_rdtsc - last_cycle_count;
        }
    }
    // Test block ends
    cout << "Test ends." << endl;
    return 0;
}

int largeReadTest()
{
    struct timeval start_time;
    struct timeval end_time;
    uint64_t time_rdtsc;
    double clock_speed;
    double time_elapsed, sec, ms;

    unsigned long long tsc_hz = get_tsc_hz();

    cout << "Test starts..." << endl;
    // Test block starts
    ofstream output_file2("out2.txt");
    ofstream output_file2_range("out2_range.txt");
    cout << endl
         << "=================================================================" << endl;
    string file_name = "fileOf" + to_string(10) + "gb";
    int fd = open(file_name.c_str(), O_CREAT|O_RDWR);
    fallocate(fd, O_RDONLY, 0, GB(10));
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(1, 1024);
    for (int i = 0; i <= 128; i++)
    {
        int size;
        char *c = (char *)calloc(KB(4), KB(4));
        // int rand_offset = distr(gen);
        lseek(fd, i * KB(1), SEEK_SET);
        time_rdtsc = rdtsc();
        gettimeofday(&start_time, NULL);
        size = read(fd, c, KB(4));
        if(size < 0){
            cout << size << "" << fd << strerror(errno) << endl;
        }
        gettimeofday(&end_time, NULL);
        time_rdtsc = rdtsc() - time_rdtsc;
        struct stat buffer;
        int status;
        status = stat(file_name.c_str(), &buffer);
        sec = (end_time.tv_sec - start_time.tv_sec);
        time_elapsed = sec;
        ms = (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        time_elapsed += ms / 1000;
        output_file2 << (double)time_rdtsc / tsc_hz << endl;
        output_file2_range << i * KB(1) << endl;
        cout << "Seq. Offset from: " << i * KB(1) << "MB. "
             << "ST Blocks " << buffer.st_blocks << ". BLKsize " << buffer.st_blksize
             << ". Cycle elapsed(using rdtsc()): " << time_rdtsc << " cycle(s)" << endl;
        cout << "Time elapsed(using clockspeed): " << (double)time_rdtsc / tsc_hz << " second(s)" << endl;
        cout << endl
             << "=================================================================" << endl;
    }
    fsync(fd);
    close(fd);
    system(("rm -f " + file_name).c_str());
    cout << "Test ends." << endl;
    return 0;
}

int fileCacheTest(int cache_linesize)
{
    long int file_size_in_bytes = cache_linesize * 1024;
    struct timeval start_time;
    struct timeval start_time_1;
    struct timeval end_time;
    struct timeval end_time_1;
    uint64_t time_rdtsc;
    uint64_t time_rdtsc_1;
    double clock_speed;
    double clock_speed_1;
    double time_elapsed, sec, ms;
    double time_elapsed_1, sec_1, ms_1;

    unsigned long long tsc_hz = get_tsc_hz();

    cout << "==============================File Cache Size: " << cache_linesize << "bytes ===================================" << endl;
    cout << "Test starts..." << endl;
    // Test block starts
    ofstream output_file3("out3.txt");
    ofstream output_file3_range("out3_range.txt");
    ofstream output_file3_1("out3_1.txt");
    ofstream output_file3_range_1("out3_range_1.txt");

    cout << endl
         << "=================================================================" << endl;
    string file_name = "fileOf" + to_string(file_size_in_bytes) + "b";
    int fd = open(file_name.c_str(), O_CREAT|O_RDWR);
    fallocate(fd, O_RDONLY, 0, file_size_in_bytes);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(1, cache_linesize);
    uniform_int_distribution<> distr_1(cache_linesize * 2, cache_linesize * 1024);

    cout << endl
         << "===================For blocks out of the size of cache_linesize===================" << endl
         << endl;

    for (int i = 1; i <= 64; i++)
    {
        int size;
        char *c = (char *)calloc(KB(4), KB(4));
        int rand_offset_1 = i + (10*64);
        lseek(fd, rand_offset_1, SEEK_SET);

        // In size out of cache_linesize
        time_rdtsc_1 = rdtsc();
        gettimeofday(&start_time_1, NULL);
        size = read(fd, c, KB(4));
        if(size < 0){
            cout << size << "" << fd << strerror(errno) << endl;
        }
        gettimeofday(&end_time_1, NULL);
        time_rdtsc_1 = rdtsc() - time_rdtsc_1;

        struct stat buffer;
        int status;
        status = stat(file_name.c_str(), &buffer);

        // In size out of cache_linesize
        sec_1 = (end_time_1.tv_sec - start_time_1.tv_sec);
        time_elapsed_1 = sec_1;
        ms_1 = (end_time_1.tv_usec - start_time_1.tv_usec) / 1000.0;
        time_elapsed_1 += ms_1 / 1000;
        output_file3_1 << (double)time_rdtsc_1 / tsc_hz << endl;
        output_file3_range_1 << i << endl;
        cout << "Seq. Offset from: " << rand_offset_1 << "B. "
             << "ST Blocks " << buffer.st_blocks << ". BLKsize " << buffer.st_blksize
             << ". Cycle elapsed(using rdtsc()): " << time_rdtsc_1 << " cycle(s)" << endl;
        cout << "Time elapsed(using clockspeed): " << (double)time_rdtsc_1 / tsc_hz << " second(s)" << endl;
    }

    cout << endl
         << "===================For blocks within size of cache_linesize===================" << endl
         << endl;

    for (int i = 1; i <= 64; i++)
    {
        int size;
        char *c = (char *)calloc(KB(4), KB(4));
        int rand_offset = distr(gen);
        lseek(fd, rand_offset, SEEK_SET);

        // In size of cache_linesize
        time_rdtsc = rdtsc();
        gettimeofday(&start_time, NULL);
        size = read(fd, c, KB(4));
        gettimeofday(&end_time, NULL);
        time_rdtsc = rdtsc() - time_rdtsc;

        struct stat buffer;
        int status;
        status = stat(file_name.c_str(), &buffer);

        // In size of cache_linesize
        sec = (end_time.tv_sec - start_time.tv_sec);
        time_elapsed = sec;
        ms = (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        time_elapsed += ms / 1000;
        output_file3 << (double)time_rdtsc / tsc_hz << endl;
        output_file3_range << i << endl;
        cout << "Seq. Offset from: " << rand_offset << "B. "
             << "ST Blocks " << buffer.st_blocks << ". BLKsize " << buffer.st_blksize
             << ". Cycle elapsed(using rdtsc()): " << time_rdtsc << " cycle(s)" << endl;
        cout << "Time elapsed(using clockspeed): " << (double)time_rdtsc / tsc_hz << " second(s)" << endl;
    }

    system(("rm -f " + file_name).c_str());
    cout << "Test ends." << endl;
    return 0;
}

int fileAllocMethodTest()
{
    struct timeval start_time;
    struct timeval end_time;
    uint64_t time_rdtsc;
    double clock_speed;
    double time_elapsed, sec, ms;

    unsigned long long tsc_hz = get_tsc_hz();

    cout << "Test starts..." << endl;

    // Test block starts
    ofstream output_file4("out4.txt");
    ofstream output_file4_range("out4_range.txt");
    cout << endl
         << "=================================================================" << endl;
    for (int i = 8; i <= 24; i++)
    {
        string file_name = "fileOf" + to_string(i* 1024) + "mb";
        int fd = open(file_name.c_str(), O_CREAT|O_RDWR);
        fallocate(fd, O_RDONLY, 0, i*1024*1024);
        int size;
        char *c = (char *)calloc(KB(i), i * 1024);
        lseek(fd, 0, SEEK_SET);
        time_rdtsc = rdtsc();
        gettimeofday(&start_time, NULL);
        size = read(fd, c, i * 1024);
        if(size < 0){
            cout << size << "" << fd << strerror(errno) << endl;
        }
        gettimeofday(&end_time, NULL);
        time_rdtsc = rdtsc() - time_rdtsc;
        fsync(fd);
        close(fd);
        struct stat buffer;
        int status;
        status = stat(file_name.c_str(), &buffer);
        sec = (end_time.tv_sec - start_time.tv_sec);
        time_elapsed = sec;
        ms = (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        time_elapsed += ms / 1000;
        cout << "File of size: " << i << "MB. "
             << "ST Blocks " << buffer.st_blocks << ". BLKsize " << buffer.st_blksize;
        cout << ". Cycle elapsed(using rdtsc()): " << time_rdtsc << " cycle(s)" << endl;
        cout << "Time elapsed(using gettimeofday()): " << time_elapsed << " second(s)" << endl;
        cout << "Time elapsed(using clockspeed): " << (double)time_rdtsc / tsc_hz << " second(s)" << endl;
        output_file4 << (double)time_rdtsc / tsc_hz << endl;
        output_file4_range << buffer.st_blocks << endl;
        system(("rm -f " + file_name).c_str());
    }
    // Test block ends
    cout << "Test ends." << endl;
    cout << endl
         << "=================================================================" << endl;
    return 0;
}

int main()
{
    // For Question 1
    cout << "Question 1:" << endl;
    readTest();
    
    // For Question 2
    cout << "Question 2:" << endl;
    largeReadTest();

    // For Question 3
    cout << "Question 3:" << endl;
    fileCacheTest(sysconf(_SC_LEVEL1_DCACHE_LINESIZE));

    // // For Question 4
    cout << "Question 4:" << endl;
    fileAllocMethodTest();
    return 0;
}
