/**
 * @file exithandlers.h
 * @brief Declarations for exit handlers
 */

#pragma once

int returnCode();

extern bool hangup;

void interruptHandler(int);
void exitHandler(int);
void signalHandler(int);
void hangup_handler(int sig);
