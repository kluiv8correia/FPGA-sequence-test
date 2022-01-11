// INCLUDES =======================
#include <Arduino.h>

#include "Sequencer.h"
// ================================

// CONSTRUCTOR AND DESTRUCTOR =====
Sequencer::Sequencer(size_t sequenceSize, uint8_t data, uint8_t clk, uint8_t reset, uint8_t result) : sequenceSize(sequenceSize), data(data), clk(clk), reset(reset), result(result) {
    sequence = (uint8_t*) malloc(sizeof(uint8_t) * sequenceSize);
    clkInterval = 100;
    bitInterval = 20;

    pinMode(data, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(reset, OUTPUT);
    pinMode(result, INPUT_PULLDOWN);
}
Sequencer::~Sequencer() {
    free(sequence);
    free(matches);
}
// ================================

// MEMBER FUNCTIONS ===============
void Sequencer::setTarget(uint8_t* target, size_t targetSize) {
    this->target = target;
    this->targetSize = targetSize;
    if (targetSize > sequenceSize)
        throw SeqException(SEQ_INVALID_LENGTH);
    else
        matches = (uint8_t*) malloc(sizeof(uint8_t) * floor(sequenceSize / targetSize));
}
void Sequencer::setTiming(uint16_t clkInterval, uint16_t bitInterval) {
    this->clkInterval = clkInterval;
    this->bitInterval = bitInterval;
}
uint8_t* Sequencer::generateTest() {
    for (size_t i=0; i<sequenceSize; ++i) {
        if (((float) random(1000) / 1000.0) > 0.5)
            sequence[i] = 1;
        else
            sequence[i] = 0;
    }
    return sequence;
}
void Sequencer::checkTest() {
    matchCount = 0;
    for (int i=0; i<=(sequenceSize-targetSize);) {
        int j=0;
        while (j < targetSize) {
            if (target[j] != sequence[i++]) {
                i = i - j;
                break;
            }
            else {
                j++;
            }
        }
        if (j == targetSize)
            matches[matchCount++] = i-1;
    }
    if (matchCount == 0)
        throw SeqException(SEQ_FAILED);
    throw SeqException(SEQ_OK, matchCount);
}
uint8_t* Sequencer::getMatches() {
    return matches;
}
seq_error_t Sequencer::sendPattern() {
    size_t counter = 0;
    seq_error_t stat = SEQ_OK;
    uint8_t signal;

    for (int i=0; i<sequenceSize; ++i) {
        digitalWrite(data, sequence[i]);
        vTaskDelay(bitInterval / portTICK_PERIOD_MS);
        digitalWrite(clk, HIGH);
        vTaskDelay(clkInterval / portTICK_PERIOD_MS);
        digitalWrite(clk, LOW);

        vTaskDelay((clkInterval - bitInterval) / portTICK_PERIOD_MS);
        signal = digitalRead(result);

        while (matches[counter] < i && counter < (matchCount-1)) {
            counter++;
        } // re-adjusts check position
        if (matches[counter] == i && matchCount) {
            if (signal == LOW) {
                stat = SEQ_FAILED;
            }
        }
        else {
            if (signal == HIGH) {
                stat = SEQ_FAILED;
            }
        }
    }

    return stat;
}
void Sequencer::resetBuffer() {
    digitalWrite(reset, HIGH);
    vTaskDelay(clkInterval / portTICK_PERIOD_MS);
    digitalWrite(reset, LOW);
}
// ================================