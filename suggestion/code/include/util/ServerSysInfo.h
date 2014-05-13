//main.c


#include <stdio.h>


#define CACHE_TWEAK_FACTOR 64
#define SMLBUFSIZ ( 256 + CACHE_TWEAK_FACTOR)


typedef unsigned long long TIC_t;
typedef          long long SIC_t;


// This structure stores a frame's cpu tics used in history
// calculations.  It exists primarily for SMP support but serves
// all environments.
typedef struct CPU_t {
   TIC_t u, n, s, i, w, x, y;                             // as represented in /proc/stat
   TIC_t u_sav, s_sav, n_sav, i_sav, w_sav, x_sav, y_sav; // in the order of our display
   unsigned id;  // the CPU ID number
} CPU_t;

static CPU_t s_stCPU;


// State display *Helper* function to calc and display the state
// percentages for a single cpu.  In this way, we can support
// the following environments without the usual code bloat.
//    1) single cpu machines
//    2) modest smp boxes with room for each cpu's percentages
//    3) massive smp guys leaving little or no room for process
//       display and thus requiring the cpu summary toggle
static void summaryhlp (CPU_t *cpu)
{
   // we'll trim to zero if we get negative time ticks,
   // which has happened with some SMP kernels (pre-2.4?)
#define TRIMz(x)  ((tz = (SIC_t)(x)) < 0 ? 0 : tz)
   SIC_t u_frme, s_frme, n_frme, i_frme, w_frme, x_frme, y_frme, tot_frme, tz;
   float scale;

   u_frme = cpu->u - cpu->u_sav;
   s_frme = cpu->s - cpu->s_sav;
   n_frme = cpu->n - cpu->n_sav;
   i_frme = TRIMz(cpu->i - cpu->i_sav);
   w_frme = cpu->w - cpu->w_sav;
   x_frme = cpu->x - cpu->x_sav;
   y_frme = cpu->y - cpu->y_sav;
   tot_frme = u_frme + s_frme + n_frme + i_frme + w_frme + x_frme + y_frme;
   if (tot_frme < 1) tot_frme = 1;
   scale = 100.0 / (float)tot_frme;

   printf("cpuIdle==%.2f\n",(float)i_frme * scale);

   // display some kinda' cpu state percentages
   // (who or what is explained by the passed prefix)
#if 0
   show_special(
      0,
      fmtmk(
         States_fmts,
         pfx,
         (float)u_frme * scale,
         (float)s_frme * scale,
         (float)n_frme * scale,
         (float)i_frme * scale,
         (float)w_frme * scale,
         (float)x_frme * scale,
         (float)y_frme * scale
      )
   );
#endif

   // remember for next time around
   cpu->u_sav = cpu->u;
   cpu->s_sav = cpu->s;
   cpu->n_sav = cpu->n;
   cpu->i_sav = cpu->i;
   cpu->w_sav = cpu->w;
   cpu->x_sav = cpu->x;
   cpu->y_sav = cpu->y;

#undef TRIMz
}

        /*
         * This guy's modeled on libproc's 'five_cpu_numbers' function except
         * we preserve all cpu data in our CPU_t array which is organized
         * as follows:
         *    cpus[0] thru cpus[n] == tics for each separate cpu
         *    cpus[Cpu_tot]        == tics from the 1st /proc/stat line */
static void cpus_refresh ()
{
   static FILE *fp = NULL;
   int i;
   int num;
   // enough for a /proc/stat CPU line (not the intr line)
   char buf[SMLBUFSIZ];

   /* by opening this file once, we'll avoid the hit on minor page faults
      (sorry Linux, but you'll have to close it for us) */
   if (!fp) {
      if (!(fp = fopen("/proc/stat", "r")))
         perror("fopen");
   }
   rewind(fp);
   fflush(fp);

   // first value the last slot with the cpu summary line
   if (!fgets(buf, sizeof(buf), fp)) perror("fgets");
   num = sscanf(buf, "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu",
      &s_stCPU.u,
      &s_stCPU.n,
      &s_stCPU.s,
      &s_stCPU.i,
      &s_stCPU.w,
      &s_stCPU.x,
      &s_stCPU.y
   );
   if (num < 4)
         printf("failed /proc/stat read");
}


int main(int argc,char *argv[])
{
	memset(&s_stCPU,0,sizeof(s_stCPU));

	for(;;)
	{
		cpus_refresh();
		summaryhlp(&s_stCPU);
		sleep(3);
	}
	
	return 0;
}
