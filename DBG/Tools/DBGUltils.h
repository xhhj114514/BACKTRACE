#ifndef DBGUltils_H
#define DBGUltils_H


#include "stm32f4xx_hal.h"
#include "printf.h"//暂时性使用祖传printf，后期更换RTT或者ITM


extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;

#include "symbol_table.h"

const char* get_function_name(uint32_t pc) {
    for (int i = 0; i < symbol_count; i++) {
        if (pc == symbol_table[i].addr) {
            return symbol_table[i].name;
        }
        // 模糊匹配：pc 在函数体内
        if (i < symbol_count - 1 && pc >= symbol_table[i].addr && pc < symbol_table[i+1].addr) {
            return symbol_table[i].name;
        }
    }
    return "???";
}

void backtrace_from_sp(uint32_t *sp, uint32_t lr)
{
    int depth = 0;
    uint32_t pc;

    printf("  SP     | LR     | PC (Func)\r\n");
    printf("---------+--------+---------------\r\n");

    while (depth < 16 && sp && (uint32_t)sp < (uint32_t)&_estack) {
        pc = sp[6];  // 栈帧中的 PC
        if (pc < 0x08000000 || pc > 0x08400000) break; // 超出 Flash

        const char* func_name = get_function_name(pc);
        printf(" 0x%08X | 0x%08X | 0x%08X [%s]\r\n",
                  (uint32_t)sp, lr, pc, func_name ? func_name : "???");

        // 下一个栈帧
        lr = sp[5];  // 上一层 LR
        sp = (uint32_t*)sp[0];  // 上一层 FP (if frame pointer enabled)
        if (!sp) break;
        depth++;
    }
}


void HardFault_Handler_C(unsigned int *hardfault_args)
{
    uint32_t stacked_r0  = hardfault_args[0];
    uint32_t stacked_r1  = hardfault_args[1];
    uint32_t stacked_r2  = hardfault_args[2];
    uint32_t stacked_r3  = hardfault_args[3];
    uint32_t stacked_r12 = hardfault_args[4];
    uint32_t stacked_lr  = hardfault_args[5];
    uint32_t stacked_pc  = hardfault_args[6];
    uint32_t stacked_psr = hardfault_args[7];

    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t bfar = SCB->BFAR;
    uint32_t mmar = SCB->MMFAR;

    const char* fault_type = "Unknown Fault";
    uint32_t fault_addr = 0;


    if (hfsr & (1U << 30)) {
        fault_type = "Forced HardFault";
    } else if (cfsr & 0xFF) {
        fault_type = "MemManage Fault";
        if (cfsr & (1U << 7)) { // MMARVALID
            fault_addr = mmar;
        }
    } else if (cfsr & 0xFF00) {
        fault_type = "Bus Fault";
        if (cfsr & (1U << 15)) { // BFARVALID
            fault_addr = bfar;
        }
    } else if (cfsr & 0xFFFF0000) {
        fault_type = "Usage Fault";
    }

    const char* ptr_type = "Unknown Pointer";
    if (fault_addr == 0x00000000) {
        fault_type = "Null Pointer Dereference";
        ptr_type   = "NULL POINTER (0x00000000)";
    } else if (fault_addr < 0x20000000 || fault_addr > 0xEFFFFFFF) {
        ptr_type = "WILD POINTER (Invalid Address)";
    } else {
        ptr_type = "Valid Pointer Region";
    }

    const char* func_name = get_function_name(stacked_pc);

    printf("\r\n\r\n=== HARDFAULT DETECTED ===\r\n");
    printf("Type:        %s\r\n", fault_type);
    printf("Fault Addr:  0x%08X\r\n", fault_addr);
    printf("Pointer:     %s\r\n", ptr_type);
    printf("PC:          0x%08X [%s]\r\n", stacked_pc, func_name);
    printf("LR:          0x%08X\r\n", stacked_lr);
    printf("R0:          0x%08X  R1: 0x%08X  R2: 0x%08X  R3: 0x%08X\r\n",
           stacked_r0, stacked_r1, stacked_r2, stacked_r3);
    printf("R12:         0x%08X  PSR: 0x%08X\r\n", stacked_r12, stacked_psr);
    // printf("CFSR:        0x%08X  HFSR: 0x%08X\r\n", cfsr, hfsr);

    //stackoverflow
    uint32_t sp = (uint32_t)hardfault_args;
    uint32_t stack_top = (uint32_t)&_estack;
    if (stack_top - sp > 0x10000) {
        printf("WARNING: Stack overflow suspected!\r\n");
    }

    // 🔸 调用栈回溯（可选）
    // printf("Call Stack Backtrace:\r\n");
    // extern void backtrace_from_sp(uint32_t *sp, uint32_t lr);
    // backtrace_from_sp((uint32_t*)sp, stacked_lr);

    while (1) {
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        // HAL_Delay(200);
    }
}



#endif // DBGUltils_H
