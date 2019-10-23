/*
 * Sample kernel call using SVC
 * Note use of stack to pass arguments to kernel
 * ECED 4402
 */
#include <stdio.h>
#include "KernelCalls.h"
#include "process.h"

int getid()
{
volatile struct kcallargs getidarg; /* Volatile to actually reserve space on stack */
getidarg.code = GETID;

/* Assign address if getidarg to R7 */
assignR7((unsigned long) &getidarg);

SVC();

return getidarg.rtnvalue;

}