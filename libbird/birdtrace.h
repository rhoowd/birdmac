/*
 * birdtrace.h
 *
 *  Created on: Mar 17, 2015
 *      Author: koo
 */

#ifndef EXAMPLES_KOO_BIRDTRACE_H_
#define EXAMPLES_KOO_BIRDTRACE_H_

#define MAX_LOG_SIZE 200

void birdtrace_start();
void birdtrace_stop();

void birdtrace_log(char* input);
void birdtrace_log_datapacket(int input);
void birdtrace_log_string(char* input);
void birdtrace_log_string_int(char* input,int input2);


#endif /* EXAMPLES_KOO_BIRDTRACE_H_ */
