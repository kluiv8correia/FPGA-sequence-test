#ifndef SEQUENCER_H
#define SEQUENCER_H

// INCLUDES -----------------
#include <exception>

#include "Logger.h"
// --------------------------


// ERROR TYPES --------------
typedef enum seq_error_t {
    SEQ_OK,
    SEQ_FAILED,
    SEQ_INVALID_LENGTH
} seq_error_t;
// --------------------------


// MAIN CLASS ---------------
class Sequencer {
    private:
    uint8_t *sequence, *target, *matches;
    size_t sequenceSize, targetSize, matchCount;
    uint8_t data, clk, reset, result;
    uint16_t clkInterval, bitInterval;

    public:
    Sequencer(size_t sequenceSize, uint8_t data, uint8_t clk, uint8_t reset, uint8_t result);
    ~Sequencer();
    void setTarget(uint8_t* target, size_t targetSize);
    void setTiming(uint16_t clkInterval, uint16_t bitInterval);
    uint8_t* generateTest();
    void checkTest();
    uint8_t* getMatches();
    seq_error_t sendPattern();
    void resetBuffer();
};
// --------------------------


// ERROR HANDLER ------------
class SeqException : public std::exception {
    private:
    seq_error_t err;
    int32_t tracking;

    public:
    SeqException (seq_error_t err) : err(err) {}
    SeqException (seq_error_t err, int32_t tracking) : err(err), tracking(tracking) {}

    void log() const throw () {
        switch (err) {
            case SEQ_OK:
                LOG_ERROR("Sequence Passed!");
            break;
            case SEQ_FAILED:
                LOG_ERROR("Sequence Failed!");
            break;
            case SEQ_INVALID_LENGTH:
                LOG_ERROR("Invalid Length");
            break;
            default:
                LOG_ERROR("Unknown Error");
        }
    }
    seq_error_t getError() const throw() {
        return err;
    }
    int32_t getTracker() const throw() {
        return tracking;
    }
};
// --------------------------

#endif