#include "mbed.h"

SPI spi(p5, p6, p7);

DigitalOut cs[7] = {p8, p9, p10, p20, p19, p19, p18};

/* 
16-bit command
first four are address
next two are command
next ten are data (msb ignored)

commands:
00 write data
01 INCR
10 DECR
11 read data

addresses:
0 volatile 0
1 volatile 1
2 eeprom 0
3 eeprom 1

*/

const uint16_t ports[4] = {0x0000, 0x1000, 0x2000, 0x3000};

uint16_t send_data(int pin, uint16_t data){
    spi.format(16,0);
    cs[pin] = 0;
    uint16_t data_from_slave = spi.write(data);
    cs[pin] = 1;
    return data_from_slave;
}

uint8_t send_data(int pin, uint8_t data){
    spi.format(8,0);
    cs[pin] = 0;
    uint8_t data_from_slave = spi.write(data);
    cs[pin] = 1;
    return data_from_slave;
}

uint16_t res_to_wiper(int resistance){
    return (resistance-75)*256/10000;
}

uint16_t set_resistance(int pin, int channel, int resistance){
    printf("%d, %d, %d\n", pin, channel, resistance);
    uint16_t res = res_to_wiper(resistance);
    uint16_t data = ports[channel] | res;
    printf("SNTV %4x\n", data);
    uint16_t data_from_slave = send_data(pin, data);
    //if (data_from_slave != 0xffff){
        return data_from_slave;
    //}
    //wait(0.75);
    //data = ports[channel+2] | res;
    //printf("SNTN %4x\n", data);
    //return send_data(pin, data);
}

uint16_t read_resistance(int pin, int channel){
    printf("%d, %d\n", pin, channel);
    uint16_t res = 0x0C00;
    uint16_t data = ports[channel] | res;
    printf("SENT %4x\n", data);
    uint16_t data_from_slave = send_data(pin, data);
    return data_from_slave;
}

// res1 is input resistance
// res2 is feedback resistance
// res3 is gnd resistance
int set_filter(int filter, int res1, int res2, int res3, float gain){
    uint16_t data_from_slave;
    
    wait(0.75);
    
    data_from_slave = set_resistance(filter*2+1, 0, res2); // R2 (feedback)
    printf("DATA %4x\n", data_from_slave);
    if (data_from_slave != 0xffff){
        //return -1;
    }
    
    wait(0.75);
    
    data_from_slave = set_resistance(filter*2, 1, res1); // R1 (input)
    printf("DATA %4x\n", data_from_slave);
    if (data_from_slave != 0xffff){
        //return -1;
    }
    
    wait(0.75);
    
    
    data_from_slave = set_resistance(filter*2+1, 1, res3); // R3 (gnd)
    printf("DATA %4x\n", data_from_slave);
    if (data_from_slave != 0xffff){
        //return -1;
    }
    
    wait(0.75);
    
    printf("gain\n");
    uint16_t data_to_slave = (uint16_t)((gain)*256);
    printf("SNTV %4x\n", data_to_slave);
    data_from_slave = send_data(filter*2, data_to_slave); // R4 (Gain)
    printf("DATA %4x\n", data_from_slave);
    if (data_from_slave != 0xffff){
        //return -1;
    }
    return 0;
}

//val 0-100
int set_gain(float val){
    uint16_t data_to_slave = val/100.0*256;
    printf("SENT %4x\n", data_to_slave);
    uint16_t data_from_slave = send_data(6, data_to_slave);
    printf("DATA %4x\n", data_from_slave);
    if (data_from_slave != 0xffff){
        //return -1;
    }
    data_to_slave = ports[2] | data_to_slave;
    //data_from_slave = send_data(7, data_to_slave);
    //printf("DATA %4x\n", data_from_slave);
    //if (data_from_slave != 0xffff){
        //return -1;
    //}
    return 0;
}

int main() {
    cs[0] = 1;
    cs[1] = 1;
    
    spi.frequency(10000);
    
    uint16_t data_to_slave;
    uint16_t data_from_slave;
    
    printf("setting values...\n");
    
    // 330nF
    wait(0.75);
    set_filter(1, 10000, 1000, 1000, 1);
    
    // 8.2nF
    wait(0.75);
    set_filter(0, 1000, 9200, 10000, 1);
    
    wait(0.75);
    float gain = .14;
    data_to_slave = (uint16_t)((gain)*256);
    printf("SENT %4x\n", data_to_slave);
    data_from_slave = send_data(2*2, data_to_slave); // R4 (Gain)
    printf("DATA %4x\n", data_from_slave);
    //set_filter(2, 0, 0, 0, 0.48);
    
    
    printf("\n");
    
    while(1) {
    }
}
