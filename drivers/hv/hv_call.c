// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023, Microsoft Corporation.
 *
 * Hypercall helper functions shared between mshv modules.
 *
 * Authors:
 *   Nuno Das Neves <nunodasneves@linux.microsoft.com>
 */

#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/mshyperv.h>

#define HV_GET_REGISTER_BATCH_SIZE	\
	(HV_HYP_PAGE_SIZE / sizeof(union hv_register_value))
#define HV_SET_REGISTER_BATCH_SIZE	\
	((HV_HYP_PAGE_SIZE - sizeof(struct hv_input_set_vp_registers)) \
		/ sizeof(struct hv_register_assoc))

int hv_call_get_vp_registers(u32 vp_index, u64 partition_id, u16 count,
			     union hv_input_vtl input_vtl,
			     struct hv_register_assoc *registers)
{
	struct hv_input_get_vp_registers *input_page;
	union hv_register_value *output_page;
	u16 completed = 0;
	unsigned long remaining = count;
	int rep_count, i;
	u64 status = HV_STATUS_SUCCESS;
	unsigned long flags;

	local_irq_save(flags);

	input_page = *this_cpu_ptr(hyperv_pcpu_input_arg);
	output_page = *this_cpu_ptr(hyperv_pcpu_output_arg);

	input_page->partition_id = partition_id;
	input_page->vp_index = vp_index;
	input_page->input_vtl.as_uint8 = input_vtl.as_uint8;
	input_page->rsvd_z8 = 0;
	input_page->rsvd_z16 = 0;

	while (remaining) {
		rep_count = min(remaining, HV_GET_REGISTER_BATCH_SIZE);
		for (i = 0; i < rep_count; ++i)
			input_page->names[i] = registers[i].name;

		status = hv_do_rep_hypercall(HVCALL_GET_VP_REGISTERS, rep_count,
					     0, input_page, output_page);
		if (!hv_result_success(status))
			break;
		completed = hv_repcomp(status);
		for (i = 0; i < completed; ++i)
			registers[i].value = output_page[i];

		registers += completed;
		remaining -= completed;
	}
	local_irq_restore(flags);

	return hv_status_to_errno(status);
}

int hv_call_set_vp_registers(u32 vp_index, u64 partition_id, u16 count,
			     union hv_input_vtl input_vtl,
			     struct hv_register_assoc *registers)
{
	struct hv_input_set_vp_registers *input_page;
	u16 completed = 0;
	unsigned long remaining = count;
	int rep_count;
	u64 status = HV_STATUS_SUCCESS;
	unsigned long flags;

	local_irq_save(flags);
	input_page = *this_cpu_ptr(hyperv_pcpu_input_arg);

	input_page->partition_id = partition_id;
	input_page->vp_index = vp_index;
	input_page->input_vtl.as_uint8 = input_vtl.as_uint8;
	input_page->rsvd_z8 = 0;
	input_page->rsvd_z16 = 0;

	while (remaining) {
		rep_count = min(remaining, HV_SET_REGISTER_BATCH_SIZE);
		memcpy(input_page->elements, registers,
		       sizeof(struct hv_register_assoc) * rep_count);

		status = hv_do_rep_hypercall(HVCALL_SET_VP_REGISTERS, rep_count,
					     0, input_page, NULL);
		if (!hv_result_success(status))
			break;
		completed = hv_repcomp(status);
		registers += completed;
		remaining -= completed;
	}

	local_irq_restore(flags);

	return hv_status_to_errno(status);
}

