/*
 * x86 host CPU type initialization and host CPU functions
 *
 * Copyright 2021 SUSE LLC
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#ifndef HOST_CPU_H
#define HOST_CPU_H

void host_cpu_instance_init(X86CPU *cpu);
void host_cpu_max_instance_init(X86CPU *cpu);
bool host_cpu_realizefn(CPUState *cs, Error **errp);

void host_cpu_vendor_fms(char *vendor, int *family, int *model, int *stepping);

/* Note: Only safe for use on x86(-64) hosts */
#ifdef __x86_64__
static uint32_t host_cpu_phys_bits(void)
{
    uint32_t eax;
    uint32_t host_phys_bits;

    host_cpuid(0x80000000, 0, &eax, NULL, NULL, NULL);
    if (eax >= 0x80000008) {
        host_cpuid(0x80000008, 0, &eax, NULL, NULL, NULL);
        /*
         * Note: According to AMD doc 25481 rev 2.34 they have a field
         * at 23:16 that can specify a maximum physical address bits for
         * the guest that can override this value; but I've not seen
         * anything with that set.
         */
        host_phys_bits = eax & 0xff;
    } else {
        /*
         * It's an odd 64 bit machine that doesn't have the leaf for
         * physical address bits; fall back to 36 that's most older
         * Intel.
         */
        host_phys_bits = 36;
    }

    return host_phys_bits;
}
#else
static uint32_t host_cpu_phys_bits(void)
{
    return 40; // TCG_PHYS_ADDR_BITS
}
#endif

#endif /* HOST_CPU_H */
