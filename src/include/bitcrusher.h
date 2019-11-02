#ifndef PEDALPI2_BITCRUSHER_H
#define PEDALPI2_BITCRUSHER_H

uint32_t increaseBitCrush(uint32_t bitcrushing_value) {
    bcm2835_delay(100); //100ms delay for buttons debouncing
    if (bitcrushing_value < 12) bitcrushing_value++;
    return bitcrushing_value;
}

uint32_t decreaseBitCrush(uint32_t bitcrushing_value) {
    bcm2835_delay(100); //100ms delay for buttons debouncing
    if (bitcrushing_value > 0) bitcrushing_value--;
    return bitcrushing_value;
}

uint32_t bitCrusherEffect(uint32_t input_signal, uint32_t bitcrushing_value) {
    return input_signal << bitcrushing_value;
}

#endif //PEDALPI2_BITCRUSHER_H
