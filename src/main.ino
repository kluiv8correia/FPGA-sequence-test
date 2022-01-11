// INCLUDES =======================
#include <Arduino.h>

#include "Logger.h"
#include "Sequencer.h"
// ================================


// DEFINES ========================
#define BAUDRATE 115200

#define CLK_INTERVAL 5 // clock pulse period
#define BIT_INTERVAL 2 // before clock rising edge 
#define SEQUENCE_SIZE 256 // 256 bits per transfer
#define TARGET_SIZE 4

#define DATA 21
#define CLK 18
#define RESULT 19
#define RESET 22
#define START 5
#define INDICATOR 2
#define RANDOM_SEED_PIN 36
// ================================


// OBJECTS ========================
Sequencer runner(SEQUENCE_SIZE, DATA, CLK, RESET, RESULT);
TaskHandle_t mainTaskHandler = NULL;
// ================================


// VARS ===========================
uint8_t target[TARGET_SIZE] = {1, 1, 0, 1}; // pattern to detect
uint8_t* buffer;
bool indicatorState = true;
bool interruptFlag = false;
bool sendStageFlag = false;
// ================================


void setup () {
    Serial.begin(BAUDRATE);

    // welcome screen
    SERIAL("\n----------------------- SEQUENCE DETECTOR TEST -----------------------\n");

    // set sequencer params
    runner.setTarget(target, TARGET_SIZE);
    runner.setTiming(CLK_INTERVAL, BIT_INTERVAL);

    // setting up the random seed pin
    pinMode(RANDOM_SEED_PIN, INPUT);

    // setting up the start pin
    pinMode(START, INPUT_PULLUP);
    pinMode(INDICATOR, OUTPUT);
    digitalWrite(INDICATOR, HIGH);
    attachInterrupt(START, PLAY_MAIN, FALLING);

    // setting up tasks
    xTaskCreatePinnedToCore(mainTask, "mainTask", 10240, NULL, 1, &mainTaskHandler, 1);
    vTaskSuspend(mainTaskHandler);

    // initial start button instruction 
    SERIAL("Press Start button to start the test ...\n");
}
void loop () {
    vTaskDelete(NULL);
}


// ISR functions ==================
void IRAM_ATTR PLAY_MAIN() {
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) 
    {
        interruptFlag = true;
        if (indicatorState) {
            indicatorState = false;
            LOG_INTERRUPT("Sequence Test Resuming...");   
            digitalWrite(INDICATOR, LOW);
            if (mainTaskHandler != NULL) {
                if (sendStageFlag) {
                    sendStageFlag = false;
                    vTaskDelete(mainTaskHandler);
                    xTaskCreatePinnedToCore(mainTask, "mainTask", 10240, NULL, 1, &mainTaskHandler, 1);
                } else {
                    vTaskResume(mainTaskHandler);
                }
            }
        } else {
            indicatorState = true;
            LOG_INTERRUPT("Sequence Test Paused, Press start button to resume...");
            digitalWrite(INDICATOR, HIGH);
            if (mainTaskHandler != NULL)
                vTaskSuspend(mainTaskHandler);
        }
    }
    last_interrupt_time = interrupt_time;
}
// ================================


// xTasks =========================
void mainTask(void* params) {
    while (1) {
        // detach until send sequence
        detachInterrupt(START);

        // generate the testcase
        randomSeed(analogRead(RANDOM_SEED_PIN));
        buffer = runner.generateTest();
        SERIAL("\nTEST SEQUENCE: ")
        for (int i=0; i<SEQUENCE_SIZE; ++i)
            SERIAL(buffer[i]);
        SERIAL("\n")

        // check for matching patterns in the sequence
        try {
            runner.checkTest();
        } catch (SeqException &err) {
            if (err.getError() != SEQ_OK)
                err.log();
            else {
                LOG_REPORT("%d Matches Found!", err.getTracker());
                SERIAL("Match Points: ");
                for(int i=0; i<err.getTracker(); i++) {
                    SERIAL(runner.getMatches()[i]);
                    SERIAL(" ");
                }
                SERIAL("\n");
            }
        }

        // send the sequence
        attachInterrupt(START, PLAY_MAIN, FALLING);
        sendStageFlag = true;
        seq_error_t stat;
        runner.resetBuffer();
        stat = runner.sendPattern();
        if (stat == SEQ_FAILED) {
            LOG_REPORT("Result Mismatch, Test Failed!");
        } else {
            LOG_REPORT("Test Passed!");
        }

        LOG_REPORT("Sequence Test Completed, press start button to create new test...");
        indicatorState = true;
        digitalWrite(INDICATOR, HIGH);
        vTaskSuspend(NULL);
    }
}
// ================================