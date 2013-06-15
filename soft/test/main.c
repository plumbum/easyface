/*
 * Copyright © 2001-2008 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "modbus.h"

/* The goal of this program is to check all major functions of
   libmodbus:
   - force_single_coil
   - read_coil_status
   - force_multiple_coils
   - preset_single_register
   - read_holding_registers
   - preset_multiple_registers
   - read_holding_registers

   All these functions are called with random values on a address
   range defined by the following defines. 
*/
#define LOOP            1
#define SLAVE           0x01
#define ADDRESS_START   0x11
#define ADDRESS_END     0x16

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void)
{
        int ret;
        int nb_fail;
        int nb_loop;
        int addr;
        int nb;
        uint8_t *tab_rq_status;
        uint8_t *tab_rp_status;
        uint16_t *tab_rq_registers;
        uint16_t *tab_rp_registers;
        modbus_param_t mb_param;

        /* RTU parity : none, even, odd */
        modbus_init_rtu(&mb_param, "/dev/ttyUSB1", 19200, "none", 8, 1);

        /* TCP */
        // modbus_init_tcp(&mb_param, "127.0.0.1", 1502);

#if 1
        modbus_set_debug(&mb_param, TRUE);
#else
        modbus_set_debug(&mb_param, FALSE);
#endif
        if (modbus_connect(&mb_param) == -1) {
                printf("ERROR Connection failed\n");
                exit(1);
        }

        /* Allocate and initialize the different memory spaces */
        nb = ADDRESS_END - ADDRESS_START;

        tab_rq_status = (uint8_t *) malloc(nb * sizeof(uint8_t));
        memset(tab_rq_status, 0, nb * sizeof(uint8_t));

        tab_rp_status = (uint8_t *) malloc(nb * sizeof(uint8_t));
        memset(tab_rp_status, 0, nb * sizeof(uint8_t));

        tab_rq_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
        memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

        tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
        memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

        uint16_t readed[16];
        uint16_t scan_cnt;

        while(1)
        {
            scan_cnt = 0;
            ret = read_input_registers(&mb_param, SLAVE, 0x4000, 1, &scan_cnt);
            if(scan_cnt > 0)
            {
                ret = read_input_registers(&mb_param, SLAVE, 0x4001, scan_cnt, readed);
                int i;
                printf("Scancode (%d):", scan_cnt);
                for(i=0; i<scan_cnt; i++)
                {
                    if(readed[i] != 0xFFFF)
                    {
                        printf(" 0x%02x", readed[i]);
                    }
                }
                printf("\r\n", readed[i]);
            }
            break;
        }

        return 0;

        /*
        ret = read_input_registers(&mb_param, SLAVE, 0x4000, 8, readed);
        printf("*******\n");
        printf("Ret: %d\n", ret);
        printf("1: %5d; 2: %5d; 3: %5d; 4: %5d; 5: %5d; 6: %5d;\n",
                readed[0], readed[1], readed[2],
                readed[3], readed[4], readed[5]);
        printf("pwr: %5d; sens: %5d; ref: %5d;\n",
                readed[6], readed[7], readed[8]);
        return 0;
        */

        
        tab_rq_registers[0] = 'A' | '0'<<8;
        tab_rq_registers[1] = 'B' | '1'<<8;
        tab_rq_registers[2] = 'C' | '2'<<8;
        tab_rq_registers[3] = 'D' | '3'<<8;
        tab_rq_registers[4] = 'E' | '4'<<8;
        tab_rq_registers[5] = 'F';
        const char message[] = "Hello world!";

        /* MULTIPLE REGISTERS */
        ret = preset_multiple_registers(&mb_param, SLAVE,
                                        3<<7 | 1, sizeof(message)/2, message);
        if (ret != 6) {
                printf("ERROR preset_multiple_registers (%d)\n", ret);
                printf("Slave = %d, address = %d, nb = %d\n",
                               SLAVE, addr, nb);
        }

        ret = preset_multiple_registers(&mb_param, SLAVE,
                                        0<<7 | 1, 6, tab_rq_registers);
        if (ret != 6) {
                printf("ERROR preset_multiple_registers (%d)\n", ret);
                printf("Slave = %d, address = %d, nb = %d\n",
                               SLAVE, addr, nb);
        }

        tab_rq_registers[5] = 'f' | 'z'<<8;

        ret = preset_multiple_registers(&mb_param, SLAVE,
                                        1<<7 | 1, 6, tab_rq_registers);
        if (ret != 6) {
                printf("ERROR preset_multiple_registers (%d)\n", ret);
                printf("Slave = %d, address = %d, nb = %d\n",
                               SLAVE, addr, nb);
        }

        return 0;

        /* SINGLE REGISTER */
        tab_rq_registers[0] = 999;
        tab_rq_registers[1] = 999;
        tab_rq_registers[2] = 999;
        tab_rq_registers[3] = 500;
        tab_rq_registers[4] = 500;
        tab_rq_registers[5] = 500;

        while(1)
        {
            ret = read_input_registers(&mb_param, SLAVE, 0x11, 10, readed);
            printf("*******\n");
            printf("Ret: %d\n", ret);
            printf("1: %5d; 2: %5d; 3: %5d; 4: %5d; 5: %5d; 6: %5d;\n",
                    readed[0], readed[1], readed[2],
                    readed[3], readed[4], readed[5]);
            printf("pwr: %5d; sens: %5d; ref: %5d;\n",
                    readed[6], readed[7], readed[8]);
        }

        int i;
        while(1)
        {
            for(i=-1000; i<1000; i++)
            {
                tab_rq_registers[0] = abs(i);
                tab_rq_registers[1] = abs(i);
                tab_rq_registers[2] = abs(i);
                tab_rq_registers[3] = 1000-abs(i);
                tab_rq_registers[4] = 1000-abs(i);
                tab_rq_registers[5] = 1000-abs(i);
                /* MULTIPLE REGISTERS */
                ret = preset_multiple_registers(&mb_param, SLAVE,
                                                0x11, 6, tab_rq_registers);
                if (ret != 6) {
                        printf("ERROR preset_multiple_registers (%d)\n", ret);
                        printf("Slave = %d, address = %d, nb = %d\n",
                                       SLAVE, addr, nb);
                }
            }
        }

        /*
        ret = preset_single_register(&mb_param, SLAVE, 0x11, tab_rq_registers[0]);
        if (ret != 1) {
                printf("ERROR preset_single_register (%d)\n", ret);
                printf("Slave = %d, address = %d, value = %d (0x%X)\n",
                       SLAVE, addr, tab_rq_registers[0], tab_rq_registers[0]);
                nb_fail++;
        }
        */
 
#if 0
        nb_loop = nb_fail = 0;
        while (nb_loop++ < LOOP) { 
                for (addr = ADDRESS_START; addr <= ADDRESS_END; addr++) {
                        int i;

                        /* Random numbers (short) */
                        for (i=0; i<nb; i++) {
                                tab_rq_registers[i] = (uint16_t) (65535.0*rand() / (RAND_MAX + 1.0));
                                tab_rq_status[i] = tab_rq_registers[i] % 2;
                        }
                        nb = ADDRESS_END - addr;

                        /* SINGLE COIL */
                        ret = force_single_coil(&mb_param, SLAVE, addr, tab_rq_status[0]);
                        if (ret != 1) {
                                printf("ERROR force_single_coil (%d)\n", ret);
                                printf("Slave = %d, address = %d, value = %d\n",
                                       SLAVE, addr, tab_rq_status[0]);
                                nb_fail++;
                        } else {
                                ret = read_coil_status(&mb_param, SLAVE, addr, 1, tab_rp_status);
                                if (ret != 1 || tab_rq_status[0] != tab_rp_status[0]) {
                                        printf("ERROR read_coil_status single (%d)\n", ret);
                                        printf("Slave = %d, address = %d\n", 
                                               SLAVE, addr);
                                        nb_fail++;
                                }
                        }

                        /* MULTIPLE COILS */
                        ret = force_multiple_coils(&mb_param, SLAVE, addr, nb, tab_rq_status);
                        if (ret != nb) {
                                printf("ERROR force_multiple_coils (%d)\n", ret);
                                printf("Slave = %d, address = %d, nb = %d\n",
                                       SLAVE, addr, nb);
                                nb_fail++;
                        } else {
                                ret = read_coil_status(&mb_param, SLAVE, addr, nb, tab_rp_status);
                                if (ret != nb) {
                                        printf("ERROR read_coil_status\n");
                                        printf("Slave = %d, address = %d, nb = %d\n",
                                               SLAVE, addr, nb);
                                        nb_fail++;
                                } else {
                                        for (i=0; i<nb; i++) {
                                                if (tab_rp_status[i] != tab_rq_status[i]) {
                                                        printf("ERROR read_coil_status\n");
                                                        printf("Slave = %d, address = %d, value %d (0x%X) != %d (0x%X)\n", 
                                                               SLAVE, addr,
                                                               tab_rq_status[i], tab_rq_status[i],
                                                               tab_rp_status[i], tab_rp_status[i]);
                                                        nb_fail++;
                                                }
                                        }
                                }
                        }

                        /* SINGLE REGISTER */
                        ret = preset_single_register(&mb_param, SLAVE, addr, tab_rq_registers[0]);
                        if (ret != 1) {
                                printf("ERROR preset_single_register (%d)\n", ret);
                                printf("Slave = %d, address = %d, value = %d (0x%X)\n",
                                       SLAVE, addr, tab_rq_registers[0], tab_rq_registers[0]);
                                nb_fail++;
                        } else {
                                ret = read_holding_registers(&mb_param, SLAVE,
                                                            addr, 1, tab_rp_registers);
                                if (ret != 1) {
                                        printf("ERROR read_holding_registers single (%d)\n", ret);
                                        printf("Slave = %d, address = %d\n",
                                               SLAVE, addr);
                                        nb_fail++;
                                } else {
                                        if (tab_rq_registers[0] != tab_rp_registers[0]) {
                                                printf("ERROR read_holding_registers single\n");
                                                printf("Slave = %d, address = %d, value = %d (0x%X) != %d (0x%X)\n",
                                                       SLAVE, addr,
                                                       tab_rq_registers[0], tab_rq_registers[0],
                                                       tab_rp_registers[0], tab_rp_registers[0]);
                                                nb_fail++;
                                        }
                                }
                        }
                        
                        /* MULTIPLE REGISTERS */
                        ret = preset_multiple_registers(&mb_param, SLAVE,
                                                        addr, nb, tab_rq_registers);
                        if (ret != nb) {
                                printf("ERROR preset_multiple_registers (%d)\n", ret);
                                printf("Slave = %d, address = %d, nb = %d\n",
                                               SLAVE, addr, nb);
                                nb_fail++;
                        } else {
                                ret = read_holding_registers(&mb_param, SLAVE,
                                                             addr, nb, tab_rp_registers);
                                if (ret != nb) {
                                        printf("ERROR read_holding_registers (%d)\n", ret);
                                        printf("Slave = %d, address = %d, nb = %d\n",
                                               SLAVE, addr, nb);
                                        nb_fail++;
                                } else {
                                        for (i=0; i<nb; i++) {
                                                if (tab_rq_registers[i] != tab_rp_registers[i]) {
                                                        printf("ERROR read_holding_registers\n");
                                                        printf("Slave = %d, address = %d, value %d (0x%X) != %d (0x%X)\n",
                                                               SLAVE, addr,
                                                               tab_rq_registers[i], tab_rq_registers[i],
                                                               tab_rp_registers[i], tab_rp_registers[i]);
                                                        nb_fail++;
                                                }
                                        }
                                }
                        }

                }
                        
                printf("Test: ");
                if (nb_fail)
                        printf("%d FAILS\n", nb_fail);
                else
                        printf("SUCCESS\n");
        }
#endif

        /* Free the memory */
        free(tab_rq_status);
        free(tab_rp_status);                                           
        free(tab_rq_registers);
        free(tab_rp_registers);

        /* Close the connection */
        modbus_close(&mb_param);
        
        return 0;
}

